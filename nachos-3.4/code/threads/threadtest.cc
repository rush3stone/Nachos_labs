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
#include "synch.h"  //lab3 synch

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
    t->setUserId(731);
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


void Lab1Excercise4Thread(int which) {

    printf("thread_name=%s (uid=%d, tid=%d) ==> ", currentThread->getName(), currentThread->getUserId(), currentThread->getThreadId());

    IntStatus currentLevel = interrupt->getLevel();
    switch (which)
    {
    case 1:
        printf("Yield\n");                    
        scheduler->Print(); printf("\n\n");   // print ready list
        currentThread->Yield();               // Yield, and turn status to Ready
        printf("***** Enter t1 again and Finish ********\n"); //test: print when thread t1 finish
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
    DEBUG('t', "Entering Lab1Excercise4_2");
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

static Thread *t[4];

void
Lab2Excercise3Execve(int which) {
    printf("\n-------------- thread=%s, tid=%d, priority=%d -----------------\n", 
        currentThread->getName(), currentThread->getThreadId(), currentThread->getPriority());
    printf("Current_ReadyList:　");
    scheduler->Print(); printf("\n\n");
    
    for(int i = 0; currentThread->getThreadId() == 0 && i < 3; i++) {
        // Lab2:　judge to preempt or not
        if(currentThread->getPriority() > t[i]->getPriority()) {
            printf("Preempt Successful! thread=%s(%d) -> thread=%s(%d)\n", 
            currentThread->getName(), currentThread->getPriority(), t[i]->getName(), t[i]->getPriority());
            currentThread->Yield(); //但是Yield之后执行的是ReadyList的首进程
        }
    }
}

void
Lab2Excercise3() {
    DEBUG('t', "Entering Lab2Excercise3");
    
    // the priority of main is 0 as default
    currentThread->setPriority(4); //但此处设置优先级并不会覆盖在ReadyList,只有在加入ReadyList时才会更新

    Thread *t1 = new Thread("t1", 3); t[0]=t1;
    Thread *t2 = new Thread("t2", 2); t[1]=t2;
    Thread *t3 = new Thread("t3", 1); t[2]=t3;

    t1->Fork(Lab2Excercise3Execve, (void*)t1->getThreadId());
    t2->Fork(Lab2Excercise3Execve, (void*)t2->getThreadId());
    t3->Fork(Lab2Excercise3Execve, (void*)t1->getThreadId());
    
    Lab2Excercise3Execve(currentThread->getThreadId()); // Execve and Yield main Thread
    currentThread->TS();

}

//　lab2: Round Robin
void ThreadWithTicks(int runningTime){
    int num;
    for(num = 0; num < runningTime*SystemTick; num++) {
        printf("*** thread with running time %d looped %d times (stats->totalTicks: %d)\n", runningTime, num+1, stats->totalTicks);
        interrupt->OneTick(); //make system time moving forward (advance simulated time)
        //Switch interrupt on and off to invoke OneTick()
        // interrupt->SetLevel(IntOn);
        // interrupt->SetLevel(IntOff);
    }
    currentThread->Finish();
}//ThreadWithTicks

void 
Lab2ChallengeRR()
{
    DEBUG('t', "Entering Lab2ChallengeRR");

    printf("\nSystem initial ticks: system=%d, user=%d, total=%d\n", stats->systemTicks, stats->userTicks, stats->totalTicks);
    
    Thread *t1 = new Thread("7");
    Thread *t2 = new Thread("5");
    Thread *t3 = new Thread("3");
    printf("\nAfter new　Threads ticks: system=%d, user=%d, total=%d\n", stats->systemTicks, stats->userTicks, stats->totalTicks);

    t1->Fork(ThreadWithTicks, (void*)7);
    t2->Fork(ThreadWithTicks, (void*)2);
    t3->Fork(ThreadWithTicks, (void*)5);
    printf("\nAfter Fork ticks: system=%d, user=%d, total=%d\n\n", stats->systemTicks, stats->userTicks, stats->totalTicks);

    //update the lastSwitchTick
    //(according to previous test, it will start from 50)
    scheduler->lastSwitchTick = stats->totalTicks;
    currentThread->Yield(); //yield the main thread

}//Lab2ChallengeRR

// ------------------------ Lab 3 Synch: Producer & Consumer ----------------------
// #define WARE_HOUSE_SIZE 5

// typedef struct Product{ //product
//   int value;
// }product;

// class wareHouse  //warehouse to save products
// {
// public:
//   wareHouse(){
//     num = 0;
//     emptyNum = new Semaphore("produceSem", WARE_HOUSE_SIZE);
//     fullNum = new Semaphore("comsumeSem", 0);
//     wareHouseLock = new Lock("wareHouseLock");
//   };
//   ~wareHouse(){
//     delete proList;
//   };

//   product *consume(){
//     DEBUG('p', "consume one product from wareHouse.\n");
//     fullNum->P();
//     wareHouseLock->Acquire();
//     product *item = &proList[--num];
//     wareHouseLock->Release();
//     emptyNum->V();
//     return item;
//   };

//   void produce(product *pro){
//     DEBUG('p', "produce one product in wareHouse.\n");
//     emptyNum->P();  //
//     wareHouseLock->Acquire();
//     proList[num++] = *pro;
//     wareHouseLock->Release();
//     fullNum->V();
//   };
//   void printProduct(){
//     DEBUG('p', "print all products in warehouse\n");
//     printf("All products(total:%d) in warehouse(size:%d):\n", num, WARE_HOUSE_SIZE);
//     for(int i = 0; i < num; i++) {
//         if(i % 10 == 0) printf("%d\n", proList[i]);
//         else printf("%d ", proList[i]);
//     }
//     printf("\n");
//   }

// private:
//   Semaphore *emptyNum;
//   Semaphore *fullNum;
//   int num;
//   product proList[WARE_HOUSE_SIZE];
//   Lock *wareHouseLock;
// }*warehouse; //wareHouse
wareHouse *warehouse;

void Producer(int itemNum) {
    for(int i = 0; i < itemNum; i++){
        printf("PPP Thread %s PPP, ", currentThread->getName());
        product* item = new product();
        item->value = i;
        warehouse->produce(item);
        printf("Produce product: %d\n", item->value);

        interrupt->OneTick();
    }
}

void Consumer(int iterNum) {
    for (int i = 0; i < iterNum; i++) {
        printf("CCC Thread %s CCC, ", currentThread->getName());
        product* item = warehouse->consume();
        printf("Consume product: %d\n", item->value);

        interrupt->OneTick();
    }
}


void Lab3ProConTest(){
    DEBUG('p', "Now Test Producer & Consumer!\n");
    warehouse = new wareHouse();

    Thread *p1 = new Thread("Producer_1");
    Thread *c1 = new Thread("Consumer_1");

    p1->Fork(Producer, (void*)4);
    c1->Fork(Consumer, (void*)3);

    currentThread->Yield(); // Yield the main thread
}

// ----------------- Lab3 Synch: Barrier ---------------------------------
#define threadNum 4
#define rows 24
#define cols 10
#define threadRows rows/threadNum
double matrix[rows][cols];
Barrier *barrier;

void barrierThread(int startRow){

    // 得到需要计算的范围
    int endRow = startRow + threadRows;

    // 第１轮计算
    for (int x = startRow; x < endRow; x++) for (int y = 0; y < cols; y++) {
        matrix[x][y] += (x+1)*(y+1);
    }
    printf("Thread \"%s\" finish First Calculation\n", currentThread->getName());
    barrier->BarrierFunc(currentThread->getThreadId());

    // 第２轮计算
    for (int x = startRow; x < endRow; x++) for (int y = 0; y < cols; y++) {
        matrix[x][y] /= startRow+1;
    }
    printf("Thread \"%s\" finish Second Calculation\n", currentThread->getName());
    barrier->BarrierFunc(currentThread->getThreadId());

    // 第3轮计算
    for (int x = startRow; x < endRow; x++) for (int y = 0; y < cols; y++) {
        matrix[x][y] -= startRow/threadRows;
    }
    printf("Thread \"%s\" finish Third Calculation\n", currentThread->getName());
}

void Lab3Barrier(){
    Thread *threads[threadNum];
    barrier = new Barrier(threadNum);

    //initialize threads
    for(int i = 0; i < threadNum; i++){
        char ThreadName[10];
        sprintf(ThreadName, "t_%d", i+1);
        threads[i] = new Thread(strdup(ThreadName));
    }

    int startRows = 0;
    for(int i = 0; i < threadNum; i++) {
        threads[i]->Fork(barrierThread, (void*)startRows);
        startRows += threadRows;
    }

     // Initialization
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i][j] = 0;
        }
    }

    printf("main() is ready.\n");

    currentThread->Yield(); // Yield the main thread

    barrier->BarrierFunc(currentThread->getThreadId());
    // main will wake everybody up when everybody is reaching the barrier

    printf("main() is going!\n");

    currentThread->Yield(); // Yield the main thread
    // Everybody doing Second calculation

    barrier->BarrierFunc(currentThread->getThreadId());

    printf("main() is going again!\n");

    currentThread->Yield(); // Yield the main thread
    // Everybody doing Third calculation 

    // Back to main and print the result
    printf("Result of data:\n");
    for (int i = 0; i < rows; i++ ) {
        for (int j = 0; j < cols; j++) {
            printf("%5.2lf ", matrix[i][j]);
        }
        printf("\n");
    }

}//Lab3Barrier



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

    case 5:
        Lab2Excercise3();
        break;

    case 7:
        printf("Lab2 Challenge Round Robin:\n");
        printf("(Don't forget to add `-rr` to activate timer.)\n");
        Lab2ChallengeRR();
        break;

    case 8:
        printf("Lab3 Synch - Producer&Consumer:\n");
        Lab3ProConTest();
        break;

    case 9:
        printf("Lab3 Sync - Barrier:\n");
        Lab3Barrier();
        break;

    default:
	    printf("No test specified.\n");
	    break;
    }
}

