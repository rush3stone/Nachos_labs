













# 虚拟内存　实习报告































**姓名：青宇　　学号：1901210$$$**

**日期：2019/11/01**

<div STYLE="page-break-after: always;"></div>
**目录**

[TOC]

<div STYLE="page-break-after: always;"></div>
## 内容一：总体概述

　　本次实验主要内容是结合操作系统课堂上讲解的原理内容，对教学操作系统Nachos的虚拟内存部分进行阅读和理解，并在此基础上对其功能进行一定的修改和增加；其中涉及到了地址空间，虚拟地址到物理地址的转换，TLB缓存以及物理空间管理方式等相关概念，需要实现多线程支持和按需调页，最后的Challenge部分还需要实现进程状态的增加和倒排页表管理内存，整个lab的任务比较有挑战性．

>  本实习希望通过修改Nachos系统平台的底层源代码，达到“实现虚拟存储系统”的目标。
>
> 【背景描述】
>
>  目前，Nachos系统所实现的内存管理模块中，虽然存在地址转换机制，但是却没有实现真正的虚拟内存。
>
>  Nachos系统的内存分配必须在用户程序载入内存时一次性完成，故此，系统能够运行的用户程序的大小被严格限制(32 * 128 B = 4KB，在Nachos系统中物理内存的页面大小PageSize为128 B，而每个用户程序最多只能获得32个物理页面，一旦超出则将导致系统出错)。
>
>  但是现代操作系统对内存管理的要求是支持多道程序，并且使得对任一程序而言，可用的内存空间应当是无限的，即需实现虚拟内存。同时还要提供存储保护机制。
>



## 内容二：任务完成情况

任务完成列表

| 第一部分        | Exercise 1      | Exercise 2      | Exercise 3      |
| --------------- | --------------- | --------------- | --------------- |
|                 | Y               | Y               | Y               |
| **第二部分**    | **Exercise 4**  | **Exercise 5**  | **Exercise 6**  |
|                 | Y               | Y               | Y               |
| **第三/四部分** | **Exercise ７** | **Challenge 1** | **Challenge 2** |
|                 | Y               | N               | N               |




**具体Exercise完成情况**

### **第一部分  TLB异常处理**

### Exercise 1 源代码阅读

  > - 阅读code/userprog/progtest.cc，着重理解nachos执行用户程序的过程，以及该过程中与内存管理相关的要点。
  > - 阅读code/machine目录下的machine.h(cc)，translate.h(cc)文件和code/userprog目录下的exception.h(cc)，理解当前Nachos系统所采用的TLB机制和地址转换机制。


- 一些初始化的定义(machine.h中)

   ```C++
   #define SectorSize 	128	// number of bytes per disk sector
   
   #define PageSize SectorSize // set the page size equal to the disk sector size, for simplicity
   #define NumPhysPages    32
   #define MemorySize 	(NumPhysPages * PageSize)
   #define TLBSize		4		// if there is a TLB, make it small
   ```

   由此可知内存总大小`MemorySize=32x128B=4KB`

- **userprog/progtest.cc**

   运行用户程序从main.cc中开始，112行判断是否使用`-x`参数，若使用则调用`StartProcess`, 而`StartProcess`定义在`protest.cc`中，主要完成以下步骤（新建进程并执行传入的程序(二进制文件形式)）

   - 打开文件，加载代码 `FileSystem::Open()`

   - 申请地址空间，并关闭文件

   - 初始化寄存器，并加载页表内容

   - 运行程序`machine->Run()` (具体实现在mipssim.cc中)　

     **疑问：**　注释说`Run()`永远不会返回：是通过异常（中断）直接重新调度吗？

     其中的程序运行代码: **machine->run()**其实逻辑很简单，就是１设置为用户态２循环调用指令OneInstruction()，每次指令结束通过interrupt->oneTick()计时，并在其中检查中断．
     
     **OneInstruction()**的实现里分别进行指令读取(ReadMem())，解析执行(`instr->Decode()`)，然后计算下条指令地址，即`PC` 地址+4，但是当产生异常时，不会执行`PC`+4的操作，`PC`还是指向原来的指令，这样每次有异常产生时，处理完异常，Nachos会执行原来的指令；（比如缺页异常时，我们只需把缺页调入TLB，Nachos会重新执行该指令并解析该虚拟地址）（但是对于系统调用，必须自己添加`PC+4`的操作啊，不然不就一直循环执行这一个系统调用了嘛！）




- **machine/machine.h(cc)**

   进行各种硬件的初始化工作（定义了虚拟和物理页大小，TLB大小，各种异常的序号，各种寄存器的编号）

   - **CheckEndian()**:检查一下当前的编码方式是否和所声称的一致

   - **Machine():** 初始化内存和寄存器值，若使用TLB的话，也需要初始化，不过也要设置USE_TLB宏（MakeFile中）

   - **RaiseException():** 通过抛出异常，进行中断响应处理

     

- **machine/translate.h/cc**
  **功能：**　实现虚拟地址到物理地址的转换

  translate.h中：

  - **TranslationEntry：** (页表项)即是普通内存中页的页表项结构，也是TLB中页的页表项结构
  定义了虚拟页到物理也的映射，以及各种标记;

  trainslate.cc中包含4块；

  - 大小端编码方式的转换　(37-71)

  - Machine::ReadMem()： 读内存

  - Machine::WriteMem()： 写内存

    读写内存时会调用Translate()，进行地址转换，若地址转换中产生异常则抛出(其中会进入SystemMode)，否则按大小读写内存；

  ```C++
  exception = Translate(addr, &physicalAddress, size, FALSE);
  if (exception != NoException) {
  	machine->RaiseException(exception, addr);
  	return FALSE;
  }
  ```

  - **Machine::Translate()：** 实现虚拟地址到物理地址的转换，主要流程如下：

    - 检查数据是否对齐以及**TLB或页表至少存在其一**　(196-204)
    - 计算页号和偏移量

    ```C++
     vpn = (unsigned) virtAddr / PageSize;
     offset = (unsigned) virtAddr % PageSize;
    ```

    - 根据是否使用TLB分别对页表和TLB进行查找　(211-234)

      不是并行执行，而是根据ifelse判断分别处理；另外，原始的Nachos代码当在TLB中查找失败时，只是返回PageFaultException，而没有继续在页表中进行查询，Exercise2需要对其实现机制进行修改．

    - 检查是否发生写只读部分或页号越界等错误

    ```C++
    if (entry->readOnly && writing) {...}
    pageFrame = entry->physicalPage; //赋值物理页框号
    if (pageFrame >= NumPhysPages) {...}
    *physAddr = pageFrame * PageSize + offset;　//得到物理地址
    ```

    - 对页表项的各个标记进行修改，如是否use，是否write等




