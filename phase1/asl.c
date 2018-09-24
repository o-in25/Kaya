#ifndef ASL
#define ASL

#include "../h/const.h"
#include "../h/types.h"

#include "../e/pcb.e"


HIDDEN semd_t* semd_h;
HIDDEN semd_t* semdFree_h;


int insertBlocked(int *semAdd, pcb_PTR p) { /* 3 cases */
	semd_t target = searchASL(semAdd);
	if (target == NULL) { /* semAdd not found */
		allocSemd(semAdd, &p);
		return NULL;
	}
	/* semAdd found */
	insertProcQ(&target.s_procQ, p)
	return 8; /* FIX */
}

pcb_PTR removeBlocked(int* semAdd){
	/* find previous node */
	semd_PTR target = searchASL(semAdd);
	/* if previous node is what is being looked for */
	if (*(target->s_next->p_semAdd) == *(semAdd)) {
		pcb_PTR removed = removeProcQ(&target.s_procQ);

		if (emptyProcQ(*(target).s_procQ)) {
			freeSemd(target);
		}
		return removed;
	}
	/* semd not found */




}

pcb_PTR outBlocked (pcb_PTR p){
	semd_t a = *(p->p_semAdd);
	if (a == NULL) return NULL;
	return outProcQ(&a.s_procQ, p);
}

pcb_PTR headBlocked (int *semAdd){
	semd_t target = searchASL(semAdd);
	if (target == NULL) return null;
	return headProcQ(target.s_procQ);
}

void initASL() {
	static semd_t semdTable[MAXPROC + 2];	/* init semd free list */
	for (int i = 0; i < MAXPROC + 2; i++) {
		semdTable[i] = mkEmptySemd();
		freeSemd(&(semdTable[i]));
	}
	semd_t dummy1 = alloc
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
