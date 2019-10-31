















# 线程机制　实习报告































**姓名：青宇　　学号：190121$$$$**

**日期：2019/10/13**

<div STYLE="page-break-after: always;"></div>
**目录**

[TOC]

<div STYLE="page-break-after: always;"></div>
## 内容一：总体概述

　　本次实验主要内容是结合操作系统课堂上讲解的原理内容，对教学操作系统Nachos的线程机制进行阅读和理解，并在此基础上对其功能进行一定的修改和增加；其中涉及到了进程和线程的相关概念，线程的各种状态及其状态之间的转换，具体需要理解进程创建、资源分配、运行、睡眠以及cpu资源让渡等操作的实现方式。其中的关键点在于理解进程切换的机制。

## 内容二：任务完成情况

任务完成列表

|          | Exercise 1 | Exercise 2 | Exercise 3 | Exercise 4 |      |
| -------- | ---------- | ---------- | ---------- | ---------- | ---- |
| 第一部分 | Y          | Y          | Y          | Y          |      |
|          |            |            |            |            |      |



**具体Exercise完成情况**

### 第一部分

#### Exercise 1 调研
    Task: 调研Linux或Windows中进程控制块（PCB）的基本实现方式，理解与Nachos的异同。
- ##### Linux的进程控制块的实现方式
  
  Linux系统的进程控制块是由task_struct结构体实现的。task_struct是Linux内核的⼀种数据结构，它会被装载到RAM⾥并且包含着进程的信息。每个进程都把它的信息放在 task_struct 这个数据结构⾥，并且可以在include/linux/sched.h ⾥找到它。所有运⾏在系统⾥的进程都以 task_struct 链表的形式存在内核⾥。task_struct 包含了如下内容：
  
  - 进程标识符：在CONFIG_BASE_SMALL配置为0的情况下，PID的取值范围是0到32767，即系统中的进程数最大为32768个；在Linux系统中，一个线程组中的所有线程使用和该线程组的领头线程（该组中的第一个轻量级进程）相同的PID，并被存放在tgid成员中。只有线程组的领头线程的pid成员才会被设置为与tgid相同的值。注意，getpid()系统调用返回的是当前进程的tgid值而不是pid值。
  
  - 进程状态：包含任务状态，退出代码，退出信号等
  
  - 进程调度：包含优先级等
  
    实时优先级范围是0到MAX_RT_PRIO-1（即99），而普通进程的静态优先级范围是从MAX_RT_PRIO到MAX_PRIO-1（即100到139）。值越大静态优先级越低。
  
  - 进程亲属关系远近的内容：包括父子关系，兄弟关系
  
    real_parent是该进程的”亲生父亲“，不管其是否被“寄养”；parent是该进程现在的父进程，有可能是”继父“；这里children指的是该进程孩子的链表，可以得到所有孩子的进程描述符，但是需使用list_for_each和list_entry，list_entry其实直接使用了container_of，同理，sibling该进程兄弟的链表，也就是其父亲的所有孩子的链表。用法与children相似；struct task_struct *group_leader这个是主线程的进程描述符，也许你会奇怪，为什么线程用进程描述符表示，因为linux并没有单独实现线程的相关结构体，只是用一个进程来代替线程，然后对其做一些特殊的处理；struct list_head thread_group;这个是该进程所有线程的链表。
  
  - 进程标记：反应进程状态的信息，但不是运行状态，用于内核识别进程当前的状态，以备下一步操作
  
  - 进程的内核栈：进程通过alloc_thread_info函数分配它的内核栈，通过free_thread_info函数释放所分配的内核栈。
  
  - 其他：包括时间内容，判断标志等等。
  
- ##### Windows的进程控制块的实现方式

    Windows中的进程控制块称为EPROCESS，其中保存有进程的各种信息；线程用ETHREAD对象表示；每个EPROCESS对象中都包含了一个指向ETHREAD结构体的链表。

- ##### Nachos与他们的异同点
    相比于其他系统，Nachos没有维护一个单独的线程控制块，而是把与线程相关的信息作为Thread结构体实例的私有变量。因此相比于其他系统把进程(线程)信息集中放在一个单独的表中，Nachos的线程信息分散在内存的各个地方，也是因为这样，一个指向具体线程的指针就是必须的。