- **userprog/exception.h(cc)**

   定义了ExceptionHandler()，处理各种异常．Nachos的原始代码只处理`SC_Halt`类型的系统调用，所以需要自己添加对于其他异常的处理函数．

   ```C++
   int type = machine->ReadRegister(2); //从特定寄存器获取系统调用的类型
   ```

   


- **userprog/addrSpace.h(cc)**

  - **AddrSpace()**: 创建地址空间，加载程序，初始化上下文 (set up the translation from program memory to physical memory. )

    - 读取源代码文件，并进行大小端不同编码方式的转换(lines 68-72)

      >  Comments:  "executable" is the file containing the object code to load into memory

    - 判断需要的地址空间大小，若大于整个物理空间则报错 (lines 74-87)

      因此也就说明，所有页都在内存中，当进行虚拟地址转换且搜索页表时，不会发生`PageFault`；当然，由于Nachos下`TLB`搜索还是有可能报`PageFaultException`的．

      > Comments: For now, this is really simple (1:1), since we are only uniprogramming, and we have a single unsegmented page table

    - 初始化页表项 (lines 88-99)

      在把新的进程的code&data放入内存之前，会把所有内存清空（所以表示每一时刻只有单线程存在于内存？）(line 103)

      ```c++
      bzero(machine->mainMemory, size);
      ```

      

    - 把代码和初始化数据复制入内存（未初始化数据默认赋值为0即可）(lines112-124)

      ```C++
      if (noffH.code.size > 0) {...}
      if (noffH.initData.size > 0) {...}
      ```

      


  - 下面的InitRegister, SaveState, RestoreState分别进行上下文的初始化，保存和重新加载

  

- **bin/noff.c**
- 基本的分区结构`segment`
  
- 基本的用户栈结构`noffHeader`，包括code,初始化data，未初始化data(使用前默认为0)



以下内容还在研究当中，之后随着理解的深入，再来补充
- addrspace.cc中(88-99)处初始化页表项的各种标记，所以是在说每个时刻，内存中只有一个进程，从而也就只有一个页表？
  



### Exercise 2 TLB MISS 异常处理

 > Task: 修改code/userprog目录下exception.cc中的ExceptionHandler函数，使得Nachos系统可以对TLB异常进行处理（TLB异常时，Nachos系统会抛出PageFaultException，详见code/machine/machine.cc）

实验对应的代码部分在translate.cc中222-234部分，此段先搜寻TLB，匹配vpn(virtual page number)，如果找到则直接返回，如果未找到，返回PageFaultException．

正确的TLB异常处理应该是当TLB发生PageFault，如果内存中有该页，将内存中的该页调入TLB（如果内存中也不存在，则需要从磁盘调页，具体在Exercise 6中实现）



**代码修改部分：** exception.cc中的ExceptionHandler函数

> **疑问：** 怎么区分TLB失效和页表失效呢？
>
> **答：** 此处先不用考虑页表失效的情况（Exercise 6实现），因为Nachos单进程机制下会把程序的所有代码和数据都加入内存(addrspace.cc lines74-87)，所以不会出现页表失效的情况；Ex2实现从内存获取页面并写入TLB的功能即可；需要区分TLB是否有空，有空则直接写入，没空则用某种置换算法进行换页（Exercise 3）

**解题思路：** 在产生`PageFaultException`时，对应的虚拟地址会一并传入`ExceptionHandler`，计算其对应的物理页号和偏移，然后直接在页表中进行查找匹配即可；

**１．更改userprog/Makefile**

如果要使用TLB，必须定义宏`USE_TLB`

```C++
DEFINES = ... -DUSE_TLB
```

**２．更改translate.cc，使TLB和页表可以共存**

 ```C++
// we must have either a TLB or a page table, but not both!
// ASSERT(tlb == NULL || pageTable == NULL);	
ASSERT(tlb != NULL || pageTable != NULL);	//At least one of them exists
 ```

**３．修改exception.cc，增加处理PageFaultException的Handler**

产生异常的虚拟地址保存在`BadVAddrReg`寄存器中(在machine.h的line60-70定义了一组特殊的寄存器)，可以通过它得到该虚拟地址；

 ```C++
// Lab3 Exercise2 & 3
if(which == PageFaultException) {
	if(machine->tlb==NULL){ //如果TLB为空，说明是没有启用LTB,此Exception由页表失效产生
        //由于Nachos默认会把程序的所有代码和数据都加入内存，所以不会出现查询页表发生PageFault的情况；
        ASSERT(FALSE); //报错
    }else{//
        int BadVAddr = machine->ReadRegister(BadVAddrReg);
        TLBHandler(BadVAddr);
    }
}
 ```

在`TLBHandler()`中处理TLB失效的异常：

```C++
int TLBreplaceIdx = 0; //TLB pointer, use for CLOCK method

void
TLBHandler(int VirtAddr) {
    unsigned int vpn;
    vpn = (unsigned)VirtAddr / PageSize; //获取页号
    //获取页面内容
    TranslationEntry phyPage = machine->pageTable[vpn];

    //machine.h中定义TLBSize为４
    machine->tlb[TLBreplaceIdx] = phyPage;
    TLBreplaceIdx = (++TLBreplaceIdx) % TLBSize; // 循环搜索
}
```

