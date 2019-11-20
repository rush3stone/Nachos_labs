## 算法分析－回溯









---

---

### Lecture 16 System-Level IO

sockets：代表网络链接的文件类型

后20分钟没有听懂啊！



---

### Lecture 17 VM Concepts

**虚拟化的例子：**

把硬盘的虚拟化为一个巨大的数组

**Why VM?** - P6  

- VM uses the DRAM as a cache for the actual data stored on disk

  So we only need to cache items that are frequently used! 

  Only actually storing the portions of the virtual address space in the physical memory!

  对于虚拟内存的理解　（可以结合Lecture-18关于Memory Mapping-P23的内容！）

  **理解１：** 虚拟内存就是个映射表结构和管理它的机制而已！作为disk和memory的中间量！

  **理解２：** virtual memory is an array of N contiguous bytes **stored on disk**. 

  所以也就是说，VM的有效内容是硬盘的内容！物理内存作为虚拟内存的Cache，把部分虚拟内存(disk)的东西加载进来了！

  >  所以回想Nachos，David关于Lab4-Ex7用文件模拟虚拟内存，也就是在模拟硬盘嘛！

  这也就对应了**虚拟内存**中页的类别：

  - Cached: already in memory

  - Uncached: not in memory, in disk

  - Unallocated: not in disk

    not in disk是一种理解；也可以理解为进程根本不可能用到虚拟内存的整个大小，所以必然有些页是无效页，也就是没有数据与它关联！

- Simplifies memory management

  Each process gets the same uniform linear address space

- Isolates address spaces

  - One process can't interfere with another's memory
  - User program can't privilege kernel information and code



#### 内存管理机制

**内存管理机制会尽可能地推迟任何内容写回磁盘的时间！**



20`P17，申请新空间(e.g. malloc)时页面的变化：从页表中取unallocated表项，分配并指向disk(虚拟内存)中的一页的，只有当这个页要使用时，才会调入内存(Demand Paging)！



**Loading:** - P22

从二进制文件(elf)中查看代码和数据段的大小，设置虚拟内存的内容，并为其创建PTEs，把其中的valid位设置为0，这样当MMU拿到PTE发现valid=0时，就会PageFault.(Demand Paging)

**也就是说：** Programs and data aren't actually loaded, they aren't just copied in to memeory. It happens as a result of page faults/misses. It is deferred to until a byte in that page is accessed. - **Demand Paging**



#### VM as a tool for Memory Protection

> X86-64实际的虚拟地址只有48位，即虚拟地址空间为2^48，其余的高位全为0或1；
> 高16位可以扩展用于内存保护，只有kernel可以access  - P24



#### Address Translation

44'-47'20' 可以重听：虚拟地址在disk上是如何存储和实现的？

**Q:** How is the virtual address space implemented on disk?

**A:**　对于虚拟内存中的未分配页，disk上自然没有对应页；对于分配了，但是全为0的虚拟页，disk上还是没有保存，只有它modified了，才会在write back to disk；对于在disk上的有对应的页面，有两种可能，一种为该虚拟页对应与disk上user file的某一部分内容，另外一种则不对应于user file，则它保存在swap area or swap file!

**Q: ** When you load a page from disk into memory, does it also cached in the cache memory hierachy?

**A: ** Yes! If you load the entire page , that page will be broken up into 64B blocks and load it into the cache!



---

### Lecture 18: VM Systems

#### Case Study: Core i7

**Q:** Why is instruction-TLB bigger than data-TLB? -P12

**A:** Professor guess that because the penalty for missing on instructions would be much larger than on data!

**21'-24'30''：**　为什么需要设置两级TLB缓存？　（没太听明白） -P12

**Q:** Why they have this second-level cache? Why they just make the L1 cache bigger?

**A:** Hedging your bets! **Still not clear!!!!**

 



**从P19-Linux部分(38'30'')开始，听得有点懵！**

关于内核部分code&data的映射，我想到了XV6中的一个点：

> XV6 kernel的映射方式，好像是一样的啊！内核部分直接进行搬运？

对于Linux, 教授的意思貌似是：
地址空间此处的Physical memory＋Kernel code and data(Identical for each process)是和真实的物理内存一一对应的，所以修改此处，相当于修改真实内存？映射也就是线性映射的！





#### Memory mapping  (50'30'')

虚存是对磁盘(disk)的映射！或者可以理解为虚存就是个大磁盘，只不过这个磁盘内容是由真实磁盘的内容初始化的!

虚存中，除了在真实disk中有对应file的页，其他初始化为0的页，我们可以理解为他在disk中对应一些匿名文件(anonymouse file)，这些匿名文件不占disk空间(全为0，只是个trick, 并非真实存在)，只有在之后程序运行过程中修改了他们，才会写回disk!



**Copy on Write(COW)** - P26!

对于内存，把PTE的private area标记为read only，每个进程的虚拟内存则标记为copy on write；当进行**Fork()**时：

- > Create exact copies of current mm_struct, vm_area_struct, and page tables. 
  >
  > Flag each page in both processes as read-only
  >
  > Flag each vm_area_struct in both processes as private COW

- 复制父进程的mm_struct, vm_area_struct, and page tables. 

- 子进程的读操作：由于复制了映射表，所以是和父进程共享物理页的

  即：只是进行读操作的内存部分，永远不会被复制，多个进程可以一直复用！

- 子进程若要进行写操作，则因为COW，就会自己单独复制该部分到新申请的内存进行写操作！其他指向该部分内存的进程，映射保持不变！

  即：Copying deferred as long as possible!



**Execve()**

What is does: Not creating a new process! It frees all the area structs and page tables for the current process, and the running a new program in a new virtual address space within the current process!