#### Exercise２ 源代码阅读

    Task: 仔细阅读下列源代码，理解Nachos现有的线程机制。
        ◦ code/threads/main.cc和code/threads/threadtest.cc
        ◦ code/threads/thread.h和code/threads/thread.cc

Nachos的进程数据结构定义在thread.h/cc文件中，而Threadtest.cc作为一个测试程序，调用基本的进程程序，进行创建和执行；以下分别对Thread的线程控制块的实现和线程状态之间的转换进行说明。

- **用于Nachos线程管理的TCB**
　Thread作为一个类，其中含有的私有变量可以认为是其线程控制块(TCB)需要保存的各种线程数据，而对外的各种函数接口则是线程可以进行的各种操作；Nachos的TCB中保存有如下数据：
  - **stackTop：**　当前栈指针;			
  - **machineState[MachineStateSize]：**　保存除了stackTop所有的寄存器的值，其中MachineSize大小为18; 
  - **stack：**　栈底（如果为空则表示当前在main进程）
  - **status：** 当前的进程状态
  - **name：**　进程的名字
另外还要分配栈(StackAllocate)、检查栈是否溢出(CheckOverflow)以及进程状态转换的各种函数接口(Fork, Sleep, Yield, Finish)；

- **Nachos的线程状态及其转换**
　Nachos的线程有四个状态：　JUST_CREATED, RUNNING, READY, BLOCKED
  - **JUST_CREATED**：初建状态
    当线程由Thread constructor创建后，但还未分配栈时处于此种进程，只有当它执行Thread::Fork()，才会分配栈空间，并更改为READY状态，然后放入readylist中；
  - **READY**：就绪状态；
    线程调度器(scheduler.cc)中维护一个readylist，保存所有就绪进程；Nachos的初始调度算法是先来先服务(FIFO), 当一个READY进程被调度器选择后，把他从readylist中删除，并更改状态为RUNNING;
  - **RUNNING**：运行状态；
    Nachos允许每个时刻，系统中只能有一个进程处于RUNNING状态，由全局变量指针'currentThread'指向当前运行的进程;
  - **BLOCKED**：阻塞状态；
    通过Thread::Sleep()可以将正在运行的进程进行阻塞，并将其从readylist中删除；如果readylist为空，则会唤醒interrupt->Idle()进程运行，直到下次中断的出现；
  

除了以上内容，Nachos的进程资源释放和进程销毁也值得一说

- **CPU资源的释放(Yield())**
  当RUNNING进程想要释放当前CPU资源时, 会调用Yield(); 而内部实现方法实际是调用scheduler.cc中的FindNextToRun()选择readylist中的一个Ready进程运行，如果readylist中没有其他进程，则会让当前进程继续运行；
  
- **进程销毁(Finish())**: 
当一个进程执行完它的所有操作后，需要销毁他的TCB和栈空间，会调用Finish(); Finish()操作按理说需要删除当前进程的栈，但是自己就是运行在栈中，怎么能删除自己呢？Nachos的实现方式是把指针threadToBeDestroyed指向当前进程，然后对调用Sleep()，即阻塞当前进程；而删除此进程栈空间的任务则交给下一个RUNNING进程（具体实现是在scheduler.cc的Run()接口中）；（可能存在问题）

