## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS		= -I meta/autotools
AUTOMAKE_OPTIONS	= foreign
EXTRA_DIST		= INSTALL configure.sh
dist_doc_DATA		= README.md README.autotools LICENSE
AM_CFLAGS		= -Wall

## --------------------------------------------------------------------

EXTRA_DIST		+= meta/scripts/hoedown.pc.in
pkgconfigdir		= $(libdir)/pkgconfig
nodist_pkgconfig_DATA	= meta/scripts/hoedown.pc

## --------------------------------------------------------------------

AM_MAKEINFOFLAGS	= --no-split

info_TEXINFOS		= doc/hoedown.texi
doc_hoedown_TEXINFOS	= doc/macros.texi

#page
#### libraries

hoedown_CURRENT		= @hoedown_VERSION_INTERFACE_CURRENT@
hoedown_REVISION	= @hoedown_VERSION_INTERFACE_REVISION@
hoedown_AGE		= @hoedown_VERSION_INTERFACE_AGE@

include_HEADERS		= \
	src/autolink.h		\
	src/buffer.h		\
	src/document.h		\
	src/escape.h		\
	src/html.h		\
	src/stack.h		\
	src/version.h
includedir		= ${prefix}/include/hoedown

lib_LTLIBRARIES		= libhoedown.la
libhoedown_la_LDFLAGS	= -version-info $(hoedown_CURRENT):$(hoedown_REVISION):$(hoedown_AGE)
libhoedown_la_SOURCES	= \
	src/autolink.c		\
	src/autolink.h		\
	src/buffer.c		\
	src/buffer.h		\
	src/document.c		\
	src/document.h		\
	src/escape.c		\
	src/escape.h		\
	src/html.c		\
	src/html.h		\
	src/html_blocks.c	\
	src/html_smartypants.c	\
	src/stack.c		\
	src/stack.h		\
	src/version.c		\
	src/version.h

EXTRA_DIST		+= html_block_names.gperf

# Perfect hashing

src/html_blocks.c: html_block_names.gperf
	gperf -L ANSI-C -N hoedown_find_block_tag -c -C -E -S 1 --ignore-case -m100 $^ > $@

#page
#### programs

bin_PROGRAMS		= hoedown smartypants

hoedown_bin_cppflags	= -I $(srcdir)/src -I $(srcdir)/bin
hoedown_bin_ldadd	= libhoedown.la

hoedown_SOURCES		= bin/hoedown.c bin/common.h
hoedown_CPPFLAGS	= $(hoedown_bin_cppflags)
hoedown_LDADD		= $(hoedown_bin_ldadd)

smartypants_SOURCES	= bin/smartypants.c bin/common.h
smartypants_CPPFLAGS	= $(hoedown_bin_cppflags)
smartypants_LDADD	= $(hoedown_bin_ldadd)

#page
#### tests

check_PROGRAMS	= \
	test/test-version		\
	test/test-buffer		\
	test/test-stack

TESTS				= $(check_PROGRAMS)

hoedown_test_cppflags		= -I$(srcdir)/src
hoedown_test_ldadd		= libhoedown.la

test_test_version_CPPFLAGS	= $(hoedown_test_cppflags)
test_test_version_LDADD		= $(hoedown_test_ldadd)

test_test_buffer_CPPFLAGS	= $(hoedown_test_cppflags)
test_test_buffer_LDADD		= $(hoedown_test_ldadd)

test_test_stack_CPPFLAGS	= $(hoedown_test_cppflags)
test_test_stack_LDADD		= $(hoedown_test_ldadd)

installcheck-local: $(TESTS)
	@for f in $(TESTS); do $(builddir)/$$f; done

## --------------------------------------------------------------------
## custom tests

test: hoedown
	python $(srcdir)/test/runner.py

test-pl: hoedown
	perl $(srcdir)/test/MarkdownTest_1.0.3/MarkdownTest.pl \
		--script=./hoedown --testdir=$(srcdir)/test/MarkdownTest_1.0.3/Tests --tidy

### end of file
