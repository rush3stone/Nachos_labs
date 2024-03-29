#### XV6同步机制　阅读要求

阅读代码:
锁部分 spinlock.h spinlock.c 以及相关其他文件代码
请大家围绕如下一些问题阐述原理课的相关内容,以及 XV6 中是如何实现的。

1. 什么是临界区?什么是同步和互斥?什么是竞争状态?临界区操作时中断是否应该开
启?中断会有什么影响?XV6 的锁是如何实现的,有什么操作?xchg 是什么指令,该指 令
有何特性?
2. 基于 XV6 的 spinlock,请给出实现信号量、读写锁、信号机制的设计方案(三选二,请
写出相应的伪代码)

---

[知乎：如何理解互斥锁、条件锁、读写锁以及自旋锁？](https://www.zhihu.com/question/66733477)



XV6中`spinlock`的实现  ***spinlock.h(cc)***中

```c++
// Mutual exclusion lock.
struct spinlock {
  uint locked;       // Is the lock held? 锁可获得时为0,已被占时为1
  
  // For debugging:
  char *name;        // Name of lock.
  struct cpu *cpu;   // The cpu holding the lock.
  uint pcs[10];      // The call stack (an array of program counters)
                     // that locked the lock.
};
```

xchg()获得锁返回０,锁已被占则返回1；它是atomic & serializable



#### 信号量



#### 读写锁

**解决方案：**

所谓读写锁，从英文名readers-writer可以很容易的理解其含义，即对于共享资源，多个读者访问并不互斥，而写者则与任何人都互斥；直接可以通过xv6已有的数据机构spinlock实现；

**具体思路**

定义两个自旋锁，分别对于读者数量和写操作加锁；再增加一个变量记录当前读者数量

```c++
spinlock 保护当前读者数量的锁;
spinlock 保护写操作的锁;
int 当前读者数量;
```

读写锁提供四个接口，分别是针对读操作的加锁/解锁，写操作的加锁/解锁

- 对读操作加锁

```c++
void ReaderLock(){
	获得读者数量锁；
    读者数量++;
    if(首个读者){
        获得写者锁
    }
    释放读者数量锁
}
void ReaderUnlock(){
    获得读者数量锁；
    读者数量--;
    if(最后一位读者){
        释放写者锁
    }
    释放读者数量锁
}
```

对写操作的加锁解锁，比较简单；

```c++
void WriterLock() {获得写者锁;}
void WriterUnlock() {释放写者锁;}
```



**完整代码实现：－读写锁**

```c++
class ReadersWriterLock {
    public:
    	void ReaderLock(){
            rlock.acquire(); //acquire lock for reader num
            readerNum++;
            if(readerNum==1){ //fisrt reader, acquire lock for writer
                wlock.acquire(); 
            }
            rlock.release(); 
        }
        void ReaderUnlock(){ //release lock for reader num
            rlock.acquire();
            readerNum--;
            if(readerNum == 0){ //last reader, acquire lock for writer
                wlock.release();
            }
            rlock.release();
        }

    	void WriterLock() {wlock.acquire();} //acquire lock for writer
        void WriterUnlock() {wlock.release();} //release lock for writer
	private:
    	spinlock rlock; //lock for reader num
        spinlock wlock; //lock for writer
        int readerNum; // num of current readers
};
```











# LAB4 同步机制 讨论记录

​                                                                                                                                    *记录人：彭宇清*

#### 时间：2019年11月16日 18:00~19:30

#### 地点：５号楼三楼会议室

#### 参加人员：彭宇清（组长）、游禹韩、杜尧帝、张越、 郭宏宇

## 讨论内容

本次讨论主要结合上一次课程中进程同步的内容讨论xv6中同步机制的实现。其中，

#### 论题1 XV6中如何使用锁的？

​		由于锁会降低并发度，所以xv6 非常谨慎地使用锁来避免竞争条件。使用锁的一个难点在于要决定使用多少个锁，以及每个锁保护哪些数据、不变量。一个简单的例子就是 IDE 驱动，`iderw`有一个磁盘请求的队列，处理器可能会并发地向队列中加入新请求。为了保护链表以及驱动中的其他不变量，`iderw` 会请求获得锁 `idelock`并在函数末尾释放锁。

对于锁的粒度选择，xv6 只使用了几个简单的锁；例如，xv6 中使用了一个单独的锁来保护进程表及其不变量，更精细的做法是给进程表中的每一个条目都上一个锁，这样在不同条目上运行的线程也能并行了。然而，当一个操作需要维持关于整个进程表的不变量时，这样的做法会让情况变得很复杂，因为此时它可能需要持有多个锁。



#### 论题2 　XV6中锁的顺序是怎样的

如果一段代码要使用多个锁，那么必须要注意代码每次运行都要以相同的顺序获得锁，否则就有死锁的危险。假设某段代码的两条执行路径都需要锁 A 和 B，但路径1获得锁的顺序是 A、B，而路径2获得锁的顺序是 B、A。这样就有能路径1获得了锁 A，而在它继续获得锁 B 之前，路径2获得了锁 B，这样就死锁了。这时两个路径都无法继续执行下去了，因为这时路径1需要锁 B，但锁 B已经在路径2手中了，反之路径2也得不到锁 A。为了避免这种死锁，所有的代码路径获得锁的顺序必须相同。避免死锁也是我们把锁作为函数使用规范的一部分的原因：调用者必须以固定顺序调用函数，这样函数才能以相同顺序获得锁。

由于 xv6 本身比较简单，它使用的锁也很简单，所以 xv6 几乎没有锁的使用链。最长的锁链也就只有两个锁。例如，`ideintr` 在调用 `wakeup` 时持有 ide 锁，而 `wakeup` 又需要获得 `ptable.lock`。还有很多使用 `sleep`/`wakeup` 的例子，它们要考虑锁的顺序是因为 `sleep` 和 `wakeup` 中有比较复杂的不变量，我们会在第5章讨论。文件系统中有很多两个锁的例子，例如文件系统在删除一个文件时必须持有该文件及其所在文件夹的锁。xv6 总是首先获得文件夹的锁，然后再获得文件的锁。



### 疑问

- **proc.cc-155行:** `np->cwd = idup(proc->cwd);`表示的含义是什么

  初步猜测：`fork()`中进行的工作是新进程创建并把父进程的一切内容赋值给它，此处应该是给新进程的文件目录变量赋值为父进程的文件目录，而调用idup()函数的作用是在其中增大ip的值，即对相应文件来说，增加其软链接的个数。

- **proc.cc-183行:** `iput(proc->cwd);`表示的含义是什么

  初步猜测：承接上个疑问，此处在`exit()`中`proc->cwd = 0;`前，结合其中的注释，应该是当销毁进程时删除其文件目录指针前，判断一个软链接的个数，如果大于１直接减１，如果是最后一个，则不止软连接要被删除，该索引节点也需要被删除。

  

### 本周课后任务

- **考虑XV6如何通过锁来保证中断处理程序正常的**

  中断处理程序有可能与另一个 CPU 上运行非中断代码使用同一个数据，这会导致出错！

- **xchg指令的机制是怎样的？**

  



---

### **组内评分**

**游禹韩**：9分

**张越**：9分

**杜尧帝**：9分

**郭宏宇**：９分

**彭宇清**：9分(记录员)
