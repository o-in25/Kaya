/********************************** pcb.c **********************************/

/* h files to include */
#include "../h/const.h"
#include "../h/types.h"
#include "/usr/include/uarm/libuarm.h"
/* e files to include */
#include "../e/pcb.e"
#include "../e/asl.e"

/* globals */
HIDDEN pcb_PTR pcbFree_h;


state_PTR mkEmptyState() {
	return NULL;
}


void freePcb(pcb_PTR p) {

	insertProcQ(&pcbFree_h, p);
}

/*
* Function: nulls out all of the fields for
* a provided pcb_t; if a null pcb_t is provided,
* the function will return null
*/
pcb_PTR cleanPcb(pcb_PTR p) {
	/* if the pcb_t is null, then
	there are no values we can clean, so
	we return null */
	if(p == NULL) {
		/* return null - there is no non-null pcb
		provided */
		return 	NULL;
	} else {
		/* if the pcb_t is not null, then its
		fields are cleaned and it is returned
		with null fields */
		/* clean its previous and next fields */
		p->p_next = NULL;
		p->p_prev = NULL;
		/* clean its relationships */
		p->p_prnt = NULL;
		p->p_child = NULL;
		p->p_nextSib = NULL;
		p->p_prevSib = NULL;
		/* clean its state */
		p->p_s = (*(mkEmptyState()));
		/* clean its semaphore */
		p->p_semAdd = NULL;
		/* returned the cleaned node */
		return p;
	}
}

/*
* Function: allocate a pcb_t from the free
* free list; remove the pcb_t so that the
* free list has n-1 pcb_t on it; if the
* free list has no remaining free pcb_t
* that is, the free list is empty,
* simply return null to indicate that there
* are no pcb_t remaining; otherwise, make the
* free list n-1 pcb_t by calling removeProcQ()
* given the free list as a pointer. Before the
* pcb_t is returned, it is cleaned so that
* can be appropriately used and a pointer to the
* returned pcb_t is provided; additionally, the
* pcb_t will be cleaned before it is returned
*/
pcb_PTR allocPcb() {
	/* since removeProcQ is a generic function,
	simply supply the address of the free list to
	return the nth-1 element from said list */
	pcb_PTR rmvdPcb = removeProcQ(&pcbFree_h);
	/* now that the removed pcb is returned (or really, its
	pointer is) it must be cleaned before it can be used */
	rmvdPcb = cleanPcb(rmvdPcb);
	/* now that its cleaned, it can be used */
	return rmvdPcb;
}

/*
* Function: initializes the pcb_t
* free list for pcb_t to be allocated to
* by some predefined constant; this function
* is an init call
*/
void initPcbs() {
	/* statically declare the list of pcbs_t */
	static pcb_t pcbTable[MAXPROC];
	int i;
	for (i = 0; i < MAXPROC; i++) {
		/* make each element in the array - that is,
		the free list - equal to null */
		pcbTable[i] = mkEmptyProcQ();
		/* insert the element into the freepcb function; since it takes
		a pointer, simply supply the address */
		freePcb(&(pcbTable[i]));
	}
}


/*
* Function: initialize the tp of an
* empty process queue - i.e. return null
*/
pcb_PTR mkEmptyProcQ() {
	/* here, proper encapsulation is used to
	initialize an empty process via a function
	rather than by asigning the address of null */
	return NULL;
}

/*
* Function: returns a boolean expression
* if a tp is null - that is, a tp points
* to an empty process queue
*/
int emptyProcQ(pcb_PTR tp) {
	/* evaluate */
	return (tp == NULL);
}

