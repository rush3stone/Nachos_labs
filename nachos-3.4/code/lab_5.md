











# 文件系统　实习报告































**姓名：彭宇清　　学号：1901210731**

**日期：2019/11/20**

<div STYLE="page-break-after: always;"></div>
**目录**

[TOC]

<div STYLE="page-break-after: always;"></div>
## 内容一：总体概述

#### 背景描述

>  Nachos文件系统建立在模拟磁盘上，提供了基本的文件操作，如创建、删除、读取、写入等等。文件的逻辑结构与物理位置之间的映射关系由文件系统统一维护，用户只需通过文件名即可对文件进行操作。
>
>  然而，相比于实际文件系统，Nachos文件系统还存在很多不足之处：
>
> - 文件长度的限制
>
>  Nachos文件系统采用直接索引方式，故文件长度不能超过4KB（更准确的说，是((128 – 2 * 4) / 4) * 128 = 3840 B）。同时，文件的长度必须在创建时予以指定，且不得更改。
>
> - 文件数目的限制
>
>  Nachos文件系统只有一级目录，系统中所有的文件都存于根目录下，且数目不能多于10个。
>
> - 粗粒度的同步互斥机制
>
>  Nachos文件系统每次只允许一个线程访问，不支持多个线程同时访问文件系统。
>
> - 性能优化与容错
>
>  Nachos文件系统没有Cache机制，也没有容错机制，即当文件系统正在使用时，如果系统突然中断，文件内容的正确性无法保证。

#### 实习建议

> - 数据结构的修改和维护
>
>  文件管理的升级基于对原有Nachos数据结构的修改。增加文件的描述信息需对文件头结构进行简单修改。多级目录中可创建目录也可创建文件，应根据实际的文件类型初始化文件头信息。
>
> - 实现多级目录应当注意
>
> 	- 目录文件的含义。每个目录对应一个文件，通过此文件可了解其子目录及父目录的信息。
>
>    - Nachos的目录文件大小是预先定义的，但实际上，目录文件的大小应根据内容确定，且能改变。
>
>    - 实现多级目录后，添加、删除目录项要根据具体的路径，对树的遍历要有深刻的理解。
>
>    - 为了实现文件长度无限，可以采取混合索引的分配方式。

本次实验主要内容是结合操作系统课堂上讲解的原理内容，通过修改Nachos系统的底层源代码，达到“完善文件系统”的目标。其中涉及到了文件的基本组织方式，文件目录结构及其存储方式，以及结合上个lab，完善Nachos文件系统的同步互斥机制，最后在以上基础上对文件系统的性能进行优化．



## 内容二：任务完成情况

任务完成列表

| 第一部分     | Exercise 1     | Exercise 2      | Exercise 3      |
| ------------ | -------------- | --------------- | --------------- |
|              | Y              | Y               | Y               |
| **第二部分** | **Exercise 4** | **Exercise 5**  | **Exercise 6**  |
|              | Y              | Y               | Y               |
| **第三部分** | **Exercise 7** | **Challenge 1** | **Challenge 2** |
|              | Y              | Y               | Y               |



**具体Exercise完成情况**

### 模块一 文件系统的基本操作

### Exercise 1 源代码阅读

>  阅读Nachos源代码中文件系统相关的代码，理解Nachos文件系统的工作原理。
>
>  code/filesys/filesys.h和code/filesys/filesys.cc
>
>  code/filesys/filehdr.h和code/filesys/filehdr.cc
>
>  code/filesys/directory.h和code/filesys/directory.cc
>
>  code /filesys/openfile.h和code /filesys/openfile.cc
>
>  code/userprog/bitmap.h和code/userprog/bitmap.cc




- **threads/synch.h(cc)**

  Nachos声明了三种同步机制：信号量，锁以及条件变量，目前只定义了**信号量**的具体内容，另外两种需要自己实现（Exercise 3）

  **变量定义:**

  - `value`：表示信号量的值，始终非负；

- `queue`：表示当信号量被申请完后，排队等待的进程；

**同步机制：**

由`P`&`V`操作分别申请和释放信号量

- `P()`: 先关中断，然后申请资源；若资源数为`0`，则先把当前进程加入`queue`，然后`sleep()`（即：不会执行资源数减１的操作，`sleep`直到被唤醒）

  - `V()`: 先关中断，然后取下`queue`中首个被睡眠进程，设置为`ready`状态；

  **`value`值的变化：** 

  最小为０；P()中之后申请资源的进程就会被`sleep`，唤醒后虽然会继续执行`value--`，但是`value`并不会减为负值，因为在V()中会先执行`value++`




