ifndef CC
  CC = gcc
endif

CFLAGS := -Wall -Wextra
LIBFLAGS := -L. -lstrbuf -lz

ifdef DEBUG
	CFLAGS := $(CFLAGS) -DDEBUG=1 --debug
else
	CFLAGS := $(CFLAGS) -O3
endif

all:
	$(CC) $(CFLAGS) -c string_buffer.c -o string_buffer.o
	ar -csru libstrbuf.a string_buffer.o
	$(CC) $(CFLAGS) strbuf_test.c -o strbuf_test $(LIBFLAGS)

clean:
	if test -e string_buffer.o; then rm string_buffer.o; fi
	if test -e libstrbuf.a; then rm libstrbuf.a; fi
	if test -e strbuf_test; then rm strbuf_test; fi
	for file in $(wildcard *.dSYM); do rm -r $$file; done
	for file in $(wildcard *.greg); do rm $$file; done
