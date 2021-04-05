
# == Defaults
BUILD       := debug
NUMTESTJOBS := 6
# ==

# == Targets
TARGETS := basicjit
# ==

# == Directories
SOURCE_DIR := src
BUILD_DIR  := build
OBJECT_DIR := $(BUILD_DIR)/$(BUILD)/obj
BINARY_DIR := $(BUILD_DIR)/$(BUILD)/bin
# ==

# == Files
$(foreach TARGET, $(TARGETS), \
	$(eval SOURCES.$(TARGET) := $(shell find $(SOURCE_DIR)/$(TARGET) -type f -name '*.c')) \
	$(eval OBJECTS.$(TARGET) := $(patsubst $(SOURCE_DIR)/%.c, $(OBJECT_DIR)/%.o, $(SOURCES.$(TARGET)))))
SOURCES        := $(shell find $(SOURCE_DIR) -type f -name '*.c')
ALL_OBJECTS    := $(patsubst $(SOURCE_DIR)/%.c, $(OBJECT_DIR)/%.o, $(SOURCES))
TARGET_OBJECTS := $(foreach TARGET, $(TARGETS), $(OBJECTS.$(TARGET)))
OBJECTS        := $(filter-out $(TARGET_OBJECTS), $(ALL_OBJECTS))
DEPENDENCIES   := $(ALL_OBJECTS:.o=.d)
BINARYS        := $(patsubst %, $(BINARY_DIR)/%, $(TARGETS))
# ==

# == Tools
CC   ?= clang
LINK := $(CC)
# ==

# == Flags
SANITIZE := address,leak,undefined
# SANITIZE := thread,undefined
WARNINGS := -Wall -Wextra -Wno-unused-parameter

CCFLAGS.debug   += -O0 -g -fsanitize=$(SANITIZE) #-DDEBUG
LDFLAGS.debug   += -O0 -g -fsanitize=$(SANITIZE)
CCFLAGS.release += -O3 -flto
LDFLAGS.release += -O3 -flto

CCFLAGS += $(CCFLAGS.$(BUILD)) $(WARNINGS) -I$(SOURCE_DIR) -MMD -MP
LDFLAGS += $(LDFLAGS.$(BUILD)) -rdynamic
LIBS    += -lm
# ==

# == Extra flags (enable/disable readline)
ifndef NOREADLINE
LIBS += -lreadline
else
CCFLAGS += -DNOREADLINE
endif
# ==

# == Extra flags (enable/disable native functions)
ifndef NONATIVEFN
LIBS += -ldl
else
CCFLAGS += -DNONATIVEFN
endif
# ==

# == Progress
ifndef ECHO
TOTAL   := \
	$(shell $(MAKE) $(MAKECMDGOALS) --no-print-directory -nrRf $(firstword $(MAKEFILE_LIST)) \
		ECHO="__HIT_MARKER__" BUILD=$(BUILD) | grep -c "__HIT_MARKER__")
TLENGTH := $(shell expr length $(TOTAL))
COUNTER  = $(words $(HIDDEN_COUNT))
COUNTINC = $(eval HIDDEN_COUNT := x $(HIDDEN_COUNT))
PERCENT  = $(shell expr $(COUNTER) '*' 100 / $(TOTAL))
ECHO     = $(COUNTINC)printf "[%*i/%i](%3i%%) %s\n" $(TLENGTH) $(COUNTER) $(TOTAL) $(PERCENT)
endif
# ==

.SILENT:
.SECONDARY:
.SECONDEXPANSION:
.PHONY: build clean test

build: $(BINARYS)
	@$(ECHO) "Build successful."

$(BINARYS): $(BINARY_DIR)/%: $(OBJECTS) $$(OBJECTS.$$*) | $$(dir $$@)
	@$(ECHO) "Building $@"
	$(LINK) $(LDFLAGS) -o $@ $^ $(LIBS)

$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/%.c $(MAKEFILE_LIST) | $$(dir $$@)
	@$(ECHO) "Building $@"
	$(CC) $(CCFLAGS) -c -o $@ $<

%/:
	@$(ECHO) "Building $@"
	mkdir -p $@

clean:
	@$(ECHO) "Cleaning local files"
	rm -rf $(BUILD_DIR)/*

test: build
	$(MAKE) -C tested BUILD=release
	@$(ECHO) "Running tests"
	BUILD=$(BUILD) ./tested/build/release/bin/tested -j$(NUMTESTJOBS) tests

-include $(DEPENDENCIES)