- **threads/synchlist.h(cc)**

  定义了一个用于同步机制的链表

  **变量定义**

  - `list`: 保存进程需要资源(`item`)
  - `lock`: 保证`list`被互斥访问
  - `listEmpty`: 保存

  **函数接口**

  - **Append():** 增加队列元素

  - **Remove():** 删除队列元素

  - **Mapcar(func):** 先使用自带的`lock`上锁，然后对`list`中每个元素调用传入的函数`func`进行操作；



### Exercise 2 扩展文件属性

 > Task:  增加文件描述信息，如“类型”、“创建时间”、“上次访问时间”、“上次修改时间”、“路径”等等。尝试突破文件名长度的限制。

#### (一) 锁

**解决思路：**

使用`Semaphore`实现模拟锁，信号量的初始数值赋为１，`P()`操作模拟`Acquire()`，`V()`操作模拟`Release()`；另外增加一个记录锁拥有者的变量；

```C++
// plus some other stuff you'll need to define
Thread *lockThread;   //thread holding the lock
Semaphore *semaphore; // use Semaphore implement lock
```

`Lock`主要接口的实现：

```c++
Lock::Lock(char* debugName) {
    name = debugName;
    lockThread = NULL;
    semaphore = new Semaphore("SemForLock", 1);
}

//	Acquire -- wait until the lock is FREE, then set it to BUSY
void Lock::Acquire() {
    DEBUG('s', "thread %s ask for lock %s.\n", currentThread->getName(), name);
    semaphore->P();
    lockThread = currentThread;
}

void Lock::Release() {
    DEBUG('s', "thread %s ask for lock %s.\n", currentThread->getName(), name);
    ASSERT(this->isHeldByCurrentThread());
    semaphore->V();
    lockThread = NULL;
}
```



> **探讨**： 既然使用Semaphore，自带关中断操作，Lock中应该不需要额外再关中断吧？
>
> **答：** Semaphore中的中断用于互斥修改信号值，所以Lock是互斥访问的吗？是！因为只有一份信号值！



---



### Exercise 3  扩展文件长度

> Task: 基于Nachos中的信号量、锁和条件变量，采用两种方式实现同步和互斥机制应用（其中使用条件变量实现同步互斥机制为必选题目）。具体可选择“生产者-消费者问题”、“读者-写者问题”、“哲学家就餐问题”、“睡眠理发师问题”等。（也可选择其他经典的同步互斥问题）

#### **(一) 信号量实现生产者-消费者**

- 在`synch.h(cc)`中分别创建商品类`product`以及存放商品的缓冲区类`wareHouse`，对于其中商品状态的修改使用`Lock`实现互斥访问；用两个信号量分别表示缓冲区的空位置数量和已保存数量，进而实现生产者消费者问题；

```C++
#define WARE_HOUSE_SIZE 5　// size of buffer
typedef struct {
  int value;
}product;

class wareHouse{　//warehouse to save products
public:
  wareHouse();
  ~wareHouse();
  product *consume();
  void produce(product *pro);
  void printProduct();

private:
  Semaphore *emptyNum;
  Semaphore *fullNum;
  int num;
  product proList[WARE_HOUSE_SIZE];
  Lock *wareHouseLock;
}; //wareHouse
```

- 生产者的实现：先查看缓冲区中是否有空位置，若有则申请缓冲区的锁，得到锁后修改缓冲区的状态，最后释放锁并修改信号量；

```c++
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
```

- 消费者的实现：信号量的操作与生产者正好相反，最后返回消费的商品值；

```C++
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
```

- **测试代码**

  在`threadtest.cc`中，调用`synch.h(cc)`中的内容，实现简单的生产者和消费者操作：

```C++
wareHouse *warehouse = new wareHouse();

void Producer(int itemNum) {
    for(int i = 0; i < itemNum; i++){
        printf("PPP Thread %s PPP, ", currentThread->getName());
        product* item;
        item->value = i;
        warehouse->produce(item);

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
```

创建两个线程，分别执行生产和消费，其中生产的商品值直接赋值为序号；

```C++
void Lab3ProConTest(){
    DEBUG('p', "Now Test Producer & Consumer!\n");
    wareHouse *warehouse = new wareHouse();

    Thread *p1 = new Thread("Producer_1");
    Thread *c1 = new Thread("Consumer_1");

    p1->Fork(Producer, (void*)4);
    c1->Fork(Consumer, (void*)3);

    currentThread->Yield(); // Yield the main thread
}
```

测试结果：

