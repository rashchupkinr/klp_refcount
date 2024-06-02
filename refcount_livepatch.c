#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/livepatch.h>
#include "kp_refcount_test/kp_refcount_test.h"
#include "kp_refcount/kp_refcount.h"

//MODULE_LICENSE("GPL"); - necessary for testing with livepatch
//MODULE_INFO(livepatch, "Y");

extern refcount_t test_refcount;
#define KP_NREF 10

struct delayed_work work_refcount_inc;
struct delayed_work work_refcount_dec;
static void test_refcount_inc(struct work_struct *work)
{
	int i;
	for (i=0; i<KP_NREF; i++)
		refcount_inc(&test_refcount);
	schedule_delayed_work(&work_refcount_dec, msecs_to_jiffies(50));
}
static void test_refcount_dec(struct work_struct *work)
{
	int i;
	for (i=0; i<KP_NREF; i++)
		refcount_dec(&test_refcount);
	schedule_delayed_work(&work_refcount_inc, msecs_to_jiffies(50));
}

static void post_patch_callback(struct klp_object *klp_obj)
{
	schedule_delayed_work(&work_refcount_dec, 0);
}

static void pre_unpatch_callback(struct klp_object *klp_obj)
{
	cancel_delayed_work_sync(&work_refcount_inc);
	cancel_delayed_work_sync(&work_refcount_dec);
}

static struct klp_func funcs[] = {
	{ }
};

static struct klp_object objs[] = {
	{
		.name = "kp_refcount_test",
		.funcs = funcs,
		.callbacks = {
			.post_patch = post_patch_callback,
			.pre_unpatch = pre_unpatch_callback,
		},
	}, { }
};

static struct klp_patch patch = {
	.mod = THIS_MODULE,
	.objs = objs,
};

static int livepatch_init(void)
{
	INIT_DELAYED_WORK(&work_refcount_inc, test_refcount_inc);
	INIT_DELAYED_WORK(&work_refcount_dec, test_refcount_dec);
	return klp_enable_patch(&patch);
}

static void livepatch_exit(void)
{
}

module_init(livepatch_init);
module_exit(livepatch_exit);
