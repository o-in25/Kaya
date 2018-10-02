#$Id: Makefile,v 1.2 2004/05/01 14:53:48 morsiani Exp morsiani $
# Makefile for mipsel-linux

INCDIR = /usr/local/include/umps2/umps
SUPDIR = /usr/local/share/umps2
LIBDIR = /usr/local/lib/umps2

DEFS = ../h/const.h ../h/types.h ../e/pcb.e ../e/asl.e ../e/initial.e ../e/interrupts.e ../e/scheduler.e ../e/exceptions.e $(INCDIR)/libumps.e Makefile

CFLAGS = -ansi -pedantic -Wall -c
LDAOUTFLAGS = -T $(SUPDIR)/elf32ltsmip.h.umpsaout.x
LDCOREFLAGS =  -T $(SUPDIR)/elf32ltsmip.h.umpscore.x
CC = mipsel-linux-gcc
LD = mipsel-linux-ld
AS = mipsel-linux-as -KPIC
EF = umps2-elf2umps

#main target
all: kernel.core.umps

kernel.core.umps: kernel
	$(EF) -k kernel

kernel: p2test.o initial.o interrupts.o scheduler.o exceptions.o asl.o pcb.o
	$(LD) $(LDCOREFLAGS) $(LIBDIR)/crtso.o p2test.o initial.o interrupts.o scheduler.o exceptions.o asl.o pcb.o $(LIBDIR)/libumps.o -o kernel

p2test.o: p2test.c $(DEFS)
	$(CC) $(CFLAGS) p2test.c

initial.o: initial.c $(DEFS)
	$(CC) $(CFLAGS) initial.c

interrupts.o: interrupts.c $(DEFS)
	$(CC) $(CFLAGS) interrupts.c

scheduler.o: scheduler.c $(DEFS)
	$(CC) $(CFLAGS) scheduler.c

exceptions.o: exceptions.c $(DEFS)
	$(CC) $(CFLAGS) exceptions.c

asl.o: ../phase1/asl.c $(DEFS)
	$(CC) $(CFLAGS) ../phase1/asl.c

pcb.o: ../phase1/pcb.c $(DEFS)
	$(CC) $(CFLAGS) ../phase1/pcb.c

# crti.o: crti.s
# 	$(AS) crti.s -o crti.o


clean:
	rm -f *.o term*.umps kernel


distclean: clean
	-rm kernel.*.umps tape0.umps
