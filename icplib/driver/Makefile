SUB_DIRS	= switchdriver

DRIVER_PATH		=$(shell cd ../../appfs/driver ; pwd)

.PHONY: all
all:
	@echo $(DRIVER_PATH)
	$(foreach DIR,$(SUB_DIRS), \
		make -C $(DIR) O=$(DRIVER_PATH) || exit "$$?"; \
	)
