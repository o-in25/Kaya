#ifndef ASL
#define ASL

#include "../h/const.h"
#include "../h/types.h"

#include "../e/pcb.e"


HIDDEN static semd_t* semd_h;
HIDDEN semd_t* semdFree;


int insertBlocked (int *semAdd, pcb_PTR p) { /* 3 cases */
	semd_t target = searchASL(semAdd);
	if (target == NULL) { /* semAdd not found */
		allocSemd(semAdd, &p);
		return NULL;
	}
	/* semAdd found */
	insertProcQ(&target.s_procQ, p)
	return 8; /* FIX */
}

pcb_PTR removeBlocked (int *semAdd){
	semd_t target = searchASL(semAdd);
	if (searchASL(semAdd) == NULL) return NULL;
	return removeProcQ(&target.s_procQ);
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

void initASL () {
	static semd_t foo[20];	/* init semd free list */
	for (int i = 0; i<20; i++) {
		foo[i] = mkEmptySemd();
		freeSemd(foo[i]);
	}

	/* init asl */
	static semd_t dummy1, dummy2;  /* set up dummy nodes */
	dummy1 = mkEmptySemd();  /* is this necessary? */
	dummy2 = mkEmptySemd();
	(*semd_h) = dummy1;
	semd_h->s_next = dummy2;
}

semd_t mkEmptySemd() {return NULL;  /* is this necessary? */}

/* search semd list method */
semd_t searchASL(int *semAdd) {
	/* get past head dummy node */
	semd_t current = *(semdFree->s_next);
	if (*(current.s_semAdd) == *semAdd) return current;
	if (*(current.s_semAdd) >= *semAdd) return NULL;
	while ((current.s_semAdd) <= *semAdd && *(current.s_semAdd) != NULL) {   /* maybe need to add the null check first */
		current = current.s_next;
		if (*(current.s_semAdd) == *semAdd) return current;
	}
	return NULL;
}

/* alloc semd method */
void allocSemd(int *semAdd, pcb_PTR p) {
	/* weave in */
}

/* free semd method */
void freeSemd(semd_t s) {
	/* empty free list case */
	if ((*semdFree) == NULL) semdFree->s_next = s;

	/* non-empty free list case */
	semd_t head = (*semdFree);
	semdFree->s_next = s;
	s->s_next = head;
}

/***************************************************************/

#endif
