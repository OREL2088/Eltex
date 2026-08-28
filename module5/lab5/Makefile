CC=x86_64-linux-gnu-gcc
obj-m += netlink.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules CC=$(CC)
	@sudo insmod netlink.ko
	gcc user.c -o user 

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	@sudo rmmod netlink
	rm -f user