/*
 *      uvcvideo.c  --  USB Video Class driver
 *
 *      Copyright (C) 2005-2006
 *          Laurent Pinchart (laurent.pinchart@skynet.be)
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 */

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/usb.h>
//#include <linux/videodev.h>
#include <linux/videodev2.h>
#include <linux/vmalloc.h>
#include <linux/wait.h>
#include <linux/mm.h>
#include <asm/atomic.h>

#include "uvcvideo.h"

/* ------------------------------------------------------------------------
 * Video buffers queue management.
 *
 * Video queues is initialized by uvc_queue_init(). The function performs
 * basic initialization of the uvc_video_queue struct and never fails.
 *
 * Video buffer allocation and freeing are performed by uvc_alloc_buffers and
 * uvc_free_buffers respectively. The former acquires the video queue lock,
 * while the later must be called with the lock held (so that allocation can
 * free previously allocated buffers). Trying to free buffers that are mapped
 * to user space will return -EBUSY.
 *
 * Video buffers are managed using two queues. However, unlike most USB video
 * drivers which use an in queue and an out queue, we use a main queue which
 * holds all queued buffers (both 'empty' and 'done' buffers), and an irq
 * queue which holds empty buffers. This design (copied from video-buf)
 * minimizes locking in interrupt, as only one queue is shared between
 * interrupt and user contexts.
 *
 * Use cases
 * ---------
 *
 * Unless stated otherwise, all operations which modify the irq buffers queue
 * are protected by the irq spinlock.
 *
 * 1. The user queues the buffers, starts streaming and dequeues a buffer.
 *
 *    The buffers are added to the main and irq queues. Both operations are
 *    protected by the queue lock, and the latert is protected by the irq
 *    spinlock as well.
 *
 *    The completion handler fetches a buffer from the irq queue and fills it
 *    with video data. If no buffer is available (irq queue empty), the handler
 *    returns immediately.
 *
 *    When the buffer is full, the completion handler removes it from the irq
 *    queue, marks it as ready (UVC_BUF_STATE_DONE) and wake its wait queue.
 *    At that point, any process waiting on the buffer will be woken up. If a
 *    process tries to dequeue a buffer after it has been marked ready, the
 *    dequeing will succeed immediately.
 *
 * 2. Buffers are queued, user is waiting on a buffer and the device gets
 *    disconnected.
 *
 *    When the device is disconnected, the kernel calls the completion handler
 *    with an appropriate status code. The handler marks all buffers in the
 *    irq queue as being erroneous (UVC_BUF_STATE_ERROR) and wakes them up so
 *    that any process waiting on a buffer gets woken up.
 *
 *    Waking up up the first buffer on the irq list is not enough, as the
 *    process waiting on the buffer might restart the dequeue operation
 *    immediately.
 *
 */

void uvc_queue_init(struct uvc_video_queue *queue)
{
	mutex_init(&queue->mutex);
	spin_lock_init(&queue->irqlock);
	INIT_LIST_HEAD(&queue->mainqueue);
	INIT_LIST_HEAD(&queue->irqqueue);
}

/*
 * Allocate the video buffers.
 *
 * Pages are reserved to make sure they will not be swaped, as they will be
 * filled in URB completion handler.
 * 
 * Buffers will be individually mapped, so they must all be page aligned.
 */
int uvc_alloc_buffers(struct uvc_video_queue *queue, unsigned int nbuffers,
		unsigned int buflength)
{
	unsigned int bufsize = PAGE_ALIGN(buflength);
	unsigned int i;
	void *mem = NULL;
	int ret;

	if (nbuffers > UVC_MAX_VIDEO_BUFFERS)
		nbuffers = UVC_MAX_VIDEO_BUFFERS;

	mutex_lock(&queue->mutex);

	if ((ret = uvc_free_buffers(queue)) < 0)
		goto done;

	/* Decrement the number of buffers until allocation succeeds. */
	for (; nbuffers > 0; --nbuffers) {
		mem = vmalloc_32(nbuffers * bufsize);
		if (mem != NULL)
			break;
	}

	if (mem == NULL) {
		ret = -ENOMEM;
		goto done;
	}

	for (i = 0; i < nbuffers; ++i) {
		memset(&queue->buffer[i], 0, sizeof queue->buffer[i]);
		queue->buffer[i].size = bufsize;
		queue->buffer[i].buf.index = i;
		queue->buffer[i].buf.m.offset = i * bufsize;
		queue->buffer[i].buf.length = buflength;
		queue->buffer[i].buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		queue->buffer[i].buf.sequence = 0;
		queue->buffer[i].buf.field = V4L2_FIELD_NONE;
		queue->buffer[i].buf.memory = V4L2_MEMORY_MMAP;
		queue->buffer[i].buf.flags = 0;
		init_waitqueue_head(&queue->buffer[i].wait);
	}

	queue->mem = mem;
	queue->count = nbuffers;
	ret = nbuffers;

done:
	mutex_unlock(&queue->mutex);
	return ret;
}

