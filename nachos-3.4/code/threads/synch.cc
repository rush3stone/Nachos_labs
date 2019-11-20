// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
        queue->Append((void *)currentThread);	// so go to sleep
        currentThread->Sleep();
    } 
    value--; 					// semaphore available, 等到被唤醒后，还是会执行这一步，但是V()中会先value++
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	    scheduler->ReadyToRun(thread);
    value++;  
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
//---------------- Lab4 Sysch -----------------------------------------
Lock::Lock(char* debugName) 
{
    name = debugName;
    lockThread = NULL;
    semaphore = new Semaphore("SemForLock", 1);
}

Lock::~Lock() 
{
    semaphore->~Semaphore();
}

// Acquire -- wait until the lock is FREE, then set it to BUSY
void Lock::Acquire() 
{
    // IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    DEBUG('s', "thread %s ask for lock %s.\n", currentThread->getName(), name);
    semaphore->P();
    lockThread = currentThread;

    // (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

// Release -- set lock to be FREE, waking up a thread waiting
//		in Acquire if necessary
void Lock::Release() 
{
    DEBUG('s', "thread %s ask for lock %s.\n", currentThread->getName(), name);
    ASSERT(this->isHeldByCurrentThread());
    semaphore->V();
    lockThread = NULL;
}

// true if the current thread holds this lock.  
// Useful for checking in Release, and in condition variable ops below.
bool Lock::isHeldByCurrentThread()
{
    return lockThread == currentThread;
}

// check if Locked or not
bool Lock::isLocked()
{
    return semaphore->getName() == 0;
}


Condition::Condition(char* debugName) 
{
    queue = new List;
}
Condition::~Condition() 
{
    queue->~List();
}

// these are the 3 operations on condition variables; 
// releasing the lock and going to sleep are *atomic* in Wait()
void Condition::Wait(Lock* conditionLock) {
    IntStatus oldLevel = interrupt->SetLevel(IntOff); //close interrupt

    ASSERT(conditionLock->isHeldByCurrentThread()); //check lock status
    ASSERT(conditionLock->isLocked());

    // Release lock if it wait
    conditionLock->Release();
    queue->Append(currentThread); 
    currentThread->Sleep();

    // after being Waked up, acquire the lock again
    conditionLock->Acquire(); 
    
    (void)interrupt->SetLevel(oldLevel);
}

// conditionLock must be held by the currentThread for all of these operations
void Condition::Signal(Lock* conditionLock) 
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff); //close interrupt

    ASSERT(conditionLock->isHeldByCurrentThread());
    DEBUG('s', "Condition %s, Signal Lock %s\n", name, conditionLock->getName());
    if(!queue->IsEmpty()) {
        Thread * thread = (Thread*) queue->Remove();
        scheduler->ReadyToRun(thread);
    }

    (void)interrupt->SetLevel(oldLevel);
}

void Condition::Broadcast(Lock* conditionLock) 
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff); //close interrupt
    ASSERT(conditionLock->isHeldByCurrentThread());
    DEBUG('s', "Condition %s, Broadcast lock %s\n", name, conditionLock->getName());
    while(!queue->IsEmpty()) { // 
        Thread * thread = (Thread*) queue->Remove();
        DEBUG('s', "Thread %s get Condition %s finally.\n", thread->getName(), conditionLock->getName());
        scheduler->ReadyToRun(thread);
    }

    (void)interrupt->SetLevel(oldLevel);
}

// ------------------- Producer & Consumer ------------------------------------------
wareHouse::wareHouse()
{
    num = 0;
    emptyNum = new Semaphore("produceSem", WARE_HOUSE_SIZE);
    fullNum = new Semaphore("comsumeSem", 0);
    wareHouseLock = new Lock("wareHouseLock");
}

wareHouse::~wareHouse()
{
    delete proList;
}


void
wareHouse::produce(product *pro)
{
    DEBUG('p', "produce one product in wareHouse.\n");
    emptyNum->P();  //
    wareHouseLock->Acquire();
    proList[num++] = *pro;
    wareHouseLock->Release();
    fullNum->V();
}

product*
wareHouse::consume()
{
    DEBUG('p', "consume one product from wareHouse.\n");
    fullNum->P();
    wareHouseLock->Acquire();
    product *item = &proList[--num];
    wareHouseLock->Release();
    emptyNum->V();
    return item;
}

void
wareHouse::printProduct()
{
    DEBUG('p', "print all products in warehouse\n");
    printf("All products(total:%d) in warehouse(size:%d):\n", num, WARE_HOUSE_SIZE);
    for(int i = 0; i < num; i++) {
        if(i % 10 == 0) printf("%d\n", proList[i]);
        else printf("%d ", proList[i]);
    }
    printf("\n");
}

// -------------------------------------------------------------------------------------


// --------------------- Lab3 Synch: Barrier ----------------------------------
Barrier::Barrier(int num){
    lock = new Lock("BarrierLock");
    totalCount = num;
}


/*
The basic barrier has mainly two variables, 
one of which records the pass/stop state of the barrier, 
the other of which keeps the total number of threads that have entered in the barrier. 
The barrier state was initialized to be "stop" by the first threads coming into the barrier. 
Whenever a thread enters, based on the number of threads already in the barrier, 
only if it is the last one, the thread sets the barrier state to be "pass" so that all the threads can get out of the barrier. 
On the other hand, when the incoming thread is not the last one, 
it is trapped in the barrier and keeps testing if the barrier state has changed from "stop" to "pass", 
and it gets out only when the barrier state changes to "pass".
*/
// barrier for p processors
void
Barrier::BarrierFunc(int p)
{

    this->lock->Acquire();
    if (this->leave_counter == p)
    {
        if (this->arrive_counter == 0){ // no other threads in barrier
            this->flag = 0; // first arriver clears flag
        }else{
            this->lock->Release();
            while (this->leave_counter != p); // wait for all to leave before clearing
            this->lock->Acquire();
            this->flag = 0; // first arriver clears flag
        }
    }
    this->arrive_counter++;
    int arrived = this->arrive_counter;
    this->lock->Release();
    if (arrived == p){ // last arriver sets flag
        this->arrive_counter = 0;
        this->leave_counter = 1;
        this->flag = 1;
    }else{
        while (this->flag == 0); // wait for flag
        this->lock->Acquire();
        this->leave_counter++;
        this->lock->Release();
    }
}

