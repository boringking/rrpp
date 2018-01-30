SUB_DIRS = libportseq

LIB_PATH	=$(shell cd ../../appfs/usr/lib ; pwd)

.PHONY: all
all:
	@echo $(LIB_PATH)
	$(foreach DIR,$(SUB_DIRS), \
		make -C $(DIR) O=$(LIB_PATH) || exit "$$?"; \
	)
	#./mvicplib.sh
