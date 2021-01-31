
CC       := clang
LD       := clang
CCFLAGS  := 
LDFLAGS  := 
LIBS     := -lm

SANITIZE := -fsanitize=address,leak,undefined
# SANITIZE := -fsanitize=thread,undefined

SRCDIR   := src
BUILDDIR := build
OBJDIR   := $(BUILDDIR)/obj
BINDIR   := $(BUILDDIR)/bin

TARGETS := basicjit
MAINS   := $(patsubst %, $(SRCDIR)/%.c, $(TARGETS))
BINS    := $(patsubst %, $(BINDIR)/%, $(TARGETS))

SRCS := $(filter-out $(MAINS), $(shell find $(SRCDIR) -type f -name '*.c'))
HDRS := $(shell find $(SRCDIR) -type f -name '*.h')
OBJS := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

TOTAL    := $(words $(OBJS) $(MAINS) $(BINS))
COUNTER   = $(words $(HIDDEN_COUNT))$(eval HIDDEN_COUNT := x $(HIDDEN_COUNT))
PROGRESS  = $(shell expr $(COUNTER) '*' 100 / $(TOTAL))

CCFLAGS += -I$(SRCDIR)

release: CCFLAGS += -O3
release: LDFLAGS += -O3
release: build-binaries

debug: CCFLAGS += -O0 -g $(SANITIZE)
debug: LDFLAGS += -O0 -g $(SANITIZE)
debug: build-binaries

build-binaries: $(BINS)
	@printf "[100%%] Build successful.\n"

$(BINS): $(BINDIR)/%: $(OBJS) $(OBJDIR)/%.o
	@printf "[%3i%%] Building $@\n" $(PROGRESS)
	@mkdir -p $(shell dirname $@)
	@$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HDRS)
	@printf "[%3i%%] Building $@\n" $(PROGRESS)
	@mkdir -p $(shell dirname $@)
	@$(CC) $(CCFLAGS) -c -o $@ $<

new-release: clean release

new-debug: clean debug

test: debug
	./tests/run-tests.sh tests ./build/bin/basicjit

clean:
	@echo Cleaning local files
	@rm -rf $(OBJDIR)/*
	@rm -rf $(BINDIR)/*

.PHONY: release debug build-binaries new-release new-debug clean test