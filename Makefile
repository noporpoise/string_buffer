ifdef DEBUG
	DEBUG_FLAGS=-DDEBUG=1 --debug
else
	DEBUG_FLAGS=-O3
endif

CFLAGS=-Wall
LIB_FLAGS=-lz

all:
	gcc -o string_buffer.o $(DEBUG_FLAGS) $(CFLAGS) -c string_buffer.c
	gcc -o string_buffer_test $(DEBUG_FLAGS) $(CFLAGS) $(LIB_FLAGS) \
	  string_buffer.o sb_test.c

clean:
	if test -e string_buffer.o; then rm string_buffer.o fi
	if test -e string_buffer_test; then rm string_buffer_test; fi
	if test -e string_buffer_test.dSYM; then rm -r string_buffer_test.dSYM; fi
	if test -e string_buffer_test.greg; then rm string_buffer_test.greg; fi
