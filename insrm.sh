#/bin/sh

insmod hello.ko
sleep 1
rmmod hello.ko
dmesg | tail -5
