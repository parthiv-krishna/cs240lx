# list out the driver program source 
# TEST_SRC = tests/part2-test1.c tests/part2-test2.c tests/part2-test3.c tests/part2-test4.c tests/part2-test5.c tests/part2-test6.c

TEST_SRC = $(wildcard tests-2.0/*.c)
TEST_SRC = $(wildcard tests-stat/*.c)
TEST_SRC = tests-stat/part3-test3.c
#   if you want to use our staff-hc-sr04.o,
#   comment SRC out and uncomment STAFF_OBJS
SUPPORT_OBJS := ckalloc.o ck-gc.o gc-asm.o kr-malloc.o ck-memcheck.o interrupts-asm.o

# SRC=ck-gc.c ck-memcheck.c ckalloc.c kr-malloc.c pi-sbrk.c

CFLAGS = -fno-toplevel-reorder


# define this if you need to give the device for your pi
TTYUSB = 

# set RUN = 1 if you want the code to automatically run after building.
RUN = 1

DEPS = ./Makefile
include $(CS240LX_2022_PATH)/libpi/mk/Makefile.template
