# Comment/uncomment the following line to disable/enable debugging
#DEBUG = y
# Add your debugging flag (or not) to CFLAGS
ifeq ($(DEBUG),y)
	DEBFLAGS = -g # "-O" is needed to expand inlines
else
	DEBFLAGS =
endif
# EXTRA_CFLAGS += $(DEBFLAGS) -I$(LDDINCDIR)

# Use MDC/MDIO to access external switch register.
# EXTERNAL_SWITCH = y
ifeq ($(EXTERNAL_SWITCH),y)
  ccflags-y += -DACCESS_REG_BY_MDIO
endif

DRIVER_NAME = ip1829drv

WORK_FN_FILE_SIZE	=$(shell (ls -l $(1) | awk '{print $$5}'))
PWD	:= $(shell pwd)
EXE	= $(DRIVER_NAME).ko
ifneq ($(O),)
KERNELDIR	?= ../../../linux/
INCLUDEPATH	?= ../../../linux/include
else

ifneq ($(INC),)
KERNELDIR		?= /lib/modules/$(shell uname -r)/build
EXTRA_CFLAGS	+= -I$(INC)
else
KERNELDIR		?= ../../../../build/linux
EXTRA_CFLAGS	+= -I$(PWD)/../../include
endif

endif

obj-m := $(DRIVER_NAME).o
$(DRIVER_NAME)-objs := ip1829.o ip1829fdat.o ip1829func.o

.PHONY: all
all:
	@echo "  Making $@ ..."
ifneq ($(CROSS_COMPILE),)
	$(MAKE) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNELDIR) M=$(PWD) modules
else
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif
	@make show_size

ifneq ($(O),)
	@cp -f $(EXE) $(O)
	@cp	*.h $(INCLUDEPATH)
endif
ifneq ($(INC),)
	cp -f *.h $(INC)/
endif
ifneq ($(OUT), )
	cp -f $(EXE) $(OUT)
endif

.PHONY: show_size
show_size:
	@echo "Info: generate AP $@ with size = $(call WORK_FN_FILE_SIZE,$(EXE))"

.PHONY: clean
clean:
	rm -rf *.o .*.d *~ core .depend .*.cmd *.ko* *.mod.c .tmp_versions modules.order Module.symvers

depend .depend dep:
	$(CC) $(CFLAGS) -M *.c > .depend

ifeq (.depend,$(wildcard .depend))
	include .depend
endif
