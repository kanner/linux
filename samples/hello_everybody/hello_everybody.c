// SPDX-License-Identifier: GPL
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/jiffies.h>
#include <linux/stat.h>
#include <linux/syscalls.h>

static struct dentry *dir;

static int hello_jiffies_get(void *data, u64 *val)
{
	*val = jiffies_64;
	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(jiffies_fops, hello_jiffies_get, NULL, "%llu");

static DECLARE_RWSEM(data_lock);
// file_data is protected by the data_lock
static char *file_data;
static char *backup_buffer;

static ssize_t data_fops_read(struct file *filp,
			      char __user *dst,
			      size_t len,
			      loff_t *offset)
{
	ssize_t copied = 0;

	down_read(&data_lock);
	if (*offset >= PAGE_SIZE)
		goto DONE_READ;
	copied = min(len, (size_t)(PAGE_SIZE - *offset));
	if (copy_to_user(dst, file_data + *offset, copied)) {
		copied = -EFAULT;
		goto DONE_READ;
	}
	*offset += copied;
DONE_READ:
	up_read(&data_lock);
	return copied;
}

static ssize_t data_fops_write(struct file *filp,
			       const char __user *src,
			       size_t len,
			       loff_t *offset)
{
	ssize_t written = 0;

	down_write(&data_lock);
	if (*offset >= PAGE_SIZE)
		goto DONE_WRITE;
	written = min(len, PAGE_SIZE - (size_t)*offset);
	// Save data in case data is partially copied
	memcpy(backup_buffer, file_data, PAGE_SIZE);
	if (copy_from_user(file_data + *offset, src, written)) {
		// failed to copy all data, restore previous file state
		memcpy(file_data, backup_buffer, PAGE_SIZE);
		written = -EFAULT;
		goto DONE_WRITE;
	}
	*offset += written;

DONE_WRITE:
	up_write(&data_lock);
	return written;
}

static const struct file_operations data_fops = {
	.read = data_fops_read,
	.write = data_fops_write
};

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
		debugfs_remove_recursive(dir);
		return PTR_ERR(jiff_file);
	}

	// uid will be 0 and it can read and write, others can only read
	struct dentry *data_file = debugfs_create_file("data", 0644, dir, NULL, &data_fops);

	if (IS_ERR(data_file)) {
		pr_err("Failed to create data: %ld", PTR_ERR(data_file));
		debugfs_remove_recursive(dir);
		return PTR_ERR(data_file);
	}
	// Set up access to data file
	struct iattr data_attr = {
		.ia_valid = ATTR_UID,
		.ia_uid = GLOBAL_ROOT_UID
	};

	inode_lock(data_file->d_inode);
	int attr_assign_ret = notify_change(&nop_mnt_idmap, data_file, &data_attr, NULL);

	if (attr_assign_ret) {
		pr_err("Error setting attributes: %d", attr_assign_ret);
		inode_unlock(data_file->d_inode);
		debugfs_remove_recursive(dir);
		return attr_assign_ret;
	}
	inode_unlock(data_file->d_inode);

	// Allocate two pages, first for file data, second for backup
	file_data = kzalloc(PAGE_SIZE * 2, GFP_KERNEL);
	if (file_data == NULL) {
		debugfs_remove_recursive(dir);
		return -ENOMEM;
	}
	backup_buffer = file_data + PAGE_SIZE;

	return 0;
}

static void exit_hello(void)
{
	pr_debug("Farewell!\n");
	kfree(file_data);
	debugfs_remove_recursive(dir);
}

module_init(init_hello);
module_exit(exit_hello);
MODULE_LICENSE("GPL");
