#define FALSE		0
#define TRUE		1
#define PAGESIZE	4096
#define SECOND		1000000

/* level 1 SYS calls */
#define READTERMINAL	9
#define WRITETERMINAL 	10
#define VSEMVIRT		11
#define PSEMVIRT		12
#define DELAY			13
#define DISK_PUT		14
#define DISK_GET		15
#define WRITEPRINTER	16
#define GET_TOD			17
#define TERMINATE		18

#define SEG0		0x00000000
#define SEG1		0x40000000
#define SEG2		0x80000000
#define SEG3		0xC0000000
