













# 线程调度　实习报告



































**姓名：青宇　　学号：190121$$$$**

**日期：2019/10/13**

<div STYLE="page-break-after: always;"></div>
**目录**

[TOC]

<div STYLE="page-break-after: always;"></div>
## 内容一：总体概述

 承接上一个lab的内容，本次lab还是线程相关的部分，需要在Nachos原有的线程调度机制基础上，进行修改和添加新的调度算法。首先通过调研Linux系统中经典的调度算法，了解其实现方式，接着阅读Nachos源码，理解现有的线程实现机制和具体的调度算法，最后结合以上内容，新增基于优先级抢占的调度算法或者自己感兴趣的有挑战的调度算法。


## 内容二：任务完成情况

任务完成列表

|          | Exercise 1 | Exercise 2 | Exercise 3 | Challenge 1 |
| -------- | ---------- | ---------- | ---------- | ----------- |
| 第一部分 | Y          | Y          | Y          | N           |



**具体Excercise完成情况**

### 第一部分

### Exercise 1　调研

> **Task:** 调研Linux或Windows中采用的进程/线程调度算法

Linux系统中运行的进程可以分成两类：实时进程和非实时进程，它们的主要区别就是通过优先级来区分的。所有优先级值在0-99范围内的，都是实时进程，所以这个优先级范围也可以叫做实时进程优先级，而 100-139范围内的是非实时进程。

Linux允许多个不同的调度算法并存，每个调度算法都有自己可以负责的一些进程，对于普通进程的调度算法是CFS，实时进程也有自己的调度类。

- CFS算法：CFS完全放弃了时间片，而是分配给进程一个处理器比重，通过nice值来计算进程获得处理器的使用权重，CFS为每个进程都引入了获得时间片的底线，称为最小粒度，默认值是1ms
- 实时调度算法和策略
  linux提供了两种实时调度策略，SCHED_FIFO和SCHED_RR，这两种调度算法类总比SCHED_NORMAL被先执行，SCHED_FIFO调度的进程除非被优先级更高的进程抢占，否则一直运行到自己主动退出。SCHED_FIFO是不带时间片的，而SCHED_RR是带时间片的，SCHED_RR里的进程执行完分配给它的时间片就不再运行了。



### Exercise 2 源代码阅读
> Task：　仔细阅读下列源代码，理解Nachos现有的线程调度算法。
>
> - code/threads/scheduler.h和code/threads/scheduler.cc
> - code/threads/switch.s
> - code/machine/timer.h和code/machine/timer.cc

通过阅读源代码，发现Nachos现有的线程调度算法是最最基本的先进先出(FIFO)算法，其实现方式具体有一下几个部分构成：

**１、基础数据结构：list.h/cc**

此块代码定义了一个队列结构，用于维护进程就绪队列，在本次实验涉及到的部分有：

  - **ListElement**: element的构造函数，初始化指针和优先级(key)，后续添加新进程的Append()，Prepend()都是默认把key设置为０；

  - **Append/Prepend**: 插入到队列尾部/头部

  - **SortedInsert**: 按照sortKey为指标把新元素插入有序队列中，本lab中sortKey被赋值为进程的优先级(priority)

    

**2、线程调度器实现：scheduler.h/cc**
  scheduler的主要工作就是在维护进程就绪队列readylist，因为Nachos是单处理器系统，所以只要关闭中断，我们就认为满足互斥条件，就可以操作就绪队列：

  - **ReadyToRun()**: 设置当前进程状态为Ready，并根据具体要求插入ReadyList

  - **FindNextToRun()**: 因为是Nachos的默认算法为FIFO，所以本方法源代码只是取下队头进程并返回；

  - **Run()**: 在进入此方法前，系统默认原来的进程状态已从Running切换为其他状态；其中主要进行的工作有：
    
    - 检查会旧进程是否栈溢出；
    - 改变当前进程指针，即赋值currentThread为nextThread,并设置状态为Running;
    - 调用SWITCH()进行进程切换，其在thread.h中进行了申明，具体定义和ThreadRoot()都在switch.s中;
    - 检查threadToBeDestroyed指针，如果非空，即有执行过Finish()操作的进程需要删除其栈空间，删除；
    
  - **Print()**: 打印ReadyList中的进程名字

    

