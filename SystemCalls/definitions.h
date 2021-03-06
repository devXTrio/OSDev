
#ifndef DEF_H
#define DEF_H

#define MAX_QUANTUM 5
#define null 0
#define NUM_REGS 3
#define MAX_PROCESSES 20
#define MAX_PRIORITY 10

/*Global Error Codes*/
#define ERROR_SUCCESS 0
#define ERROR_QUEUE_EMPTY -1
#define ERROR_QUEUE_FULL -2
#define ERROR_PROCESS_NOT_EXIST -3
#define ERROR_GROUP_NOT_EXIST -4
#define ERROR_SWITCH_DEFAULT -5
#define ERROR_NO_READY_PROCESS -6
#define ERROR_MAX_PROCESSES -7
#define ERROR_PROCESS_NOT_UNIQUE -8
#define ERROR_WRONG_SCHEDULER -9
#define ERROR_INVALID_PARAMETER -10

enum QUEUES {
    NEW,
    WAITING,
    READY0,
    READY1,
    READY2,
    READY3,
    TERMINATED,
    RUNNING
} queue_enum;

enum SCHEDS {
    GROUP,
    PRIORITY
} scheduler;

struct process_control_block {
    int pid;   /* Process ID */
    int psw;   /* Program status word */
    int page_table;   /* Pagetable info */
    int regs[NUM_REGS];   /* Array of registers */
    struct process_control_block *next;
    struct process_control_block *prev;
    int empty;
    int priority;
    int quantum_count;
    enum QUEUES group;
};

struct queue_t {
    struct process_control_block *head;
    struct process_control_block *tail;
    int size;
    struct process_control_block *top;
};

int process_counter;
int pid_counter;
int global_quantum_count;
enum QUEUES current_group;

#endif
