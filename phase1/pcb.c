/*************************************************** pcb.c **************************************************************
	pcb.c encpsulates the functionality of Process Control Blocks, henceforth known as pcb_t; the pcb.c module is
	responsible for three major pcb_t functions: first, a free list of MAXPROC pcb_t are allocated - where
	MAXPROC = 20; then, pcb_t themsleves are to keep a child-parent-sibling relationships, where the siblings of
	a pcb_t are kepted in a doulbey liked list that is null terminated; third, it is responsible
	for keeping process queues of pcb_t to be allocated fromon and off the free list.

	This module contributes function definitions and a few sample fucntion implementations  to the contributors put forth by
	the Kaya OS project

***************************************************** pcb.c ************************************************************/



/* h files to include */
#include "../h/const.h"
#include "../h/types.h"
#include "/usr/include/uarm/libuarm.h"
/* e files to include */
#include "../e/pcb.e"
#include "../e/asl.e"

/* globals */
/* the pcb_t free list of size 20 */
HIDDEN pcb_PTR pcbFree_h;


/************************************************************************************************************************/
/******************************************** HELPER FUNCTIONS  *********************************************************/
/************************************************************************************************************************/


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
		/* clean its semaphore */
		p->p_semAdd = NULL;
		/* returned the cleaned node */
		return p;
	}
}

/*
* Function: makes the child
* of a specified parent null
* for p_next and p_prev; if the parent
* is null, that is there is not parent,
* then null is returned;
*/
pcb_PTR cleanChild(pcb_PTR prnt) {
	/* if the parent does not have
	* a child, return null */
	if(emptyChild(prnt)) {
		prnt->p_child = NULL;
	} else {
		/* clean the child */
		prnt->p_child->p_prevSib = NULL;
		prnt->p_child->p_nextSib = NULL;
	}
}

/*
* Function: makes the child pcb_t
* of a parent pcb_t to be the head
* or initial child; this simply sets
* the values and does not return
* anything
*/
void MkHeadChild(pcb_PTR prnt, pcb_PTR p) {
	/* if there is no child, then the child
	waill have no parents */
		if(emptyChild(prnt)) {
			/* make null */
			prnt->p_child = NULL;
		} else {
			/* the pcb_t child is the newly
			added p */
			prnt->p_child = p;
			/* the newly added pcb_t parent
			is the pcb_t parent passed as an
			argument */
			p->p_prnt = prnt;
		}
}


/************************************************************************************************************************/
/********************************** PCB_T ALLOCATION AND DEALLOCATION  **************************************************/
/************************************************************************************************************************/


