rm p1test.elf.*
rm mikabooq.o
arm-none-eabi-gcc -mcpu=arm7tdmi -c mikabooq.c -o mikabooq.o -I /usr/include/uarm
arm-none-eabi-gcc -mcpu=arm7tdmi -c p1test.c -o p1test.o -I /usr/include/uarm
arm-none-eabi-ld -T /usr/include/uarm/ldscripts/elf32ltsarm.h.uarmcore.x -o p1test.elf /usr/include/uarm/crtso.o /usr/include/uarm/libuarm.o p1test.o
elf2uarm -k p1test.elf
