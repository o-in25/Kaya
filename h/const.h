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
/* Maximum 32-bit signed
/* Hardware and software constants */
#define PAGESIZE 4096	/* page size in bytes */
#define WORDLEN	4		/* word size in bytes */
#define PTEMAGICNO 0x2A


#define ROMPAGESTART 0x20000000	 /* ROM Reserved Page */
#define RESERVED 0x0000000A


/* processor state areas */
/* SYSYCALLS */
#define SYSCALLNEWAREA 0x200003D4
#define SYSCALLOLDAREA 0X20000348
/* Program trap */
#define PRGMTRAPNEWAREA 0x200002BC
#define PRGMTRAPOLDAREA 0x200001A4
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

/* status register fields.
The status is a read/writable register that controls the 
processor mode of operation, the addres translation 
mode and the interrupt mask */
/************************* INTERRUPTS *************************/
/* use bitwise OR to turn on */
#define INTERRUPTSON 0x00000004 
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

/* interrupts pending */
/* an 8-bit field indicating on which interrupt lines interrupts are currently pending. 
If an interrupt is pending on interrupt line i, then Cause.IP[i] is set to 1. */
/* total lines */
#define LINECOUNT 8
#define DEVICECOUNT 8
#define LINEONE 0x00000000
/* equivalent to 0000 0010 */
#define LINETWO 0x00000002
/* equivalent to 0000 0100 */
#define LINETHREE 0x00000004
/* equivalent to 0000 1000 */
#define LINEFOUR 0x00000008
/* equivalent to 0001 0000 */
#define LINEFIVE 0x00000010
/* equivalent to 0010 0000 */
#define LINESIX 0x00000020
/* equivalent to 0100 0000 */
#define LINESEVEN 0x00000040
/* equivalent to 1000 0000 */
#define LINEEIGHT 0x00000080
/* start with the first device */
#define STARTDEVICE 0x00000001
/* Important Point: Many interrupt lines may be active at the same time. 
Furthermore, many devices on the same interrupt line may be requesting service. 
Cause.IP is always up to date, immediately responding 
to external (and internal) device events */

/* Set the timer constant - that is 5 miliseconds */
#define QUANTUM 5000

/* initial bit map address */
#define INTBITMAP 0x1000003C

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


/* vectors number and type */
#define VECTSNUM 4
#define TLBTRAP	0
#define PROGTRAP 1
#define SYSTRAP	2
#define TRAPTYPES 3

/* device interrupts */
#define NOSEM 3
#define DISKINT	3
#define TAPEINT 4
#define NETWINT 5
#define PRNTINT 6
#define TERMINT	7
#define DEVREGLEN 4	/* device register field length in bytes & regs per dev */
#define DEVREGSIZE 16 	/* device register size in bytes */

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

/* device common COMMAND codes */
#define RESET 0
#define ACK	1

/* operations */    
#define	MIN(A,B)	((A) < (B) ? A : B)
#define MAX(A,B)	((A) < (B) ? B : A)
#define	ALIGNED(A)	(((unsigned)A & 0x3) == 0)

/* Useful operations */
#define STCK(T) ((T) = ((* ((cpu_t *) TODLOADDR)) / (* ((cpu_t *) TIMESCALEADDR))))
#define LDIT(T)	((* ((cpu_t *) INTERVALTMR)) = (T) * (* ((cpu_t *) TIMESCALEADDR)))


#endif
