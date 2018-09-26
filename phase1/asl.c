/*************************************************** asl.c **************************************************************
	asl.c implements a semaphore list - an important OS concept; here, the asl will be seen as an integer value and
	will keep addresses of Semaphore Descriptors, henceforth known as semd_t; much like in the pcb.c, the asl will keep an
	asl free list with MAXPROC free semd_t; this class will encapsulate the functionality needed too perform operations on
	the semd_t

	This module contributes function definitions and a few sample fucntion implementations to the contributors put forth by
	the Kaya OS project

***************************************************** asl.c ************************************************************/
#ifndef ASL
#define ASL

/* h files to include */
#include "../h/const.h"
#include "../h/types.h"
/* e files to include */
#include "../e/pcb.e"

/* globals */
/* pointer to the head of the active semd_t list - the asl */
HIDDEN semd_PTR semd_h;
/* pointer to the head free list of semd_t */
HIDDEN semd_PTR semdFree_h;


/************************************************************************************************************************/
/******************************************** HELPER FUNCTIONS  *********************************************************/
/************************************************************************************************************************/

/*
* Function: searches the semd_t asl list for
* the specified semd_t address passed in as an
* argument to the function; since there are two
* dummy semd_t on the list, there are no erronous
* exit conditions
*/
semd_PTR findSemd(int* semAdd) {
	/* retrieve the head of the list */
	semd_PTR currentSemd = semd_h;
	/* the findSemd task will now search the semd_t free
	list via the subsequent semd_t s_next field to
 	search for the next free address location; since the
	list uses a dummy node at both the front as 0 and at
	the end as MAXINT where MAXINT is the largest possible
	integer, the exit condition is always met, since the
	next semd_t must be < MAXINT */
	/* for convenience */
	semd_PTR nextSemd = currentSemd->s_next;
	/* while the semd_h address is less than the
	specified integer address */
	while(nextSemd->s_semAdd < semAdd) {
		/* if the loop hasnt jumped, assign to the next value
		in the linked list */
		currentSemd = currentSemd->s_next;
	}
	/* return the found smed_t */
	return currentSemd;
}


/************************************************************************************************************************/
/*************************************** ACTIVE SEMAPHORE LIST **********************************************************/
/************************************************************************************************************************/


int insertBlocked(int* semAdd, pcb_PTR p) {
	semd_PTR prev = findSemd(semAdd);
	if (*(prev->s_next->s_semAdd) != *(semAdd)) { /* semAdd not found */
		semd_t newSemd = allocSemd();
		if(newSemd == NULL) {
			return TRUE;
		}
		newSemd.s_procQ = mkEmptyProcQ();
		*(newSemd.s_semAdd) = *(semAdd);

		newSemd.s_prev = prev;
		newSemd.s_next = prev->s_next;
		prev->s_next->s_prev = &(newSemd);
		prev->s_next = &(newSemd);

		(*p).p_semAdd = semAdd;
		insertProcQ(newSemd.s_procQ, p);

		return FALSE;
	}
	/* semAdd found */
	(*p).p_semAdd = semAdd;
	insertProcQ(prev->s_next->s_procQ, p);

	return FALSE;
}

pcb_PTR removeBlocked(int* semAdd){
	/* find previous node */
	semd_PTR prev = searchASL(semAdd);
	/* did we find the right node? */
	if (*(prev->s_next->s_semAdd) == *(semAdd)) {
		pcb_PTR removed = removeProcQ(*(prev).s_procQ);

		if (emptyProcQ(prev->s_next->s_procQ)) {
			freeSemd(prev->s_next);
		}
		return removed;
	}
	/* semd not found */
	return NULL;
}

pcb_PTR outBlocked (pcb_PTR p){
	int* targetSemAdd = p->p_semAdd;
	semd_PTR prev = searchASL(targetSemAdd);

	if (*(prev->s_next->s_semAdd) == *(semAdd)) {
		pcb_PTR removed = outProcQ(*(prev).s_procQ, p);

		if (emptyProcQ(prev->s_next->s_procQ)) {
			freeSemd(prev->s_next);
		}
		return removed;
	}

	return outProcQ(&a.s_procQ, p);
}

pcb_PTR headBlocked (int *semAdd){
	semd_PTR prev = searchASL(semAdd);
	if (*(prev->s_next->s_semAdd) != *(semAdd)) {
		return NULL;
	}
	return headProcQ(prev->s_next->s_procQ);
}

void initASL() {
	static semd_t semdTable[MAXPROC + 2];	/* init semd free list */
	for (int i = 0; i < MAXPROC + 2; i++) {
		semdTable[i] = mkEmptySemd();
		freeSemd(&(semdTable[i]));
	}
	semd_t dummy1 = allocSemd();
	semd_t dummy2 = allocSemd();
	*(dummy1.s_semAdd) = 0;
	*(dummy1.s_semAdd) = MAXINT;
	dummy1.s_next = &(dummy2);
	dummy1.s_prev = NULL;
	dummy1.s_procQ = mkEmptyProcQ();
	dummy2.s_next = NULL;
	dummy2.s_prev = &(dummy1);
	dummy2.s_procQ = mkEmptyProcQ();

	semd_h = &(dummy1);
}

semd_t mkEmptySemd() {
	return NULL;  /* is this necessary? */
}



/* alloc semd method */
HIDDEN semd_PTR allocSemd() {
	if (semdFree_h == NULL){
		return NULL;
	}
	/* free list is not empty */
	semd_t firstNode = *(semdFree_h);
	semd_PTR secondNode = firstNode.s_next;

	firstNode.s_next = NULL;   /* washing dishes just before using them */
	firstNode.s_semAdd = NULL;
	firstNode.s_procQ = NULL;

	semdFree_h = secondNode;
	return &(firstNode);
}

/* return an asl node to the free list */
void freeSemd(semd_PTR s) {
	/* empty free list case */
	if (*(semdFree_h) == NULL) {
		(*semdFree_h) = s;
	}

	/* non-empty free list case */
	semd_t head = (*semdFree_h);
	semdFree_h->s_next = s;
	s->s_next = head;
}

/***************************************************************/

#endif
