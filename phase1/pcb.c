#include "../h/const.h"
#include "../h/types.h"

#include "/usr/include/uarm/libuarm.h"
#include "../e/pcb.e"
#include "../e/asl.e"

#define TRUE 1
#define FALSE 0

/* PCB.C */

HIDDEN pcb_PTR pcbFree_h;

state_PTR mkEmptyState() {
	return NULL;
}

void freePcb(pcb_PTR p){
	insertProcQ(&pcbFree_h, p);
}


pcb_PTR allocPcb(){
	pcb_PTR tmp = removeProcQ(&pcbFree_h);

	tmp->p_next = NULL;  /* initialize fields */
	tmp->p_prev = NULL;
	tmp->p_prnt = NULL;
	tmp->p_child = NULL;
	tmp->p_sib = NULL;
	tmp->p_s = (*(mkEmptyState()));   /* wtf */
	tmp->p_semAdd = NULL;

	if(tmp != NULL) return tmp;
	return NULL;
}

void initPcbs() {
	static pcb_t pcbTable[MAXPROC];
	int i;
	for (i = 0; i < MAXPROC; i++) {
		foo[i] = mkEmptyProcQ();
		freePcb(&(pcbTable[i]));
	}
}



pcb_PTR mkEmptyProcQ (){
	return NULL;
}

int emptyProcQ (pcb_PTR tp){
	return (tp == NULL);
}

/*
* Function: insert the pcb_t p into the
* process queue tp
*/
void insertProcQ(pcb_PTR *tp, pcb_PTR p){
	pcb_PTR tail = (*tp);
	/* in order to insert a given pcb_t into a
	given process queue given by tp,
	the queue must be verified for emptiness;
	if it is not empty, this becomes the facile
	task of rearanging the pointers */
	if (emptyProcQ(tail) {
		/* the queue is empty, so assign this
		pcb_t to be circular by making itself
		its previous and next element */
		p->p_next = p;
		p->p_prev = p;
	} else {
		/* since the list is not empty, simply
		reasign the pointers to account for the newly
		added element */
		tail->p_next = p;
		tail->p_prev
	}
	tail = p;
}

pcb_PTR removeProcQ (pcb_PTR *tp){
	return outProcQ(tp, *tp);
}

/*
* Function: remove the pcb_t pointed to by p
* from the process queue pointed to by tp;
* update the process queue's tp if necessary;
* if the desired entry is not in the indicated queue,
* return null; else, return p
*/
pcb_PTR outProcQ(pcb_PTR* tp, pcb_PTR p){
	/* dereference ahead of time */
	pcb_PTR tail = (*tp);
	/* removing the pcb_t from the process
	pointed to by tp has three cases to consider;
	first, if the tp is null, meaning there is no list for
	a pcb_t to be removed from; second, the pcb_t to be removed
	is the tp, in which case a if the tp is the only remaining
	node on the list, the tp must be set the null - otherwise its
	pointers must be rearranged. the last remaining case if
	tp is an arbitrary element in the list */
	if(emptyProcQ(tail)) {
		/* no list */
		return NULL;
	} else {
		/* a list of >= 1 */
		/* what is being removed is the tp */
		if(p == tail) {
			/* a list of 1 */
				if(tail.p_next == tail) {
					tail = NULL;
				} else {
					/* a list of > 1 */
					/* reasign the tail pointer */
					tail = tail.p_prev;
					/* swap the pointers with the soon to be removed pcb_t */
					tail.p_next->p_prev = tail.p_next;
					tail.p_prev->p_next = tail.p_prev;
				}
				/* return the block */
				return p;
		} else {
			/* what is being removed is not the tp */
			pcb_PTR currentPcb = tail->p_next;
			/* start from the start of the queue */
			while(currentPcb != tail) {
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

	pcb_PTR current = tmp->p_next;
	while(current != tmp) {
		if (current == p) {  /* find right pcb then... */
			tmp = (*tp)->p_next;
			(*tp)->p_next = tmp->p_next;
			tmp->p_next = NULL;
			return tmp;
		}
		current = current->p_next;
	}

	return NULL; /* pcb not found */
}

pcb_PTR headProcQ (pcb_PTR tp){
	if (emptyProcQ(tp)) return NULL;
	return (tp->p_next);
}



int emptyChild (pcb_PTR p){
	return (p->p_child == NULL);
}

void insertChild (pcb_PTR prnt, pcb_PTR p){
	insertProcQ(&prnt->p_child, p);
}

pcb_PTR removeChild (pcb_PTR p){
	return removeProcQ(&p->p_child);
}

pcb_PTR outChild (pcb_PTR p){ /* do you need to search each process block to find the one that has p as a child? */
	pcb_PTR *prnt = &(p->p_prnt);
	return outProcQ(prnt, p);
}
