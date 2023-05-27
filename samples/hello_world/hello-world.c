// SPDX-License-Identifier: GPL-2.0
/*
 * Hello world example
 *
 * Copyright (C) 2023 Andrew Kanner <andrew.kanner@gmail.com>
 */
#include <linux/module.h>
#include <linux/delay.h>

static int sleep_secs;
module_param(sleep_secs, int, 0644);
MODULE_PARM_DESC(sleep_secs, "sleep_secs (default=0)");

static int __init hello_world_init(void)
{
	pr_info("Hello world!\n");
	if (sleep_secs) {
		pr_info("sleeping %d seconds ...\n", sleep_secs);
		msleep(sleep_secs * 1000);
	}
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
