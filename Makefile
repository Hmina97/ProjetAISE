

CC = gcc
CCFLAGS = -std=gnu99 -Wall -O0 -no-pie

LDFLAGS = -L.  -g -W -rdynamic
LIB =   -L.  -lunwind-ptrace -lunwind-generic -lunwind-x86_64 -lunwind -g -rdynamic

EXECUTABLES = \
	debug_m  \
	fich_test

ADDR = `objdump -d fich_test | grep do_stuff | head -n 1 | awk '{print $1;}'`

.PHONY: all clean test_debug_m

all: $(EXECUTABLES)

lib.a: debug.c debug.h
	$(CC) $(CCFLAGS) -O -c debug.c
	ar rcs lib.a debug.o

debug_m: debug_m.c lib.a
	$(CC) $(CCFLAGS) $< -o  $@ $(LDFLAGS)

test_debug_m: debug_m test
	./debug_m fich_test 0x$(ADDR)


fich_test: test.c
	$(CC) $(CCFLAGS) $^ -o $@

clean:
	rm -f $(EXECUTABLES) *.o *.a
