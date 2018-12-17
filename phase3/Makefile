#$Id: Makefile,v 1.2 2004/05/01 14:53:48 morsiani Exp morsiani $
# Makefile for mipsel-linux

INCDIR = /usr/local/include/umps2/umps
SUPDIR = /usr/local/share/umps2
LIBDIR = /usr/local/lib/umps2

DEFS = ../h/const.h ../h/types.h ../e/pcb.e ../e/asl.e ../e/initial.e ../e/interrupts.e ../e/scheduler.e ../e/exceptions.e ../e/adl.e ../e/initProc.e ../e/vmIOsupport.e ../e/avsl.e $(INCDIR)/libumps.e Makefile

TDEFS = ./testers/print.e ./testers/h/tconst.h ../h/const.h ../h/types.h $(INCDIR)/libumps.e Makefile

CFLAGS = -ansi -pedantic -Wall -c
LDAOUTFLAGS = -T $(SUPDIR)/elf32ltsmip.h.umpsaout.x
LDCOREFLAGS =  -T $(SUPDIR)/elf32ltsmip.h.umpscore.x
CC = mipsel-linux-gcc 
LD = mipsel-linux-ld
AS = mipsel-linux-as -KPIC
EF = umps2-elf2umps
UDEV = umps2-mkdev

#main target
all: kernel.core.umps strConcatTape.umps drawTape.umps readTape.umps fibTape.umps swapTape.umps todTape.umps diskTape.umps pvATape.umps pvBTape.umps printerTape.umps disk0.umps disk1.umps

disk0.umps:
	$(UDEV) -d disk0.umps

disk1.umps:
	$(UDEV) -d disk1.umps

readTape.umps: read_t.aout.umps
	$(UDEV) -t readTape.umps read_t.aout.umps

fibTape.umps: fib_t.aout.umps
	$(UDEV) -t fibTape.umps fib_t.aout.umps

swapTape.umps: swap_t.aout.umps
	$(UDEV) -t swapTape.umps swap_t.aout.umps

todTape.umps: tod_t.aout.umps
	$(UDEV) -t todTape.umps tod_t.aout.umps

diskTape.umps: disk_t.aout.umps
	$(UDEV) -t diskTape.umps disk_t.aout.umps

pvATape.umps: pvA_t.aout.umps
	$(UDEV) -t pvATape.umps pvA_t.aout.umps

pvBTape.umps: pvB_t.aout.umps
	$(UDEV) -t pvBTape.umps pvB_t.aout.umps

printerTape.umps: printer_t.aout.umps
	$(UDEV) -t printerTape.umps printer_t.aout.umps

read_t.aout.umps: read_t
	$(EF) -a read_t

read_t: print.o readTest.o $(LIBDIR)/crti.o
	$(LD) $(LDAOUTFLAGS) $(LIBDIR)/crti.o print.o readTest.o $(LIBDIR)/libumps.o -o read_t

fib_t.aout.umps: fib_t
	$(EF) -a fib_t

fib_t: print.o fibTest.o $(LIBDIR)/crti.o
	$(LD) $(LDAOUTFLAGS) $(LIBDIR)/crti.o print.o fibTest.o $(LIBDIR)/libumps.o -o fib_t

swap_t.aout.umps: swap_t
	$(EF) -a swap_t

swap_t: print.o swapTest.o $(LIBDIR)/crti.o
	$(LD) $(LDAOUTFLAGS) $(LIBDIR)/crti.o print.o swapTest.o $(LIBDIR)/libumps.o -o swap_t

tod_t.aout.umps: tod_t
	$(EF) -a tod_t

tod_t: print.o todTest.o $(LIBDIR)/crti.o
	$(LD) $(LDAOUTFLAGS) $(LIBDIR)/crti.o print.o todTest.o $(LIBDIR)/libumps.o -o tod_t

disk_t.aout.umps: disk_t
	$(EF) -a disk_t

disk_t: print.o diskTest.o $(LIBDIR)/crti.o
	$(LD) $(LDAOUTFLAGS) $(LIBDIR)/crti.o print.o diskTest.o $(LIBDIR)/libumps.o -o disk_t