/*
* Function: pcb_t that are no longer in use
* are returned to the free list here; this simply
* requires the uses of the already written
* insertProcQ, but must be cleaned before they
* are inserted onto the list pointed to by
* insertProcQ - that is, the free list.
*/
void freePcb(pcb_PTR p) {
	/* since it will prove detremental to process
	management if a pcb_t has predefined values on it
	before going to the free list, it must be cleaned first */
	pcb_PTR temp = cleanPcb(p);
	/* now its cleaned */
	p = temp;
	/* insert into a specified process queue - that is,
	the free list; use the predefined encapsulated function
	to achieve this */
	insertProcQ(&pcbFree_h, p);
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
	pcb_PTR temp = removeProcQ(&(pcbFree_h));
	/* now that the removed pcb is returned (or really, its
	pointer is) it must be cleaned before it can be used */
	pcb_PTR rmvdPcb = cleanPcb(temp);
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
	/* delcare the index */
	int i;
	/* make the head element in the array - that is,
	the head of the free list - equal to null; by doing so, since
	this function will ostensibily call insertProcQ, then by
	having the head equal to null, the pcb_t pointed to by
	p will have itself asigned to its previous and next - allowing
	for the n+1th element to then be allocated to the nth previous
	and next - and so on */
	pcbFree_h = mkEmptyProcQ();
	for (i = 0; i < MAXPROC; i++) {
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


/************************************************************************************************************************/
/**************************************** PROCESS TREE MAINTENANCE  *****************************************************/
/************************************************************************************************************************/

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
	if (emptyProcQ(tailPcb)) {
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
		/* dereference for convenience */
		return outProcQ(tp, tailPcb->p_next);
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
			if(tailPcb->p_next == tailPcb) {
				tailPcb = NULL;
			} else {
				/* a list of > 1 */
				/* reasign the tail pointer */
				tailPcb = tailPcb->p_prev;
				/* swap the pointers with the soon to be removed pcb_t */
				tailPcb->p_next->p_prev = tailPcb->p_next;
				tailPcb->p_prev->p_next = tailPcb->p_prev;
			}
			/* return the block */
			return p;
		} else {
			/* what is being removed is not the tp */
			pcb_PTR currentPcb = tailPcb->p_next;
			/* start from the start of the queue */
			while(currentPcb != tailPcb) {
				/* the p matches the tp */
				if(currentPcb == p) {
					/* set the next and prev to be null */
					currentPcb->p_next = NULL;
					currentPcb->p_prev = NULL;
					/* reasign the pointers */
					currentPcb->p_prev->p_next = currentPcb->p_next;
					currentPcb->p_next->p_prev = currentPcb->p_prev;
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


/*
* Function: returns a pointer to the head
* of a process queue signified by tp - however
* this head should not be removed; if there is no
* head of the process queue - that is, there is
* no process queue, return null
*/
pcb_PTR headProcQ(pcb_PTR tp) {
	/* if the tp points tp null,
	that is, there is no process queue,
	return null */
	if (emptyProcQ(tp)) {
		return NULL;
	}
	/* since the tp will point to head by the
	p_next field, return that */
	return (tp->p_next);
}


/************************************************************************************************************************/
/**************************************** PROCESS TREE MAINTENANCE  *****************************************************/
/************************************************************************************************************************/

/*
* Function: takes a pcb_t as an
* argument and returns a boolean expression
* as to whether or not a particular
* pcb_t has a child or not
*/
int emptyChild(pcb_PTR p) {
	/* if the pcb_t has a child, return true,
	and if otherwise - false - as captured by
	the boolean expression */
	return (p->p_child == NULL);
}

/*
* Function: takes a parent pcb_t and
* provides a pcb_t to be linked to that
* parent
*/
void insertChild(pcb_PTR prnt, pcb_PTR p) {
	/* here, there are 2 cases to consider;
	first - if the inserted pcb_t is the first
	and only child of the parent pcb_t; in this
	case, the newly aded pcb_t previous sibling
	must be set to null - since it is the
	only child; otherwise, if the newly aded
	pcb_t is one of >0 childs, then the new child
	must be set as the parent pcb_t */
	if(emptyChild(prnt)) {
		/* since there is no child, clean it */
		cleanChild(prnt);
	} else {
		/* asign the parent pcb_t next
		child's next sibling to the back
		of the child list */
		prnt->p_child->p_nextSib = p;
		p->p_prevSib = prnt->p_child;
		/* the end of the list */
		p->p_nextSib = NULL;
	}
	/* reasign the pcb_t head child by
	calling the encapsulated MkHeadChild()
	function for easibility */
	MkHeadChild(prnt, p);
}

/*
* Function: takes a parent pcb_t and
* removes and returns the first pcb_t child -
* baring there is one; if the parent
* pcb_t, however, is null, then this
* function must return null to hanle
* that case
*/
pcb_PTR removeChild(pcb_PTR p) {
	/* in removing a child, three cases exist
	that most be considered; first, that the parent
	pcb_t has no child; in this case, simply
	return null per the function implementation
	definition; next, if the child being removed
	is the last child, in which case the parent
	pcb_t must indicate this; third, the pcbs_t
	to be removed has >0 siblings, in which case
	their pointers must be reasigned */
	if(emptyChild(p)) {
		/* there is no child */
		return NULL;
	} else {
		/* here, there is at least >=1 siblings,
		in which case if it is the last child, the
		parent must capture this */
		pcb_PTR childPcb = (p->p_child);
		/* the pcb_t is the only sibling */
		if(childPcb->p_prevSib == NULL) {
			/* the removed child pcb_t will
			have no parent */
			childPcb->p_prnt = NULL;
			/* the parent will have no
			pcb_t child */
			p->p_child = NULL;
		} else {
			/* the parent has more than one child;
			this will then involve rearanging the
			previous child pcb_t to refelct this */
			p->p_child = childPcb->p_prevSib;
			/* since the list is null terminated
			the next remaining child must be set to
			null */
			childPcb->p_prevSib->p_nextSib = NULL;
			/* the removed child pcb_t will
			have no parent */
			childPcb->p_prnt = NULL;
			/* clean the pcb_t */
			cleanChild(childPcb);
		}
		return childPcb;
	}
}

/*
* Function: makes the pcb_t given by p
* no longer a child of the its parent
* pcb_t, and remove it from the list;
* however, this can be any child in the list;
* if the pcb_t has no parent, simply return
* null
*/
pcb_PTR outChild(pcb_PTR p) {
		/* consider the case of having no parent */
		if(p->p_prnt == NULL) {
			/* the pcb_t has no parent */
			return NULL;
		} else {
			/* the pcb_t has a parent; here, there
			are two cases to consider: first, the
			pcb_t that is being removed is the head
			of the list; second, the pcb_t being
			removed is an arbitrary element in the list */
			if(p == p->p_prnt->p_child) {
				/* the pcb_t being removed is the head
				of the list - in this case, call the previously
				written function */
				return removeChild(p->p_prnt);
			} else {
				/* the remaining case to consider is if the
				child pcb_t being removed has a a sibling behind
				it, or if the the child pcb_t has no sibling
				behind it */
				if(p->p_child->p_prevSib == NULL) {
					/* rearrange the next sibling pcb_t
					to have its next child be null, since the
					removed pcb_t was the last child */
					p->p_nextSib->p_prevSib = NULL;
					/* the crux of the function - make the
					parent be null */
					p->p_prnt = NULL;
					/* clean the remaining sibling */
					p->p_nextSib = NULL;
					/* return the cleaned parent */
					return p;
				} else {
					/* the case in which the pcb_t has siblings
					and is in the list - simply reagrange the
					siblings */
					p->p_prevSib->p_nextSib = p->p_nextSib;
					p->p_nextSib->p_prevSib = p->p_prevSib;
					/* the crux of the function - make the
					parent be null */
					p->p_prnt = NULL;
					/* clean the remaining sibling */
					p->p_nextSib = NULL;
					p->p_prevSib = NULL;
					/* return the cleaned parent */
					return p;
				}
			}
		}
}