**４．测试TLBHandler()**

直接调用在userprog/目录下使用命令`./nachos -x ../test/matmult`调用，会报错，应为所需空间超过4KB，修改程序，使得其变为10维矩阵相乘，则运行结果如下，即TLB有效！

```bash
stone@stone:/mnt/shared/Nachos/nachos-3.4/code/userprog$ ./nachos -x ../test/matmult -d
== Tick 91849 ==
	interrupts: on -> off
Time: 91849, interrupts off
Pending interrupts:
End of pending interrupts
	interrupts: off -> on
Reading VA 0x20, size 4
	Translate 0x20, read: *** no valid TLB entry found for this virtual page!
Exception: page fault/no TLB entry
=> TLB Miss, Page Fault.
we put page 0 at TLB position 0
...
Shutdown, initiated by user program.
Machine halting!

Ticks: total 91866, idle 0, system 10, user 91856
Disk I/O: reads 0, writes 0
Console I/O: reads 0, writes 0
Paging: faults 0
Network I/O: packets received 0, sent 0
```



---



### Exercise 3 置换算法

> Task: 为TLB机制实现至少两种置换算法，通过比较不同算法的置换次数可比较算法的优劣。

 当TLB失效，且其中没有空闲位置时，对TLB进行置换，选择FIFO和CLOCK算法

- **增加记录TLB命中率的变量：** 
   在`machine.h`中申明变量`translateCount`和`TLBMissCount`，并在`translate.cc`中初始化，分别记录总的地址转换次数和TLB Miss的次数

```C++
int translateCount = 0;  // total addr translation time
int TLBMissCount = 0; // TLB miss time
```
​		在`translate.cc`的`ReadMem()`, `WriteMem()`中，当发生缺页异常时，`TLBMissCount`自增，在`translate()`中，每次地址解析都自增`translateCount`；

```c++
if (exception != NoException) {
	...
    TLBMissCount++;  //Lab3: use to compute TLB Hit rate　(ReadMem/WriteMem)
    ...
}
translateCount++; //Lab3: use to compute TLB Hit rate　(translate)
```

​		最后在执行`exception.cc`中的系统调用`halt`时，进行`LTB Hit Rate`的计算；其中DEBUG标记为`DEBUG('T')`，方便之后追踪；

```C++
if(type == SC_Halt){
    PrintTLBStatus(); // TLB Hit Rate
}
...
void
PrintTLBStatus(){
#ifdef USE_TLB
    DEBUG('T', "TLBSize=%d, TLB Miss: %d, TLB Hit: %d, Total Translation: %d, TLB Miss Rate: %.2lf%%\n", TLBSize, TLBMissCount, translateCount-TLBMissCount, translateCount, (double)(TLBMissCount*100)/(translateCount));
#endif
}
```



#### **(一) FIFO置换**

- 在`exception.cc`中，修改`TLBHandler()`，增加分支
```C++
#if TLB_FIFO
    TLBasFIFO(phyPage);
#elif TLB_CLOCK
    TLBasClock(phyPage);
#else //test Lab3 Ex2　
    macine->tlb[TLBreplaceIdx] = phyPage;
    TLBreplaceIdx = (++TLBreplaceIdx) % 4; //TLBSize is 4
#endif
```
- 实现TLBasFIFO置换算法：如果存在空闲则直接存入，若全满，则替换首页，暂时不考虑是否被修改的问题；
```c++
#ifdef TLB_FIFO
void
TLBasFIFO(TranslationEntry page) {
    int TLBreplaceIdx = -1;
    for(int i = 0; i < TLBSize; i++) { //查找空闲页
        if(machine->tlb[i].valid == FALSE) {
            TLBreplaceIdx = i;
            break;
        }
    }
    if(TLBreplaceIdx == -1) { //所有页向前移动一位，即tlb[0]处保存的页被覆盖
        TLBreplaceIdx = TLBSize - 1;
        for(int i = 0; i <= TLBSize-2; i++) {
            machine->tlb[i] = machine->tlb[i+1];
        }
    }
    machine->tlb[TLBreplaceIdx] = page; //更新
}    
#endif
```


#### **(二) CLOCK置换**

-  结合页表项中记录`use`，实现时钟循环置换算法

```C++
void
TLBasClock(TranslationEntry page) {
    while(1){
        TLBreplaceIdx = TLBreplaceIdx % TLBSize;
        if(machine->tlb[TLBreplaceIdx].valid == FALSE) { //空，直接加入
            break;
        } else{
             if(machine->tlb[TLBreplaceIdx].use) {　//已使用,更改标记，扫描下一页
                machine->tld[TLBreplaceIdx].use = FALSE;
                TLBreplaceIdx++;
            }else{　//找到，替换
                break;
            }
        }
    }//while
    machine->tlb[TLBreplaceIdx] = page;
    machine->tlb[TLBreplaceIdx].use = TRUE; //标记已使用
}
#endif
```
**代码测试：**　分别用两种置换算法运行同一个程序，输出缺页率进行对比；

> 注：Debug对应参数： `S` - syscall, `T` - TLB, `t` - thread, `M` - Memory

```bash
stone@stone:/mnt/shared/Nachos/nachos-3.4/code/userprog$ ./nachos -x ../test/matmult -d T
# FIFO
TLBSize=4, TLB Miss: 8630, TLB Hit: 104664, Total Translation: 113294, TLB Miss Rate: 7.62%
Machine halting!

# CLOCK
TLBSize=4, TLB Miss: 8779, TLB Hit: 104900, Total Translation: 113679, TLB Miss Rate: 7.72%
Machine halting!
```

**遗留问题：**

- 为什么两种置换算法的TLB失效率相差不大呢？

  是因为矩阵相乘的原因吗？





---

### 第二部分 多页式内存管理

