# Common
prefix= /usr/local
libdir= $(prefix)/lib
incdir= $(prefix)/include

CC=   clang

CFLAGS+= -std=c99
CFLAGS+= -Wall -Wextra -Werror -Wsign-conversion
CFLAGS+= -Wno-unused-parameter -Wno-unused-function

LDFLAGS=

PANDOC_OPTS= -s --toc --email-obfuscation=none

# Platform specific
platform= $(shell uname -s)

ifeq ($(platform), Linux)
	CFLAGS+= -DHT_PLATFORM_LINUX
	CFLAGS+= -D_POSIX_C_SOURCE=200809L -D_BSD_SOURCE
endif

# Debug
debug=0
ifeq ($(debug), 1)
	CFLAGS+= -g -ggdb
else
	CFLAGS+= -O2
endif

# Coverage
coverage?= 0
ifeq ($(coverage), 1)
	CC= gcc
	CFLAGS+= -fprofile-arcs -ftest-coverage
	LDFLAGS+= --coverage
endif

# Target: libhashtable
libhashtable_LIB= libhashtable.a
libhashtable_SRC= $(wildcard src/*.c)
libhashtable_INC= src/hashtable.h
libhashtable_OBJ= $(subst .c,.o,$(libhashtable_SRC))

$(libhashtable_LIB): CFLAGS+=

# Target: tests
tests_SRC= $(wildcard tests/*.c)
tests_OBJ= $(subst .c,.o,$(tests_SRC))
tests_BIN= $(subst .o,,$(tests_OBJ))

$(tests_BIN): CFLAGS+= -Isrc
$(tests_BIN): CFLAGS+= -D_GNU_SOURCE
$(tests_BIN): CFLAGS+= `pkg-config --cflags glib-2.0`
$(tests_BIN): LDFLAGS+= -L.
$(tests_BIN): LDFLAGS+= `pkg-config --libs-only-L glib-2.0`
$(tests_BIN): LDLIBS+= -lhashtable
$(tests_BIN): LDLIBS+= `pkg-config --libs-only-l glib-2.0`

# Target: doc
doc_SRC= $(wildcard doc/*.mkd)
doc_HTML= $(subst .mkd,.html,$(doc_SRC))

# Rules
all: lib tests doc

lib: $(libhashtable_LIB)

tests: lib $(tests_BIN)

doc: $(doc_HTML)

$(libhashtable_LIB): $(libhashtable_OBJ)
	$(AR) cr $@ $(libhashtable_OBJ)

tests/%: tests/%.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

doc/%.html: doc/*.mkd
	pandoc $(PANDOC_OPTS) -t html5 -o $@ $<

clean:
	$(RM) $(libhashtable_LIB) $(wildcard src/*.o)
	$(RM) $(tests_BIN) $(wildcard tests/*.o)
	$(RM) $(wildcard **/*.gc??)
	$(RM) -r coverage

coverage:
	lcov -o /tmp/libhashtable.info -c -d .
	genhtml -o coverage -t libhashtable /tmp/libhashtable.info
	rm /tmp/libhashtable.info

install: all
	mkdir -p $(libdir) $(incdir)
	install -m 644 $(libhashtable_LIB) $(libdir)
	install -m 644 $(libhashtable_INC) $(incdir)

uninstall:
	$(RM) $(addprefix $(libdir)/,$(libhashtable_LIB))
	$(RM) $(addprefix $(incdir)/,$(libhashtable_INC))

tags:
	ctags -o .tags -a $(wildcard src/*.[hc])

.PHONY: all lib tests doc clean coverage install uninstall tags
