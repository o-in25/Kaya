#ifndef CONSTS
#define CONSTS

/****************************************************************************
 *
 * This header file contains utility constants & macro definitions.
 *
 ****************************************************************************/

/* Maximum process blocks */
#define MAXPROC 20
#define MAXINT 2147483647
#define MAXSEMALLOC 49 
/* Maximum 32-bit signed */
/* Hardware and software constants */
#define PAGESIZE 4096	/* page size in bytes */
#define WORDLEN	4		/* word size in bytes */
#define PTEMAGICNO 0x2A
#define TIME 100000

#define ROMPAGESTART 0x20000000	 /* ROM Reserved Page */

/* miscellaneous */
#define RESERVED 0x00000028
#define FULLBYTE 0x000000FF

/* processor state areas */
/* SYSYCALLS */
#define SYSCALLNEWAREA 0x200003D4
#define SYSCALLOLDAREA 0X20000348
/* Program trap */
#define PRGMTRAPNEWAREA 0x200002BC
#define PRGMTRAPOLDAREA 0x20000230
/* Table management */
#define TBLMGMTNEWAREA 0x200001A4
#define TBLMGMTOLDAREA 0x20000118
/* Interrupt handling */
#define INTRUPTNEWAREA 0x2000008C
#define INTRUPTOLDAREA 0x20000000

/* timer, timescale, TOD-LO and other bus regs */
#define RAMBASEADDR	0x10000000
#define TODLOADDR	0x1000001C
#define INTERVALTMR	0x10000020
#define TIMESCALEADDR	0x10000024

/************************* INTERRUPTS *************************/
/* use bitwise OR to turn on */
#define INTERRUPTSON 0x00000004 
#define INTERRUPTSCON 0x00000001
/* use bitwise AND to turn off */
#define INTERUPTSOFF 0xFFFFFFFB
/************************* KERNEL/USER ************************/
/* use bitwise OR to turn on */
#define KERNELMODEON 0x00000004 
/* use bitwise AND to turn off */
#define KERNELMODEOFF 0xFFFFFFF7
/************************ VIRTUAL MEMORY **********************/
/* use bitwise OR to turn on */
#define VIRTUALMEMON 0x20000000 
/* use bitwise AND to turn off */
#define VIRTUALMEMOFF 0xFDFFFFFF
/**************************** ALL OFF *************************/
#define ALLOFF 0x00000000
/************************* INTERRUPTS *************************/

/* status codes */
 #define IEc 0x00000001
 #define KUc 0x00000002
 #define IEp 0x00000004
 #define KUp 0x00000008
 #define IEo 0x00000010
 #define KUo 0x00000020
 #define IM  0x0000FF00
 #define BEV 0x00400000
 #define VMc 0x01000000
 #define VMp 0x02000000
 #define VMo 0x04000000
 #define TE  0x08000000
 #define CU  0x10000000

/* devices */
#define FIRST 0x00000001
#define SECOND 0x00000002
#define THIRD 0x0000004
#define FOURTH 0x00000008
#define FIFTH 0x000000010
#define SIXTH 0x00000020
#define SEVENTH 0x00000040
#define EIGHTH 0x00000080
#define STARTDEVICE 0x00000001

/* device register area */
#define DEVREG 0x10000050
#define DEVREGAREA 0x000002D0
#define TRANSREADY 0x0000000F

/* Set the timer constant - that is 5 miliseconds */
#define INTERVAL 100000
#define QUANTUM 5000

/* initial bit map address */
#define INTBITMAP 0x1000003C
#define INTDEVREG 0x10000050

/* syscalls */
#define CREATEPROCESS 1
#define TERMINATEPROCESS 2
#define VERHOGEN 3
#define PASSEREN 4
#define SPECIFYEXCEPTIONSTATEVECTOR 5
#define GETCPUTIME 6
#define WAITFORCLOCK 7
#define WAITFORIODEVICE 8

/* utility constants */
#define	TRUE 1
#define	FALSE 0
#define ON 1
#define OFF 0
#define HIDDEN static
#define EOS	'\0'
#define NULL ((void *)0xFFFFFFFF)
#define SUCCESS 0
#define FAILURE -1

