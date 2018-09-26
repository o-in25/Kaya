#$Id: Makefile,v 1.2 2004/05/01 14:53:48 morsiani Exp morsiani $
# Makefile for mips-linux

INCDIR = /usr/local/include/umps2/umps
SUPDIR = /usr/local/share/umps2
LIBDIR = /usr/local/lib/umps2

DEFS = ../h/const.h ../h/types.h ../e/asl.e ../e/pcb.e $(INCDIR)/libumps.e Makefile

CFLAGS = -ansi -pedantic -Wall -c
LDAOUTFLAGS = -T $(SUPDIR)/elf32ltsmip.h.umpsaout.x
LDCOREFLAGS =  -T $(SUPDIR)/elf32ltsmip.h.umpscore.x
#LDAOUTFLAGS = -T $(SUPDIR)/elf32btsmip.h.umpsaout.x
#LDCOREFLAGS =  -T $(SUPDIR)/elf32btsmip.h.umpscore.x
CC = mipsel-linux-gcc 
LD = mipsel-linux-ld
AS = mipsel-linux-as -KPIC


#main target
all: kernel.core.umps 

kernel.core.umps: kernel
	umps2-elf2umps -k kernel

kernel: p1test.o asl.o pcb.o
	$(LD) $(LDCOREFLAGS) $(LIBDIR)/crtso.o p1test.o asl.o pcb.o $(LIBDIR)/libumps.o -o kernel

p1test.o: p1test.c $(DEFS)
	$(CC) $(CFLAGS) p1test.c
 
asl.o: asl.c $(DEFS)
	$(CC) $(CFLAGS) asl.c

pcb.o: pcb.c $(DEFS)
	$(CC) $(CFLAGS) pcb.c
 
# crti.o: crti.s
# 	$(AS) crti.s -o crti.o


clean:
	rm -f *.o term*.umps kernel


distclean: clean
	-rm kernel.*.umps
