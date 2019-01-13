ifeq ($(KERNELRELEASE),)
    KERNELDIR ?= /lib/modules/$(shell uname -r)/build
    PWD := $(shell pwd)
    RULE = $(MAKE) -C $(KERNELDIR) M=$(PWD) $@

# test: test.o
# 	gcc $^ -o $@
modules:
	$(RULE)
modules_install:
	$(RULE)
clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions modules.order Module.symvers # test

.PHONY: modules modules_install clean
else
    obj-m := rootkit.o
    ccflags-y := # -Wno-declaration-after-statement
endif
