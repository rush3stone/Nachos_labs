## 国庆前 思考题

### Nachos线程理解
[阅读材料](https://www.ida.liu.se/~TDDI04/material/begguide/roadmap/node12.html)


- Nachos中Makefile文件的解读
  
- 新增一个源码文件，如何修改Makefile

- 在nachos-3.4/code目录下，执行make操作，主要做了哪些工作？


４、VoidFunctionPTr的定义
// This declares the type "VoidFunctionPtr" to be a "pointer to a
// function taking an integer argument and returning nothing".  With
// such a function pointer (say it is "func"), we can call it like this:
//
//	(*func) (17);  指向函数的指针，这个函数接受一个整数参数，返回nothing

    ```C++
    typedef void (*VoidFunctionPtr)(int arg); 
    typedef void (*VoidNoArgFunctionPtr)(); 
    ```


### Nachos框架

- Nachos内核由哪几部分组成？
  Nachos3.4的内核，以全局变量形式，定义在system.cc文件中；主要包括中断系统（interrupt）、调度器（schedular）、线程指针（currentThread）、文件系统（fileSystem）、网络（postOffice）；这个文件中还包含一些其他的变量，用来模拟虚拟机的部件，比如时钟（timer）、磁盘（systemDisk）、MIPS处理器+内存（machine)



## Lab 1-2遗留问题；


**中断和时钟相关代码：**

### **system.h/cc - 系统初始化**
- 如何有参数　**-rs**　就会开启时钟中断，randomYield赋值为true;
  之后在创建完statistics, interrupt, scheduler之后，创建一个Timer对象，以TimerInterruptHandler作为Handler；

- TimerInterruptHandler()先判断interrupt->getStatus()是否IdleMode(即MachineStatus),是则调用interrupt->YieldOnReturn()


#### **interrupt.h/cc - 中断程序**

[beginner guide](https://www.ida.liu.se/~TDDI04/material/begguide/)


- PendingInterrupt: 软中断(software interrupt)；为内部程序的中断请求

- Interrupt: 外中断（hardware interrupt）；与硬件底层相关

- Schedule(): 创建新的PendingInterrupt对象，加入Pending队列，以设定的时间when进行排序



#### **疑问**

- thread.h中最后的extern "C" {}中表达的含义是什么？

​    只是ThreadRoot(), SWITCH()函数，具体的定义在switch.s中，汇编代码;是表示直接作为Ｃ代码的一部嵌入其中吗？



