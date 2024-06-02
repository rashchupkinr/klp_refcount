#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/livepatch.h>
#include "kp_refcount_test/kp_refcount_test.h"
#include "kp_refcount/kp_refcount.h"

//MODULE_LICENSE("GPL"); - necessary for testing with livepatch
//MODULE_INFO(livepatch, "Y");

extern refcount_t test_refcount;
#define KP_NREF 10
static struct ref_holder {
	unsigned char v;
} kp_ref_holders[KP_NREF] = { 0 };
kp_refcount_t *kp_test_ref = 0;

struct delayed_work kp_work_refcount_inc;
struct delayed_work kp_work_refcount_dec;
static void kp_test_refcount_inc(struct work_struct *work)
{
	int i;
	for (i=0; i<KP_NREF; i++)
		kp_refcount_inc(kp_test_ref, &kp_ref_holders[i].v);
	schedule_delayed_work(&kp_work_refcount_dec, msecs_to_jiffies(50));
}
static void kp_test_refcount_dec(struct work_struct *work)
{
	int i;
	for (i=0; i<KP_NREF; i++)
		kp_refcount_dec(kp_test_ref, &kp_ref_holders[i].v);
	schedule_delayed_work(&kp_work_refcount_inc, msecs_to_jiffies(50));
}

static int livepatch_kp_refcount_test_iter(void)
{
	int i;
	if (!kp_test_ref)
		return -1;
	for (i=0; i<NREF; i++)
		refcount_inc(&test_refcount);
	for (i=0; i<NREF; i++)
		if (kp_refcount_dec_and_test(kp_test_ref, 0))
			return -1;
	return 0;
}

static void kp_post_patch_callback(struct klp_object *klp_obj)
{
	schedule_delayed_work(&kp_work_refcount_dec, 0);
}

static void kp_pre_unpatch_callback(struct klp_object *klp_obj)
{
	cancel_delayed_work_sync(&kp_work_refcount_inc);
	cancel_delayed_work_sync(&kp_work_refcount_dec);
}

static struct klp_func funcs[] = {
	{
		.old_name = "refcount_test_iter",
		.new_func = livepatch_kp_refcount_test_iter,
	}, { }
};

static struct klp_object objs[] = {
	{
		.name = "kp_refcount_test",
		.funcs = funcs,
		.callbacks = {
			.post_patch = kp_post_patch_callback,
			.pre_unpatch = kp_pre_unpatch_callback,
		},
	}, { }
};

static struct klp_patch patch = {
	.mod = THIS_MODULE,
	.objs = objs,
};

static int livepatch_init(void)
{
	INIT_DELAYED_WORK(&kp_work_refcount_inc, kp_test_refcount_inc);
	INIT_DELAYED_WORK(&kp_work_refcount_dec, kp_test_refcount_dec);
	kp_test_ref = kp_refcount_alloc(&test_refcount, GFP_KERNEL);
	if (!kp_test_ref) {
		pr_alert("kp_refcount_livepatch: memory allocation_failed");
		return -1;
	}
	return klp_enable_patch(&patch);
}

static void livepatch_exit(void)
{
	kp_refcount_free(kp_test_ref);
}

module_init(livepatch_init);
module_exit(livepatch_exit);
