ifndef CC
  CC = gcc
endif

CFLAGS := -Wall -Wextra
LIBFLAGS := -L. -lstrbuf -lz

ifdef DEBUG
	OPT = -O0 -DDEBUG=1 --debug -g -ggdb
else
	OPT = -O3
endif


all: libstrbuf.a strbuf_test

libstrbuf.a: string_buffer.c string_buffer.h stream_buffer.h
	$(CC) $(CFLAGS) $(OPT) -c string_buffer.c -o string_buffer.o
	ar -csru libstrbuf.a string_buffer.o

strbuf_test: strbuf_test.c libstrbuf.a
	$(CC) $(CFLAGS) $(OPT) strbuf_test.c -o strbuf_test $(LIBFLAGS)

clean:
	rm -rf string_buffer.o libstrbuf.a strbuf_test *.dSYM *.greg
	rm -rf tmp.strbuf.*.txt tmp.strbuf.*.txt.gz

.PHONY: all clean
