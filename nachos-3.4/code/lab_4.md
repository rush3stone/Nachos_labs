













# 同步机制　实习报告































**姓名：彭宇清　　学号：1901210731**

**日期：2019/11/15**

<div STYLE="page-break-after: always;"></div>
**目录**

[TOC]

<div STYLE="page-break-after: always;"></div>
## 内容一：总体概述

>   本实习希望通过修改Nachos系统平台的底层源代码，达到“扩展同步机制，实现同步互斥实例”的目标。
>

​		本次实验主要内容是结合操作系统课堂上讲解的原理内容，对教学操作系统Nachos的同步机制部分进行阅读和理解，并在此基础上对其功能进行一定的修改和增加；其中涉及到了进程同步和互斥的机制，信号量，锁和条件变量等概念，还有各种同步与互斥的算法以及在基本机制基础上的各种变形，整个lab的任务相比前几次来说比较容易．



## 内容二：任务完成情况

任务完成列表

| 第一部分     | Exercise 1     | Exercise 2      | Exercise 3      |
| ------------ | -------------- | --------------- | --------------- |
|              | Y              | Y               | Y               |
| **第二部分** | **Exercise 4** | **Challenge 1** | **Challenge 2** |
|              | Y              | Y               | N               |



**具体Exercise完成情况**

### Exercise 1 调研

> 调研Linux中实现的同步机制。具体内容见课堂要求。

Linux中的同步机制有许多中，包括：互斥锁，条件变量，读写锁，信号量，自旋锁，屏障，原子操作以及各类IPC机制（包括信号、管道、FIFO、socket、消息队列、共享内存）等等；下面简单介绍其中的几种：

- 自旋锁

  在多处理器之间设置一个全局变量V,表示锁。并定义当V=1时为锁定状态，V=0时为解锁状态。自旋锁同步机制是针对多处理器设计的，属于忙等机制。自旋锁机制只允许唯一的一个执行路径持有自旋锁。如果处理器A上的代码要进入临界区，就先读取V的值。如果V!=0说明是锁定状态，表明有其他处理器的代码正在对共享数据进行访问，那么此时处理器A进入忙等状态（自旋）；如果V=0，表明当前没有其他处理器上的代码进入临界区，此时处理器A可以访问该临界资源。然后把V设置为1，再进入临界区，访问完毕后离开临界区时将V设置为0.

```C++
// 初始化自旋锁
int pthread_spin_init (pthread_spinlock_t *lock , int pshared) ;
// 销毁自旋锁
int pthread_spin_destroy ( pthread_spinlock_t * lock ) ;
// 加锁
int pthread_spin_lock ( pthread_spinlock_t *lock) ;
// 解锁
int pthread_spin_unlock ( pthread_spinlock_t *lock ) ;
// 尝试加锁
int pthread_spin_trylock ( pthread_spinlock_t * lock ) ;
```

- 条件变量

```C++
// 初始化条件变量
int pthread_cond_init (pthread_cond_t * restrict cond , \
                       pthread_condattr_t * restrict attr ) ;
// 销毁条件变量
int pthread_cond_destroy ( pthread_cond_t * cond ) ;
// 等待事件发生
int pthread_cond_wait (pthread_cond_t * restrict cond , \
                       pthread_mutex_t * restrict mutex ) ;
// 带超时的等待，防止死锁的一种方式
int pthread_cond_timedwait (pthread_cond_t * restrict cond , \
                       pthread_mutex_t * restrict mutex , \
                       const struct timespec * restrict tsptr ) ;
// 向任意一个在等待的进程或线程通知锁可用
int pthread_cond_signal ( pthread_cond_t *cond ) ;
// 通知所有在等待的进程或者线程锁可用
int pthread_cond_broadcast ( pthread_cond_t *cond ) ;
```

- 信号量

```C++
// 初始化信号量
int sem_init ( sem_t * sem , int pshared , unsigned int value ) ;
// 销毁信号量
int sem_destroy ( sem_t * sem ) ;
// 信号量加1操作
int sem_post ( sem_t * sem ) ;
// 等待信号量值大于0 ， 并将信号量减1操作
int sem_wait (sem_t * sem ) ;
// 尝试等待
int sem_trywait ( sem_t * sem ) ;
```



### Exercise 2 源代码阅读

  > 阅读下列源代码，理解Nachos现有的同步机制。
  >
  > - code/threads/synch.h和code/threads/synch.cc
  > - code/threads/synchlist.h和code/threads/synchlist.cc


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



### Exercise 3 实现锁和条件变量

 > Task: 可以使用sleep和wakeup两个原语操作（注意屏蔽系统中断），也可以使用Semaphore作为唯一同步原语（不必自己编写开关中断的代码）。

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



#### (二) 条件变量

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



---



### Exercise 4  实现同步互斥实例

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



#### (二) 条件变量实现Barrier

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

### Challenge 1  实现barrier

> 可以使用Nachos 提供的同步互斥机制（如条件变量）来实现barrier，使得当且仅当若干个线程同时到达某一点时方可继续执行。

**背景：**  Barrier是针对一组进程或线程设计的机制，表示只有当这一组进程(线程)都达到某个条件时，才可以继续向下执行！

> Wiki: A barrier for a group of threads or processes in the source code means any thread/process must stop at this point and cannot proceed until all other threads/processes reach this barrier.

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





### Challenge 2  实现read/write lock

> 基于Nachos提供的lock(synch.h和synch.cc)，实现read/write lock。使得若干线程可以同时读取某共享数据区内的数据，但是在某一特定的时刻，只有一个线程可以向该共享数据区写入数据。  

（待做）



### Challenge 3  研究Linux的kfifo机制

> 研究Linux的kfifo机制是否可以移植到Nachos上作为一个新的同步模块

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