**３、SWITCH相关：** SWITCH－the machine dependent context switch routine.
  - 需要注意的是，在SWITCH()前后，进程进行了切换，也就是说在Run()内，上下半段以SWITCH()为分界，分别运行在两个进程内；

    

**４、时钟模拟：time.h/cc**
　time.h/cc模拟了计算机的时钟，它每隔Ｘ秒产生CPU中断，所以它可以被用来实现时间片或让某进程睡眠指定时间；

- 变量说明：在stats.h中定义了两个变量：
  - **TimerTicks**：时间片长度，默认设置为100，但如果进程设置doRandom为真的话，取1-2*TimerTick之间的随机值为时间片长度
  - **totalTicks**：系统运行总时间
  
- 在TimeExpired()中调用interrupt的scheduler进行切换，以TimerHandler和时间片长度为参数；

- **OneTick**: 表示一次时钟，其中判断若YieldOnReturn==True,则执行Yield()，两种情况下会调用OneTick方法：
  
    - interrupts are re-enabled
    - a user instruction is executed
  
- **Idle**： 当前无就绪进程时进入此接口，因为必须要有一个进程在运行，才能接受中断；当pendingInterrupt队列中的中断还未到时间时，不断检查；但如果其队列为空，则应该停止系统，因为如果console 或 network在运行的话，pending Interrupt队列不会为空；

  

**5、待补充**

时钟和中断部分的详细理解，等这周做完challenge部分再追加！



### Exercise 3 扩展线程调度算法，实现基于优先级的抢占式调度算法。
>Task：扩展线程调度算法，实现基于优先级的抢占式调度算法。

**总体思路**：抢占的时机应该是当一个进程从其他状态变为Ready状态时，判断与当前运行进程优先级的大熊阿关系，若优先级更高，则进行切换，因此只需在状态转变之后进行判断即可，对于Nahcos就是修改scheduler.cc中ReadyToRun()的操作方式：

**1、增加Thread类的成员变量priority**

- 在thread.h中添加优先级priority的申明；
- 在thread.h新增Thread类的构造函数，新进程Fork()后，设置进程优先级；
  **排序失效可能原因：**　新进程Fork()时，要提前进行初始化，不能等到在执行Fork()中的execve函数时才初始化priority,这时候在ReadyList里的key值不会覆盖啊！
  **解决方案１：**　增加一个Thread()的构造函数，Thread(name, priority),在初始化时就对优先级赋值; 
  **解决方案２：**　在调用Fork()之前先调用setPriority()函数进行优先级赋值！
```c++
    // Lab2: add priority
    int priority;

    //Lab2: add parameter-priority in Thread()
    Thread(char* debugName, int newPriority);		// initialize a Thread 
```
**2、如果想要初始化时就设置优先级，可以增加一个构造函数，创建进程的同时对优先级进行赋值**
- 在thread.cc中定义新的Thread()构造函数，新增对于优先级priority的赋值
```c++
    // Lab2: initialize priority
    Thread::Thread(char* threadName, int newPriority){
        for(int i = 0; i < maxThreadNum; i++) {
            if(!tidFlag[i]) {
                
                // Lab2: initialize priority
                this->priority = newPriority;
                break;
            }
        }
    }
```
**3、改变ReadyList的排序方式**
　原始的Nachos是按照先进先出的方式进行插入的，改变成以优先级为考量参数进行插入，优先级数值越小，级别越高；

- 在scheduler.cc中的ReadyToRun()，更改原有的加入ReadyList的方式，改为按序插入，list类自带有按照key值进行插入的方法SortedInsert()，其中的key值赋值为一个进程的优先级(priority)；
- 注意到scheduler.cc的FindNextToRun()方法中，FIFO算法只是不断从ReadyList头部取下首进程而已，那对于优先级抢占，由于每次插入时已进行排序插入，所以还是一样，直接取队列头即可；

```C++
    // Lab2: preemptive Priority-based algorithm
    // insert the thread according to its priority
    readyList->SortedInsert((void *)thread, thread->getPriority());
```

**测试代码：**
　调用ReadyToRun()的三个时机如下：

- **新进程执行Fork操作时**: 应该在调用Fork()后，在外部判断其优先级是否足以抢占当前运行进程；
  目前可实现得就是新进程Fork()后的优先级抢占，更复杂的需要结合中断实现，后补；
