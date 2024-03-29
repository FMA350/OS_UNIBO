CC     	 = arm-none-eabi-gcc
LL		 = arm-none-eabi-ld

CFLAGS 	 = -mcpu=arm7tdmi -c
IFLAG 	 = -include 'stdint.h' -I /usr/include/uarm -I src -I src/clock -I src/handlers -I src/SSI -I src/SSI/services -I src/tests -I src/test -I src/uARM
#-include $(HDRFILES)
#-include 'uARMtypes.h'
IBLDFLAG = -I $(BLDPATH)
LDFLAGS  = -T /usr/include/uarm/ldscripts/elf32ltsarm.h.uarmcore.x

EXEPATH  = elf-files
SRCPATH  = src
OBJDIR   = obj

STRUCTURE := $(shell find $(SRCPATH) -type d)

SRCFILES   := $(addsuffix /* , $(STRUCTURE))
SRCFILES   := $(wildcard $(SRCFILES))

CFILES 	   := $(filter %.c,$(SRCFILES))
HDRFILES   := $(filter %.h,$(SRCFILES))
OBJFILES   := $(CFILES:%.c=%.o)
#OBJFILES   := $(subst $(SRCPATH),$(OBJDIR), $(CFILES:%.c=%.o))
 #OBJFILES    := $(filter-out $(OBJDIR)/init.o, $(OBJFILES))
OBJFILES := $(filter-out src/init.o, $(OBJFILES))
# /usr/include/uarm/crtso.o /usr/include/uarm/libuarm.o

EXECUTABLE = $(EXEPATH)/p2test.elf

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJFILES)
	$(LL) $(LDFLAGS) $(OBJFILES) -o $@
	elf2uarm -k $(EXECUTABLE)

%.o:%.c
	$(CC) $(CFLAGS) $(IFLAG) $< -o $@


# $(OBJFILES): $(HDRFILES)
# 	# echo $(HDRFILES)
# 	$(CC) $(CFLAGS) $< -o $@ $(IFLAG)

.PHONY : clean
clean:
	rm $(OBJFILES)
	rm $(EXEPATH)*/.elf
