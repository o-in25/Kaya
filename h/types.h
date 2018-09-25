#ifndef TYPES
#define TYPES

/****************************************************************************
 *
 * This header file contains utility types definitions.
 *
 ****************************************************************************/


typedef signed int cpu_t;


typedef unsigned int memaddr;

typedef struct {
	unsigned int d_status;
	unsigned int d_command;
	unsigned int d_data0;
	unsigned int d_dataa1;
} device_t;

#define t_recv_status	d_status
#define t_recv_command d_command
#define t_transm_status	d_data0
#define t_transm_command d_data1

#define DEVINTNUM 5
#define DEVPERINT 8

typedef struct {
	unsigned int rambase;
	unsigned int ramsize;
	unsigned int execbase;
	unsigned int execsize;
	unsigned int bootbase;
	unsigned int bootsize;
	unsigned int todhi;
	unsigned int todlo;
	unsigned int intervaltimer;
	unsigned int timescale;
	unsigned int inst_dev[DEVINTNUM];
	unsigned int interrupt_dev[DEVINTNUM];
	device_t   devreg[DEVINTNUM * DEVPERINT];
} devregarea_t;


#define STATEREGNUM	31

/* state */
typedef struct state_t {
	unsigned int s_asid;
	unsigned int s_cause;
	unsigned int s_status;
	unsigned int s_pc;
	int	s_reg[STATEREGNUM];
} state_t, *state_PTR;

/* registers */
#define	s_at	s_reg[0]
#define	s_v0	s_reg[1]
#define s_v1	s_reg[2]
#define s_a0	s_reg[3]
#define s_a1	s_reg[4]
#define s_a2	s_reg[5]
#define s_a3	s_reg[6]
#define s_t0	s_reg[7]
#define s_t1	s_reg[8]
#define s_t2	s_reg[9]
#define s_t3	s_reg[10]
#define s_t4	s_reg[11]
#define s_t5	s_reg[12]
#define s_t6	s_reg[13]
#define s_t7	s_reg[14]
#define s_s0	s_reg[15]
#define s_s1	s_reg[16]
#define s_s2	s_reg[17]
#define s_s3	s_reg[18]
#define s_s4	s_reg[19]
#define s_s5	s_reg[20]
#define s_s6	s_reg[21]
#define s_s7	s_reg[22]
#define s_t8	s_reg[23]
#define s_t9	s_reg[24]
#define s_gp	s_reg[25]
#define s_sp	s_reg[26]
#define s_fp	s_reg[27]
#define s_ra	s_reg[28]
#define s_HI	s_reg[29]
#define s_LO	s_reg[30]



/* process table entry type */
typedef struct pcb_t {
    struct pcb_t* p_next;
    struct pcb_t* p_prev;
    struct pcb_t* p_prnt;
		struct pcb_t* p_child;
		/* double linked children */
		struct pcb_t* p_nextSib;
		struct pcb_t* p_prevSib;
    state_t	p_s;
    int* p_semAdd;
}  pcb_t, *pcb_PTR;


/* semaphore table entry type */
typedef struct semd_t {
	struct semd_PTR s_next;
	struct semd_PTR s_prev;
	int *s_semAdd;
	pcb_PTR s_procQ;
}  semd_t, *semd_PTR;

#endif
