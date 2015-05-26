@defgroup CoOS_User_Manaul_Memory_Management_cn CoOS内存管理

CooCox CoOS 内存管理  {#CooCox_CoOS_Memory_Management_cn}
============= 


## 静态内存分配

静态内存分配适用于在代码编译时就已经确定需要占用多少内存的情况，在整个代码运行过程中不能进行释放和重新分配。相对于动态内存分配，静态内存分配不需要消耗CPU资源，也不会出现分配不成功的现象（因为分配不成功将直接导致编译失败），因此其速度更快，也更安全。
     
在CooCox CoOS中，各个模块控制块所需的内存都是静态分配的，如任务控制块（TCB）、事件控制块（ECB）、事件标志控制块（FCB）、事件标志结点（FLAG_NODE）等等。

程序1 CooCox CoOS任务控制块的空间分配
~~~.C
OsConfig.h 
    #define CFG_MAX_USER_TASKS  (8)       // Determine the largest number of                                                                    // user's task in the system 
task.h 
    #define SYS_TASK_NUM               (1)     // Determine the number of system tasks 
     task.c                                                  
                                                                 // Allocate TCB space statically 
OSTCB    TCBTbl[CFG_MAX_USER_TASKS+SYS_TASK_NUM]; 
~~~

## 动态内存分配

动态内存分配适用于系统运行时内存块大小不能在代码编译时确定，而要根据代码的运行环境来确定的情况。可以说，静态内存分配是按计划分配，动态内存分配是按需要分配。

虽然动态内存分配比较灵活，能极大的提高内存的利用率，但由于每一次的分配和释放都需要消耗CPU资源，并且有可能存在分配失败和内存碎片问题，所以每一次动态内存分配都需要程序员来确定是否成功。

下面介绍一下常规的动态内存分配方式—— malloc() 和 free()的实现方式。在通常的编译器或者系统内核中，内存块根据其是否已被分配分别存放于空闲链表和已分配链表中。

调用malloc函数时，系统相关操作步骤如下：

- 1) 沿空闲链表寻找一个大小满足用户需求的内存块。由于查找算法不同，系统所找到的符合用户需求的内存块也不尽相同。而最常用的查找算法为最先匹配算法，即使用第一块满足用户需求的内存块进行分配，这样可以避免每次分配时都遍历所有的空闲链表项。
- 2) 将该内存块一分为二：其中一块的大小与用户请求的大小相等，另一块存储剩余的字节。
- 3) 将分配给用户的那一块内存传给用户，并将另一块（如果有的话）返回到空闲链表上。

调用free函数时，系统将用户释放的内存块链接到空闲链表上，并判断该内存块前后的内存块是否空闲，若为空闲则合并成一个大的内存块。
![M_list](images/7.jpg)

由上述可知，多次内存分配和释放后，空闲链表会被分割成很多的小块，若此时用户申请一个大的内存块，则空闲链表上可能没有满足用户需求的片段了，这就产生了所谓的内存碎片。另外，在每次释放内存时，系统都需要从空闲链表第一项开始检查整个链表以确定该内存块需要插回的位置，这就导致释放内存的时间不确定或过于漫长。
     
为了解决上述问题，CooCox CoOS提供了两种分区机制：固定长度分区和可变长度分区。

### 固定长度分区
     
在CooCox CoOS中提供固定长度分区的内存分配方式，系统根据用户给定的参数，将一个大的内存块分成整数个大小固定的内存块，通过链表的形式将这些内存块连接起来，用户可以从中分配和释放固定大小的内存块，这样既保证了分配和释放时间的固定，也解决了内存碎片的问题。
![固定长度分区](images/8.jpg)
CooCox CoOS一共可以管理32个大小不同的固定长度分区。用户可以通过调用 CoCreateMemPartition() 来创建一个固定长度分区，成功创建内存分区后，用户可以调用 CoGetMemoryBuffer() 和 CoFreeMemoryBuffer() 来进行内存块的分配和释放，也可以调用 CoGetFreeBlockNum()来获得当前内存分区空闲内存块的个数。

程序2 固定长度分区的创建和使用
~~~.C
U8 memPartition[128*20];
OS_MMID memID;
void  myTask (void* pdata)
{
        U8* data;
        memID = CoCreateMemPartition(memPartition,128,20);
        if(CoGetFreeBlockNum(memID ) != 0)
        {
                data = (U8*)CoGetMemoryBuffer(memID );
        }
        ..................
        CoFreeMemoryBuffer(memID ,data);
}
~~~

### 可变长度分区

