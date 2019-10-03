// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "elevatortest.h"

// testnum is set in main.cc
int testnum = 1;

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, (void*)1);
    SimpleThread(0);
}

void 
Lab1Excercise3Thread(int which) {
    int loopTime = 5;
    for(int i = 0; i < loopTime; i++) {
        printf("thread %d (uid=%d tid=%d) loop %d time\n", currentThread->getName(), currentThread->getUserId(), currentThread->getThreadId(), i);
        currentThread->Yield();
    }//for
}

void 
Lab1Excercise3() {
    DEBUG('t', "Entering Excercise3");

    Thread *t = new Thread("t-1");
    t->Fork(Lab1Excercise3Thread, (void*)t->getUserId());
    Lab1Excercise3Thread(0);
}

void 
Lab1Excercise4_1() {
    DEBUG('t', "Entering Excercise4");
    int testTimes = 129;
    for (int i = 0; i < testTimes; i++) {
        Thread *t = new Thread("new thread");
        printf("name: %s (tid=%d)\n", t->getName(), t->getThreadId());
    }//for
}


// void TS() {
//     const char* TSToString[] = {"JUST_CREATED", "RUNNING", "READY", "BLOCKED"};
//     printf("Name\tUId\tTId\tThreadStatus\n");
//     for(int i = 0; i < maxThreadNum; i++) {
//         if(tidFlag[i])
//             printf("%s\t%d\t%d\t%s\n", tidPointer[i]->getName(), tidPointer[i]->getUserId(), tidPointer[i]->getThreadId(), TSToString[tidPointer[i]->getThreadStatus()]);
//     }
// }

void Lab1Excercise4Thread(int which) {

    printf("thread_name=%s, uid=%d, tid=%d\n", currentThread->getName(), currentThread->getUserId(), currentThread->getThreadId());

    IntStatus currentLevel = interrupt->getLevel();
    switch (which)
    {
    case 1:
        printf("Yield\n");                    
        scheduler->Print(); printf("\n\n");   // print ready list
        currentThread->Yield();               // Yield, and turn status to Ready
        break;
    case 2:
        printf("Sleep\n");
        scheduler->Print(); printf("\n\n");
        interrupt->SetLevel(IntOff);          // disable interrupt
        currentThread->Sleep();               // Sleep, and turn status to Blocked
        interrupt->SetLevel(currentLevel);    // return to the original level
        break;
    case 3:
        printf("Finish\n");
        scheduler->Print(); printf("\n\n");   
        currentThread->Finish();              // Finish, and clean
        break;
    default:
        printf("Yield (default)\n");
        scheduler->Print(); printf("\n\n");
        currentThread->Yield();
        break;
    }

}


void 
Lab1Excercise4_2() {
    DEBUG('t', "Entering Excercise4_2");
    Thread *t1 = new Thread("t1");
    Thread *t2 = new Thread("t2");
    Thread *t3 = new Thread("t3");

    t1->Fork(Lab1Excercise4Thread, (void*)1); //ready
    t2->Fork(Lab1Excercise4Thread, (void*)2); //blocked
    t3->Fork(Lab1Excercise4Thread, (void*)3); //running
    
    Thread *t4 = new Thread("t4");  // just_created

    Lab1Excercise4Thread(0); //Yield the current thread (i.e. main that is defined in system.cc)

    printf("---------- Threads Status --------------\n");
    currentThread->TS();
    printf("----------- End ------------------------\n");

}

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    switch (testnum) {
    case 1:
	    ThreadTest1();
	    break;

    case 2:
        Lab1Excercise3();
        break;

    case 3:
        Lab1Excercise4_1();   // test the limit of threads
        break;        

    case 4:
        Lab1Excercise4_2();  // test TS commnand, show the status of all thread
        break;

    default:
	    printf("No test specified.\n");
	    break;
    }
}