```bash
stone@stone:/mnt/shared/Nachos/nachos-3.4/code/threads$ ./nachos -q 8
Lab3 Synch: Producer&Consumer:
PPP Thread Producer_1 PPP, Produce product: 0
PPP Thread Producer_1 PPP, Produce product: 1
PPP Thread Producer_1 PPP, Produce product: 2
PPP Thread Producer_1 PPP, Produce product: 3
PPP Thread Producer_1 PPP, Produce product: 4
PPP Thread Producer_1 PPP, CCC Thread Consumer_1 CCC, Consume product: 4
CCC Thread Consumer_1 CCC, Consume product: 3
CCC Thread Consumer_1 CCC, Consume product: 2
CCC Thread Consumer_1 CCC, Consume product: 1
CCC Thread Consumer_1 CCC, Consume product: 0
CCC Thread Consumer_1 CCC, Produce product: 5
PPP Thread Producer_1 PPP, Produce product: 6
PPP Thread Producer_1 PPP, Produce product: 7
Consume product: 7
No threads ready or runnable, and no pending interrupts.
Assuming the program completed.
Machine halting!
```



**遗留问题：** 为什么我把缓冲区的定义放在`synch.h(cc)`中，就会报多重定义的错，直接定义在`threadtest.cc`中就没问题呢？

**解决：** 需要把类的定义放下`#endif`的前面，需要定义了`SYNCH_H`才能正常运行



### Exercise 4 实现多级目录

**解决思路**

​	参考Linux的文件系统，本目录还有`.` `..`两个目录项，分别指向本目录和父目录



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

  

### Exercise 5 动态调整文件长度

>  对文件的创建操作和写入操作进行适当修改，以使其符合实习要求。 

**解题思路：** 增加一个队列，保存等待当前条件变量的进程；对于条件变量进行加锁，条件变量的接口分别都用上一部分`Lock`的内容进行实现即可！

增加保存等待进程的队列

```C++
// plus some other stuff you'll need to define
List *queue;   // waiting queue
```

构造函数，初始化进程队列

 ```C++
Condition::Condition(char* debugName){
    queue = new List;
}
 ```

使用conditionLock实现`Wait`，先判断锁的状态，然后释放锁，进程进入睡眠状态，等待被唤醒；直到被唤醒然后重新获得锁

 ```C++
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
//其他实现见代码中
 ```

`Signal()`以及`Broadcast()`的实现同上，把保存在队列`queue`中已经进入`sleep`状态的首个(所有)进程加入唤醒！

```C++
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
```



**测试部分在后面进行**





### 模块二 文件访问的同步与互斥

### Exercise 6 源代码阅读

> - 阅读Nachos源代码中与异步磁盘相关的代码，理解Nachos系统中异步访问模拟磁盘的工作原理。 filesys/synchdisk.h和filesys/synchdisk.cc
> - 利用异步访问模拟磁盘的工作原理，在Class Console的基础上，实现Class SynchConsole。



### Exercise 7 实现文件系统的同步互斥机制

>**达到如下效果：**
>
>- 一个文件可以同时被多个线程访问。且每个线程独自打开文件，独自拥有一个当前文件访问位置，彼此间不会互相干扰。
>- 所有对文件系统的操作必须是原子操作和序列化的。例如，当一个线程正在修改一个文件，而另一个线程正在读取该文件的内容时，读线程要么读出修改过的文件，要么读出原来的文件，不存在不可预计的中间状态。
>- 当某一线程欲删除一个文件，而另外一些线程正在访问该文件时，需保证所有线程关闭了这个文件，该文件才被删除。也就是说，只要还有一个线程打开了这个文件，该文件就不能真正地被删除。



### 模块三　Challenges题目

### Challenge 1  性能优化

> - 例如，为了优化寻道时间和旋转延迟时间，可以将同一文件的数据块放置在磁盘同一磁道上
>
> - 使用cache机制减少磁盘访问次数，例如延迟写和预读取。**背景：**  Barrier是针对一组进程或线程设计的机制，表示只有当这一组进程(线程)都达到某个条件时，才可以继续向下执行！

> 

**解决思路：** 

**From Wikipedia** : The basic barrier has mainly two variables, one of which records the pass/stop state of the barrier, the other of which keeps the total number of threads that have entered in the barrier. The barrier state was initialized to be "stop" by the first threads coming into the barrier. Whenever a thread enters, based on the number of threads already in the barrier, only if it is the last one, the thread sets the barrier state to be "pass" so that all the threads can get out of the barrier. On the other hand, when the incoming thread is not the last one, it is trapped in the barrier and keeps testing if the barrier state has changed from "stop" to "pass", and it gets out only when the barrier state changes to "pass". 



- **在synch.h中，创建Barrier类**

```C++
class Barrier{
    Barrier();
    void  BarrierFunc(Barrier* b, int p);
    // how many processors have entered the barrier　initialize to 0
    int arrive_counter;
    // how many processors have exited the barrier　initialize to p
    int leave_counter;
    int flag;   //
    Lock *lock;
};

```

