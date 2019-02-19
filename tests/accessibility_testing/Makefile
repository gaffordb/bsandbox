# Base file copied from:
#  https://www.cs.swarthmore.edu/~newhall/unixhelp/howto_makefiles.html

CC = gcc
CFLAGS  = -g -Wall

test: test.c sfp_list.o ../../sandbox_types.h
	$(CC) $(CFLAGS) -o test test.c sfp_list.o; ./test

sfp_list.o: ../../sfp_list.c ../../sandbox_types.h
	$(CC) -c ../../sfp_list.c -o ./sfp_list.o

clean: 
	$(RM) ./test *.o *~
