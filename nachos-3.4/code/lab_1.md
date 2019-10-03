## 初步逻辑
1、thread/nachos是通过在目录code下运行make命令，从而在thread目录下生成的可执行文件
2、运行nachos相当于执行main.cc程序
3、初始的main.cc程序功能是运行threadtest.cc,在两个进程之间进行切换，并打印出相关信息

---
## Lab1
### Excercise3

结论：所以我现在还不知道Fork(), Yield()等是如何实现的，但是先用着呗！！

- thread类在thread.h中，对于thread修改、添加都在此处声明（如Ex3要求的属性tiu、uid，操作它俩的函数等），而具体的函数定义则在thread.cc中定义

- thread控制块（TCB）在thread.cc中，即Thread的构造函数嘛，对定义的各种属性进行赋值；注意在析构函数中释放已结束的进程

- thread.cc中的定义了Thread的各种函数接口，如fork等；

    看到Fork()有栈分配等函数，猜测：new Thread()后还未进行Fork()的进程输入just_created状态；

- 全局变量需要在system.h中进行声明，声明为外部变量，而具体定义需要在system.c中进行定义和赋初值；

- 最后在threadtest中进行测试，new一个新的Thread，初始化id，并Fork(),然后执行自定义操作！


### Excercise-4-1

实验报告中把两种方法都进行涉及吧！

- 思路1：既然是限制，就是在在申请新进程时进行判断，那应该是在new Thread时还是在Fork()中呢？当然是在new的时候吗，只分配了128个数组元素，用完就返回NULL？当申请新进程返回NULL后就打印失败信息，并退出；
    问题：在Thread()构造函数中进行判断时，Thread()已经定义并构造出来了啊！
    
- 思路2：设置记录当前进程数的全局变量！

所以Github中的lab1-ex4做法并没有进程在后台运行；但其实是满足条件的，因为每个数组本省就标记了当前进程号是否已经分配了；


### Excercise-4-2

- TS()命令为什么不设置为Thread的一个内部命令呢，而是只设置为Thread的测试命令
  尝试自己把TS加入为Thread的内部函数接口！ (已完成，在任何一个进程下，都可以直接调用TS命令)

- ThreadStatus以枚举的方式已经被加入到Thread类中，用0-3四个整数值分别表示四种状态  

- readylist是scheduler类中的变量，保存就绪进程
    FindNextToRun()只是删除readylist中头结点，指向下一个就绪编号即可


- Sleep()操作必须进行关中断的原因，在sleep()前的函数介绍有说（保证操作的原子性）
    //	NOTE: we assume interrupts are already disabled, because it
    //	is called from the synchronization routines which must
    //	disable interrupts for atomicity.   We need interrupts off 
    //	so that there can't be a time slice between pulling the first thread
    //	off the ready list, and switching to it.

    另一方面，即使我不进行Sleep()，进程执行完操作也是变成Blocked状态啊;

---

## 思考题

### Nachos线程理解
[阅读材料](https://www.ida.liu.se/~TDDI04/material/begguide/roadmap/node12.html)


  
- Nachos中Makefile文件的解读
  
- 新增一个源码文件，如何修改Makefile

- 在nachos-3.4/code目录下，执行make操作，主要做了哪些工作？


- 线程管理对应的代码在哪里？Nachos线程的TCB中管理了哪些基本信息？

  在thread.cc中，Thread:Thread()定义了控制块TCB，他是继承了在thread.h中定义的Thread类， 



- Nachos线程有几个状态？他们如何转换的
  线程调度器维护一个readylist，保存就绪进程；
  Nachos线程有四个状态：
  - ready：就绪，当被调度器选择后，把他从readylist中删除，并更改状态为running
  - running：由全局变量指针currentThread指向当前运行的进程
  - blocked：等待外部事件；
    通过Thread::Sleep()进行睡眠；其实执行完操作的进程，如果不进行明确的Finish()操作，都会进人blocked状态?! 实验过程中确实是这样
  - just_created：线程由Thread constructor创建，但还未分配栈；Thread::Fork()将其变成CPU可执行，即放入readylist中
  
- 如何生成一个新线程？线程的栈是如何分配的？
    ```C++
    myThread = new Thread("this is my thread"); //创建对象
    myThread->Fork("functionToRun",0); //Fork子进程
    ```

- 线程是如何切换的（switch）
  [阅读材料](https://www.ida.liu.se/~TDDI04/material/begguide/roadmap/node13.html#SECTION00041000000000000000)


- 线程fork时指定了线程要执行的函数，线程在执行该函数前都做了哪些工作

- 为什么Thread类中stackTop和machineState的顺序不能交换？

#### 线程是如何销毁的
    
- 线程finish的过程是怎么样的？

- 线程结束的时候，其栈空间是如何被释放的？
    thread.cc中，在类Thread的析构函数中进行操作

- 这样是否会存在问题

### Nachos时钟

- Nachos的时钟（Tick）什么时候会前进？如何前进

- 除了OneTick函数之外，还有哪个函数对stats里面的ticks数据做了修改？


### Nachos框架

- Nachos内核由哪几部分组成？
  Nachos3.4的内核，以全局变量形式，定义在system.cc文件中；主要包括中断系统（interrupt）、调度器（schedular）、线程指针（currentThread）、文件系统（fileSystem）、网络（postOffice）；这个文件中还包含一些其他的变量，用来模拟虚拟机的部件，比如时钟（timer）、磁盘（systemDisk）、MIPS处理器+内存（machine)



---
#　ＰＳ:

- #ifdef & #ifndef 判断该宏定义是否已存在，再决定是否执行接下来的部分

