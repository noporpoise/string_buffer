ifdef DEBUG
	FLAGS=-DDEBUG=1 --debug
endif

all:
	gcc -o string_buffer_test $(FLAGS) -Wall -lz string_buffer.c sb_test.c

clean:
	if test -e string_buffer_test; then rm string_buffer_test; fi
	for file in $(wildcard *.dSYM); do rm -r $$file; done
	for file in $(wildcard *.greg); do rm $$file; done
