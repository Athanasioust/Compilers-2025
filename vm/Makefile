CC=gcc
BASE_DIR=../
CFLAGS=-Wall -g
LIBS=-lm
OBJ=avm.c avm_execution.c avm_functions.c avm_tables.c avm_libfuncs.c

all:
	$(CC) $(CFLAGS) $(OBJ) -o $(BASE_DIR)/vm.exe $(LIBS)

.PHONY: clean
clean:
	rm -rf *.o *.out *.exe