obj-m += chardev.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

go: all
	@sudo insmod chardev.ko

test:
	@echo Hi, I am chardev | sudo tee /dev/chardev

test2:
	@echo Bye, bye | sudo tee /dev/chardev

cat:
	@sudo cat /dev/chardev

rm:
	@sudo rmmod chardev

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean