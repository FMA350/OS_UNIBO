#! /bin/bash

arm-none-eabi-gcc -mcpu=arm7tdmi -c -o mikabooq.o mikabooq.c -I . -include 'stdint.h'
arm-none-eabi-gcc -mcpu=arm7tdmi -c -o p1test.o p1test.c -I . -I /usr/include/uarm -include 'stdint.h'

arm-none-eabi-ld \
   -T /usr/include/uarm/ldscripts/elf32ltsarm.h.uarmcore.x \
   -o p1test.elf /usr/include/uarm/crtso.o \
   /usr/include/uarm/libuarm.o p1test.o mikabooq.o

elf2uarm -k p1test.elf

rm *.o *.elf