#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x3ce9013b, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x7a4f9ece, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x12da5bb2, __VMLINUX_SYMBOL_STR(__kmalloc) },
	{ 0xedf9bfcf, __VMLINUX_SYMBOL_STR(dev_set_drvdata) },
	{ 0xc8b57c27, __VMLINUX_SYMBOL_STR(autoremove_wake_function) },
	{ 0x91ac481c, __VMLINUX_SYMBOL_STR(video_device_release) },
	{ 0x13449927, __VMLINUX_SYMBOL_STR(video_usercopy) },
	{ 0x5e80b13b, __VMLINUX_SYMBOL_STR(no_llseek) },
	{ 0xcaf9403, __VMLINUX_SYMBOL_STR(usb_kill_urb) },
	{ 0x303d3cd9, __VMLINUX_SYMBOL_STR(__video_register_device) },
	{ 0x64aa4868, __VMLINUX_SYMBOL_STR(mutex_unlock) },
	{ 0x999e8297, __VMLINUX_SYMBOL_STR(vfree) },
	{ 0x91715312, __VMLINUX_SYMBOL_STR(sprintf) },
	{ 0x68dfc59f, __VMLINUX_SYMBOL_STR(__init_waitqueue_head) },
	{ 0x8e63c7e4, __VMLINUX_SYMBOL_STR(usb_string) },
	{ 0x3a605f76, __VMLINUX_SYMBOL_STR(video_device_alloc) },
	{ 0xf97456ea, __VMLINUX_SYMBOL_STR(_raw_spin_unlock_irqrestore) },
	{ 0xf1d36b17, __VMLINUX_SYMBOL_STR(current_task) },
	{ 0xce18114d, __VMLINUX_SYMBOL_STR(usb_deregister) },
	{ 0xafca4414, __VMLINUX_SYMBOL_STR(mutex_lock_interruptible) },
	{ 0x97ecbf05, __VMLINUX_SYMBOL_STR(__mutex_init) },
	{ 0x50eedeb8, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x5152e605, __VMLINUX_SYMBOL_STR(memcmp) },
	{ 0xc694e034, __VMLINUX_SYMBOL_STR(video_unregister_device) },
	{ 0x77fa8d8c, __VMLINUX_SYMBOL_STR(usb_set_interface) },
	{ 0xb6ed1e53, __VMLINUX_SYMBOL_STR(strncpy) },
	{ 0xb4390f9a, __VMLINUX_SYMBOL_STR(mcount) },
	{ 0x7b927483, __VMLINUX_SYMBOL_STR(usb_control_msg) },
	{ 0x5cff35f2, __VMLINUX_SYMBOL_STR(usb_driver_claim_interface) },
	{ 0x16305289, __VMLINUX_SYMBOL_STR(warn_slowpath_null) },
	{ 0xec916570, __VMLINUX_SYMBOL_STR(mutex_lock) },
	{ 0xf0ea661c, __VMLINUX_SYMBOL_STR(usb_free_coherent) },
	{ 0x89488357, __VMLINUX_SYMBOL_STR(vm_insert_page) },
	{ 0xbc5671dc, __VMLINUX_SYMBOL_STR(v4l_printk_ioctl) },
	{ 0x801eff78, __VMLINUX_SYMBOL_STR(usb_submit_urb) },
	{ 0x6d160d0b, __VMLINUX_SYMBOL_STR(usb_get_dev) },
	{ 0x9b48c1fe, __VMLINUX_SYMBOL_STR(video_devdata) },
	{ 0xf0fdf6cb, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0xa0787a19, __VMLINUX_SYMBOL_STR(usb_put_dev) },
	{ 0x4292364c, __VMLINUX_SYMBOL_STR(schedule) },
	{ 0xa0b04675, __VMLINUX_SYMBOL_STR(vmalloc_32) },
	{ 0xc8e862d6, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0xb3219a01, __VMLINUX_SYMBOL_STR(usb_get_intf) },
	{ 0x21fb443e, __VMLINUX_SYMBOL_STR(_raw_spin_lock_irqsave) },
	{ 0xe45f60d8, __VMLINUX_SYMBOL_STR(__wake_up) },
	{ 0x4f68e5c9, __VMLINUX_SYMBOL_STR(do_gettimeofday) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x2e60bace, __VMLINUX_SYMBOL_STR(memcpy) },
	{ 0x622fa02a, __VMLINUX_SYMBOL_STR(prepare_to_wait) },
	{ 0x4a72f1c9, __VMLINUX_SYMBOL_STR(usb_register_driver) },
	{ 0x74c134b9, __VMLINUX_SYMBOL_STR(__sw_hweight32) },
	{ 0x75bb675a, __VMLINUX_SYMBOL_STR(finish_wait) },
	{ 0xce985b25, __VMLINUX_SYMBOL_STR(usb_ifnum_to_if) },
	{ 0x32d90ba, __VMLINUX_SYMBOL_STR(vmalloc_to_page) },
	{ 0x18be49c1, __VMLINUX_SYMBOL_STR(usb_alloc_coherent) },
	{ 0x47c8baf4, __VMLINUX_SYMBOL_STR(param_ops_uint) },
	{ 0x8a27c25c, __VMLINUX_SYMBOL_STR(dev_get_drvdata) },
	{ 0x35fcbcd, __VMLINUX_SYMBOL_STR(usb_free_urb) },
	{ 0xcdb812c5, __VMLINUX_SYMBOL_STR(usb_alloc_urb) },
	{ 0x7cc673f5, __VMLINUX_SYMBOL_STR(usb_put_intf) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=videodev,usbcore";

MODULE_ALIAS("usb:v045Ep00F8d*dc*dsc*dp*ic0Eisc01ip00in*");
MODULE_ALIAS("usb:v046Dp08C1d*dc*dsc*dp*icFFisc01ip00in*");
MODULE_ALIAS("usb:v046Dp08C2d*dc*dsc*dp*icFFisc01ip00in*");
MODULE_ALIAS("usb:v046Dp08C3d*dc*dsc*dp*icFFisc01ip00in*");
MODULE_ALIAS("usb:v046Dp08C5d*dc*dsc*dp*icFFisc01ip00in*");
MODULE_ALIAS("usb:v046Dp08C6d*dc*dsc*dp*icFFisc01ip00in*");
MODULE_ALIAS("usb:v046Dp08C7d*dc*dsc*dp*icFFisc01ip00in*");
MODULE_ALIAS("usb:v19ABp1000d*dc*dsc*dp*ic0Eisc01ip00in*");
MODULE_ALIAS("usb:v041Ep4057d*dc*dsc*dp*ic0Eisc01ip00in*");
MODULE_ALIAS("usb:v18CDpCAFEd*dc*dsc*dp*ic0Eisc01ip00in*");
MODULE_ALIAS("usb:v*p*d*dc*dsc*dp*ic0Eisc01ip00in*");
