#include "../h/const.h"
#include "../h/types.h"

#include "/usr/include/uarm/libuarm.h"
#include "../e/pcb.e"
#include "../e/asl.e"

#define TRUE 1
#define FALSE 0

/* PCB.C */

HIDDEN pcb_PTR pcbFree_h;

void freePcb (pcb_PTR p){
	insertProcQ(&pcbFree_h, p);
}

pcb_PTR allocPcb (){
	pcb_PTR tmp = removeProcQ(&pcbFree_h);
	
	tmp->p_next = NULL;  /* initialize fields */
	tmp->p_prev = NULL;
	tmp->p_prnt = NULL;
	tmp->p_child = NULL;
	tmp->p_sib = NULL;
	//tmp->p_s = NULL;   
	//tmp->p_semAdd = NULL;  

	if(tmp != NULL) return tmp;
	return NULL;
}

void initPcbs (){
	static pcb_t *foo[MAXPROC];	
	for (int i = 0; i<MAXPROC; i++) {
		foo[i] = mkEmptyProcQ();
		freePcb(foo[i]);
	}
}



pcb_PTR mkEmptyProcQ (){
	return NULL;
}

int emptyProcQ (pcb_PTR tp){
	return (tp == NULL);
}

void insertProcQ (pcb_PTR *tp, pcb_PTR p){
	if (emptyProcQ(*tp)) { /* empty q */
		*tp = p;
		p->p_next = p;
		return;
	}
	// non-empty q
	p->p_next = (*tp)->p_next;
	(*tp)->p_next = p;
	*tp = p;
}

pcb_PTR removeProcQ (pcb_PTR *tp){
	return outProcQ(tp, *tp);
}

pcb_PTR outProcQ (pcb_PTR *tp, pcb_PTR p){
	pcb_PTR tmp;
	pcb_PTR tmp2;	
	tmp = *tp;
	if (tmp == p) { /* first pcb is p */
		(*tp)->p_next = tmp->p_next;
		
		tmp->p_next = NULL;
		
		return tmp;
	}

	pcb_PTR current = tmp->p_next;
	while (current != tmp) {
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



