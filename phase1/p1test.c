/*********************************P1TEST.C*******************************
 *
 *	Test program for the modules ASL and pcbQueues (phase 1).
 *
 *	Produces progress messages on terminal 0 in addition 
 *		to the array ``okbuf[]''
 *		Error messages will also appear on terminal 0 in 
 *		addition to the array ``errbuf[]''.
 *
 *		Aborts as soon as an error is detected.
 *
 *      Modified by Michael Goldweber on May 15, 2004
 */

#include "../h/const.h"
#include "../h/types.h"

#include "/usr/local/include/umps2/umps/libumps.e"
#include "../e/pcb.e"
#include "../e/asl.e"


#define MAXPROC	20
#define	MAXSEM	MAXPROC

char okbuf[2048];			/* sequence of progress messages */
char errbuf[128];			/* contains reason for failing */
char msgbuf[128];			/* nonrecoverable error message before shut down */
int sem[MAXSEM];
int onesem;
pcb_t	*procp[MAXPROC], *p, *qa, *q, *firstproc, *lastproc, *midproc;
char *mp = okbuf;


#define TRANSMITTED	5
#define ACK	1
#define PRINTCHR	2
#define CHAROFFSET	8
#define STATUSMASK	0xFF
#define	TERM0ADDR	0x10000250

typedef unsigned int devreg;

/* This function returns the terminal transmitter status value given its address */ 
devreg termstat(memaddr * stataddr) {
	return((*stataddr) & STATUSMASK);
}

/* This function prints a string on specified terminal and returns TRUE if 
 * print was successful, FALSE if not   */
unsigned int termprint(char * str, unsigned int term) {
	memaddr * statusp;
	memaddr * commandp;
	devreg stat;
	devreg cmd;
	unsigned int error = FALSE;
	
	if (term < DEVPERINT) {
		/* terminal is correct */
		/* compute device register field addresses */
		statusp = (devreg *) (TERM0ADDR + (term * DEVREGSIZE) + (TRANSTATUS * DEVREGLEN));
		commandp = (devreg *) (TERM0ADDR + (term * DEVREGSIZE) + (TRANCOMMAND * DEVREGLEN));
		
		/* test device status */
		stat = termstat(statusp);
		if (stat == READY || stat == TRANSMITTED) {
			/* device is available */
			
			/* print cycle */
			while (*str != EOS && !error) {
				cmd = (*str << CHAROFFSET) | PRINTCHR;
				*commandp = cmd;

				/* busy waiting */
				stat = termstat(statusp);
				while (stat == BUSY)
					 stat = termstat(statusp);
				
				/* end of wait */
				if (stat != TRANSMITTED)
					error = TRUE;
				else
					/* move to next char */
					str++;
			} 
		}
		else
			/* device is not available */
			error = TRUE;
	}
	else
		/* wrong terminal device number */
		error = TRUE;

	return (!error);		
}


/* This function placess the specified character string in okbuf and
*	causes the string to be written out to terminal0 */
void addokbuf(char *strp) {
	char *tstrp = strp;
	while ((*mp++ = *strp++) != '\0')
		;
	mp--;
	termprint(tstrp, 0);
}


/* This function placess the specified character string in errbuf and
*	causes the string to be written out to terminal0.  After this is done
*	the system shuts down with a panic message */
void adderrbuf(char *strp) {
	char *ep = errbuf;
	char *tstrp = strp;
	
	while ((*ep++ = *strp++) != '\0');
	
	termprint(tstrp, 0);
		
	PANIC();
}