- **当前运行进程执行Yield操作时**：ReadyList中之前已按照优先级排好序，所以选择下一个进程直接从队头摘取就好，不用额外判断，不需要修改代码；
- **进程status从Blocked变为Ready状态时**：需要结合时钟中断；

依次创建三个新进程，其优先级设置为3,2,1，main进程的优先级默认为0；而在每个子进程的execve()函数中，进行优先级的比较，判断是否足以抢占当前进程。




## 内容三：遇到的困难以及解决办法

#### 困难 １：新建进程在何处判断进程是否可抢占？

**错误逻辑：**在ReadyToRun()中把新进程(或队首进程)与currentThread的优先级进行比较，如果currentThread优先级较低，则currentThread进行yield()；(因为当前进程一定比原来ReadyList中所有进程的优先级都要高)
**错误原因：** ReadyToRun()中的Thread参数就是当前进程currentThread啊！所以上面思路逻辑有问题！你需要找到Running进程进行抢占，如何找到running进程？
**解决办法：** 不能在ReadyToRun()中进行切换，因为他还在新建的进程中，应该是从外部进行判断其优先级，如果需要切换，则当前Running进程进行CPU资源让渡；



#### 困难 2：关于底层硬件部分代码的理解

本次实验的主要难点在于理解Nachos中断的机制，自己对于底层与硬件相关的部分，尤其一些汇编代码看起来还比较吃力，这也导致自己challenge部分没有完成，接下来一周，计划在阅读CMU的深入理解计算机课程基础上，完成时间片轮转算法的实现。




## 内容四：收获及感想

本周既调研了Linux和Windows这种成熟系统的进程调度算法，也详细看了XV6和Nachos两种教学操作系统这一模块的底层代码实现，对于各种算法的理解更为透彻，也了解到，当理论算法需要考虑到硬件效果和用户需求时，需要进行一些相应的调整。另一方面，发现和同学的热烈讨论对于理解机制和完成lab有非常大的帮助，有些原本自己苦思冥想不得解的问题，和同学一交流，发现只是某个点不知道或者忽略了某一行注释，这样问题就迎刃而解了；而讨论过程中，因为大家各自不同看法的争辩，有时会产生一种全新的思路，又或者之前有疑惑的部分反倒在讨论另外一个问题时得到了解决，因此对于每周硬性的小组讨论要求，从最初的内心抗拒，也变成了现在的收获满满，所以希望接下来小组能在学习中扮演更加重要的角色吧。



## 内容五：对课程的意见和建议

 同Lab-1线程机制的实验报告



## 内容六：参考文献

