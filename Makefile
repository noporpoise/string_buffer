ifndef CC
  CC = gcc
endif

CFLAGS := -Wall -Wextra
LIBFLAGS := -L. -lstrbuf -lz

ifdef DEBUG
	CFLAGS := $(CFLAGS) -O0 -DDEBUG=1 --debug -g -ggdb
	OPT = -00
else
	CFLAGS := $(CFLAGS)
	OPT = -03
endif


all: libstrbuf.a strbuf_test

libstrbuf.a: string_buffer.c string_buffer.h buffered_input.h
	$(CC) $(CFLAGS) $(OPT) -c string_buffer.c -o string_buffer.o
	ar -csru libstrbuf.a string_buffer.o

strbuf_test: strbuf_test.c libstrbuf.a
	$(CC) $(CFLAGS) -O0 strbuf_test.c -o strbuf_test $(LIBFLAGS)

clean:
	rm -rf string_buffer.o libstrbuf.a strbuf_test *.dSYM *.greg
	rm -rf tmp.strbuf.*.txt tmp.strbuf.*.txt.gz

.PHONY: all clean
