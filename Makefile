# Makefile

# Copyright (c) 2009, Natacha Porté
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

DEPDIR=depends

# "Machine-dependant" options
OS := $(shell uname -s | sed -e 's/[-.0-9]//g')
$(info  building for host platform '$(OS)' )
ifeq "$(OS)" "MINGW_NT"

ifndef MCFLAGS
MCFLAGS=
endif
ifndef MLFLAGS
MLFLAGS=
endif
ifndef MFLAGS
MFLAGS=
endif

else 

ifndef MCFLAGS
MCFLAGS=-fPIC
endif
ifndef MLFLAGS
MLFLAGS=
endif
ifndef MFLAGS
MFLAGS=
#MFLAGS=-m32 -mmacosx-version-min=10.5 --sysroot /Developer/SDKs/MacOSX10.5.sdk
endif

endif


CFLAGS=-c -g -O3 -Wall -Werror -Wsign-compare -Isrc -Ihtml $(MCFLAGS) $(MFLAGS)
LDFLAGS=-g -O3 -Wall -Werror $(MLFLAGS) $(MFLAGS)
CC=gcc


UPSKIRT_SRC=\
	src/markdown.o \
	src/stack.o \
	src/buffer.o \
	src/autolink.o \
	html/html.o \
	html/html_smartypants.o \
	html/houdini_html_e.o \
	html/houdini_href_e.o

all:		libupskirt.so upskirt smartypants html_blocks

.PHONY:		all clean

# libraries

libupskirt.so:	libupskirt.so.1
	-ln -f -s $^ $@

libupskirt.so.1: $(UPSKIRT_SRC)
	$(CC) $(LDFLAGS) -shared $^ -o $@

# executables

upskirt:	examples/upskirt.o $(UPSKIRT_SRC)
	$(CC) $(LDFLAGS) $^ -o $@

smartypants: examples/smartypants.o $(UPSKIRT_SRC)
	$(CC) $(LDFLAGS) $^ -o $@

# perfect hashing
html_blocks: src/html_blocks.h

src/html_blocks.h: html_block_names.txt
	-todos -a -d html_block_names.txt
	gperf -N find_block_tag -H hash_block_tag -C -c -E --ignore-case -L ANSI-C $^ > $@

src/markdown.o: src/markdown.c src/html_blocks.h


# housekeeping
clean:
	rm -f src/*.o html/*.o examples/*.o
	rm -f libupskirt.so libupskirt.so.1 upskirt smartypants
	rm -f upskirt.exe smartypants.exe
	rm -rf $(DEPDIR)

superclean: clean
	rm -f src/html_blocks.h


# dependencies

include $(wildcard $(DEPDIR)/*.d)


# generic object compilations

%.o:	src/%.c examples/%.c html/%.c
	@mkdir -p $(DEPDIR)
	@$(CC) -MM $< > $(DEPDIR)/$*.d
	$(CC) $(CFLAGS) -o $@ $<