> 背景：目前Nachos系统中，类Class Thread的成员变量AddrSpace* space中使用TranslationEntry* pageTable来管理内存。应用程序的启动过程中，对其进行初始化；而在线程的切换过程中，亦会对该变量进行保存和恢复的操作（使得类Class Machine中定义的Class Machine::TranslationEntry* pageTable始终指向当前正在运行的线程的页表）。



### Exercise 4 内存全局管理数据结构 (BitMap)

> Task: 设计并实现一个全局性的数据结构（如空闲链表、位图等）来进行内存的分配和回收，并记录当前内存的使用状态。

**解题思路：**

​	Nachos已经实现了BitMap数据结构，直接使用即可！

​	以物理页为单位进行内存分配，`Nachos`中`NumPhysPages=32`，故只需32bit就能记录所有内存情况；

**解决步骤：**

- 在`userprog/MakeFile`中增加宏定义`USE_BITMAP`

  ```makefile
  DEFINES = -DUSER_PROGRAM ... -DUSE_TLB -DUSE_BITMAP
  ```

- 在`machine.h/cc`中分别申明和定义`bitmap1`，考虑到之后文件系统部分可能还要使用`BitMap`，这里命名为`bitmap1`；

  ```c++
  #ifdef USE_BITMAP
  	BitMap *bitmap1;  //(machine.h) 
  	bitmap1 = new BitMap(NumPhysPages); //(machine.cc)
  #endif
  ```
  
- 在`addrspace.cc`中，修改物理页的分配方式，增加分支；

  ```C++
  AddrSpace::AddrSpace(OpenFile *executable){
      for (i = 0; i < numPages; i++) {
  #ifdef USE_BITMAP  // Lab3 Exercise-4： BitMap 
          pageTable[i].physicalPage = machine->bitmap1->Find();
  #else
          pageTable[i].physicalPage = i;
  #endif //end BITMAP 
      }//for
  	....
  #ifdef USE_BITMAP
      machine->bitmap1->Print(); //print out the BitMap status
  #endif
     
  }//AddrSpace
  ```

- 在`exception.cc`中增加系统调用`SC_Exit`的判断分支，并在其中判断是否启用`BitMap`并释放内存空间；

  ```C++
  #if USE_BITMAP 
        machine->bitmap1->AllClear(); //call Clear() numBits times!
  #endif
  ```

- **纠错：** 上述方式只能针对单进程，当同时有多个进程在内存中运行时，显然不能把输入其他人的`bits`也给删除了，所以需要针对地址空间单独构造一个释放`BitMap`的函数！

  > 小组讨论－尧帝思路：直接用machine->addrspace调用，进而清楚地址空间即可，不需要单独创建一个函数接口（代码待修改）
  
  ```c++
  #if USE_BITMAP 
        currentThread->space->ClearBitMap();
  #endif
  // addrspace.cc
  #ifdef USE_BITMAP
  void AddrSpace::ClearBitMap()
  {
      for(int i = 0; i < numPages; i++) {
          machine->bitmap1->Clear(pageTable[i].physicalPage);
      }
  }
#endif
  ```
  
  



**代码测试：** 增加`BitMap`类中相关的`DEBUG`语句，设置参数为`B`；修改`matmult.cc`中矩阵维度为`2`，运行！

```bash
stone@stone:/mnt/shared/Nachos/nachos-3.4/code/userprog$ ./nachos -x ../test/matmult -d BT
Clear page 0 in BitMap.
...
Clear page 31 in BitMap.
Set page 0 in BitMap.
Set page 1 in BitMap.
Set page 2 in BitMap.
Set page 3 in BitMap.
...
Set page 15 in BitMap.
Bitmap set:
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 
-----------------------------------------
=> TLB Miss, Page Fault.
...
=> TLB Miss, Page Fault.
TLBSize=4, TLB Miss: 38, TLB Hit: 1166, Total Translation: 1204, TLB Miss Rate: 3.16%
Clear page 0 in BitMap.
...
Clear page 31 in BitMap.
No threads ready or runnable, and no pending interrupts.
Assuming the program completed.
Machine halting!

Cleaning up...
```

**遗留问题：**　

- 系统调用`SC_Exit`中考虑不够周全，此处仅为了测试，结合Exercise 5需要进行优化；

**Bug后记：**

- BitMap怎么都不起作用的原因？

  居然就是一句申请空间的问题！

  ```c++
  bitmap1 = new BitMap(NumPhysPages);
  //bitmap1 = &BitMap(NumPhysPages);  //这是什么鬼!!!!
  ```

  

---



### Exercise 5 多线程支持

> Task：目前Nachos系统的内存中同时只能存在一个线程，我们希望打破这种限制，使得Nachos系统支持多个线程同时存在于内存中。

保存上下文切换的数据值，具体定义在`thread.h`的(143-155)

from the comments: 一个运行用户程序的进程是有两组寄存器的，分别对应用户态和内核态

// A thread running a user program actually has *two* sets of CPU registers: one for its state while executing user code, one for its state while executing kernel code.

> 背景：初始的Nahcos，每次加入新进程前，都会先判断新进程的总大小，若比物理内存小，则清空内存，然后全部装入！



---

### 后补：

　每次`addrspace.cc`中最后还是都会清空内存啊！所以并没有实现多进程在内存中，只是实现了BitMap的分配而已！也即BitMap的存在只能说明进程还没有被删除，但并不是都在内存，而是在disk上！

目前来看，只是在初始化地址空间时，会清空`mainMemory`，但是后续切换时算不算移出内存了呢？

**所以：**　至少应该在`addrspace.cc`中内存的赋值处有修改，否则和原来并没有本质区别！Word版的报告有这种意思，但还是说得不够清晰！

---



#### Nachos中多进程原理梳理

**用户进程上下文：**