- **在synch.cc中，实现Barrier的同步机制**

```c++
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

```

**测试代码:**

在`threadtest.cc`中申请四个进程，分别计算矩阵的不同行，计算过程分为三次，中间设置两次`Barrier`，最后查看进程的运行过程和计算结果

```c++
void barrierThread(int startRow){

    // 得到需要计算的范围
    int endRow = startRow + threadRows;

    // 第１轮计算
    for(int x = startRow; x < endRow; x++) 
        for (int y = 0; y < cols; y++) {
        	matrix[x][y] += (x+1)*(y+1);
    }
    printf("Thread \"%s\" finish First Calculation\n", currentThread->getName());
    barrier->BarrierFunc(currentThread->getThreadId());

    // 第２轮计算
    for (int x = startRow; x < endRow; x++) 
        for (int y = 0; y < cols; y++) {
        	matrix[x][y] /= startRow+1;
    }
...
}

```

总的申请进程：

```C++
void Lab3Barrier(){
    Thread *threads[threadNum];
    barrier = new Barrier(threadNum);

    //初始化进程
    for(int i = 0; i < threadNum; i++){
        char ThreadName[10];
        sprintf(ThreadName, "t_%d", i+1);
        threads[i] = new Thread(strdup(ThreadName));
    }
	//进程Fork并加入就绪队列
    int startRows = 0;
    for(int i = 0; i < threadNum; i++) {
        threads[i]->Fork(barrierThread, (void*)startRows);
        startRows += threadRows;
    }
    currentThread->Yield();　//main进程让出资源，开始第一轮计算
    barrier->BarrierFunc(currentThread->getThreadId());
    printf("main() is going!\n");

    currentThread->Yield(); // 进行第二轮计算
    barrier->BarrierFunc(currentThread->getThreadId());

    currentThread->Yield(); // Yield the main thread
    
	...//其他输出内容
}//Lab3Barrier

```



**测试结果：**

可以看到，每轮计算都是全部完成后才开始下一轮，即`Barrier`有效．计算结果只截取了部分进行展示如下：

```bash
stone@stone:/mnt/shared/Nachos/nachos-3.4/code/threads$ ./nachos -q 9
Lab3 Sync - Barrier:
main() is ready.
Thread "t_1" finish First Calculation
Thread "t_2" finish First Calculation
Thread "t_3" finish First Calculation
Thread "t_4" finish First Calculation
main() is going!
Thread "t_1" finish Second Calculation
Thread "t_2" finish Second Calculation
Thread "t_3" finish Second Calculation
Thread "t_4" finish Second Calculation
main() is going again!
Thread "t_1" finish Third Calculation
Thread "t_2" finish Third Calculation
Thread "t_3" finish Third Calculation
Thread "t_4" finish Third Calculation
Result of data:
 1.00  2.00  3.00  4.00  5.00  6.00  7.00  8.00  9.00 10.00 
 2.00  4.00  6.00  8.00 10.00 12.00 14.00 16.00 18.00 20.00 
 ...
 1.21  2.42  3.63  4.84  6.05  7.26  8.47  9.68 10.89 12.11 
 1.26  2.53  3.79  5.05  6.32  7.58  8.84 10.11 11.37 12.63 
No threads ready or runnable, and no pending interrupts.
Assuming the program completed.
Machine halting!

```





### Challenge 2  实现pipe机制

>  重定向openfile的输入输出方式，使得前一进程从控制台读入数据并输出至管道，后一进程从管道读入数据并输出至控制台。  

（待做）







---

## 内容三：遇到的困难以及解决办法



**困难1**：对于MakeFile的修改非常陌生

听了两边CMU的深入理解计算系统课程Linking部分，对于编译和链接的过程理解算是比较好了，然后结合MakeFile中其他程序的编写方式，也就完成了自己程序的MakeFile修改．

**困难2**：很多时候程序报错都不知道如何调试

之前每次编译出错的时候，都需要一行一行地梳理代码，查找哪个环节出了问题；最近这次Lab时才发现Nachos自带的DEBUG功能真的很方便，通过添加自己定义的参数，可以在程序运行时输出自己想要看的部分．具体可以参考这份链接 [Tracing and Debugging Nachos Programs](https://users.cs.duke.edu/~chase/cps110-archive/nachos-guide/nachos-labs-13.html#29510)

 

## 内容四：收获及感想

​		本次Lab的任务相对来说还算轻松．感悟比较深的一点是，课堂的理论知识因为掌握得已经比较好了，所以会感觉很简单，但是真实转化为可运行的代码还是需要下点功夫的，当然也是自己工程代码能力确实比较不足，还需要多努力修炼！

​		



## 内容五：对课程的意见和建议

- 同前几次，本次暂无．



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