从上文分析的常规动态内存实现可知：内存释放时需要同时操作空闲链表和已分配链表，这要求比较长的操作时间，且影响CPU运行效率。对此CooCox CoOS重新设计链表，使分配和释放内存均只需要搜索一张表即可。

![可变长度分区](images/9.jpg)

由上图可知，系统内的所有空闲内存块可单独组成一个单向链表，这样内存分配时，查表比较方便。对于所有的内存块，不管其是已分配还是空闲，CooCox CoOS均通过一个双向链表将其链接起来。因此当一个已分配的内存块释放时，无需再从空闲链表头开始查找确定该内存块的插入点，而只需在该内存块双向链表找到前一个空闲块，即可将此内存块插入至空闲链表，这极大地提高了内存块的释放速度。
     
在CooCox CoOS中，由于所有的内存块都要求4字节对齐（若需要分配的空间未4字节对齐，强制为4字节对齐），所以内存块head所保存的前后内存块地址的后两位均无效。因此，CooCox CoOS通过最低位来判断该内存块是否为空闲块，若最低位为0，则表示为空闲块，否则为已分配内存块。因此若内存块链表指向的是空闲块，直接获得链表地址即可。若指向的是已分配内存块，则需要进行减一操作。

程序3 已分配内存块head

~~~.C
typedef struct UsedMemBlk 
{
    void* nextMB;  
    void* preMB;     
}UMB,*P_UMB; 
~~~

程序4 空闲内存块head

~~~.C
typedef struct FreeMemBlk
{
       struct FreeMemBlk* nextFMB;
       struct UsedMemBlk* nextUMB;
       struct UsedMemBlk* preUMB;
}FMB,*P_FMB;
~~~

对于内存块1和2，当其释放时，内存块本身保存了前一个空闲内存块地址，因此极容易插回空闲链表，这时候程序只需要根据该内存块的后一内存块是否为空闲块来决定是否进行合并。

对于内存块3和4，当其释放时，其前一个内存块并不是空闲内存块，这时还需要通过双向链表获得此时的前一个空闲内存块地址，才能将其插回至空闲链表。

程序5 获得前一个空闲内存块地址

~~~.C
U8 memPartition[128*20];
OS_MMID memID;
void  myTask (void* pdata)
{
        U8* data;
        memID = CoCreateMemPartition(memPartition,128,20);
        if(CoGetFreeBlockNum(memID ) != 0)
        {
                data = (U8*)CoGetMemoryBuffer(memID );
        }
        ..................
        CoFreeMemoryBuffer(memID ,data);
}
~~~

用户可以在config.h文件中确定是否需要在内核内添加可变长度分区，并同时设置该内存分区的大小。

Code 6 
~~~.C
config.h file
Config.h
#define CFG_KHEAP_EN            (1)                
#if CFG_KHEAP_EN >0
#define KHEAP_SIZE               (50)                // size(word)
#endif 
~~~

确定开启可变长度分区后，用户可以在代码里调用 CoKmalloc() 和 CoKfree() 来实现内存的分配和释放， CoKmalloc() 申请的内存大小以字节为单位。

程序7 可变长度分区的使用

~~~.C
void myTask (void* pdata)
{
      void* data;
      data = CoKmalloc(100);
      ......
      CoKfree(data );
}
~~~


## 堆栈溢出检查

堆栈溢出是指任务在运行时使用的堆栈大小超过了分配给任务堆栈的大小，结果导致向堆栈外的内存写入了数据。这样可能导致覆盖了系统或者其他任务的数据，也可能会导致内存访问异常。在多任务内核中，为每一个任务分配的堆栈大小均为固定，在系统运行时，若发生堆栈溢出且没有做处理，则可能导致系统崩溃。
     
在CooCox CoOS中创建任务时，系统将在任务控制块中保存堆栈的栈底地址，并在栈底地址所对应的内存块中写入一特殊值，用此来判断堆栈是否溢出。CooCox CoOS会在每次任务调度时检查是否发生堆栈溢出。

程序8 堆栈溢出检查
~~~.C
if((pCurTcb->stkPtr < pCurTcb->stack)||(*(U32*)(pCurTcb->stack) != MAGIC_WORD))       
{                                                             
       CoStkOverflowHook(pCurTcb->taskID);         /* Yes,call hander */      
}
~~~

当任务发生堆栈溢出时，系统自动调用 CoStkOverflowHook(taskID) 函数，用户可以在此函数中添加对堆栈溢出的处理，函数的参数为发生堆栈溢出的任务ID号。

程序9 堆栈溢出处理函数
~~~.C
void CoStkOverflowHook(OS_TID taskID)
{
    /* Process stack overflow in here */
    for(; ;) 
    {
        …
    }
}
~~~