/*
 * Free the video buffers.
 *
 * This function must be called with the queue lock held.
 */
int uvc_free_buffers(struct uvc_video_queue *queue)
{
	unsigned int i;

	for (i = 0; i < queue->count; ++i) {
		if (queue->buffer[i].vma_use_count != 0)
			return -EBUSY;
	}

	if (queue->count) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15)
		unsigned long addr = (unsigned long)queue->mem;
		size_t size = queue->count * queue->buffer[0].size;
		while (size > 0) {
			ClearPageReserved(vmalloc_to_page((void*)addr));
			addr += PAGE_SIZE;
			size -= PAGE_SIZE;
		}
#endif
		vfree(queue->mem);
		queue->count = 0;
	}

	return 0;
}

void uvc_query_buffer(struct uvc_buffer *buf,
		struct v4l2_buffer *v4l2_buf)
{
	memcpy(v4l2_buf, &buf->buf, sizeof *v4l2_buf);

	if(buf->vma_use_count)
		v4l2_buf->flags |= V4L2_BUF_FLAG_MAPPED;
		
	switch (buf->state) {
	case UVC_BUF_STATE_ERROR:
	case UVC_BUF_STATE_DONE:
		v4l2_buf->flags |= V4L2_BUF_FLAG_DONE;
		break;
	case UVC_BUF_STATE_QUEUED:
	case UVC_BUF_STATE_ACTIVE:
		v4l2_buf->flags |= V4L2_BUF_FLAG_QUEUED;
		break;
	case UVC_BUF_STATE_IDLE:
	default:
		break;
	}
}

/*
 * Queue a video buffer. Attempting to queue a buffer that has already been
 * queued will return -EINVAL.
 */
int uvc_queue_buffer(struct uvc_video_queue *queue, struct v4l2_buffer *v4l2_buf)
{
	struct uvc_buffer *buf;
	unsigned long flags;
	int ret = 0;

	uvc_trace(UVC_TRACE_CAPTURE, "Queuing buffer %u.\n", v4l2_buf->index);

	if (v4l2_buf->type != V4L2_BUF_TYPE_VIDEO_CAPTURE ||
	    v4l2_buf->memory != V4L2_MEMORY_MMAP) {
		uvc_trace(UVC_TRACE_CAPTURE, "[E] Invalid buffer type (%u) "
			"and/or memory (%u).\n", v4l2_buf->type,
			v4l2_buf->memory);
		return -EINVAL;
	}

	mutex_lock(&queue->mutex);
	if (v4l2_buf->index >= queue->count)  {
		uvc_trace(UVC_TRACE_CAPTURE, "[E] Out of range index.\n");
		ret = -EINVAL;
		goto done;
	}

	buf = &queue->buffer[v4l2_buf->index];
	if (buf->state != UVC_BUF_STATE_IDLE) {
		uvc_trace(UVC_TRACE_CAPTURE, "[E] Invalid buffer state "
			"(%u).\n", buf->state);
		ret = -EINVAL;
		goto done;
	}

	buf->state = UVC_BUF_STATE_QUEUED;
	buf->buf.bytesused = 0;
	list_add_tail(&buf->stream, &queue->mainqueue);
	spin_lock_irqsave(&queue->irqlock, flags);
	list_add_tail(&buf->queue, &queue->irqqueue);
	spin_unlock_irqrestore(&queue->irqlock, flags);

done:
	mutex_unlock(&queue->mutex);
	return ret;
}

static int uvc_queue_waiton(struct uvc_buffer *buf, int nonblocking)
{
	if (nonblocking) {
		return (buf->state != UVC_BUF_STATE_QUEUED &&
			buf->state != UVC_BUF_STATE_ACTIVE)
			? 0 : -EAGAIN;
	}

	return wait_event_interruptible(buf->wait,
		buf->state != UVC_BUF_STATE_QUEUED &&
		buf->state != UVC_BUF_STATE_ACTIVE);
}

/*
 * Dequeue a video buffer. If nonblocking is false, block until a buffer is
 * available.
 */
int uvc_dequeue_buffer(struct uvc_video_queue *queue,
		struct v4l2_buffer *v4l2_buf, int nonblocking)
{
	struct uvc_buffer *buf;
	int ret = 0;

