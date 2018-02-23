//
// ola.c
//
// Demo kmod
//

#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tony Camuso <tony.camuso@hp.com>");
MODULE_DESCRIPTION("Demo kmod");

static int __init ola_init(void)
{
	printk(KERN_INFO "*************************************************\n");
	printk(KERN_INFO "******************  Ola Mundo! ******************\n");
	printk(KERN_INFO "*************************************************\n");

	return 0;
}

static void __exit ola_exit(void)
{
	printk(KERN_INFO "-------------------------------------------------\n");
	printk(KERN_INFO "--------------- Tchau Tchau, Mundo! -------------\n");
	printk(KERN_INFO "-------------------------------------------------\n");
}

module_init(ola_init);
module_exit(ola_exit);
