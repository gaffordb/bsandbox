# Base file copied from:
#  https://www.cs.swarthmore.edu/~newhall/unixhelp/howto_makefiles.html

CC = gcc
CFLAGS  = -g -Wall #-fsanitize=address

all: sandbox ./Makefile

sandbox:  sandbox.o sfp_list.o 
	$(CC) $(CFLAGS) -o sandbox sandbox.o sfp_list.o

sandbox.o:  my_sandbox.c sandbox_types.h
	$(CC) $(CFLAGS) -c my_sandbox.c -o sandbox.o

sfp_list.o:  sfp_list.c sandbox_types.h
	$(CC) $(CFLAGS) -c sfp_list.c

clean: 
	$(RM) ./sandbox *.o *~
