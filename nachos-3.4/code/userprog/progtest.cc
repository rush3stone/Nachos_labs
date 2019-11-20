// progtest.cc 
//	Test routines for demonstrating that Nachos can load
//	a user program and execute it.  
//
//	Also, routines for testing the Console hardware device.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "console.h"
#include "addrspace.h"
#include "synch.h"

//----------------------------------------------------------------------
// StartProcess
// 	Run a user program.  Open the executable, load it into
//	memory, and jump to it.
//----------------------------------------------------------------------

void
StartProcess(char *filename)
{
    OpenFile *executable = fileSystem->Open(filename);
    AddrSpace *space;

    if (executable == NULL) {
        printf("Unable to open file %s\n", filename);
        return;
    }
    space = new AddrSpace(executable);    
    currentThread->space = space;

    delete executable;			// close file

    space->InitRegisters();		// set the initial register values
    space->RestoreState();		// load page table register

    machine->Run();			// jump to the user progam
    ASSERT(FALSE);			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}

// Data structures needed for the console test.  Threads making
// I/O requests wait on a Semaphore to delay until the I/O completes.

static Console *console;
static Semaphore *readAvail;
static Semaphore *writeDone;

//----------------------------------------------------------------------
// ConsoleInterruptHandlers
// 	Wake up the thread that requested the I/O.
//----------------------------------------------------------------------

static void ReadAvail(int arg) { readAvail->V(); }
static void WriteDone(int arg) { writeDone->V(); }

//----------------------------------------------------------------------
// ConsoleTest
// 	Test the console by echoing characters typed at the input onto
//	the output.  Stop when the user types a 'q'.
//----------------------------------------------------------------------

void 
ConsoleTest (char *in, char *out)
{
    char ch;

    console = new Console(in, out, ReadAvail, WriteDone, 0);
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);
    
    for (;;) {
        readAvail->P();		// wait for character to arrive
        ch = console->GetChar();
        console->PutChar(ch);	// echo it!
        writeDone->P() ;        // wait for write to finish
        if (ch == 'q') return;  // if q, quit
    }
}

//------------------------- Lab4 Exercise 5: MultiThread ---------------------------
void
userThreadExecve(int num)
{
    printf("Running user program thread %d\n", num);
    currentThread->TS();

    currentThread->space->InitRegisters();		// set the initial register values
    currentThread->space->RestoreState();		// load page table register
    currentThread->space->PrintState();         // print out current addrspace state

    machine->Run();			// jump to the user progam
    ASSERT(FALSE);			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"

}//userThread


//create a Thread
Thread 
*SingleThread(OpenFile *executable, int num) 
{
    printf("Creating user program thread %d\n", num);

    char ThreadName[20];
    sprintf(ThreadName, "userT%d", num); //name the thread with num
    Thread *newThread = new Thread(strdup(ThreadName), -1); //比main进程的优先级还要高
    // Thread *newThread = new Thread(ThreadName, -1);

    AddrSpace *space;
    space = new AddrSpace(executable);   
    newThread->space = space;

    return newThread;
}//SingleThread

void 
MultiThread(char *filename)
{
    OpenFile *executable = fileSystem->Open(filename);

    if (executable == NULL) {
        printf("Unable to open file %s\n", filename);
        return;
    }

    Thread *t1 = SingleThread(executable, 1);
    Thread *t2 = SingleThread(executable, 2);

    delete executable;			// close file

    t1->Fork(userThreadExecve, (void*)1); 
    t2->Fork(userThreadExecve, (void*)2); //

    currentThread->Yield();  //main Thread yield
}
