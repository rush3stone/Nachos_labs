# Lab-2 线程调度　实验报告







# 内容一：总体概述

 *【用简洁的语言描述本次**lab**的主要内容；阐述本次**lab**中涉及到的重要的概念，技术，原理等，以及其他你认为的最重要的知识点。这一部分主要是看大家对**lab**的总体的理解。*

​	***要求：*** *简洁，不需要面面俱到，把重要的知识点阐述清楚即可。】*



**查看关于如何追踪和Debug代码的内容：**　[相关链接](https://users.cs.duke.edu/~chase/cps110-archive/nachos-guide/nachos-labs-13.html#29510)



# 内容二：任务完成情况

任务完成列表

第一部分

第二部分

**具体Excercise完成情况**

## 第一部分

### Excercise1　调研

​	***要求：*** *表述清楚即可，* *可采用图表来辅助说明，不需要贴代码】*

**Task:** 调研Linux或Windows中采用的进程/线程调度算法。具体内容见课堂要求。



### Excercise2 源代码阅读

**Task：**　仔细阅读下列源代码，理解Nachos现有的线程调度算法。

- code/threads/scheduler.h和code/threads/scheduler.cc
- code/threads/switch.s
- code/machine/timer.h和code/machine/timer.cc

*【对于阅读代码类的**exercise**，请对其中你认为重要的部分（比如某文件，或某个类、或某个变量、或某个函数……）做出说明。*



#### **scheduler.h/cc - 线程切换**

- 切换策略：（FIFO）

​    The Nachos ***\*scheduling policy\**** is simple: threads reside on a single, unprioritized ready list, and threads are selected in a round-robin fashion. That is, threads are always appended to the end of the ready list, and the scheduler always selects the thread at the front of the list.

- list.h/cc
  - ListElement(): element的构造函数，初始化指针和优先级(key)
    Append(), Prepend()都是默认把key设置为０；
  - Prepend(): 插入到队列头部
  - SortedInsert(): 按照优先级参数sortKey进行并排序

- scheduler.cc 
  只是在维护readylist!
  - 因为是uniprocessor,所以: If interrupts are disabled, we can assume mutual exclusion(互斥)

  - **ReadyToRun()**: 设置当前进程状态为Ready，并把他append在队列末尾
  - **FindNextToRun()**: 因为是FIFO，所以只是取下队头进程并返回
  - **Run()**: （默认原来的进程状态已从Running切换）
  1.检查会旧进程是否栈溢出；2.赋值currentThread为nextThread,并设置状态为Running;3.调用SWITCH()进行进程切换，SWITCH()在thread.h中进行了申明，具体定义和ThreadRoot()都在switch.s中，汇编代码；4.检查threadToBeDestroyed指针，如果非空，即有执行过Finish()操作的进程需要删除其占空间，删除；
    
  - **Print()**: 打印ReadyList中的进程名字

     SWITCH: the machine dependent context switch routine.

  - 需要注意的是，在SWITCH()前后，进程进行了切换，也就是说在Run()内，上下半段以SWITCH()为分界，分别运行在两个进程内；

- switch.s文件

  其中保存有上下文切换的细节，都是汇编代码

  

#### **interrupt.h/cc - 中断程序**

[beginner guide](https://www.ida.liu.se/~TDDI04/material/begguide/)


- PendingInterrupt: 软中断(software interrupt)；为内部程序的中断请求

- Interrupt: 外中断（hardware interrupt）；与硬件底层相关

  

#### **time.h/cc - 时钟**

time.h前的注释明确说了，它每隔Ｘ秒产生CPU中断，所以它可以被用来实现时间片或让某进程睡眠指定时间；

- 时间片长度：

  固定长度定义在stats.h的TimerTick变量中，Nachos初始定为100;实际运行过程中，取１－２*TimerTick之间的随机值为时间片长度

- Idle()接口： 当前无就绪进程时，进入此接口；因为必须要有一个进程在运行，才能接受中断；

​    MachineStatus有三种状态 {IdleMode, SystemMode, UserMode}，分别对应空闲，内核态，用户态；

​    当pendingInterrupt队列中的中断还未到时间时，不断检查；但如果其队列为空，则应该停止系统，因为如果console 或 network在运行的话，pending Interrupt队列不会为空；

#### **疑问**

- thread.h中最后的extern "C" {}中表达的含义是什么？

​    只是ThreadRoot(), SWITCH()函数，具体的定义在switch.s中，汇编代码;是表示直接作为Ｃ代码的一部嵌入其中吗？



### Excercise3 线程调度算法扩展

**Task：**扩展线程调度算法，实现基于优先级的抢占式调度算法。

**要求：**对于要编程实现的**exercise**，请对你增加或修改的内容作出说明。如果增加或修改了某个类，请写出这个类写在那个文件中，它的的功能是什么，你自己添加或修改的成员变量以及成员函数有哪些，它们的功能是什么；特别的，对于复杂的函数，请说明你的实现方法。不需要贴具体的实现代码。*

#### **基于优先级的抢占式算法**
所以只需读懂scheduler.h/cc就能实现基于优先级的抢占式算法？！switch.s中的汇编代码直接调用即可

- Thread类中增加变量：priority - 优先级
  那每次创建进程时，进程的优先级如何设置呢？execve()貌似只能传入一个参数，之前都是传入which,即表示tid; 
  那其实可以把priority传入，tid会在new Thread时自动赋值

- Ready队列中的进程需要按照优先级进行排序
  List已有接口SortedInsert(); 每次要把一个进程变为Ready状态时，即执行ReadyToRun()时
  把新进程用List::SortedInsert()插入ReadyList中；

- 判断是否抢占的时机(即何处调用ReadyToRun())：
  - １、Fork(): Fork()中是在本进程内，应该在调用Fork()后，在外部判断；
      目前可实现得就是新进程Fork()后的优先级抢占，更复杂的需要结合中断实现，后补；
  - ２、Yield()：ReadyList中已按照优先级排好序，直接从头摘取就好，不用额外判断；
  - 3、进程status从Blocked变为Ready状态时：需要结合中断，之后补充;
  
- **测试代码**
  - main Thread的优先级默认是０，可以用setPriority()进行修改，但是只有在它执行Yield()并重新进入ReadyList后才能生效；
  - 创建三个子进程，并在main中查看是否可抢占；
  

- 新进程Fork()后，进程优先级，即ReadyList的排序错误，为什么？？
  **原因：**　新进程Fork()时，要提前进行初始化，不能等到在执行Fork()中的execve函数时才初始化priority,这时候在ReadyList里的key值不会覆盖啊！
  **解决方案１：**　增加一个Thread()的构造函数，Thread(name, priority),在初始化时就对优先级赋值; 
  **解决方案２：**　在调用Fork()之前先调用setPriority()函数进行优先级赋值！



  

  注意scheduler.cc中FindNextToRun()功能，FIFO算法下只是不断从ReadyList头部取下首进程而已，那对于优先级抢占，由于每次插入时已进行排序插入，所以还是一样，直接取队列头即可；



## 第二部分

### Challenge1　时间片轮转算法

读懂time.h/cc的原理

time.h前的说明文件明确说了，它每隔Ｘ秒产生CPU中断，所以它可以被用来实现时间片或让某进程睡眠指定时间；

### Challenge2　多级队列反馈调度算法





# 内容三：遇到的困难以及解决办法

### 困难１：在何处判断进程是否可抢占？

**错误逻辑１**　在ReadyToRun()中把新进程(或队首进程)与currentThread的优先级进行比较，如果currentThread优先级较低，则currentThread进行yield()；(因为当前进程一定比原来ReadyList中所有进程的优先级都要高)
**原因**: ReadyToRun()中的Thread参数就是当前进程currentThread啊！所以上面思路逻辑有问题！你需要找到Running进程进行抢占？如何找到running进程？
  应该在调用ReadyToRun()的地方进行操作　**还是有问题**，比如下面情况１
  １、Fork()之后进行修改!　也不对，Fork()中也是在本进程内，应该在调用Fork()的地方
      因此目前可实现得就是新进程Fork()后的优先级抢占，更复杂的需要结合中断实现
  ２、Yield()虽然也调用了，但直接从ReadyList摘取就好，不用额外调用
  3、进程状态从Blocked变为Ready状态时，会调用ReadyToRun()，但还没做到那，之后补充;


### 困难２

啦啦啦



# 内容四：收获及感想

*【自己的收获，任何关于实习的感想，可以是技术方面的或非技术方面的，可随意发挥。*

 ***要求：****内容不限，体裁不限，字数不限，多多益善，有感而发。】*



# 内容五：对课程的意见和建议

 ……

​	*【请写下你认为课程需要改进的地方，任何方面，比如进度安排、难易程度、课堂讲解、考核方式、题目设置……甚至如果你认为源代码哪里写得不好也欢迎提出。*

​	*各位同学反馈的信息对课程建设会有极大帮助。】*



# 内容六：参考文献

 *【我们希望大家不要使用复制粘贴来拼凑你的报告。****详细****地列出你在完成**lab**的过程中引用的书籍，网站，讲义，包括你咨询过的大牛们。*

 	***要求：****诚实，格式尽量按照论文的要求，请参考“论文参考文献格式**.doc**”**】*

   

