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


#include "file_test.h"





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
void errorToString(int error, char* command){
    kprint("Error in"); kprint(command); kprint("code"); kprint(itoC(error));
    
    switch(error){
    case ERROR_INVALID_DEVICE_NUM:
        kprint("Device number not valid");
        break;
    case ERROR_DEVICE_MOUNTED:
        kprint("Device mounted\n");
        break;
    case ERROR_FILE_NOT_OPEN:
        kprint("File is not open\n");
        break;
    case ERROR_BLOCK_NOT_IN_FILE:
        kprint("Block is not in file\n");
        break;
    case ERROR_BUFFER_FULL:
        kprint("Buffer is full\n");
        break;
    case ERROR_BUFFER_NOT_EXIST:
        kprint("Buffer does not exist\n");
        break;
    case ERROR_NOT_INITIALIZED_OR_FORMATED:
        kprint("Device is not initialized or formatted\n");
        break;
    case ERROR_DEVICE_NOT_KNOWN:
        kprint("Device is not known\n");
        break;
    case ERROR_SOURCE_QUEUE_NOT_EXIST:
        kprint("Source Queue does not exist\n");
        break;
    case ERROR_BAD_FILE_PTR:
        kprint("Bad file pointer\n");
        break;
    case ERROR_BAD_DIR_QUEUE:
        kprint("Bad directory queue\n");
        break;
    case ERROR_DIR_QUEUE_EMPTY:
        kprint("Directory queue empty\n");
        break;
    case ERROR_ARG_NOT_INT:
        kprint("Argument is not an integer\n");
        break;
    case ERROR_BAD_BLOCK_PTR:
        kprint("Bad block pointer\n");
        break;
    case ERROR_BAD_BLOCK_QUEUE:
        kprint("Bad block queue\n");
        break;
    case ERROR_BLOCK_QUEUE_EMPTY:
        kprint("Block queue empty\n");
        break;
    case ERROR_BAD_FS_NAME:
        kprint("Bad file system name\n");
        break;
    case ERROR_FILE_ALREADY_EXISTS:
        kprint("File already exists\n");
        break;
    case ERROR_DIR_IS_FILE:
        kprint("Directory is a file\n");
        break;
    case ERROR_DIR_NOT_FOUND:
        kprint("Directory is not found\n");
        break;
    case ERROR_INVALID_BLOCK_SIZE:
        kprint("Invalid block size\n");
        break;
    case ERROR_FILE_IS_DIR:
        kprint("File is a directory\n");
        break;
    case ERROR_FS_NAME_ARG:
        kprint("File system arguement not valid\n");
        break;
    case ERROR_BAD_OPTION:
        kprint("Bad option\n");
        break;
    case ERROR_TOO_MANY_OPEN_FILES:
        kprint("Too many open files\n");
        break;
    case ERROR_NO_FREE_BLOCKS:
        kprint("No free blocks\n");
        break;
    case ERROR_ADDR_OUT_OF_BOUNDS:
        kprint("Address out of bounds\n");
        break;
    case ERROR_BLOCK_ALREADY_EMPTY:
        kprint("Block already empty\n");
        break;
    case ERROR_BLOCK_ALREADY_FULL:
        kprint("Block already full\n");
        break;
    case ERROR_FILES_ARE_OPEN:
        kprint("Files are open\n");
        break;
    case ERROR_ALREADY_MOUNTED:
        kprint("Device already mounted\n");
        break;
    case ERROR_ALREADY_UNMOUNTED:
        kprint("Device already unmounted\n");
        break;
    case ERROR_FS_NAME_NOT_EXISTS:
        kprint("File system name does not exist\n");
        break;
    case ERROR_FILE_IS_READ_ONLY:
        kprint("File is read only\n");
        break;
    case ERROR_FILE_HANDLE_OUT_OF_RANGE:
        kprint("File handle is out of range\n");
        break;
    case ERROR_FILE_NOT_FOUND:
        kprint("File not found\n");
        break;
    case ERROR_FILE_ALREADY_OPEN:
        kprint("File is already open\n");
        break;
    default:
        kprint("Unknown error\n");
    }
}
void checkCommand(int size)
{
	if(commandstr[0] == 'i' && commandstr[1] == 'n')
	{
		char p[3];
		p[0] = commandstr[3];
		p[1] = commandstr[4];
		pid = ptoI(p[0]);
		pid = pid *10;
		pid += ptoI(p[1]);
		int e = init_fs(pid);
		if( e<0)
		{
			errorToString(e,"INIT_FS");
			kprint_newline();
		}
		else
		{
			kprint("Done");
			kprint_newline();	
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
	const char *str = "OS@DevXFileSystems :D :P :>";
	clear_screen();
	kprint(str);
	//printf("test");
	//kprint_newline();
	//kprint_newline();

	idt_init();
	kb_init();
	

	while(1);
}