- Nachos中，一个用户进程的上下文包括：

  专门针对`user program`的一组寄存器（可以看作是用户栈？No!）和地址空间

  ```C++
  //寄存器＆地址空间 (thread.h - line148 & 154)
  int userRegisters[NumTotalRegs];	// user-level CPU register state
  AddrSpace *space;			// User code this thread is running.
  
  //地址空间的内容（上面space的属性） (addrspace.h - lines35-37)
  TranslationEntry *pageTable;	// Assume linear page table translation for now!
  unsigned int numPages;		// Number of pages in the virtual address space
  ```

  

- 用户进程上下文的**保存和恢复**：

  寄存器部分：申明在`thread.h`中 `lines143-155`，并在`thread.cc`中定义；

  ```C++
  void SaveUserState();		// save user-level register state
  void RestoreUserState();	// restore user-level register state
  ```

  地址空间（内存状态）部分，定义在`addrspace.cc`中`lines 31-32`

  ```c++
  void SaveState();			// Save/restore address space-specific
  void RestoreState();		// info on a context switch 
  ```

- 用户进程切换时：在 `scheduler.cc`中

  `lines 101-106`　保存被切换进程的上下文

  ```c++
  if (currentThread->space != NULL) {	// if this thread is a user program,
  	currentThread->SaveUserState(); // save the user's CPU registers
  	currentThread->space->SaveState();
  }
  ```

  `lines 135-140`  恢复将要运行进程的上下文

  ```c++
  if (currentThread->space != NULL) {		// if there is an address space
  	currentThread->RestoreUserState();     // to restore, do it.
  	currentThread->space->RestoreState();
  }
  ```



**疑问：**　`machine->Run()`永远不会返回，那么进程到底是如何结束的呢？

​		下面的解释只是说明了系统调用Exit()会释放空间，但是Exit()是只能由用户显式地调用吗？

​		我的理解：　`Run()`中会执行`OneInstriction()`和`OneTick()`，如果其中发生异常，自然会切换进程；如果指令都是正常执行，那在`OneInstruction()`中，执行到最后，指令取完，自然会报错，进而切换进程！

​		可是我看了`OneInstruction()`，当取指令出错时，它只是直接返回了啊？！！！

**结束用户进程：**

- 由注释可得，`machine->Run()`不会返回，那么进程的结束和内存空间释放在什么时候进行呢？

  在用户进程结束时(return)执行`Exit`系统调用，在`Exit()`中操作

  由`exception.cc`中注释(`lines36-45`)可知进行系统调用时各个参数和返回值对应的寄存器；

  ```c++
  // 	For system calls, the following is the calling convention:
  //
  // 	system call code -- r2
  //		arg1 -- r4
  //		arg2 -- r5
  //		arg3 -- r6
  //		arg4 -- r7
  //
  //	The result of the system call, if any, must be put back into r2. 
  //
  // And don't forget to increment the pc before returning. (Or else you'll
  // loop making the same system call forever!
  ```

写一个单独处理`systems call: Exit`的函数，结束进程并释放内存空间；

```c++
//Exit syscall
void ControlAddrSpaceWithExit(int type) {
    if(type == SC_Exit) {
        PrintTLBStatus(); //Print out TLB current status

    //由line36注释：4号寄存器保存传入的第一个参数，对于Exit来说，正常退出，会传回状态０
    int status = machine->ReadRegister(4);
    if(status == 0) {
        DEBUG('S', "User Program Exit Correctly(status 0)\n");
    } else {
        DEBUG('S', "User Program Exit With Status %d\n", status);
    }
	...
        currentThread->Finish(); //结束进程
    }//if
}//ControlAddrSpaceWithExit
```



#### 多进程实现

**整体思路：**　在progtest.cc中改变进程的启动方式，原来的StartProcess()是启动单进程，自己新建一个函数，在其中创建并运行多个进程；

- １．为了方便查看内存空间的当前状态，新增一个函数，输出地址空间的状态

```c++
//addrspace.h
//Lab3: print out address space-specific
void PrintState();

//addrspace.cc
void AddrSpace::PrintState()
{
    printf("======== addrspace information ==========\n");
    printf("numPages = %d\n", numPages);
    printf("VPN\tPPN\tvalid\treadOnly\tuse\tdirty\n");
    for(int i = 0; i < numPages; i++) {
        printf("%d\t", pageTable[i].virtualPage);
        printf("%d\t", pageTable[i].physicalPage);
        printf("%d\t", pageTable[i].valid);
        printf("%d\t", pageTable[i].readOnly);
        printf("%d\t", pageTable[i].use);
        printf("%d\t", pageTable[i].dirty);
    }
#ifdef USE_BITMAP
    machine->bitmap1->Print();
#endif
    printf("=========================================\n");
}
```



- 2．在progtest.cc中添加多线程测试函数`MultiThread()` ，通过在main.cc中添加的参数`-X`(大写)运行该程序，启动多进程！

  其中的测试机制为：同时创建两个进程，然后在运行过程中，分别打印出内存当前的状态

```C++
else if (!strcmp(*argv, "-X")){ //Lab3 Exercise: test MultiThread
    ASSERT(argc > 1);
    MultiThread(*(argv + 1)); //第2个参数为程序的可执行文件名
    argCount = 2;
}
```

```C++
void 
MultiThread(char *filename)
{
    OpenFile *executable = fileSystem->Open(filename);

    if (executable == NULL) {
        printf("Unable to open file %s\n", filename);
        return;
    }

    Thread *t1 = SingleThread(executable, 1); //创建一个进程(progtest.cc)
    Thread *t2 = SingleThread(executable, 2);

    delete executable;			// close file

    t1->Fork(userThread, (void*)1); //执行该进程(progtest.cc)
    t2->Fork(userThread, (void*)2);

    currentThread->Yield();  //main Thread yield
}
```

- ３．进程切换前清空TLB

```c++
//addrspace.cc - SaveState() 
#ifdef USE_TLB // Lab3: 切换进程前，清空TLB
    DEBUG('T', "Clean up TLB due to Context Switch!\n");
    for (int i = 0; i < TLBSize; i++) {
        machine->tlb[i].valid = FALSE;
    }
#endif
```