void main() {
	int i;

	initPcbs();
	addokbuf("Initialized process control blocks   \n");

	/* Check allocProc */
	for (i = 0; i < MAXPROC; i++) {
		if ((procp[i] = allocPcb()) == NULL)
			adderrbuf("allocPcb: unexpected NULL   ");
	}
	if (allocPcb() != NULL) {
		adderrbuf("allocPcb: allocated more than MAXPROC entries   ");
	}
	addokbuf("allocPcb ok   \n");

	/* return the last 10 entries back to free list */
	for (i = 10; i < MAXPROC; i++)
		freePcb(procp[i]);
	addokbuf("freed 10 entries   \n");

	/* create a 10-element process queue */
	qa = NULL;
	if (!emptyProcQ(qa)) adderrbuf("emptyProcQ: unexpected FALSE   ");
	addokbuf("Inserting...   \n");
	for (i = 0; i < 10; i++) {
		if ((q = allocPcb()) == NULL)
			adderrbuf("allocPcb: unexpected NULL while insert   ");
		switch (i) {
		case 0:
			firstproc = q;
			break;
		case 5:
			midproc = q;
			break;
		case 9:
			lastproc = q;
			break;
		default:
			break;
		}
		insertProcQ(&qa, q);
	}
	addokbuf("inserted 10 elements   \n");

	if (emptyProcQ(qa)) adderrbuf("emptyProcQ: unexpected TRUE"   );

	/* Check outProc and headProc */
	if (headProcQ(qa) != firstproc)
		adderrbuf("headProcQ failed   ");
	q = outProcQ(&qa, firstproc);
	if (q == NULL || q != firstproc)
		adderrbuf("outProcQ failed on first entry   ");
	freePcb(q);
	q = outProcQ(&qa, midproc);
	if (q == NULL || q != midproc)
		adderrbuf("outProcQ failed on middle entry   ");
	freePcb(q);
	if (outProcQ(&qa, procp[0]) != NULL)
		adderrbuf("outProcQ failed on nonexistent entry   ");
	addokbuf("outProcQ ok   \n");

	/* Check if removeProc and insertProc remove in the correct order */
	addokbuf("Removing...   \n");
	for (i = 0; i < 8; i++) {
		if ((q = removeProcQ(&qa)) == NULL)
			adderrbuf("removeProcQ: unexpected NULL   ");
		freePcb(q);
	}
	if (q != lastproc)
		adderrbuf("removeProcQ: failed on last entry   ");
	if (removeProcQ(&qa) != NULL)
		adderrbuf("removeProcQ: removes too many entries   ");

        if (!emptyProcQ(qa))
                adderrbuf("emptyProcQ: unexpected FALSE   ");

	addokbuf("insertProcQ, removeProcQ and emptyProcQ ok   \n");
	addokbuf("process queues module ok      \n");

	addokbuf("checking process trees...\n");

	if (!emptyChild(procp[2]))
	  adderrbuf("emptyChild: unexpected FALSE   ");
	
	/* make procp[1] through procp[9] children of procp[0] */
	addokbuf("Inserting...   \n");
	for (i = 1; i < 10; i++) {
		insertChild(procp[0], procp[i]);
	}
	addokbuf("Inserted 9 children   \n");

	if (emptyChild(procp[0]))
	  adderrbuf("emptyChild: unexpected TRUE   ");

	/* Check outChild */
	q = outChild(procp[1]);
	if (q == NULL || q != procp[1])
		adderrbuf("outChild failed on first child   ");
	q = outChild(procp[4]);
	if (q == NULL || q != procp[4])
		adderrbuf("outChild failed on middle child   ");
	if (outChild(procp[0]) != NULL)
		adderrbuf("outChild failed on nonexistent child   ");
	addokbuf("outChild ok   \n");

	/* Check removeChild */
	addokbuf("Removing...   \n");
	for (i = 0; i < 7; i++) {
		if ((q = removeChild(procp[0])) == NULL)
			adderrbuf("removeChild: unexpected NULL   ");
	}
	if (removeChild(procp[0]) != NULL)
	  adderrbuf("removeChild: removes too many children   ");

	if (!emptyChild(procp[0]))
	    adderrbuf("emptyChild: unexpected FALSE   ");
	    
	addokbuf("insertChild, removeChild and emptyChild ok   \n");
	addokbuf("process tree module ok      \n");

	for (i = 0; i < 10; i++) 
		freePcb(procp[i]);


	

}

