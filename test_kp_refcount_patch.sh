#!/bin/sh
rmmod kp_refcount_livepatch
rmmod kp_refcount_test
rmmod kp_refcount
insmod kp_refcount/kp_refcount.ko
insmod kp_refcount_test/kp_refcount_test.ko
while $(true); do
	insmod kp_refcount_livepatch.ko
	livepatch_enabled=/sys/kernel/livepatch/kp_refcount_livepatch/enabled
	while [ ! -e $livepatch_enabled -o $(cat $livepatch_enabled) -eq 0 ]; do
		sleep 0.01;
	done
	sleep 1
	echo 0 > $livepatch_enabled
	rmmod kp_refcount_livepatch
done
