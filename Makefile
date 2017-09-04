CC     	 = arm-none-eabi-gcc
LL		   = arm-none-eabi-ld

CFLAGS 	 = -mcpu=arm7tdmi -c
IFLAG 	 = -include 'stdint.h' -include 'uARMtypes.h' -I $(SRCPATH) -I /usr/include/uarm -I .
IBLDFLAG = -I $(BLDPATH)
LDFLAGS  = -T /usr/include/uarm/ldscripts/elf32ltsarm.h.uarmcore.x

EXEPATH  = elf-files/
SRCPATH  = src/
BLDPATH  = bleeding-edge/
CFILES   = init.c p2test.c scheduler.c mikabooq.c interrupts.c ssi.c syscall.c trap.c syslib.c accounting.c \
			create_process.c create_thread.c do_io.c getcputime.c get_errno.c get_id.c setmgr.c \
			terminate_process.c terminate_thread.c wait_for_clock.c debug_tests.c
SOURCES  = $(addprefix $(SRCPATH), $(CFILES))
OBJECTS  = $(SOURCES:.c=.o) /usr/include/uarm/crtso.o /usr/include/uarm/libuarm.o

EXECUTABLE = $(EXEPATH)p2test.elf

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(LL) $(LDFLAGS) $(OBJECTS) -o $@
	elf2uarm -k $(EXECUTABLE)

.c.o:
		$(CC) $(CFLAGS) $(IFLAG) $< -o $@

clean:
	rm $(SRCPATH)*.o $(EXEPATH)*.elf
