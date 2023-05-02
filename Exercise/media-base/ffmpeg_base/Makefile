
TOP		= .
BINPATH = $(TOP)/bin

SUBDIRS = $(wildcard $(TOP)/src/*)
TARGETS = $(notdir $(SUBDIRS))

all:
	@echo "Please input one of:\n$(TARGETS)"

clean:
	-rm -rf $(BINPATH)/*

$(filter-out all clean,$(MAKECMDGOALS)):
ifeq ($(filter $(MAKECMDGOALS),$(TARGETS)),)
	@echo "Not found[$(MAKECMDGOALS)]! Please input one of:\n$(TARGETS)"
else
	@echo "TGTNAME = $(MAKECMDGOALS)"
	make -C build TGTNAME=$(MAKECMDGOALS)
endif

#EXCPARAMS = "all clean"
#为何不能用参数替换filter-out的选项?
.PHONY: $(EXCPARAMS) $(filter-out all clean,$(MAKECMDGOALS))

