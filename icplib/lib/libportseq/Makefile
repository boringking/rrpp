WORK_FN_FILE_SIZE	=$(shell (ls -l $(1) | awk '{print $$5}'))

ifneq ($(O), )
INCLUDEPATH		= $(shell cd ../../../linux/include; pwd)
LIBPATH			= $(shell cd ../../../appfs/usr/lib; pwd)
EXE				= $(O)/libportseq.so
else
EXE				= libportseq.so
MGCC			= mips-linux-uclibc-gcc

ifneq ($(INC), )
INCLUDEPATH		= $(INC)
else
INCLUDEPATH		= $(shell cd ../../../../build/linux/include; pwd)
endif

endif

MIPS_GCC_OPTION ?= -Wall $(libs)
MIPS_GCC		?= ${MGCC} $(MIPS_GCC_OPTION) -I$(INCLUDEPATH)

.PHONY: all
all:
	@echo "Making lib/libportseq all ..."
	make $(EXE)
ifneq ($(INC),)
	cp -f *.h $(INC)/
else
	@cp *.h $(INCLUDEPATH)
	@make show_size
endif
ifneq ($(OUT), )
	cp -f $(EXE) $(OUT)/
endif

$(EXE):portnumber.o
	$(MIPS_GCC) -shared -o $@ $^

portnumber.o:portnumber.c
	$(MIPS_GCC) -c -fPIC $^

.PHONY: show_size
show_size:
	@echo "Info: generate AP $@ with size = $(call WORK_FN_FILE_SIZE,$(EXE))"

.PHONY: clean
clean:
	rm -rf *.o *.so
