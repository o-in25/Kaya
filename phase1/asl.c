#ifndef ASL
#define ASL

#include "../h/const.h"
#include "../h/types.h"

#include "../e/pcb.e"


HIDDEN semd_t* semd_h;
HIDDEN semd_t* semdFree_h;


int insertBlocked(int *semAdd, pcb_PTR p) { /* 3 cases */
	semd_PTR prev = searchASL(semAdd);
	if (*(prev->s_next->s_semAdd) != *(semAdd)) { /* semAdd not found */
		semd_t newSemd = allocSemd();
		if (newSemd == NULL) {
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

/******************** helper function ***************/
/** search semd list method **/
/***************************************************/
semd_PTR searchASL(int* semAdd) {
	/* get past head dummy node */
	semd_PTR temp = *(semd_h).s_next;
	semd_t current = *(temp);
	/* if the node being searched for is found */
	while((*(current.s_semAdd)) < *semAdd) {
		current = *(current.s_next);
	}
	/* return previous node */
	return current.s_prev;
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
