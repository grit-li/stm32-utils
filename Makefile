TOP := .

include $(TOP)/Makefile.build
include $(TOP)/Makefile.func

DIRS += stm32_hexinfo
DIRS += stm32_bin2hex

.PHONY: rebuild all clean $(DIRS)
all: 
	@$(call FOREACH_EXECUTE_FUNC,$(DIRS),$@)

clean:
	@$(call FOREACH_EXECUTE_FUNC,$(DIRS),$@)
	@$(RM) -rf $(TOP)/out

rebuild:
	@$(MAKE) clean --no-print-directory
	@$(MAKE) all --no-print-directory
