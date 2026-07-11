#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x1b96180a, "module_layout" },
	{ 0x56c87b84, "kobject_put" },
	{ 0xea3c74e, "tasklet_kill" },
	{ 0xfe990052, "gpio_free" },
	{ 0xc1514a3b, "free_irq" },
	{ 0xedc03953, "iounmap" },
	{ 0x622559e2, "class_unregister" },
	{ 0xbd78d155, "device_destroy" },
	{ 0x92d5838e, "request_threaded_irq" },
	{ 0x6e5ec684, "gpiod_to_irq" },
	{ 0x40864e3, "gpiod_direction_input" },
	{ 0x9471e05a, "gpio_to_desc" },
	{ 0x47229b5c, "gpio_request" },
	{ 0xe97c4103, "ioremap" },
	{ 0xa4446815, "sysfs_create_group" },
	{ 0x868f8b1f, "kobject_create_and_add" },
	{ 0x8330dda4, "kernel_kobj" },
	{ 0x9fcdb8ea, "class_destroy" },
	{ 0x6dc527c6, "device_create" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0x4147b1fd, "__class_create" },
	{ 0xde9d9b1e, "__register_chrdev" },
	{ 0x51a910c0, "arm_copy_to_user" },
	{ 0x9d2ab8ac, "__tasklet_schedule" },
	{ 0xca54fee, "_test_and_set_bit" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0x822137e2, "arm_heavy_mb" },
	{ 0xbcab6ee6, "sscanf" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0xc5850110, "printk" },
	{ 0xe707d823, "__aeabi_uidiv" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "05582F364361BECDBC538A0");
