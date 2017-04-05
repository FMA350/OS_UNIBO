#! /bin/bash

arm-none-eabi-gcc -mcpu=arm7tdmi -c -o main.o main.c \
	-I . -I /usr/include/uarm \

arm-none-eabi-gcc -mcpu=arm7tdmi -c -o p2test.o p2test.c \
	-I . -I /usr/include/uarm \

arm-none-eabi-gcc -mcpu=arm7tdmi -c -o mikabooq.o mikabooq.c \
	-I . -I /usr/include/uarm \
	-include 'stdint.h' -include 'uARMtypes.h'

arm-none-eabi-ld \
   -T /usr/include/uarm/ldscripts/elf32ltsarm.h.uarmcore.x \
   -o p2test.elf /usr/include/uarm/crtso.o \
   /usr/include/uarm/libuarm.o main.o mikabooq.o p2test.o

elf2uarm -k p2test.elf

rm *.o *.elf