[博客：Linux进程调度详解](https://www.cnblogs.com/luxiaolong-lxl/p/process.html)

[CMU-深入理解计算机系统(csapp): Linking](http://www.cs.cmu.edu/~./213/schedule.html)

[Github-Nachos课程操作实验参考](https://github.com/Vancir/os-lab)

[**Nachos Beginner's Guide**](https://www.ida.liu.se/~TDDI04/material/begguide/)

[Tracing and Debugging Nachos Programs](https://users.cs.duke.edu/~chase/cps110-archive/nachos-guide/nachos-labs-13.html#29510)

---
## 补充内容

### Challenge2　多级队列反馈调度算法


**４、时钟模拟：time.h/cc**
　time.h/cc模拟了计算机的时钟，它每隔Ｘ秒产生CPU中断，所以它可以被用来实现时间片或让某进程睡眠指定时间；

- 变量说明：在stats.h中定义了两个变量：
  - **TimerTicks**：时间片长度，默认设置为100，但如果进程设置doRandom为真的话，取1-2*TimerTick之间的随机值为时间片长度
  - **totalTicks**：系统运行总时间
  
- **TimeExpired**：模拟时间片到，时钟中断
  调用interrupt的scheduler，保存当前内容，并调用新的进程？



**5. 中断interrupt.h/cc**

- **PendingInterrupt**
  其实是个很简单的类，定义了中断属性（int值），执行时间，以及执行函数而已

- **SetLevel** 设置中断, 可见开一次中断，时钟计数一次
  在其中先需要判断当前是由谁在调用SetLevel()的，如果是interrupt handler，则不能再进行关中断
  (注释：interrupt handlers are prohibited from enabling　interrupts)
  如果本次是实现开中断，则调用一次OneTick()，　即时钟计数一次



- **OneTick**: 表示一次时钟，其中判断若YieldOnReturn==True,则执行Yield()，两种情况下会调用OneTick方法：
 >interrupts are re-enabled
　a user instruction is executed

  1. 时钟数自增
  2. 关中断，处理到时间的pendingInterupt
  3. 判断是否进行上下文切换（即切换进程？）判断一下yieldOnTurn是否为真
　（yieldOnReturn: 标记从中断返回时是否切换上下文）
    若是先设置machineStatus=SystemMode，然后currentThread->Yield，
    最后返回用户态？（其中old表示进入时的machineStatus(153line)，应该是用户态吧）

  注意:内核态下每次时钟自增10,而用户态自增1(SystemTick=10, UserTick=1)

- **YieldOneReturn**：cause a context switch on return from an interrupt handler
  只是改变yieldOnReturn全局变量，真正的context switch在中断返回时，比如时钟中断是在其内容都执行完，程序最后判断变量yieldOnReturn的状态

- **Idle**： 当前无就绪进程时进入此接口，因为必须要有一个进程在运行，才能接受中断；
  当pendingInterrupt队列中的中断还未到时间时，不断检查；
  但如果其队列为空，则应该停止系统，因为如果console 或 network在运行的话，pending Interrupt队列不会为空；

- **Schedule**：新定义一个pendingInterrupt并加入pendingList
  
- **CheckIfDue**：
  传入的参数advanceClock，说明时钟是否需要增加
  pendingInterruptList是按照时间排序的，所以队首元素为最早会发生的中断
  １. 摘下队首中断，并用when记录其发生时间
  ２. 如果队列为空，则返回Fasle
  ３. 如果advanceClock为真，且取下的中断已经过了执行时间，则更改总时间和idleTick，
  　　看起来总时间里不包含idleTick
　    但是为什么advanceClock不真，即使时间到了，还是不执行，返回False呢？？
    　反倒是时间没到(when＜totalClock)，程序继续往下走了
　４. 下一块判断是否为idleMode（Check if there is nothing more to do, and if so, quit）
  ５. 如果可以运行到最后一部分，则就是执行此中断的handler了

　疑问：所以传入的参数advanceClock到底什么作用呢？


## 实习思路
- 1. 修改scheduler.h/cc
 增加lastSwitchTick,记录上次切换的时间，之后结合totalTicks就可以计算出当前进程运行时间，判断是否超过时间片
- 2. 增加时间片控制函数RRHandler
  其功能很简单，就是判断一下当前进程运行时间是否超过时间片，若超过则切换进程，其实只是设置一下YieldOnReturn，真正的切换在OneTick函数中，OneTick在中断返回时调用
- 3. 在system.cc中新建一个Timer,该类内部会调用scheduler
  还是没懂Timer的模拟内部机制啊
- 4. 在threadtest.cc中调用OneTick(),模拟时钟

---

【实习建议】

**1.**	**数据结构的修改和维护**

​	线程核心管理机制的升级首先依赖于对线程数据结构的修改。例如实现“时间片轮转”线程调度机制时，必须首先在线程数据结构中增加“已使用时间片计数”这样一个变量。同时在“线程创建”、“时钟中断”、“线程切换”等代码中，增加对这个数据成员的维护性代码。

**2.**	**线程管理机制所依赖的细节技术处理**

​	线程管理机制的运行过程非常复杂，实践过程中应把握好关键的处理步骤：

- 时钟中断处理：在Nachos中，时钟中断处理包含了引起线程调度的代码，修改线程调度机制必须调整这部分代码。
- 线程上下文切换：在发生线程切换时，必须妥善保存线程的上下文。由于修改了线程的底层数据结构，因此上下文切换的代码也必须修改。
- 线程调度：这是本实习的核心代码，应认真阅读后再进行修改。

---
#　PS:

- #ifdef & #ifndef 判断该宏定义是否已存在，再决定是否执行接下来的部分

---
SWITCH(thread *t1, thread *t2) 两个变量是依次保存在栈中，因此先弹出t1，保存在eax寄存器中


