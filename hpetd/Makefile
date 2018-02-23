# Makefile for kernel modules
#
# IMPORTANT:
# ==========
# Be certain that the /lib/modules/$(uname -r)/build directory is linked
# to the directory that contains the header files (/include/ ...) for
# the kernel for which you will be building this module.
#
# If you have a kernel tree already set up, from the top level of the
# kernel tree, linux-$(uname -r), type:
#
# 	make modules_prepare
#
# Alternatively, you can "sudo yum install kernel-headers" then link
# to the directory created by that install.#
# For example:
#
# 	ln -s /usr/src/kernels/2.6.33.6-147.fc13.i686/ \
# 	      /lib/modules/2.6.33.6-147.fc13.i686.PAE/build

obj-m := hpetd.o

# KDIR is the kernel directory that is the top-level
# of the kernel Makefile and header files.
KDIR := /lib/modules/$(shell uname -r)/build

# PWD is print working directory where your kernel
# module code resides.
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f Module.symvers

