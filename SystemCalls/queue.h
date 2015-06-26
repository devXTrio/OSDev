

#define NUM_REGS 3
#define MAX_PROCESSES 20

struct process_control_block
{
	int pid;  // process id
	int psw; // program status
	int page_table;  // pagetable info
	int regs[NUM_REGS];  // Array of Registers
	struct process_control_block *next,*prev;



} process[MAX_PROCESSES];

struct queue_manager
{
	struct process_control_block *head, *current, *previous, *tail;
	int count;

	void queue_manager()
	{
	head=0;
	previous=0;
	tail=0;
	current=0;
	count=0;
	}

void Enqueue (int process_number, int psw_id, int pagetable, int reg0, int reg1, int reg2)
{
	
	if (head==NULL && count==0)
	{
		head=process[count];
		head->pid=process_number;
		head->page_table=pagetable;
		head->psw=psw_id;
		head->regs[0]=reg0;
		head->regs[1]=reg1;
		head->regs[2]=reg2;
		head->prev=NULL;
		head->next=NULL;
		tail=head;
	}
	else if (count<20)
	{
	count++;
	tail->next=process[count];
	current=tail;
	tail=tail->next;
		tail->pid=process_number;
		tail->page_table=pagetable;
		tail->psw=psw_id;
		tail->regs[0]=reg0;
		tail->regs[1]=reg1;
		tail->regs[2]=reg2;
		tail->prev=current;
		current=tail;
		tail->next=NULL;
		


	}

	else
	{
	//error
	}
}   // end of enqueue function

int Dequeue ()
{
	process_control_block *temp;
	if (count>0)
	{
	temp=head;
	head=head->next;
	head->prev=NULL;
	count--;
	}
	return temp->pid;

}


int Delete_process_number(int pid_del)
{
	process_control_block *temp,*curr;
	temp=head=curr;
	bool  found=false;
	if (count>0)
	{
		while(curr->pid!=pid_del && !found)
		{
		if(curr->pid==pid_del)
		{
		found=true;	
			if (curr->pid!=head->pid && curr->pid!=tail->pid)
		{	curr->prev->next=curr->next;
			curr->next->prev=curr->prev;
		}
			else if (curr->pid==head->pid)
		{		head=head->next;
				curr->next->prev=curr->prev;
				found=true;
		}
			else if (curr->pid==tail->pid)
				{
					found=true;
					tail=tail->prev;
					current=tail;
					curr->prev->next=curr->next;
			}
			else if (tail->pid==head->pid && curr->pid==head->pid)
			{
				found=true;
	head=NULL;
	previous=NULL;
	tail=NULL;
	current=NULL;		
			
			}

		count--;
		curr=curr->next;
			


		} // end of if 
		} // end of while
	}
	return curr->pid;

}

 void list()
 {
	process_control_block *curr;
	curr=head;
	while(curr!=NULL)
	{
		// cout<<curr->pid;
	
	curr=curr->next;
	}

 
 }

void init_Q()
{
	head=NULL;
	previous=NULL;
	tail=NULL;
	current=NULL;
	count=0;
// cout << "first queue"
// cout<< " max elements are 20"
}

};