/* page tables and virtual memory */
#define MAXUPROC 8
#define SWAPSIZE 3 * MAXUPROC

/* segment */
#define SEGSTART 0x20000500
#define SEGWIDTH 0x0000000C
#define PADDRBASE 0x20000
#define KUSEG3ADDRBASE 0xC0000
#define DISKCOUNT 8

/* Page Table Bit Locations */
#define DIRTY 0x00000400
#define VALID 0x00000200
#define GLOBAL 0x00000100

/* masks */
#define ASIDMASK 6




/* bits 6-11 are the ASID in the CP0 register */
#define ENTRYHIASID 0x00000FC0
#define BASEADDR 0x8000
#define TEXTAREASEGMENTMASK BASEADDR + 0x00B0

#define OSAREA (PAGESIZE * KSEGOSPTESIZE)
#define KSEGOSARA ROMPAGESTART + OSAREA


/* verify that a page table address is found in the segment table */
#define MAGICNO 0x0000002A
#define PGTBLHEADERWORD 24

/* virtual memory & swap pool */
#define SWAPPOOLSIZE 
#define VPN 12
#define KUSEGPTESIZE 32
#define KSEGOSPTESIZE 64



/* vectors number and type */
#define VECTSNUM 4
#define TLBTRAP	0
#define PROGTRAP 1
#define SYSTRAP	2
#define TRAPTYPES 

/* device interrupts */
#define NOSEM 3
#define DISKINT	3
#define TAPEINT 4
#define NETWINT 5
#define PRNTINT 6
#define TERMINT	7
#define DEVREGLEN 4	/* device register field length in bytes & regs per dev */
#define DEVREGSIZE 16 	/* device register size in bytes */
#define DEVINTNUM 5
#define DEVPERINT 8

/* devices */
#define TAPEDEV (((TAPEINT - 3) * DEVREGSIZE * DEVPERINT) + INTDEVREG)
#define DISKDEV (((DISKINT - 3) * DEVREGSIZE * DEVPERINT) + INTDEVREG)
#define BUFFER (KSEGOSARA - (DISKCOUNT * PAGESIZE))

/* disk parameters */
#define DISKPARAMS 6
#define SECTOR 0
#define CYLINDER 1
#define HEAD 2
#define DISKNUM 3
#define PAGELOCATION 4
#define READWRITE 5

/* device register field number for non-terminal devices */
#define STATUS 0
#define COMMAND	1
#define DATA0 2
#define DATA1 3

/* device register field number for terminal devices */
#define RECVSTATUS 0
#define RECVCOMMAND 1
#define TRANSTATUS 2
#define TRANCOMMAND 3

/* device common STATUS codes */
#define UNINSTALLED	0
#define READY 1
#define BUSY 3
#define READ 3
#define WRITE 4

/* device common COMMAND codes */
#define RESET 0
#define ACK	1
#define EMPTY 0
#define EOT 0
#define EOF 1

/* syscalls */
#define CREATEPROCESS 1
#define TERMINATEPROCESS 2
#define VERHOGEN 3
#define PASSEREN 4
#define SPECTRAPVEC 5
#define GETCPUTIME 6
#define WAITCLOCK 7
#define WAITIO 8
#define READ_FROM_TERMINAL 9
#define WRITE_TO_TERMINAL 10
#define V_VIRTUAL_SEMAPHORE 11
#define P_VIRTUAL_SEMAPHORE 12
#define DELAY 13
#define DISK_PUT 14
#define DISK_GET 15
#define WRITE_TO_PRINTER 16
#define GET_TOD 17
#define GETTIME 17
#define TERMINATE 18

/* operations */    
#define	MIN(A,B)	((A) < (B) ? A : B)
#define MAX(A,B)	((A) < (B) ? B : A)
#define	ALIGNED(A)	(((unsigned)A & 0x3) == 0)

/* Useful operations */
#define STCK(T) ((T) = ((* ((cpu_t *) TODLOADDR)) / (* ((cpu_t *) TIMESCALEADDR))))
#define LDIT(T)	((* ((cpu_t *) INTERVALTMR)) = (T) * (* ((cpu_t *) TIMESCALEADDR)))


#endif
