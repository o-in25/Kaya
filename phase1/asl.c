/*************************************************** asl.c **************************************************************
	asl.c implements a semaphore list - an important OS concept; here, the asl will be seen as an integer value and
	will keep addresses of Semaphore Descriptors, henceforth known as semd_t; much like in the pcb.c, the asl will keep an
	asl free list with MAXPROC free semd_t; this class will encapsulate the functionality needed too perform operations on
	the semd_t

	This module contributes function definitions and a few sample fucntion implementations to the contributors put forth by
	the Kaya OS project

***************************************************** asl.c ************************************************************/



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
	/* IMPORTANT! the first semd_t must be skipped since
	the first semd_t will be a dummy node */
	currentSemd = currentSemd->s_next;
	/* while the semd_h address is less than the
	specified integer address */
	while(currentSemd->s_next->s_semAdd < semAdd) {
		/* the findSemd task will now search the semd_t free
		list via the subsequent semd_t s_next field to
		search for the next free address location; since the
		list uses a dummy node at both the front as 0 and at
		the end as MAXINT where MAXINT is the largest possible
		integer, the exit condition is always met, since the
		next semd_t must be < MAXINT */
		currentSemd = currentSemd->s_next;
		/* the loop hasnt jumped, assign to the next value
		in the linked list - as done above */
	}
	/* IMPORTANT! return the found smed_t; since this function
	returns the n-1th semd_t, the wrapper function that
	calls this MUST call s_next on the returning semd_t,
	otherwise the current semd_t will be returned to
	provide further reusability and encapsulation */
	return currentSemd;
}


/************************************************************************************************************************/
/*************************************** ACTIVE SEMAPHORE LIST **********************************************************/
/************************************************************************************************************************/

/*
*	Function: insert the pcb_t provided as an a
* argument to the tail of that pcb_t process
* queue at the semd_t address provided; this method
* can get tricky: if there is no semd_t descriptor,
* as in, there is it is not active because it is
* nonexistent in the asl list a new semd_t must initalized,
* an be allocated to take its place - however, if the
* free list is blocked - return true; in a successful operation
* the function returns null
*/
int insertBlocked(int* semAdd, pcb_PTR p) {
	/* determine in the prospcetive insert is blocked */
	/* find the location of the closest semd_t */
	semd_PTR locSemd = findSemd(semAdd);
	/* with the retrieved location, find if it matches
	the desciption - if it does not, this must be taken
	care of later in the function */
	if(locSemd->s_next->s_semAdd == semAdd) {
		/* the located semd_t matches the semaphore
		address - the easier case */
		/* asign the s_semAdd - per the function
		implementation definition */
		p->s_semAdd = semAdd;
		/* insert the formatted pcb_t into the process
		queue; since our work for this was completed in
		pcb.c, simply utilize the work of this function
		to couple the modules; per the documentation on
		the findSemd function, the NEXT semd_h must be
		provided since that helper function does not
		encapsulate that functionality */
		insertProcQ(locSemd->s_next->s_procQ, p);
		/* since this operation is successful -i.e. the
		entry is NOT blocked, return false to indicate this */
		return FALSE;
	} else {
		/* this is the harder of the two cases; here, the semd_t
		address does NOT match the address passed as an argument;
		two things must be considered; first, there is a possibility
		that the semd_t free list is empty - meaning this operation
		could not be completed - yielding false; should, this
		not be the case - as in, there IS a free and ready semd_t in
		the free list, allocate it and indicate the operation is successful
		with a false value */
		if(allocSemd() != NULL) {
			/* there are free semd_t on the free list because
			the function did not return null - the sign of no remaining
			pcb_t, so add one */
			semd_PTR openSemd = allocSemd();
			/* give the new semd_t its new address */
			openSemd->s_semAdd = semAdd;
			/* give the pcb_t its corresponding addresse */
			p->p_semAdd = openSemd->s_semAdd;
			/* arrange the new semd_t so that is in the appropriate place in
			the semd_t free list */
			openSemd->s_next = locSemd->s_next;
			locSemd->s_next = openSemd;
			/* pointers rearranged;
			asign its necessary fields to function */
			openSemd->s_semAdd = semAdd;
			openSemd->s_procQ = mkEmptyProcQ();
			/* everything is all set - insert the newly added
			pcb_t process queue into its corresponding process queue -
			but with an address since insertProcQ takes a pointer
			as an argument */
			insertProcQ(&(locSemd->s_procQ), p)
			/* the function was able to succesfully allocate a new
			semd_t and asign the proccess queue in the field of the
			pcb_t - signify this successful operation */
			return FALSE;
		} else {
			/* no more free semd_t on the free list - out work
			here is done, so mark the operation as an unsuccessful one */
			return TRUE;
		}
	}
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
