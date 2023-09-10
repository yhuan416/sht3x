KERNELDIR := /home/yhuan/stm32mp135/linux/linux-5.15.24
CURRENT_PATH := $(shell pwd)

obj-m := sht3x.o

build: kernel_modules

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean