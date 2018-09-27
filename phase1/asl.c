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
/******************************************** DEBUG FUNCTIONS  *********************************************************/
/************************************************************************************************************************/
void debugA(int a) {
	int i;
	i = 0;
}

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

/*
* Function: allocates a new semd_t to be null;
* this is isomporhpic to mkEmptyProcQ in pcb.c;
* by allowing for a function to delegate the creation
* of semd_t, its implementation can be changed universally
*/
semd_PTR mkEmptySemd() {
	/* make a "new" semd_t */
	return NULL;
}

/*
* Function: evaluates if the semd_t is null or not;
* this is employed for the sake of good encapsulation as
* well as single responsobilities and reusability;
*/
int emptySemd(semd_PTR s) {
	return (s == NULL);
}

/*
* Function: makes the semd_t next field pointed
* to by the function argument to be null; this
* is helpful as to seperate allocation of responsobilities
*/
void mkFreeSemd(semd_PTR s) {
	/* set to null */
	s->s_next = NULL;
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
		/* since there is no semd_t free list make the
		supplied argument the head of list */
		semdFree_h = s;
		/* if the semd_t free list is empty,
		then it has no next semd_t once the
		new one is added; set this value */
		semdFree_h->s_next = NULL;
	} else {
		/* the semd_t free list is not empty, so simply
		just asign the semd_t argument's next field equal
		to the head of the semd_t free list */
		semdFree_h = s;
		/* asign then next semd_t */
		s->s_next = semdFree_h;
	}
}

/*
* Function: nulls out all of the values of a
* semd_t so that it is clean and can be ready to
* used - since a semd_t cannot come off the
* free list with defined values
*/
semd_PTR cleanSemd(semd_PTR s) {
	/* clean up */
	s->s_procQ = NULL;
	s->s_next = NULL;
	s->s_semAdd = NULL;
	return s;
}

/*
*	Function: allocates a semd_t from the semd_t
* free list and returns a pointer to it;
* should the send_t free list head is null,
* then there are no free semd_t to allocate
*/
semd_PTR allocSemd() {
	/* check if there are free semd_t on the
	free list by checking for null */
	if(semdFree_h == NULL) {
		return NULL;
	} else {
		/* asign the new semd_t from the head - since
		it is not null */
		semd_PTR openSemd = semdFree_h;
		if(semdFree_h->s_next == NULL) {
			/* the semd_h does not have a next - it
			is the last final one */
			semdFree_h = NULL;
		} else {
			semdFree_h = semdFree_h->s_next;
		}
		/* clean the semd so it can be fresh */
		openSemd->s_procQ = NULL;
		openSemd->s_next = NULL;
		openSemd->s_semAdd = mkEmptyProcQ();
		/* returned the new, cleaned semd_t */
		return openSemd;
	}
}

/*
*	Function: builds the bigger dummy node, that is a placeholder
* node for a semd_t; as noted in the initASL documentation,
* the use of a dummy node will allow for less error-prone
* boundry conditions
*/
semd_PTR mkLhsEdgeSemd() {
		/* allocate a new sem_d */
		semd_PTR edgeSemd = allocSemd();
		/* asign its address to be a radical
		boundry - i.e. p */
		edgeSemd->s_semAdd = 0;
		return edgeSemd;
}

/*
*	Function: builds the samller dummy node, that is a placeholder
* node for a semd_t; as noted in the initASL documentation,
* the use of a dummy node will allow for less error-prone
* boundry conditions
*/
semd_PTR mkRhsEdgeSemd() {
		/* allocate a new sem_d */
		semd_PTR edgeSemd = allocSemd();
		/* asign its address to be a radical
		boundry - i.e. MAXINT */
		edgeSemd->s_semAdd = 2147483646;
		return edgeSemd;
}


/************************************************************************************************************************/
/*************************************** ACTIVE SEMAPHORE LIST **********************************************************/
/************************************************************************************************************************/

/*
* Function: the first and perhaps most important
* stored procedure - the allocation of the
* active semaphore list asl of type semd_t;
* here, the semd_t free list is allocated to be
* of size MAXPROC, where MAXPROC = 20;
* IMPORTANT! this implementation of the
* semd_t free list uses 2 DUMMY nodes as to avoid
* error-prone exit conditions - which will be referenced
* further on in the documentation; thus, the semd_t
* free list will have MAXPROC + 2 dummy nodes to account
* for the space necessary to house enough semd_t on the
* free list
*/
void initASL() {
	/* first, initialize the semdTable - which is the array the
	elements of type semd_t are kept; notice the addition of the
	dummy nodes */
	static semd_t semdTable[(MAXPROC + 2)];
	/* for each semd_t in the semd_t free list,
	itialize the semd_t at i to be freed */
	int i;
	for (i = 0; i < MAXPROC; i++) {
		/* make each semd_t to be empty */
		freeSemd(&(semdTable[i]));
	}
	/* here, the semd_t edge (dummy) nodes to ensure
	that no address is greather than or less than the
	specified address values; this makes a call to the
	mkEdgeSemd function - which takes the address value as
	the argument; the minium will be 0 and the maxiumum
	will be the largest possible unsigned interger value -
	to ensure when travsering the semd_t asl, will never
	return null - indicating the edge of the list */

	/* two extra nodes placed as dummies on the semaphore list */
/* initialize the active array with 2 dummy nodes */
semd_h = &(semdTable[MAXPROC + 1]);
semd_h -> s_next = NULL;
/* last node in active list */
semd_h -> s_semAdd = (int*)MAXINT;
semd_h -> s_procQ = NULL;

(semdTable[MAXPROC]).s_next = semd_h;
semd_h = &(semdTable[MAXPROC]);
semd_h -> s_semAdd = 0; /* frist node in active list */
semd_h -> s_procQ = NULL;
}

/*
* Function: insert the pcb_t provided as an a
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
		p->p_semAdd = semAdd;
		/* insert the formatted pcb_t into the process
		queue; since our work for this was completed in
		pcb.c, simply utilize the work of this function
		to couple the modules; per the documentation on
		the findSemd function, the NEXT semd_h must be
		provided since that helper function does not
		encapsulate that functionality */
		insertProcQ(&(locSemd->s_next->s_procQ), p);
		/* since this operation is successful -i.e. the
		entry is NOT blocked, return false to indicate this */
		return FALSE;
	} else {
		/* there are free semd_t on the free list because
		the function did not return null - the sign of no remaining
		pcb_t, so add one - the open semd_t */
		semd_PTR openSemd = allocSemd();
		/* this is the harder of the two cases; here, the semd_t
		address does NOT match the address passed as an argument;
		two things must be considered; first, there is a possibility
		that the semd_t free list is empty - meaning this operation
		could not be completed - yielding false; should, this
		not be the case - as in, there IS a free and ready semd_t in
		the free list, allocate it and indicate the operation is successful
		with a false value */
		if(openSemd == NULL) {
			/* no more free semd_t on the free list - out work
			here is done, so mark the operation as an unsuccessful one */
			debugA(420);
			return TRUE;
		} else {
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
			insertProcQ(&(openSemd->s_procQ), p);
			/* give the pcb_t its corresponding addresse */
			/* give the new semd_t its new address */
			openSemd->s_semAdd = semAdd;
			/* give the pcb_t its corresponding addresse */
			p->p_semAdd = semAdd;
			/* the function was able to succesfully allocate a new
			semd_t and asign the proccess queue in the field of the
			pcb_t - signify this successful operation */
			return FALSE;
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
pcb_PTR removeBlocked(int* semAdd) {
	/* find the semd_t */
	semd_PTR locSemd = findSemd(semAdd);
	/* IMPORTANT! since, by the function definition of findSemd,
	the n-1th semd_t is returned and NOT the semd_t in search of,
	grab the nth semd_t; this is NOT to be confused as an index, but
	rather as the next address pointer; since there are dummy nodes */
	if(locSemd->s_next->s_semAdd != semAdd || locSemd->s_next == NULL) {
		/* per function implementation definiton, return null */
		return NULL;
	} else {
		/* the address has been found succesfully */
		pcb_PTR headPcb = removeProcQ(&(locSemd->s_next->s_procQ));
		/* now it is time to check if the pcb_t process queue is
		empty - which means that the head of the process queue
		was the only pcb_t on the queue; if the queue is not empty, then
		the pcb_t is simply returned - since the semd_t is still in use */
		if(emptyProcQ(locSemd->s_next->s_procQ)) {
			/* we have "issues" - the semd_t is now free; since we have
			finite i.e. MAXPROC available semd_t at any given time, it is time
			to free this one up so it can be used later; IMORTANT! the
			pointers must be rearanged to handle the n-th in progress
			semd_t on the free list; since the locSemd is the previous
			smed_t, asign its next to be the next, next semd_t. create
			a temporary semd_t to assist in this process */
			semd_PTR headSemd = locSemd->s_next;
			locSemd->s_next = headSemd->s_next;
			/* the semd_t is cleaned */
			headSemd->s_next = NULL;
			/* free it up */
			freeSemd(headSemd);
		}
		/* return the head */
		return headPcb;
	}
}

/*
* Function: remove the pcb_t passed in as the argument from
* the semd_t that contains the specified pcb; if the pcb_t
* does not appear in the process queue in the associated
* semd_t, return null
*/
pcb_PTR outBlocked(pcb_PTR p) {
		/* find the location of the semaphore */
		semd_PTR locSemd = findSemd(p->p_semAdd);
		/* seach for a winner; this has to sub-cases;
		if the semd_d does not exist, it cannot have a
		returning pcb; if the removed pcb_t was the head -
		that is the LAST pcb_t, then the semaphore associacted
		must be returned to the free list; again, s_next is called
		by the implementation definition of findSemd */
		if((locSemd->s_next->s_semAdd) == (p->p_semAdd)) {
			/* winner - we found our semd_t */
			pcb_PTR rmvdPcb = outProcQ(&(locSemd->s_next->s_procQ), p);
			/* now check if the newly removed pcb_t is causing the
			semd_t to be free - so it can be re allocated to the
			free list of semd_t */
			if(emptyProcQ(locSemd->s_next->s_procQ)) {
				/* the semaphore is now free - time to allocate it
				back - we like to share */
				semd_PTR openSemd = locSemd->s_next;
				/* rearange the order of the semd_t free list by shifting
				around the pointers that keep the list in order - a
				very important step */
				locSemd->s_next = locSemd->s_next->s_next;
				/* free up the unused semd_t */
				freeSemd(openSemd);
			}
			/* now that the semd_t is free on the list, the last importamt step is
			to disassociate that pcb_t with a semd_t. this is a simple manipulation of
			the struct fields */
			rmvdPcb->p_semAdd = NULL;
			return rmvdPcb;
			/* return this cleaned pcb_t */
		}

		/* error condition: there is no associated sempaphore desciptior
		with the given address */
		return NULL;
	}
/*
* Function: returns a pointer to the pcb_t
* that is at the HEAD of the pcb_t process queue
* with its associated semd_t address;
* if there is no associated semaphore descriptor or
* if the process queue associated with the
* semaphore address is empty - return null in both cases
*/
pcb_PTR headBlocked(int* semAdd){
	/* first, find the semaphore via the passed in
	semaphore address */
	semd_PTR locSemd = findSemd(semAdd);
	/* first case to consider; if find the semaphore desicrptor; if it there is no
	matching semaphore desciptior, return null; call the
	semd_t s_next, since findSemd returns the semd_t behind */
	if(locSemd->s_next->s_semAdd == semAdd) {
		/* semaphore address found; the last - theres a match;
		next, check if the the pcb_t process queue associated
		with the semd_t is null */
		pcb_PTR headPcb = headProcQ(locSemd->s_procQ);
		/* if the pcb_t is null return null */
		if(headPcb == NULL) {
			/* no head pcb_t */
			return NULL;
		} else {
			/* return the pointer to the head */
			return headPcb;
		}
	} else {
		/* no matching semaphore desciptior - return null */
		return NULL;
	}
}
