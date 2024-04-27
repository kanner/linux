// SPDX-License-Identifier: GPL
#include <linux/module.h>

static int init_hello(void)
{
	pr_debug("Hello, KernelCare!");
	return 0;
}

static void exit_hello(void)
{
	pr_debug("Farewell!\n");
}

module_init(init_hello);
module_exit(exit_hello);
MODULE_LICENSE("GPL");
