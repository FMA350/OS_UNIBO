CC     	 = arm-none-eabi-gcc
LL		   = arm-none-eabi-ld

CFLAGS 	 = -mcpu=arm7tdmi -c
IFLAG 	 = -include 'stdint.h' -include 'uARMtypes.h' -I $(SRCPATH) -I /usr/include/uarm -I .
IBLDFLAG = -I $(BLDPATH)
LDFLAGS  = -T /usr/include/uarm/ldscripts/elf32ltsarm.h.uarmcore.x

EXEPATH  = elf-files/
SRCPATH  = src/
BLDPATH  = bleeding-edge/
CFILES   = p2test.c initial.c scheduler.c mikabooq.c interrupts.c
SOURCES  = $(addprefix $(SRCPATH), $(CFILES))
OBJECTS  = $(SOURCES:.c=.o) /usr/include/uarm/crtso.o /usr/include/uarm/libuarm.o

EXECUTABLE = $(EXEPATH)p2test.elf

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(LL) $(LDFLAGS) $(OBJECTS) -o $@
	elf2uarm -k $(EXECUTABLE)

.c.o:
		$(CC) $(CFLAGS) $(IFLAG) $< -o $@
# 
# bleeding: $(EXECUTABLE)
#
# $(EXECUTABLE): $(OBJECTS)
# 	$(LL) $(LDFLAGS) $(OBJECTS) -o $@
# 	elf2uarm -k $(EXECUTABLE)
# $(OBJECTS):
# 	$(CC) $(CFLAGS) $(IBLDFLAG) $(IFLAG) $< -o $@

clean:
	rm $(SRCPATH)*.o $(EXEPATH)*.elf