/*
* Function: insert the pcb_t p into the
* process queue tp
*/
void insertProcQ(pcb_PTR *tp, pcb_PTR p) {
	pcb_PTR tailPcb = (*tp);
	/* in order to insert a given pcb_t into a
	given process queue given by tp,
	the queue must be verified for emptiness;
	if it is not empty, this becomes the facile
	task of rearanging the pointers */
	if (emptyProcQ(tailPcb) {
		/* the queue is empty, so assign this
		pcb_t to be circular by making itself
		its previous and next element */
		p->p_next = p;
		p->p_prev = p;
	} else {
		/* since the list is not empty, simply
		reasign the pointers to account for the newly
		added element */
		p->p_next = tailPcb->p_next;
		/* the newest element has the tail as its previous */
		p->p_prev = tailPcb;
		tailPcb->p_next = p;
		tailPcb->p_next->p_prev = p;
	}
	/* reasign the tp */
	tailPcb = p;
}


/*
* Function: removes the first element from the
* processes queue whose tp is passed in as an
* argument; return null if the tp is null - meaning
* there is no list
*/
pcb_PTR removeProcQ(pcb_PTR *tp) {
	/* first, consider the case in which the process queue is
	empty, then simply use the encapsulated functionality
	of the outProcQ function */
	if(emptyProcQ(*tp)) {
		/* empty list */
		return NULL;
	} else {
		/* since this functionality is already
		written, use the encapsulated function */
		pcb_PTR tailPcb = (*tp);
		return outProcQ(tp, tailPcb);
	}
}

/*
* Function: remove the pcb_t pointed to by p
* from the process queue pointed to by tp;
* update the process queue's tp if necessary;
* if the desired entry is not in the indicated queue,
* return null; else, return p
*/
pcb_PTR outProcQ(pcb_PTR* tp, pcb_PTR p) {
	/* dereference ahead of time */
	pcb_PTR tailPcb = (*tp);
	/* removing the pcb_t from the process
	pointed to by tp has three cases to consider;
	first, if the tp is null, meaning there is no list for
	a pcb_t to be removed from; second, the pcb_t to be removed
	is the tp, in which case a if the tp is the only remaining
	node on the list, the tp must be set the null - otherwise its
	pointers must be rearranged. the last remaining case if
	tp is an arbitrary element in the list */
	if(emptyProcQ(tailPcb)) {
		/* no list */
		return NULL;
	} else {
		/* a list of >= 1 */
		/* what is being removed is the tp */
		if(p == tailPcb) {
			/* a list of 1 */
			if(tailPcb.p_next == tailPcb) {
				tailPcb = NULL;
			} else {
				/* a list of > 1 */
				/* reasign the tail pointer */
				tailPcb = tailPcb.p_prev;
				/* swap the pointers with the soon to be removed pcb_t */
				tailPcb.p_next->p_prev = tailPcb.p_next;
				tailPcb.p_prev->p_next = tailPcb.p_prev;
			}
			/* return the block */
			return p;
		} else {
			/* what is being removed is not the tp */
			pcb_PTR currentPcb = tailPcb->p_next;
			/* start from the start of the queue */
			while(currentPcb != tailPcb) {
				if(currentPcb == p) {
					/* set the next and prev to be null */
					currentPcb->p_next = NULL;
					currentPcb->p_prev = NULL;
					/* reasign the pointers */
					currentPcb->p_prev->p_next = currentPcb->p_next
					currentPcb->p_next->p_prev = currentPcb->p_prev
				} else {
					/* try again, moving up the list */
					currentPcb = currentPcb->p_next;
				}
			}
			/* the pcb_t was not found */
			return NULL;
		}
	}
}


pcb_PTR headProcQ(pcb_PTR tp) {
	if (emptyProcQ(tp)) return NULL;
	return (tp->p_next);
}

int emptyChild(pcb_PTR p) {
	return (p->p_child == NULL);
}

void insertChild(pcb_PTR prnt, pcb_PTR p) {
	insertProcQ(&prnt->p_child, p);
}

pcb_PTR removeChild(pcb_PTR p) {
	return removeProcQ(&p->p_child);
}

pcb_PTR outChild(pcb_PTR p) { /* do you need to search each process block to find the one that has p as a child? */
	pcb_PTR *prnt = &(p->p_prnt);
	return outProcQ(prnt, p);
}
