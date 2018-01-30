SUB_DIRS	= include driver lib
.PHONY: all
all:
	$(foreach DIR,$(SUB_DIRS), \
		make -C $(DIR) O=$(O) || exit "$$?"; \
	)

