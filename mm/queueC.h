

#define NUM_REGS 3
#define MAX_PROCESSES 20

struct process_control_block
{
	int pid;  // process id
	int psw; // program status
	int page_table;  // pagetable info
	int regs[NUM_REGS];  // Array of Registers
	struct process_control_block *next,*prev;
	int count;



} process[MAX_PROCESSES];

  
