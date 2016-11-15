
obj-m := s3435042.o

# Kernel headers must be installed

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
 
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	$(CC) s3435042UserApplication.c -o test
 
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