- ４．测试多线程运行

  使用如下命令运行code/test/中的程序`matmult.cc`，把其中内容修改为如下(原矩阵运算所需内存空间太大，第二个进程会内有空间可用)

```c++
int main() {
	int a = 1, b = 2;
    a = 2;b = 1;
    Exit(87);
}
```

​	运行结果：`#选择对应参数，查看syscall, TLB, thread相关的信息`

```bash
stone@stone:/mnt/shared/Nachos/nachos-3.4/code/userprog$ ./nachos -x ../test/matmult -d STt 
Creating user program thread 1
Creating user program thread 2
Forking thread "User_program_1" with func = 0x804ede0, arg = 1
Putting thread User_program_1 on ready list.
Forking thread "User_program_2" with func = 0x804ede0, arg = 2
Putting thread User_program_2 on ready list.
Yielding thread "main"
Putting thread main on ready list.
Switching from thread "main" to thread "User_program_1"
Running user program thread 1
======== addrspace information ==========
numPages = 11
VPN	PPN	valid	readOnly	use	dirty
0	0	1	0	0	0	
1	1	1	0	0	0	
2	2	1	0	0	0	
3	3	1	0	0	0	
4	4	1	0	0	0	
5	5	1	0	0	0	
6	6	1	0	0	0	
7	7	1	0	0	0	
8	8	1	0	0	0	
9	9	1	0	0	0	
10	10	1	0	0	0	
=========================================
TLBSize=4, TLB Miss: 4, TLB Hit: 28, Total Translation: 32, TLB Miss Rate: 12.50%
User Program Exit With Status 731
Finishing thread "User_program_1"
Sleeping thread "User_program_1"
Switching from thread "User_program_1" to thread "User_program_2"
Running user program thread 2
======== addrspace information ==========
numPages = 11
VPN	PPN	valid	readOnly	use	dirty
0	11	1	0	0	0	
1	12	1	0	0	0	
2	13	1	0	0	0	
3	14	1	0	0	0	
4	15	1	0	0	0	
5	16	1	0	0	0	
6	17	1	0	0	0	
7	18	1	0	0	0	
8	19	1	0	0	0	
9	20	1	0	0	0	
10	21	1	0	0	0	
=========================================
TLBSize=4, TLB Miss: 4, TLB Hit: 55, Total Translation: 59, TLB Miss Rate: 6.78%
User Program Exit With Status 731
Finishing thread "User_program_2"
Sleeping thread "User_program_2"
Switching from thread "User_program_2" to thread "main"
Now in thread "main"
Deleting thread "User_program_2"
Finishing thread "main"
Sleeping thread "main"
No threads ready or runnable, and no pending interrupts.
Assuming the program completed.
Machine halting!

Ticks: total 104, idle 0, system 60, user 44
Disk I/O: reads 0, writes 0
Console I/O: reads 0, writes 0
Paging: faults 0
Network I/O: packets received 0, sent 0

Cleaning up...
```

由上面输出信息可以看出，两个进程分别进行了创建，运行，切换，并输出了对应的内存空间占用情况，最后返回`main`进程，退出程序；即可证明系统已经可支持多用户进程运行！





---

### Excercise 6 缺页中断支持

> Task：基于TLB机制的异常处理和页面替换算法的实践，实现缺页中断处理（注意！TLB机制的异常处理是将内存中已有的页面调入TLB，而此处的缺页中断处理则是从磁盘中调入新的页面到内存）、页面替换算法等。



---

###　后记

disk不是直接就在二进制文件里嘛！可是要让虚拟内存如何映射呢？文件存储的地方可以获得吗，每次都要读文件(认为是读disk)？

- **machine/disk.h/cc:** Nachos已经模拟实现了disk的结构！

  所以是不是应该是启动时，创建`disk`实例，然后把所有的文件都赋值在其中！

  **初步思路：**　程序本身就是在disk中，这里用文件模拟虚拟内存的意思是，模拟虚拟内存的结构(即disk与memory的映射)，只不过模拟的方式是直接进行复制，还是有问题吧，应该是建立一个映射表就可以嘛！　姑且能算实现了基本过程吧！

  当我找到你，试探你眼睛！

- mainMemory被定义`numPhysPages * PageSize`，即`32*128B`

  所以存储时是按字节进行存储的！



---

**思路：**　所谓缺页中断，即所需页在磁盘而非内存中．用一个文件模拟虚拟内存，初始化时，写入虚拟内存（该文件）．

#### (一)用文件模拟虚拟内存

**修改addrspace.cc**

- **创建所需大小的虚拟内存**　(用文件模拟)

```C++
 //---Lab3 Exercise 6&7
// Virutal Memory: 按照executable所需的size创建磁盘（文件模拟）
DEBUG('a', "Demand paging: creating virtual memory!\n");
bool success_create_vm = fileSystem->Create("VirtualMemory", size);
ASSERT(success_create_vm);

//注：不要再分配物理内存，更改为FALSE
pageTable[i].valid = FALSE;  //Lab3 Ex6: Virtual Memory
```

- **把执行文件的代码和数据复制给虚拟内存**（而非物理内存）

```c++
OpenFile *vm = fileSystem->Open("VirtualMemory");
char* tempVirtualMem;
tempVirtualMem = new char[size];
for(int i = 0; i < size; i++)
	tempVirtualMem[i] = 0;

// then, copy in the code and data segments into memory
if (noffH.code.size > 0) {
    DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", 
          noffH.code.virtualAddr, noffH.code.size);
    //写入虚拟内存而非物理内存
    executable->ReadAt(&(tempVirtualMem[noffH.code.virtualAddr]), 
                       noffH.code.size, noffH.code.inFileAddr);
    vm->WriteAt(&(tempVirtualMem[noffH.code.virtualAddr]),
                noffH.code.size, noffH.code.virtualAddr*PageSize);   //注意写入位置的不同，因为在虚拟内存中没有Header,只有code和data部分     
}
//data部分同上
```



#### (二) 实现缺页中断

