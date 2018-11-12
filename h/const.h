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
#define RESERVED 0x0000000A
#define NOTRES 0xFF



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

/* status register fields.
The status is a read/writable register that controls the 
processor mode of operation, the addres translation 
mode and the interrupt mask */
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

		 /* status register fields...
 Status is a read/writable register that controls the usability of the coprocessors,
 the processor mode of operation (kernel vs. user), the address translation
 mode, and the interrupt masking bits.
 All bit fields in the Status register are read/writable */
 #define ALLOFF 0x00000000
 /*bit 0 -the “current” global interrupt enable bit. When 0, regardless
 of the settings in Status.IM all external interrupts are disabled. When 1,
 external interrupt acceptance is controlled by Status.IM. */
 #define IEc 0x00000001
 /* bit 1 - The “current” kernel-mode user-mode control bit. When Status.KUc=0
 the processor is in kernel-mode */
 #define KUc 0x00000002
 /* bits 2-3 - the “previous” settings of the Status.IEc
  and Status.KUc */
 #define IEp 0x00000004
 #define KUp 0x00000008
 /* bits 4-5 - the “previous” settings of the Status.IEp and Status.KUp
 - denoted the “old” bit settings. */
 #define IEo 0x00000010
 #define KUo 0x00000020
 /* NOTE: These six bits; IEc, KUc, IEp, KUp, IEo, and KUo act as a 3-slot deep
 KU/IE bit stack. Whenever an exception is raised the stack is pushed and
 whenever an interrupted execution stream is restarted, the stack is popped.
 See Section 3.2 for a more detailed explanation */
 /*********************************************************/
 /* bits 8-15 - The Interrupt Mask. An 8-bit mask that enables/disables
 external interrupts. When a device raises an interrupt on the i-th line, the
 processor accepts the interrupt only if the corresponding Status.IM[i] bit is
 on.*/
 #define IM  0x0000FF00
 /* bit 22 - The Bootstrap Exception Vector. This bit determines the
 starting address for the exception vectors. */
 #define BEV 0x00400000
 /* bit 24 - The “current” VM on/off flag bit. Status.VMc=0 indicates
 that virtual memory translation is currently off.*/
 #define VMc 0x01000000
 /* bit 25 - the “previous” setting of the Status.VMc bit*/
 #define VMp 0x02000000
 /* bit 26 - the “previous” setting of the Status.VMp bit - denoted the
 “old” bit setting */
 #define VMo 0x04000000
 /* NOTE: These three bits; VMc, VMp, and VMo act as a 3-slot deep VM bit stack.
 Whenever an exception is raised the stack is pushed and whenever an interrupted 
 execution stream is restarted, the stack is popped. See Section 3.2
 for a more detailed explanation.*/
 /*********************************************************/
 /* bit 27 - the processor Local Timer enable bit. A 1-bit mask that enables/disables
 the processor’s Local Timer. See Section 5.2.2 for more information
 about this timer */
 #define TE  0x08000000
 /* Bits 28-31 - a 4-bit field that controls coprocessor usability. The bits
 are numbered 0 to 3; Setting Status.CU[i] to 1 allows the use of the i-th
 co-processor. Since µMPS2 only implements CP0 only Status.CU[0] is
 writable; the other three bits are read-only and permanently set to 0.
 Trying to make use of a coprocessor (via an appropriate instruction) without
 the corresponding coprocessor control bit set to 1 will raise a Coprocessor
 Unusable exception. In particular untrusted processes can be prevented
 from CP0 access by setting Status.CU[0]=0. CP0 is always accessible/usable
 when in kernel mode (Status.KUc=0), regardless of the value
 of Status.CU[0]. */
 #define CU  0x10000000
 /* NOTE: Important Point: Since CP1 (the floating point co-processor) is not implemented,
 floating point instruction execution attempts generate a Coprocessor
 Unusable exception. */



/* interrupts pending */
/* an 8-bit field indicating on which interrupt lines interrupts are currently pending. 
If an interrupt is pending on interrupt line i, then Cause.IP[i] is set to 1. */
/* total lines */
#define FIRST 0x00000001
#define SECOND 0x00000002
#define THIRD 0x0000004
#define FOURTH 0x00000008
#define FIFTH 0x000000010
#define SIXTH 0x00000020
#define SEVENTH 0x00000040
#define EIGHTH 0x00000080
/* start with the first device */
#define STARTDEVICE 0x00000001
/* Important Point: Many interrupt lines may be active at the same time. 
Furthermore, many devices on the same interrupt line may be requesting service. 
Cause.IP is always up to date, immediately responding 
to external (and internal) device events */

/* device register */
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
