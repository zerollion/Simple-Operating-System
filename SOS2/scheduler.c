////////////////////////////////////////////////////////
// A Round-Robin Scheduler with 50% share to console
// 
// Process queue is maintained as a doubly linked list
// TODO: processes should be on different queues based 
//       on their state

//Tan Zhen 872692777
//Assignment 3
//18th Feb, 2015

#include "kernel_only.h"

extern GDT_DESCRIPTOR gdt[6];	// from startup.S
extern TSS_STRUCTURE TSS;	// from systemcalls.c

PCB console;	// PCB of the console (==kernel)
PCB *current_process; // the currently running process
PCB *processq_next = NULL; // the next user program to run

void init_scheduler() {
	current_process = &console; // the first process is the console
}

/*** Add process to process queue ***/
// Returns pointer to added process
PCB *add_to_processq(PCB *p) {
	disable_interrupts();

	// TODO: add process p to the queue of processes, always 
	// maintained as a circular doubly linked list;
	// processq_next always points to the next user process
	// that will get the time quanta;
	// if the process queue is non-empty, p should be added immediately
	// before processq_next
	// For details, read assignment background material

	if (processq_next == NULL){ 
		// if the process queue is empty just add and link p
		processq_next = p;
		processq_next->next_PCB = p;
		processq_next->prev_PCB = p;
		p->state = READY;
	}
	else{
		// if the process queue is not empty, add p to the head and fix circular property
		p->next_PCB = processq_next;
		p->prev_PCB = processq_next->prev_PCB;
		processq_next->prev_PCB->next_PCB = p;
		processq_next->prev_PCB = p;
		p->state = READY;
	}

	enable_interrupts();

	return p;		
}

/*** Remove a TERMINATED process from process queue ***/
// Returns pointer to the next process in process queue
PCB *remove_from_processq(PCB *p) {
	// TODO: remove process p from the process queue

	// TODO: free the memory used by process p's image

	// TODO: free the memory used by the PCB

	// TODO: return pointer to next process in list

	// if p is the only process in the queue, deallocate it and return null
	if (p->next_PCB == p)	{
		dealloc_memory((void *)p->memory_base);
		dealloc_memory(p);
		return NULL;
	}
	// if not, delete p, link p's previous and next processes, return next process
	else{
		PCB *tempPCB = p->next_PCB;
		p->next_PCB->prev_PCB = p->prev_PCB;
		p->prev_PCB->next_PCB = p->next_PCB;
		dealloc_memory((void *)p->memory_base);
		dealloc_memory(p);
		return tempPCB;
	}
}

/*** Schedule a process ***/
// This function is called whenever a scheduling decision is needed,
// such as when the timer interrupts, or the current process is done
void schedule_something() { // no interruption when here

	// TODO: see assignment background material on what this function should do 

	// firstly, if the queue is empty or current process is not the console
	// switch to console
	if ((processq_next == NULL) || (current_process != &console)){
		current_process = &console;
		current_process->state = RUNNING;
		switch_to_kernel_process(&console);
	}
	// if the current process is in terminated state, remove it from the queue
	//	recursive call
	if (processq_next->state == TERMINATED){
		processq_next = remove_from_processq(processq_next);
		schedule_something();
	}
	//	if the current process is in waiting state, change it to ready
	if (processq_next->state == WAITING)	{
		if (get_epochs() >= processq_next->sleep_end){
			processq_next->state = READY;
			processq_next->sleep_end = 0;
		}
	}

	// Round-Robin Scheduler part
	// while loop, finding the next ready process, run for a while, switch
	PCB *temp = processq_next;
	do{
		if (processq_next->state == READY){
			temp = processq_next;
			processq_next = processq_next->next_PCB;
			current_process = temp;
			current_process->state = RUNNING;
			switch_to_user_process(temp);
		}
		processq_next = processq_next->next_PCB;
	} while (processq_next != temp);

	// when all process is done, change control to the kernel
	processq_next = processq_next->next_PCB;
	current_process = &console;
	current_process->state = RUNNING;
	switch_to_kernel_process(&console);
	// TODO: comment the following when you start working on this function
	//switch_to_kernel_process(&console);
}

/*** Switch to kernel process described by the PCB ***/
// We will use the "fastcall" keyword to force GCC to pass 
// the pointer in register ECX;
// process switched to is a kernel process; so no ring change
__attribute__((fastcall)) void switch_to_kernel_process(PCB *p)  {

	// load CPU state from process PCB
	asm volatile ("movl %0, %%edi\n": :"m"(p->cpu.edi));
	asm volatile ("movl %0, %%esi\n": :"m"(p->cpu.esi));
	asm volatile ("movl %0, %%eax\n": :"m"(p->cpu.eax));
	asm volatile ("movl %0, %%ebx\n": :"m"(p->cpu.ebx));
	asm volatile ("movl %0, %%edx\n": :"m"(p->cpu.edx));
	asm volatile ("movl %0, %%ebp\n": :"m"(p->cpu.ebp));
	asm volatile ("movl %0, %%esp\n": :"m"(p->cpu.esp));

	// switching within the same ring; IRET requires the following in stack (see IRET details)
	asm volatile ("pushl %0\n"::"m"(p->cpu.eflags));
	asm volatile ("pushl %0\n": :"m"(p->cpu.cs));
	asm volatile ("pushl %0\n": :"m"(p->cpu.eip));

	// this should be the last one to be copied
	asm volatile ("movl %0, %%ecx\n": :"m"(p->cpu.ecx));

	// issue IRET; see IRET details
	asm volatile("sti\n"); // interrupts cleared in timer/syscall handler
	asm volatile("iretl\n"); // this completes the timer/syscall interrupt
}

/*** Switch to user process described by the PCB ***/
/*** DO NOT UNCOMMENT ***/
/*
__attribute__((fastcall)) void switch_to_user_process(PCB *p) {

	// You implemented this in the previous assignment
	...

}*/