衔接Exercise 2的内容，当不仅TLB失效，页表也失效时，需要从磁盘调页，由`exception.cc`中的`PageFaultException()`实现

```c++
// Lab3 Exercise-6 缺页中断
TranslationEntry
PageFaultHandler(int vpn) {
    //查看内存有空页
    int emptyPageFrame = machine->AllocateMem();
    //调页
    if(emptyPageFrame == -1){
        emptyPageFrame = DemandPageReplacement(vpn);
    }
    machine->pageTable[vpn].physicalPage = emptyPageFrame;

    // 从虚拟内存加载页面
    DEBUG('a', "Demand paging: loading page from virtual memory!\n");
    OpenFile *vm = fileSystem->Open("VirtualMemory"); //VirtualMemory(文件)定义在addrspace.cc中
    vm->ReadAt(&(machine->mainMemory[emptyPageFrame*PageSize]), PageSize, vpn*PageSize);
    delete vm; // close the file

    // 更新页面的标记
    machine->pageTable[vpn].valid = TRUE;
    machine->pageTable[vpn].use = FALSE;
    machine->pageTable[vpn].dirty = FALSE;
    machine->pageTable[vpn].readOnly = FALSE;

    currentThread->space->PrintState(); // 输出内存情况（BitMap）
}
```



---

### 第三部分 Lazy-loading

### Exercise 7 按需调页

> 我们已经知道，Nachos系统为用户程序分配内存必须在用户程序载入内存时一次性完成，故此，系统能够运行的用户程序的大小被严格限制在4KB以下。请实现Lazy-loading的内存分配算法，使得当且仅当程序运行过程中缺页中断发生时，才会将所需的页面从磁盘调入内存。



---

### 后补：

since any delayed load must get applied before we trap to the kernel. 也就是在陷入内核前就需要把页调入？



```c++
//----------------------------------------------------------------------
// Machine::DelayedLoad
// 	Simulate effects of a delayed load.
//
// 	NOTE -- RaiseException/CheckInterrupts must also call DelayedLoad,
//	since any delayed load must get applied before we trap to the kernel.
//----------------------------------------------------------------------
#define LoadReg		37	// The register target of a delayed load.
#define LoadValueReg 	38	// The value to be loaded by a delayed load.

void
Machine::DelayedLoad(int nextReg, int nextValue)
{
    registers[registers[LoadReg]] = registers[LoadValueReg];
    registers[LoadReg] = nextReg;
    registers[LoadValueReg] = nextValue;
    registers[0] = 0; 	// and always make sure R0 stays zero.
}
```



所以David是在哪一部分对VM进行了赋值呢？初始申请地址空间的时候吗？

对的！WriteAt(VirtualMemory)，但是这样对于多线程如何操作呢？多个文件进行模拟，那岂不是有多个磁盘了！按理说只能有一个磁盘啊！

---





**原理理解：**

其实总的物理页还就是32个128B的页面；本进程的所有页面都已加载在虚拟内存里；所谓缺页，就是在物理内存中找不到该页，那么就从其中选择一页置换出去，把缺页从磁盘中加载进来；

**解决思路：**

- 查找物理内存中未被修改的页面，找到一个，直接替换；
- 若没有未修改页面，则选择第一个已修改页进行替换，并把它写回磁盘；
- （使用宏定义使得按需调页与之前的Exercise共存）

```c++
int 
DemandPageReplacement(int vpn){
    int pageFrame = -1;
    for(int selectVPN = 0; selectVPN < machine->pageTableSize; selectVPN++){
        if(machine->pageTable[selectVPN].valie) { 
            if(!(machine->pageTable[selectVPN].use)) { //未修改
                pageFrame = machine->pageTable[selectVPN].physicalPage; //替换
                break;
            }
        }
    }//for
    if(pageFrame == -1){
        for (int replaced_vpn = 0; replaced_vpn < machine->pageTableSize, replaced_vpn != vpn; replaced_vpn++) {
            if (machine->pageTable[replaced_vpn].valid) {　//选择第一个有效页，直接替换
                machine->pageTable[replaced_vpn].valid = FALSE;
                pageFrame = machine->pageTable[replaced_vpn].physicalPage;

                // 写回磁盘
                OpenFile *vm = fileSystem->Open("VirtualMemory");
                vm->WriteAt(&(machine->mainMemory[pageFrame*PageSize]), PageSize, replaced_vpn*PageSize);
                delete vm; // close file
                break;
            }
        }
    }//if
    return pageFrame;
}//DemandPageReplacement
```

**代码测试：**

```bash
stone@stone:/mnt/shared/Nachos/nachos-3.4/code/userprog$ ./nachos -X ../test/matmult -d TM
Creating user program thread 1
Creating user program thread 2
Running user program thread 1
======== addrspace information ==========
numPages = 15
VPN	PPN	valid	readOnly	use	dirty
0	0	1	0	0	0	
1	1	1	0	0	0	
2	2	1	0	0	0	
3	3	1	0	0	0	
4	4	1	0	0	0	
5	5	1	0	0	0	
6	6	1	0	0	0	
7	7	1	0	0	0	
8	8	1	0	0	0	
9	9	1	0	0	0	
10	10	1	0	0	0	
11	11	1	0	0	0	
12	12	1	0	0	0	
13	13	1	0	0	0	
14	14	1	0	0	0	
=========================================
TLBSize=4, TLB Miss: 15, TLB Hit: 233, Total Translation: 248, TLB Miss Rate: 6.05%
Running user program thread 2
======== addrspace information ==========
numPages = 15
VPN	PPN	valid	readOnly	use	dirty
0	0	1	0	0	0	
1	1	1	0	0	0	
2	2	1	0	0	0	
3	3	1	0	0	0	
4	4	1	0	0	0	
5	5	1	0	0	0	
6	6	1	0	0	0	
7	7	1	0	0	0	
8	8	1	0	0	0	
9	9	1	0	0	0	
10	10	1	0	0	0	
11	11	1	0	0	0	
12	12	1	0	0	0	
13	13	1	0	0	0	
14	14	1	0	0	0	
=========================================
TLBSize=4, TLB Miss: 29, TLB Hit: 466, Total Translation: 495, TLB Miss Rate: 5.86%
No threads ready or runnable, and no pending interrupts.
Assuming the program completed.
Machine halting!
```



