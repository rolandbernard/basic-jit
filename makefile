ODIR=./build/objs
BDIR=./build/bin

IDIR=./src
SDIR=./src

CC=gcc
LINK=gcc
DFLAGS=-g -O0 -fsanitize=address,undefined -DDEBUG
RFLAGS=-O3
CFLAGS=-I$(IDIR) -Wall $(RFLAGS)
LIBS=-lm

_SRC=$(shell find $(SDIR) -type f -name '*.c')
OBJ=$(patsubst $(SDIR)/%.c,$(ODIR)/%.o,$(_SRC))

DEPS=$(shell find $(IDIR) -type f -name '*.h')

_BIN=basicjit
BIN=$(patsubst %,$(BDIR)/%,$(_BIN))

.PHONY: all
all: $(BIN) 

.PHONY: install
install: all
	cp $(BIN) /usr/bin/

$(BDIR)/basicjit: $(OBJ)
	mkdir -p `dirname $@`
	$(LINK) $(CFLAGS) -o $@ $^ $(LIBS)

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	mkdir -p `dirname $@`
	$(CC) $(CFLAGS) -c -o $@ $<
	
.PHONY: new
new: clean all
	
.PHONY: clean
clean:
	rm -fr $(ODIR)/*

.PHONY: cleanall
cleanall:
	rm -fr $(ODIR)/* $(BDIR)/*

