
# == Defaults
BUILD := debug
# ==

# == Targets (and sources relative to SOURCE_DIR/)
TARGETS += basicjit
SOURCES.basicjit := basicjit.c
# ==

# == Directories
SOURCE_DIR := src
BUILD_DIR  := build
OBJECT_DIR := $(BUILD_DIR)/$(BUILD)/obj
BINARY_DIR := $(BUILD_DIR)/$(BUILD)/bin
# ==

# == Files
$(foreach TARGET, $(TARGETS), $(eval OBJECTS.$(TARGET) := $(patsubst %.c, $(OBJECT_DIR)/%.o, $(SOURCES.$(TARGET)))))
TARGET_OBJECTS := $(foreach TARGET, $(TARGETS), $(OBJECTS.$(TARGET)))
SOURCES := $(shell find $(SOURCE_DIR) -type f -name '*.c')
OBJECTS := $(filter-out $(TARGET_OBJECTS), $(patsubst $(SOURCE_DIR)/%.c, $(OBJECT_DIR)/%.o, $(SOURCES)))
HEADERS := $(shell find $(SOURCE_DIR) -type f -name '*.h')
BINARYS := $(patsubst %, $(BINARY_DIR)/%, $(TARGETS))
# ==

# == Tools
CC := clang
LD := clang
# ==

# == Flags
SANITIZE := address,leak,undefined
# SANITIZE := thread,undefined

CCFLAGS.debug   := -O0 -g -fsanitize=$(SANITIZE) -DDEBUG
LDFLAGS.debug   := -O0 -g -fsanitize=$(SANITIZE)
CCFLAGS.release := -O3
LDFLAGS.release := -O3

CCFLAGS  := $(CCFLAGS.$(BUILD)) -I$(SOURCE_DIR) 
LDFLAGS  := $(LDFLAGS.$(BUILD))
LIBS     := -lm
# ==

# == Progress
TOTAL    := $(words $(sort $(OBJECTS) $(TARGET_OBJECTS) $(BINARYS)))
COUNTER   = $(words $(HIDDEN_COUNT))$(eval HIDDEN_COUNT := x $(HIDDEN_COUNT))
PROGRESS  = $(shell expr $(COUNTER) '*' 100 / $(TOTAL))
# ==

.SILENT:
.SECONDEXPANSION:
.PHONY: build release debug new new-release new-debug clean test

build: $(BINARYS)
	printf "[100%%] Build successful.\n"

release:
	$(MAKE) BUILD=release

debug:
	$(MAKE) BUILD=debug

$(BINARYS): $(BINARY_DIR)/%: $(OBJECTS) $$(OBJECTS.$$*)
	printf "[%3i%%] Building $@\n" $(PROGRESS)
	mkdir -p $(shell dirname $@)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/%.c $(HEADERS) $(MAKEFILE_LIST)
	printf "[%3i%%] Building $@\n" $(PROGRESS)
	mkdir -p $(shell dirname $@)
	$(CC) $(CCFLAGS) -c -o $@ $<

new: clean build

new-release: clean release

new-debug: clean debug

clean:
	echo Cleaning local files
	rm -rf $(BUILD_DIR)/*

test: build
	./tests/run-tests.sh tests $(BINARY_DIR)/basicjit
