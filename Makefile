cc     = arm-none-eabi-gcc
CFLAGS = -mcpu=arm7tdmi-I
IFLAG  = -I /usr/include/uarm
LDFLAGS=
EXECUTABLE=p2test.elf

SOURCES=initial.c interrupts.c p2test.c scheduler.c mikabooq.c
OBJECTS=$(SOURCES:.cpp=.o)

all:uArmos

uArmos: initial.o

clean:
	rm *.o