	if (v4l2_buf->type != V4L2_BUF_TYPE_VIDEO_CAPTURE ||
	    v4l2_buf->memory != V4L2_MEMORY_MMAP) {
		uvc_trace(UVC_TRACE_CAPTURE, "[E] Invalid buffer type (%u) "
			"and/or memory (%u).\n", v4l2_buf->type,
			v4l2_buf->memory);
		return -EINVAL;
	}

	mutex_lock(&queue->mutex);
	if (list_empty(&queue->mainqueue)) {
		uvc_trace(UVC_TRACE_CAPTURE, "[E] Empty buffer queue.\n");
		ret = -EINVAL;
		goto done;
	}

	buf = list_entry(queue->mainqueue.next, struct uvc_buffer, stream);
	uvc_trace(UVC_TRACE_CAPTURE, "Dequeuing buffer %u.\n", buf->buf.index);
	if ((ret = uvc_queue_waiton(buf, nonblocking)) < 0)
		goto done;

	switch (buf->state) {
	case UVC_BUF_STATE_ERROR:
		uvc_trace(UVC_TRACE_CAPTURE, "[W] Corrupted data "
			"(transmission error).\n");
		ret = -EIO;
	case UVC_BUF_STATE_DONE:
		buf->state = UVC_BUF_STATE_IDLE;
		break;

	case UVC_BUF_STATE_IDLE:
	case UVC_BUF_STATE_QUEUED:
	case UVC_BUF_STATE_ACTIVE:
	default:
		uvc_trace(UVC_TRACE_CAPTURE, "[E] Invalid buffer state %u "
			"(driver bug?).\n", buf->state);
		ret = -EINVAL;
		goto done;
	}

	list_del(&buf->stream);
	uvc_query_buffer(buf, v4l2_buf);

done:
	mutex_unlock(&queue->mutex);
	return ret;
}

/*
 * Enable or disable the video buffers queue.
 *
 * The queue must be enabled before starting video acquisition and must be
 * disabled after stopping it. This ensures that the video buffers queue
 * state can be properly initialized before buffers are accessed from the
 * interrupt handler.
 *
 * Enabling the video queue initializes parameters (such as sequence number,
 * sync pattern, ...). If the queue is already enabled, return -EBUSY.
 *
 * Disabling the video queue cancels the queue and removes all buffers from
 * the main queue.
 *
 * This function can't be called from interrupt context. Use
 * uvc_queue_cancel() instead.
 */
int uvc_queue_enable(struct uvc_video_queue *queue, int enable)
{
	unsigned int i;
	int ret = 0;

	mutex_lock(&queue->mutex);
	if (enable) {
		if (queue->streaming) {
			ret = -EBUSY;
			goto done;
		}
		queue->last_fid = -1;
		queue->sequence = 0;
		queue->streaming = 1;
	}
	else {
		uvc_queue_cancel(queue);
		INIT_LIST_HEAD(&queue->mainqueue);

		for (i = 0; i < queue->count; ++i)
			queue->buffer[i].state = UVC_BUF_STATE_IDLE;

		queue->streaming = 0;
	}

done:
	mutex_unlock(&queue->mutex);
	return ret;
}

/*
 * Cancel the video buffers queue.
 *
 * Cancelling the queue marks all buffers on the irq queue as erroneous,
 * wakes them up and remove them from the queue.
 *
 * This function acquires the irq spinlock and can be called from interrupt
 * context.
 */
void uvc_queue_cancel(struct uvc_video_queue *queue)
{
	struct uvc_buffer *buf;
	unsigned long flags;

	spin_lock_irqsave(&queue->irqlock, flags);
	while (!list_empty(&queue->irqqueue)) {
		buf = list_entry(queue->irqqueue.next, struct uvc_buffer, queue);
		list_del(&buf->queue);
		buf->state = UVC_BUF_STATE_ERROR;
		wake_up(&buf->wait);
	}
	spin_unlock_irqrestore(&queue->irqlock, flags);
}

struct uvc_buffer *uvc_queue_next_buffer(struct uvc_video_queue *queue,
		struct uvc_buffer *buf)
{
	struct uvc_buffer *nextbuf;
	unsigned long flags;

	spin_lock_irqsave(&queue->irqlock, flags);
	list_del(&buf->queue);
	if (!list_empty(&queue->irqqueue))
		nextbuf = list_entry(queue->irqqueue.next, struct uvc_buffer,
					queue);
	else
		nextbuf = NULL;
	spin_unlock_irqrestore(&queue->irqlock, flags);

	buf->buf.sequence = queue->sequence++;
	do_gettimeofday(&buf->buf.timestamp);

	wake_up(&buf->wait);
	return nextbuf;
}

