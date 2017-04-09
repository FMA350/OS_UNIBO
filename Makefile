CC     = arm-none-eabi-gcc
LL		 = arm-none-eabi-ld

CFLAGS = -mcpu=arm7tdmi -c -I . -I /usr/include/uarm
IFLAG  = -I /usr/include/uarm -I . -include 'stdint.h' -include 'uARMtypes.h' -I $(SRCPATH)
LDFLAGS= -T /usr/include/uarm/ldscripts/elf32ltsarm.h.uarmcore.x

EXEPATH = elf-files/
SRCPATH = src/
CFILES  = p2test.c initial.c scheduler.c mikabooq.c interrupts.c
SOURCES = $(addprefix $(SRCPATH), $(CFILES))
OBJECTS = $(SOURCES:.c=.o) /usr/include/uarm/crtso.o /usr/include/uarm/libuarm.o

EXECUTABLE = $(EXEPATH)p2test.elf

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(LL) $(LDFLAGS) $(OBJECTS) -o $@
	elf2uarm -k $(EXECUTABLE)

.c.o:
	$(CC) $(CFLAGS) $(IFLAG) $< -o $@

clean:
	rm $(SRCPATH)*.o $(EXEPATH)*.elf
