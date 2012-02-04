ifdef DEBUG
	FLAGS=-DDEBUG=1 --debug
endif

all:
	gcc -o string_buffer_test $(FLAGS) -Wall -lz string_buffer.c sb_test.c

clean:
	if test -e string_buffer_test; then rm string_buffer_test; fi
	if test -e string_buffer_test.dSYM; then rm -r string_buffer_test.dSYM; fi
	if test -e string_buffer_test.greg; then rm string_buffer_test.greg; fi
