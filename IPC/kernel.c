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
//#include <string.h>
#include <ctype.h>
#include "keyboard_map.h"


#include "ipc_definitions.h"
#include "ipc_queuemanager.h"
#include "ipc.h"
#include "ipc_test.h"






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
    int init_num[10];



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
void listMQ(enum MESSAGE_QUEUES queuelist, int x)
{
	struct queue_message_t *structqueue = get_message(queuelist);
	if (structqueue->initialized == 0){
        	kprint("Queue Not Initialized @ index:>");
		kprint(itoC(x));        	
		return;
	}
	struct message *temp = structqueue->head;
	kprint_newline();
        kprint("Start of message queue @ index:>");
	kprint(itoC(x));
	while(temp)
	{
		int h = temp->source;
        	kprint("source:|  ");  kprint(itoC(h));
		h = temp->destination;
		kprint_newline();
		kprint("destination:|");  kprint(itoC(h));
//		h = temp->destination;
		kprint_newline();
		kprint("message:|");  kprint(temp->string);
//		kprint("message: %s  |\n", MESSAGE.string);
        	temp = temp->prev;
	}
	kprint_newline();
	kprint("End of Queue for messages in IPC");	
}
void checkCommand(int size)
{
	if(commandstr[0] == 'i' && commandstr[1] == 'p' && commandstr[2] == 'c')
	{
		int l = 0;
		/*Initialize init_num as -1*/
		kprint_newline();
		kprint("Initializing with values from 1-4");
                for(l = 0; l < 10; l++){
                    init_num[l] = l+1;
                }
		for(l = 0;l<5;l++)
		{
                    init_queue(init_num[l]);
		}
		
		
	}
	else if(commandstr[0] == 'l' && commandstr[1] == 'i')
	{
		kprint_newline();
		kprint("trying to print the list");
		kprint_newline();
		int i = 0;
		for(i=0;i<4;i++)
		{
			listMQ(i,i);

		}
	} 
	else if(commandstr[0] == 's' && commandstr[1] == 'e')
	{
		int source = ptoI(commandstr[3]);
		int destination = ptoI(commandstr[5]);
		char me[100];
		int l = 0;
		for(l = 0; commandstr[l] != '\0'; l++)
		{
			me[l] = commandstr[l];
		}
		me[l] = '\0';
		int e = send(source, destination, me);
		if( e != ERROR_SUCCESS)
			{
				kprint("NOT SEND CHECK AGAIN");
			}		
	}
	else if(commandstr[0] == 'r' && commandstr[1] == 'e')
	{
		int destination = ptoI(commandstr[3]);
		
		char me[100];
		int e = retrieve(destination, me);
		if (e == ERROR_SUCCESS)
		{
			kprint(me);
			kprint_newline();
		}
		else
		{
			kprint("error");
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
	//init(PRIORITY);	
	const char *str = "OS@Testing IPC :D :P :>";
	clear_screen();
	kprint(str);
	//printf("test");
	//kprint_newline();
	//kprint_newline();

	idt_init();
	kb_init();
	

	while(1);
}

