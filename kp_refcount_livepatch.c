#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/livepatch.h>
#include "kp_refcount_test/kp_refcount_test.h"
#include "kp_refcount/kp_refcount.h"

//MODULE_LICENSE("GPL"); - necessary for testing with livepatch
//MODULE_INFO(livepatch, "Y");

void livepatch_start_refcount_test(void)
{
	printk(KERN_ERR "livepatch_start_refcount_test()\n");
	if (refcount_test())
		printk(KERN_ERR "livepatch_refcount_test: error\n");
	else
		printk(KERN_ERR "livepatch_refcount_test: success\n");
}

static struct klp_func funcs[] = {
	{
		.old_name = "start_refcount_test",
		.new_func = livepatch_start_refcount_test,
	}, { }
};

static struct klp_object objs[] = {
	{
		.name = "kp_refcount_test",
		.funcs = funcs,
	}, { }
};

static struct klp_patch patch = {
	.mod = THIS_MODULE,
	.objs = objs,
};

static int livepatch_init(void)
{
	return klp_enable_patch(&patch);
}

static void livepatch_exit(void)
{
}

module_init(livepatch_init);
module_exit(livepatch_exit);
