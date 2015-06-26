/*
* Copyright (C) 2015  DevX Ali Azam Rana 13F-8059
			   Huzaifa Ahmed 13F-8150
			   Uzair Fiaz 13F-8033
			   Abdullah Amjad 13F-8109		
* License: GPL version 2 or higher http://www.gnu.org/licenses/gpl.html
*/
#include <stdio.h>
#include <linux/kernel.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "keyboard_map.h"

#include "processmanager_test.h"


#include "processmanager.h"
//#include "definitions.h"
#include "queuemanager.c"
#include "queuemanager.h"

/* there are 25 lines each of 80 columns; each element takes 2 bytes */
#define LINES 25
#define COLUMNS_IN_LINE 80
#define BYTES_FOR_EACH_ELEMENT 2
#define SCREENSIZE BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE * LINES

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define IDT_SIZE 256
#define INTERRUPT_GATE 0x8e
#define KERNEL_CODE_SEGMENT_OFFSET 0x08

#define ENTER_KEY_CODE 0x1C
//queue objects
    struct process_control_block ret;

//------------------
//queue variables
    int pid = 0;
    char *command;
    char commandstr[100];	
    int psw = 0;
    int page_table = 0;
    int reg1 = 0;
    int reg2 = 0;
    int reg3 = 0;
    int regs[3];
    	int i = 0;
    enum SCHEDS scheduler = GROUP;

//----------------
extern unsigned char keyboard_map[128];
extern void keyboard_handler(void);
extern char read_port(unsigned short port);
extern void write_port(unsigned short port, unsigned char data);
extern void load_idt(unsigned long *idt_ptr);
extern int ptoI(char x);
extern char* itoC(int x);
/* current cursor location */
unsigned int current_loc = 0;
/* video memory begins at address 0xb8000 */
char *vidptr = (char*)0xb8000;
char in;
int counter;
struct IDT_entry{
	unsigned short int offset_lowerbits;
	unsigned short int selector;
	unsigned char zero;
	unsigned char type_attr;
	unsigned short int offset_higherbits;
};

struct IDT_entry IDT[IDT_SIZE];


void idt_init(void)
{
	unsigned long keyboard_address;
	unsigned long idt_address;
	unsigned long idt_ptr[2];

	/* populate IDT entry of keyboard's interrupt */
	keyboard_address = (unsigned long)keyboard_handler;
	IDT[0x21].offset_lowerbits = keyboard_address & 0xffff;
	IDT[0x21].selector = KERNEL_CODE_SEGMENT_OFFSET;
	IDT[0x21].zero = 0;
	IDT[0x21].type_attr = INTERRUPT_GATE;
	IDT[0x21].offset_higherbits = (keyboard_address & 0xffff0000) >> 16;

	/*     Ports
	*	 PIC1	PIC2
	*Command 0x20	0xA0
	*Data	 0x21	0xA1
	*/

	/* ICW1 - begin initialization */
	write_port(0x20 , 0x11);
	write_port(0xA0 , 0x11);

	/* ICW2 - remap offset address of IDT */
	/*
	* In x86 protected mode, we have to remap the PICs beyond 0x20 because
	* Intel have designated the first 32 interrupts as "reserved" for cpu exceptions
	*/
	write_port(0x21 , 0x20);
	write_port(0xA1 , 0x28);

	/* ICW3 - setup cascading */
	write_port(0x21 , 0x00);
	write_port(0xA1 , 0x00);

	/* ICW4 - environment info */
	write_port(0x21 , 0x01);
	write_port(0xA1 , 0x01);
	/* Initialization finished */

	/* mask interrupts */
	write_port(0x21 , 0xff);
	write_port(0xA1 , 0xff);

	/* fill the IDT descriptor */
	idt_address = (unsigned long)IDT ;
	idt_ptr[0] = (sizeof (struct IDT_entry) * IDT_SIZE) + ((idt_address & 0xffff) << 16);
	idt_ptr[1] = idt_address >> 16 ;

	load_idt(idt_ptr);
}

void kb_init(void)
{
	/* 0xFD is 11111101 - enables only IRQ1 (keyboard)*/
	write_port(0x21 , 0xFD);
}

void kprint(const char *str)
{
	unsigned int i = 0;
	while (str[i] != '\0') {
		vidptr[current_loc++] = str[i++];
		vidptr[current_loc++] = 0x07;
	}
}