pvA_t.aout.umps: pvA_t
	$(EF) -a pvA_t

pvA_t: print.o pvTestA.o $(LIBDIR)/crti.o
	$(LD) $(LDAOUTFLAGS) $(LIBDIR)/crti.o print.o pvTestA.o $(LIBDIR)/libumps.o -o pvA_t

pvB_t.aout.umps: pvB_t
	$(EF) -a pvB_t

pvB_t: print.o pvTestB.o $(LIBDIR)/crti.o
	$(LD) $(LDAOUTFLAGS) $(LIBDIR)/crti.o print.o pvTestB.o $(LIBDIR)/libumps.o -o pvB_t

printer_t.aout.umps: printer_t
	$(EF) -a printer_t

printer_t: print.o printerTest.o $(LIBDIR)/crti.o
	$(LD) $(LDAOUTFLAGS) $(LIBDIR)/crti.o print.o printerTest.o $(LIBDIR)/libumps.o -o printer_t

readTest.o: ./testers/readTest.c $(TDEFS)
	$(CC) $(CFLAGS) ./testers/readTest.c

fibTest.o: ./testers/fibTest.c $(TDEFS)
	$(CC) $(CFLAGS) ./testers/fibTest.c

swapTest.o: ./testers/swapTest.c $(TDEFS)
	$(CC) $(CFLAGS) ./testers/swapTest.c

todTest.o: ./testers/todTest.c $(TDEFS)
	$(CC) $(CFLAGS) ./testers/todTest.c

diskTest.o: ./testers/diskTest.c $(TDEFS)
	$(CC) $(CFLAGS) ./testers/diskTest.c

pvTestA.o: ./testers/pvTestA.c $(TDEFS)
	$(CC) $(CFLAGS) ./testers/pvTestA.c

pvTestB.o: ./testers/pvTestB.c $(TDEFS)
	$(CC) $(CFLAGS) ./testers/pvTestB.c

printerTest.o: ./testers/printerTest.c $(TDEFS)
	$(CC) $(CFLAGS) ./testers/printerTest.c

print.o: ./testers/print.c $(TDEFS)
	$(CC) $(CFLAGS) ./testers/print.c


kernel.core.umps: kernel
	$(EF) -k kernel

kernel: initial.o interrupts.o scheduler.o exceptions.o asl.o pcb.o adl.o avsl.o vmIOsupport.o initProc.o
	$(LD) $(LDCOREFLAGS) $(LIBDIR)/crtso.o initial.o interrupts.o scheduler.o exceptions.o asl.o pcb.o adl.o avsl.o vmIOsupport.o initProc.o $(LIBDIR)/libumps.o -o kernel

initProc.o: initProc.c $(DEFS)
	$(CC) $(CFLAGS) initProc.c

vmIOsupport.o: vmIOsupport.c $(DEFS)
	$(CC) $(CFLAGS) vmIOsupport.c

avsl.o: avsl.c $(DEFS)
	$(CC) $(CFLAGS) avsl.c

adl.o: adl.c $(DEFS)
	$(CC) $(CFLAGS) adl.c

initial.o: ../phase2/initial.c $(DEFS)
	$(CC) $(CFLAGS) ../phase2/initial.c

interrupts.o: ../phase2/interrupts.c $(DEFS)
	$(CC) $(CFLAGS) ../phase2/interrupts.c
 
scheduler.o: ../phase2/scheduler.c $(DEFS)
	$(CC) $(CFLAGS) ../phase2/scheduler.c

exceptions.o: ../phase2/exceptions.c $(DEFS)
	$(CC) $(CFLAGS) ../phase2/exceptions.c
 
asl.o: ../phase1/asl.c $(DEFS)
	$(CC) $(CFLAGS) ../phase1/asl.c

pcb.o: ../phase1/pcb.c $(DEFS)
	$(CC) $(CFLAGS) ../phase1/pcb.c

# crti.o: crti.s
# 	$(AS) crti.s -o crti.o


clean:
	rm -f *.o *.umps kernel


distclean: clean
	-rm kernel.*.umps tape0.umps
