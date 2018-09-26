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

void emptySemd(semd_PTR s) {
	return (s == NULL);
}

/*
* Function: makes the semd_t next field pointed
* to by the function argument to be null; this
* is helpful as to seperate allocation of responsobilities
*/
void mkFreeSemd(semd_PTR s) {
	/* set to null */
	semdFree->s_next = NULL;
}

/*
* Function: takes a semd_t and points it onto
* the semd_t free list; if there is nothing on
* the semd_t free list, a free list is "created"
* by making the newly added semd_t next semd_t
* to be null; if its not empty
*/
void freeSemd(semd_PTR s) {
	/* call the encapsulated emptySemd function
	to test for the case that the semd_t free list
	is null */
	if(emptySemd(semdFree_h)) {
		mkFreeSemd(s);
	} else {
		/* non-empty free list case */
		s->s_next = semdFree_h;
	}
	semdFree_h = s;
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
			pcb_t, so add one - the open semd_t */
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

/*
* Function: search the asl semd_t list for the specified
* semd_t address; in the case that it is not found, simply
* exit and return null; in the case that it is found, remove
* the HEAD pcb_t from that process queue of the found semd_t
* descriptor and return its pointer; this, too, like insertBlocked
* can be tricky in its own right: if this process queue then becomes
* null, meaning that emptyProcQ is null, then this semd_t must be
* removed and sent to the semd_t free list
*/
pcb_PTR removeBlocked(int* semAdd){
	/* find the semd_t */
	semd_PTR locSemd = findSemd(semAdd);
	/* IMPORTANT! since, by the function definition of findSemd,
	the n-1th semd_t is returned and NOT the semd_t in search of,
	grab the nth semd_t; this is NOT to be confused as an index, but
	rather as the next address pointer; since there are dummy nodes */
	if(locSemd->s_next->semAdd != semAdd) {
		/* per function implementation definiton, return null */
		return NULL;
	} else {
		/* the address has been found succesfully */
		pcb_PTR headPcb = removeProcQ(&(semAdd->s_procQ));
		/* now it is time to check if the pcb_t process queue is
		empty - which means that the head of the process queue
		was the only pcb_t on the queue; if the queue is not empty, then
		the pcb_t is simply returned - since the semd_t is still in use */
		if(!(emptyProcQ(headPcb)) {
			/* we are all set - return the pcb_t since the semd_t is
			still in use */
			return headPcb;
		} else {
			/* we have "issues" - the semd_t is now free; since we have
			finite i.e. MAXPROC available semd_t at any given time, it is time
			to free this one up so it can be used later; IMORTANT! the
			pointers must be rearanged to handle the n-th in progress
			semd_t on the free list; since the locSemd is the previous
			smed_t, asign its next to be the next, next semd_t */
			locSemd->s_next = locSemd->s_next->s_next;
			/* the semd_t is cleaned */
			locSemd->s_next->s_next = NULL;
			/* free it up */
			freeSemd(locSemd->s_next);
		}

	}


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


/***************************************************************/

#endif