void kprint_newline(void)
{
	unsigned int line_size = BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE;
	current_loc = current_loc + (line_size - current_loc % (line_size));
}

int ptoI(char x)
{
 int g = x;
 g = g-48;
 return g;
}
char gi;
char* itoC(int x)
{
 gi = (unsigned char)x+48;
 return &gi;
}
void clear_screen(void)
{
	unsigned int i = 0;
	while (i < SCREENSIZE) {
		vidptr[i++] = ' ';
		vidptr[i++] = 0x07;
	}
}
void checkCommand(int size)
{
	if(commandstr[0] == 'e')
	{counter++;
		char p[3];
		p[0] = commandstr[2];
		p[1] = commandstr[3];
		pid = ptoI(p[0]);
		pid = pid *10;
		pid += ptoI(p[1]);
		psw = ptoI(commandstr[5]);
		page_table = ptoI(commandstr[7]);
		reg1 = ptoI(commandstr[9]);
		reg2 = ptoI(commandstr[11]);
		reg3 = ptoI(commandstr[13]);
		regs[0] = reg1;
		regs[1] = reg2;
		regs[2] = reg3;	
		int e = enqueue(READY0, pid, psw, page_table, regs, 0,0,0);
		if(e == -1)
			kprint("Enqueue Not Done");
		else
			kprint("Done!!!");
		kprint_newline();
	}
	else if(commandstr[0] == 'd')
	{
		ret = dequeue(READY0);
		if(ret.pid == -1)
			kprint("Error");
		else
			{
			kprint_newline();
			kprint("DEQUEUED ID: ");
			int	x=ret.pid/10;
			int	y=ret.pid%10;
				kprint(itoC(x));
				kprint(itoC(y));
			
			//kprint(itoC(ret.pid));			
			kprint("Dequed");
			}
		kprint_newline();
	}
	else if(commandstr[0] == 's')
	{
		char p[3];
		p[0] = commandstr[2];
		p[1] = commandstr[3];
		pid = ptoI(p[0]);
		pid = pid *10;
		pid += ptoI(p[1]);
		ret = delete(READY0, find_process(READY0, pid));
		if (ret.pid == -1) {
                
		kprint("Could not dequeue: process not found.");
            	} else if (ret.pid == -2) {

                kprint("Could not dequeue: queue empty.");

            	} else {
			kprint_newline();
				kprint("SEARCH ID: ");
			int	x=ret.pid/10;
			int	y=ret.pid%10;
				kprint(itoC(x));
				kprint(itoC(y));
			//kprint(itoC(ret.pid));			
            	}
		kprint_newline();
	}
	else if(commandstr[0] == 'l' && commandstr[1] == 'p')
	{
		kprint("Trying to Print!!!! No Errors Will be shown ");
			kprint_newline();
		    struct process_control_block *temp = ready0.head;
			while(temp)
			{
				int x=0;
				int y;
				kprint("PID:> ");
				x=temp->pid/10;
				y=temp->pid%10;
				kprint(itoC(x));
				kprint(itoC(y));
				kprint_newline();	
				kprint("PSW:> ");
				kprint(itoC(temp->psw));
				kprint_newline();	
				kprint("Page Table:> ");
				kprint(itoC(temp->page_table));
				kprint_newline();
				kprint("REGS sepereated by Spaces MAX3:> ");
				kprint(itoC(temp->regs[0]));
				kprint(" ");	
				kprint(itoC(temp->regs[1]));
				kprint(" ");	
				kprint(itoC(temp->regs[2]));
				kprint_newline();		
				temp = temp->prev;
			}

	}
	else if(commandstr[0] == 'i' && commandstr[1] == 'n' && commandstr[2] == 't')
	{
	    kprint("\n***initializing***\n");
            init(PRIORITY);

		
	}
	else if (commandstr[0]=='g' && commandstr[1]=='o')
	{
		if(counter>1)
		{
			kprint_newline();		
			go();	
			kprint("PROCESS ON THE GO");
		}
		else
			{
			kprint_newline();		
					
			kprint("PROCESS CANT GO OR ONLY ONE PROCESS IN QUEUE!!!");
		
			}		
	}
	else if (commandstr[0]=='w' )
	{
		
			kprint_newline();		
			wait_();	
			kprint("PROCESS IS WAITING");
				
	}

	else if (commandstr[0]=='e' && commandstr[1]=='o'  )
	{
		
			kprint_newline();		
			eolife();	
			kprint("EOLIFE");
				
	}

	else if (commandstr[0]=='u' && commandstr[1]=='w'  )
	{
		
			kprint_newline();		
			char p[3];
			p[0] = commandstr[2];
			p[1] = commandstr[3];
			pid = ptoI(p[0]);
			pid = pid *10;
			pid += ptoI(p[1]);
			struct process_control_block * r;
			r = find_process(READY0, pid);
			if(r != NULL){
			kprint("Following Process is @ unwait:>");
			kprint(p);kprint(p+1);
			}
			else
			{
			kprint("Following Process is not found:>");
			kprint(p);kprint(p+1);
			}	
	}

	else if(commandstr[0] == 'l')
	{
		kprint("Trying to Print!!!! No Errors Will be shown ");
			kprint_newline();
		    struct process_control_block *temp = ready0.head;
			while(temp)
			{
				int x=0;
				int y;
				kprint("PID:> ");
				x=temp->pid/10;
				y=temp->pid%10;
				kprint(itoC(x));
				kprint(itoC(y));
				kprint_newline();	
				kprint("PSW:> ");
				kprint(itoC(temp->psw));
				kprint_newline();	
				kprint("Page Table:> ");
				kprint(itoC(temp->page_table));
				kprint_newline();
				kprint("REGS sepereated by Spaces MAX3:> ");
				kprint(itoC(temp->regs[0]));
				kprint(" ");	
				kprint(itoC(temp->regs[1]));
				kprint(" ");	
				kprint(itoC(temp->regs[2]));
				kprint_newline();		
				temp = temp->prev;
			}

	}
	else if(commandstr[0] == 'c')
	{
		counter++;
			kprint("Trying to Create Process!!!! No Errors Will be shown & Pid is determined automatically & will be unique");
			kprint_newline();
		psw = ptoI(commandstr[2]);
		page_table = ptoI(commandstr[4]);
		reg1 = ptoI(commandstr[6]);
		reg2 = ptoI(commandstr[8]);
		reg3 = ptoI(commandstr[10]);
		regs[0] = reg1;
		regs[1] = reg2;
		regs[2] = reg3;
		int e = create1(psw, page_table, regs, 0);
			if(e == ERROR_MAX_PROCESSES)
			{
				kprint("Error");
			}
			else if(e == ERROR_GROUP_NOT_EXIST)
			{
				kprint("Invalid Group Number");
			}
			else if(e == ERROR_PROCESS_NOT_UNIQUE ||
						e == ERROR_QUEUE_FULL ||
						e == ERROR_QUEUE_EMPTY ||
						e == ERROR_QUEUE_FULL)
			{
				kprint("FATAL ERROR");
			}
			else
			{
				kprint("She is done!!!");
			}						
	}

}

