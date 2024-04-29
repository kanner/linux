// SPDX-License-Identifier: GPL
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/jiffies.h>
#include <linux/stat.h>

static struct dentry *dir;

static int hello_jiffies_get(void *data, u64 *val)
{
	*val = jiffies_64;
	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(jiffies_fops, hello_jiffies_get, NULL, "%llu");

static int init_hello(void)
{
	pr_debug("Hello, KernelCare!");
	dir = debugfs_create_dir("kernelcare", NULL);
	if (IS_ERR(dir)) {
		pr_err("Failed to create dir: %ld", PTR_ERR(dir));
		return PTR_ERR(dir);
	}
	struct dentry *jiff_file = debugfs_create_file("jiffies", 0444, dir, NULL, &jiffies_fops);

	if (IS_ERR(jiff_file)) {
		pr_err("Failed to create jiffies: %ld", PTR_ERR(jiff_file));
		return PTR_ERR(jiff_file);
	}
	return 0;
}

static void exit_hello(void)
{
	pr_debug("Farewell!\n");
	debugfs_remove_recursive(dir);
}

module_init(init_hello);
module_exit(exit_hello);
MODULE_LICENSE("GPL");