以下内容还在研究当中，之后随着课程的推进，再来补充
- 线程是如何切换的（switch）
  【待补充】[阅读材料](https://www.ida.liu.se/~TDDI04/material/begguide/roadmap/node13.html#SECTION00041000000000000000)
  
- 为什么Thread类中stackTop和machineState的顺序不能交换？
　只有这样SWITCH才能正常工作！【THEY MUST be in this position for SWITCH to work.】



#### Exercise 3 扩展线程的数据结构

 > Task: 增加“用户ID、线程ID”两个数据成员，并在Nachos现有的线程管理机制中增加对这两个数据成员的维护机制。

 **代码修改部分：** 往Thread类, thread.h中添加成员变量(tid, uid)及其方法，具体见注释；
 ```C++
    // Lab1: add tid & uid
    int tid; //Thread ID
    int uid; // User ID

    // Lab1: setUid, getUid, getTid
    int getUserId() {return (uid);}
    int getThreadId() {return (tid);}
    void setUserId(int u_id) {uid = u_id;}

 ```

 **代码测试部分：**　在Threadtest.c中进行创建新进程，并Fork()，在其execve函数中输出两个成员变量

 ```bash
    tone@stone:/mnt/shared/Nachos/nachos-3.4/code/threads$ ./nachos -q 2
    thread 134534786 (uid=0 tid=0) loop 0 time
    thread 134535276 (uid=731 tid=1) loop 0 time
    thread 134534786 (uid=0 tid=0) loop 1 time
    thread 134535276 (uid=731 tid=1) loop 1 time
    thread 134534786 (uid=0 tid=0) loop 2 time
    thread 134535276 (uid=731 tid=1) loop 2 time
    thread 134534786 (uid=0 tid=0) loop 3 time
    thread 134535276 (uid=731 tid=1) loop 3 time
    thread 134534786 (uid=0 tid=0) loop 4 time
    thread 134535276 (uid=731 tid=1) loop 4 time
    No threads ready or runnable, and no pending interrupts.
    Assuming the program completed.
    Machine halting!
 ```



#### Exercise 4-1 增加全局线程管理机制

    Task: 在Nachos中增加对线程数量的限制，使得Nachos中最多能够同时存在128个线程；

 设置全局变量maxThreadNum为128表示进程数上限，同时用全局变量数组标记某号进程是否已经分配；
 需要在system.h中进行声明，声明为外部变量，而具体定义需要在system.c中进行定义和赋初值；

**代码修改部分**

**１、增加全局变量：** 在system.h中
 增加变量保存线程数并标记具体线程号是否已分配，增加Thread指针指向所有128个线程

```C++
    system.h
    //Lab1: Excercise4
    #define maxThreadNum 128

    // Lab1: Excercise-4
    extern bool tidFlag[maxThreadNum];          
    extern Thread *tidPointer[maxThreadNum];   // point of all Thread
```
**２、初始化全局变量：** 
- 在system.cc中，把进程是否分配的Flag初始值设置为FALSE，进程指针初始值设置为NULL;
```C++
    // Lab1: Ex4 
    for(int i = 0; i < maxThreadNum; i++) {
        tidFlag[i] = FALSE;
        tidPointer[i] = NULL;
    }//for
```
- 在thread.cc中的析构函数~Thread中对上述两个变量赋值为Fasle和NULL；
```c++
    //Lab1: 
    tidFlag[this->tid] = FALSE;
    tidPointer[this->tid] = NULL;
```
**3、分配具体进程号，并判断是否达到上限：**　在thread.h & thread.cc中进行修改
　其中限制进程数上限的思路有两种： 
  - 思路１：设置一个记录当前进程数的全局变量！
  - 思路２：因为已经设置了大小为128的数组标记进程号是否分配，所有用完就输出错误并返回即可；
    问题：在Thread()构造函数中进行判断时，Thread()已经定义并构造出来了啊！ 
　选择思路２的实现方法，在thread.cc中遍历进程标志，若有空，立即分配，如果128个进程分配完，则报错；
```C++
    //thread.cc 的Thread()中
    //Lab1: Excercise4 初始化tidFlag, tidPointer, 并判断是否超过
    for(int i = 0; i < maxThreadNum; i++) {
        if(!tidFlag[i]) {
            tidFlag[i] = TRUE;
            tidPointer[i] = this;
            this->tid = i;
            break;
        }
        if(i == maxThreadNum-1) {
            printf("Failed! There are already 128 threads. No more allowed.\n");
        }
        ASSERT(i != maxThreadNum-1); //abort
    }
```
**代码测试：**　在threadtest.cc中不断申请新的进程，直到报错；

```bash
    name: new thread (tid=120)
    name: new thread (tid=121)
    name: new thread (tid=122)
    name: new thread (tid=123)
    name: new thread (tid=124)
    name: new thread (tid=125)
    name: new thread (tid=126)
    name: new thread (tid=127)
    Failed! There are already 128 threads. No more allowed.
```



#### Exercise 4-2 增加TS命令

    Task: 仿照Linux中PS命令，增加一个功能TS(Threads Status)，能够显示当前系统中所有线程的信息和状态。

**１、增加全局指针变量指向所有进程**　分别在system.h/cc中声明、定义指向128个进程的指针，并在thread.cc中进程初始化时进行赋值。

```C++
    extern Thread *tidPointer[maxThreadNum];   // point of all Thread
    tidPointer[i] = NULL; //system.cc

    tidPointer[i] = this; // thread.cc
```
**2、增加Thread类的新方法TS():** 
　在thread.cc中类似与Fork和Yield等函数接口，增加TS()接口，其内部遍历所有进程标志，输出编号已经被分配进程的状态；

```C++
    //　Lab1: EX4
    //----------------------------------------------------------------------
    // Thread::TS
    // 	print infomation of all threads
    //----------------------------------------------------------------------
    void
    Thread::TS() {
        const char* TSToString[] = {"JUST_CREATED", "RUNNING", "READY", "BLOCKED"};
        printf("Name\tUId\tTId\tThreadStatus\n");
        for(int i = 0; i < maxThreadNum; i++) {
            if(tidFlag[i])
                printf("%s\t%d\t%d\t%s\n", tidPointer[i]->getName(), tidPointer[i]->getUserId(), tidPointer[i]->getThreadId(), TSToString[tidPointer[i]->getThreadStatus()]);
        }
    }
```
**代码测试：**
　分别对创建三个进程并执行Fork，对其中两个进程分别执行Yield(让出CPU资源),Sleep(阻塞)和Finish(结束)，另外创建一个进程但不执行Fork；过程中每次都打印出ReadyList，最后输出所有进程状态，正确情况应该是包含主进程在的五个进程，一个进程销毁，其他三个进程分别处于RUNNING, READY, BLOCKED和JUST_CREATED状态；
```bash
    ---------- Threads Status --------------
    Name	UId	TId	ThreadStatus
    main	0	0	RUNNING
    t1	0	1	READY
    t2	0	2	BLOCKED
    t4	0	4	JUST_CREATED
    ----------- End ------------------------
    No threads ready or runnable, and no pending interrupts.
    Assuming the program completed.
    Machine halting!
```

---



## 内容三：遇到的困难以及解决办法

#### 困难：理解整个Nahcos的运行流程
　等做完Lab回头来看就发现都是比较基础的东西，但是当拿到源码时还是头疼了好一会儿，不知道从何下手。开始着手做实验之后，结合老师给的参考资料，通读了Nachos源码的整体结构，还是有点云里雾里，然后通过阅读网上的资料，了解了其中代码每个模块的作用后，思路就比较清晰了！




## 内容四：收获及感想

- 关于gcc对于文件的编译和链接问题

  通过对于Makefile的学习，以及结合CMU的深入理解计算机系统课程Linking部分，让我对于gcc的编译链接有了更深的理解，也了解了Makefile的基本原理和书写规范。

- 关于操作系统进程的整个运行机制

  之前准备考研时也系统学习了操作系统课程，但是当书本知识真的要转化为实操时，就会发觉原来的各种理解还不够透彻。通过学习和修改真实操作系统的源码，让自己对于脑中一些不清楚或理解有误的知识点变得明晰起来，也坚定了想要学好操作系统，必须吃透底层源码的决心。


## 内容五：对课程的意见和建议
- 评价标准和内容太过多样化，感觉不知道重点应该放在哪一部分，而整个上学期的课程任务还是很重的，不可能把所有时间都放在操作系统这门课上；建议Nachos和XV6两份源代码只研究其中一份，这样学生可以真的吃得比较透彻；
- 对于比较晦涩难懂的知识点（比如硬件相关的部分），建议老师可以在课上现场，结合Nachos源码进行操作演示，这样即使稍微不太明确的地方，下课之后再看看代码，也能想明白了！
	​	

## 内容六：参考文献

[陈皓－和我一起写Makefile](https://blog.csdn.net/haoel/article/details/2886)

[博客：Linux进程控制块解析](https://www.cnblogs.com/33debug/p/6705391.html)

[CMU-深入理解计算机系统(csapp): Linking](http://www.cs.cmu.edu/~./213/schedule.html)

[Github-Nachos课程操作实验参考](https://github.com/Vancir/os-lab)

[Nachos Beginner's Guide](https://www.ida.liu.se/~TDDI04/material/begguide/)

[Tracing and Debugging Nachos Programs](https://users.cs.duke.edu/~chase/cps110-archive/nachos-guide/nachos-labs-13.html#29510)