### 第四部分 Challenges

### Challenge 1  实现SUSPENDED状态

> 为线程增加挂起SUSPENDED状态，并在已完成的文件系统和内存管理功能的基础之上，实现线程在“SUSPENDED”，“READY”和“BLOCKED”状态之间的切换。

（待做）

### Challenge 2  实现倒排页表

> 多级页表的缺陷在于页表的大小与虚拟地址空间的大小成正比，为了节省物理内存在页表存储上的消耗，请在Nachos系统中实现倒排页表。  

（待做）



---

## 内容三：遇到的困难以及解决办法

#### 新疑问：

- Nachos本身自带有BitMap和DelayedLoad结构，细看如何使用啊！





困难1**：对于MakeFile的修改非常陌生

听了两边CMU的深入理解计算系统课程Linking部分，对于编译和链接的过程理解算是比较好了，然后结合MakeFile中其他程序的编写方式，也就完成了自己程序的MakeFile修改．

**困难2**：很多时候程序报错都不知道如何调试

之前每次编译出错的时候，都需要一行一行地梳理代码，查找哪个环节出了问题；最近这次Lab时才发现Nachos自带的DEBUG功能真的很方便，通过添加自己定义的参数，可以在程序运行时输出自己想要看的部分．具体可以参考这份链接 [Tracing and Debugging Nachos Programs](https://users.cs.duke.edu/~chase/cps110-archive/nachos-guide/nachos-labs-13.html#29510)



**后记：简要整理一下几个流程，方便之后复习**

#### 地址转换的整体流程：

地址转换发生在需要读写内存时，具体代码为`Machine::ReadMem()/WriteMem()`(作为Machine类的接口，但是具体实现在`translate.cc`中)，其中会调用`Machine::Translate()`进行转换，如果这个过程中发生异常，则调用`Machine::RaiseException()`，其中先切换为`SystemMode`然后根据异常的类别进行对应处理．



#### **备注**：新增用户程序的流程

- 在code/test/下新建一个自己的源码文件`pyq_test.c`，简单定义其功能

- 修改/code/test/MakeFile，按照其他程序编写对应make代码

- 编译：新增的用户程序`pyq_test.c`，只需要在`code/test/`下`make`编译一次就行，其他关于用户程序控制内容的修改都在`/code/userprog/`下单独编译就行；

- 在userprog/下，使用命令`./nachos -x ../test/pyq_test`，即运行该用户程序

  (因为main.cc中line111部分，在/userprog中命令行参数`-x`表示运行一个用户程序，另外最后追加`-d`参数可以查看整个程序的`DEBUG`内容，其后再追加参数`m`可只查看用`m`标记的debug语句)

- **注意：** 如果想要运行自己新增的程序，发现必须使用`sudo`或者用`chmod`命令修改权限，而且只修改`owner`的权限都不行，需要给`group`和`others`也添加`wx`权限才行！（不过原有的`matmult`等程序权限确实也是那样，只是不明白为什么权限必须要设置成这样，才可以正常执行）



#### 用户程序的执行过程

- 命令行传入参数`-x`和已编译好的程序二进制文件名`filename`，

- `main.cc`中使用参数`-x`调用`progtest.cc`中的`StartProcess()`函数，其功能：新建进程，初始化内存，执行传入的一个程序；

- 多进程：（Exercise 5实现）可以类比`StartProcess()`新增一个函数`MultiThread()`，只不过其中启动多个进程，分别初始化并执行对应的程序文件

- 注：用户程序都是传入二进制文件名；

  ```c++
  OpenFile *executable = fileSystem->Open(filename);
  ```

  我把文件都放在`code/test/`目录下，使用如下命令运行：(e.g. matmult.cc)

  ```bash
  /code/userprog$ ./nachos -x ../test/matmult -d STt
  ```

  > 注：Debug对应参数： `S` - syscall, `T` - TLB, `t` - thread, `M` - Memory

 

## 内容四：收获及感想

​		本次Lab的任务真是非常非常重啊，再加上其他几门课程的期中考试，就导致整个完成度还有待提高，最后的Challenge部分想要下周抽时间补充．总的来说，通过这次Lab过程，梳理了整个操作系统的运行流程，也对于Nachos的实现机制有了更深的理解，对于多线程支持以及按需调页部分，这段期中忙完之后，还需要再强化．想来还是自己没有合理的安排时间，总是任务堆到眼前才忙起来，之后的Lab要早做准备，早做计划．



## 内容五：对课程的意见和建议
- 希望可以设置多次提交的选项

  每次Lab结束，其实自己都觉得不少地方做的不够细致，之后想要抽时间再把其中内容进行完善．如果ddl之后仍可以提交并且后期的改进也可以获得一定的分数，那么大家持续学习的欲望就会更加强烈．

- Nachos和XV6任务

  目前两份源码任务都是同一个ddl进行提交，实在是压力比较大．还是希望能把两者分离开，这样可以提高每一份的完成质量，也能让自己对每一个系统的理解都更加透彻．


​	

## 内容六：参考内容

> [陈皓－和我一起写Makefile](https://blog.csdn.net/haoel/article/details/2886)
>
> [CMU-深入理解计算机系统(csapp): Virtual Memory](http://www.cs.cmu.edu/~./213/schedule.html)
>
> [Github-Nachos课程操作实验参考](https://github.com/Vancir/os-lab)
>
> [Nachos Beginner's Guide](https://www.ida.liu.se/~TDDI04/material/begguide/)
>
> [Tracing and Debugging Nachos Programs](https://users.cs.duke.edu/~chase/cps110-archive/nachos-guide/nachos-labs-13.html#29510)