void keyboard_handler_main(void) {
	unsigned char status;
	char keycode;
	char x;

	/* write EOI */
	write_port(0x20, 0x20);

	status = read_port(KEYBOARD_STATUS_PORT);
	/* Lowest bit of status will be set if buffer is not empty */
	if (status & 0x01) {
		keycode = read_port(KEYBOARD_DATA_PORT);
		
		if(keycode < 0)
			return;

		if(keycode == ENTER_KEY_CODE) {
			commandstr[++i] = '\0';			
			checkCommand(i);			
			i = 0;			
			kprint_newline();
			
			//kprint(commandstr);
			return;
		}

		vidptr[current_loc++] = keyboard_map[(unsigned char) keycode];
		//in = keyboard_map[(unsigned char) keycode];
		vidptr[current_loc++] = 0x07;
		x = keyboard_map[(unsigned char) keycode];
		commandstr[i] = x;
		i++;			
//		//kprint(&in);		
//		vidptr[current_loc++] = 0x07;	
		

//----My Mod
		//&keyboard_map[(unsigned char) keycode];
		//kprint(&keyboard_map[(unsigned char) keycode]);
				//vidptr[current_loc++] = 0x07;	
		//kprint_newline();
//-----------------------------
	}
}
void kmain(void)
{
	init(PRIORITY);	
	const char *str = "OS@Testing Q and PCBs:>";
	clear_screen();
	kprint(str);
	//printf("test");
	//kprint_newline();
	//kprint_newline();

	idt_init();
	kb_init();
	

	while(1);
}

