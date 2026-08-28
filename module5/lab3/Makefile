obj-m += blink_leds.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

go:
	@sudo insmod blink_leds.ko
	@sudo dmesg | grep "blink_leds"

0:
	@echo 0 | sudo tee /sys/kernel/systest/test

1:
	@echo 1 | sudo tee /sys/kernel/systest/test

2:
	@echo 2 | sudo tee /sys/kernel/systest/test

3:
	@echo 3 | sudo tee /sys/kernel/systest/test

4:
	@echo 4 | sudo tee /sys/kernel/systest/test

5:
	@echo 5 | sudo tee /sys/kernel/systest/test

6:
	@echo 6 | sudo tee /sys/kernel/systest/test

7:
	@echo 7 | sudo tee /sys/kernel/systest/test

rm:
	@sudo rmmod blink_leds

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean