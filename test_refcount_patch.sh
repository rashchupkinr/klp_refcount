#!/bin/sh
rmmod refcount_livepatch
rmmod kp_refcount_test
insmod kp_refcount_test/kp_refcount_test.ko
while $(true); do
	insmod refcount_livepatch.ko
	livepatch_enabled=/sys/kernel/livepatch/refcount_livepatch/enabled
	while [ ! -e $livepatch_enabled -o $(cat $livepatch_enabled) -eq 0 ]; do
		sleep 0.01;
	done
	sleep 1
	echo 0 > $livepatch_enabled
	rmmod refcount_livepatch
done
