// SPDX-License-Identifier: GPL-2.0
/*
 * Hello world example
 *
 * Copyright (C) 2023 Andrew Kanner <andrew.kanner@gmail.com>
 */
#include <linux/module.h>

static int __init hello_world_init(void)
{
	pr_info("Hello world!\n");
	return 0;
}

static void __exit hello_world_exit(void)
{
	pr_info("Bye-bye world!\n");
}

module_init(hello_world_init);
module_exit(hello_world_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrew Kanner <andrew.kanner@gmail.com>");
