# list out the driver program source 
TEST_SRC = 1-fft-test.c
TEST_SRC = 2-fft-time.c
TEST_SRC = 3-fft-freqout.c
TEST_SRC = 4-fft-freqviz.c
OPT_LEVEL = -O3

# your source, shared by driver code.
#   if you want to use our staff-hc-sr04.o,
#   comment SRC out and uncomment STAFF_OBJS
SRC = fft.c

# define this if you need to give the device for your pi
TTYUSB = 

# set RUN = 1 if you want the code to automatically run after building.
RUN = 1

DEPS = ./Makefile
SUPPORT_OBJS := $(SRC:.c=.o)
include $(CS240LX_2022_PATH)/libpi/mk/Makefile.template
