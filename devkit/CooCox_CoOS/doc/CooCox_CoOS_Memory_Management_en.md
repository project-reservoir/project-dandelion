@defgroup CoOS_User_Manaul_Memory_Management_en CoOS Memory Management

CooCox CoOS  Memory Management  {#CooCox_CoOS_Memory_Management_en}
============= 


## Static Memory Allocation

Static memory allocation applies to the condition that you know how much memory you need to occupy while compiling, and the static memory can’t be released or redistributed while system running. Compared with the dynamic memory allocation, static memory allocation does not need to consume CPU resources. At the same time, it would not lead to allocate unsuccessfully (because allocating unsuccessfully will directly result in the failure of the compiler). Therefore it is faster and more secure.

In CooCox CoOS, the memory that each module’s control block needs is allocated statically, such as the task control block（TCB）、event control block(ECB) 、flag control block（FCB）、flag node（FLAG_NODE）and so on.

Code 1 CooCox CoOS's space allocation of the TCB
~~~.C
OsConfig.h 
    #define CFG_MAX_USER_TASKS  (8)       // Determine the largest number of                                                                    // user's task in the system 
task.h 
    #define SYS_TASK_NUM               (1)     // Determine the number of system tasks 
     task.c                                                  
                                                                 // Allocate TCB space statically 
OSTCB    TCBTbl[CFG_MAX_USER_TASKS+SYS_TASK_NUM]; 
~~~

## Dynamic memory management

Dynamic memory allocation applies to the conditions that the memory size can not be determined while compiling but by the runtime environment of the code during the system is running. It could be said that the static memory allocation is based on plans while the dynamic memory allocation is based on need.

Dynamic memory allocation is more flexible, and can greatly improve the utilization of the memory. However, since every allocation and release will consume CPU resources, and there may be a problem of allocation failure and memory fragmentation, you need to determine whether the allocation is successful or not during every dynamic memory allocation.

Here are some conventional methods of dynamic memory allocation—— the implementation of malloc ( ) and free ( ). In the usual kernel or compiler, the memory block is allocated to the free list or the list of allocation respectively according to its allocation condition.

The related steps of the system are as follows while calling malloc():

- 1) Find a memory block whose size meets user’s demand. Since the search algorithm is different, the memory blocks found by the system are not the same. The most commonly used algorithm is the first matching algorithm, that is, allocate the first memory block which meets user’s demand. By doing this, it could avoid to traverse all the items in the free list during each allocation.

- 2) Divide the memory block into two pieces: the size of first piece is the same to user’s demand, and the second one storages the remaining bytes.
     
- 3) Pass the first piece of memory block to the user. The other one (if any) will be returned to the free list.
     
System will link the memory block that you released to the free list, and determine whether the memory’s former or latter memory is free. If it is free, combine them to a larger memory block.
![M_list](images/7.jpg)

From the above, we can see that the free list will be divided into many small pieces after allocating and releasing the memory many times. If you want to apply for a large memory block at this time, the free list may not have the fragment that meets user’s demand. This will lead to the so-called memory fragmentation. In addition, the system needs to check the whole free list from the beginning to determine the location that the memory block needs to plug in, which leads the time of releasing the memory too long or uncertain.

CooCox CoOS provides two mechanisms of partitioning to solve these problems: the partition of fixed length and the partition of variable length.

### Fixed-length partition
     
It provides memory allocation mode of fixed length partition in CooCox CoOS. The system will divide a large memory block into numbers of pieces whose size are fixed, then link these pieces through the linked list. You can allocate or release the memory block of fixed length through it. In this way, we not only can ensure that the time of allocation or release is fixed, but also can solve the problem of memory fragmentation
![8](images/8.jpg)
CooCox CoOS can manage a total of 32 fixed-length partitions of different size. You can create a partition of fixed length by calling CoCreateMemPartition(). After being created successfully, you can allocate or release the memory block by calling CoGetMemoryBuffer() and CoFreeMemoryBuffer(). You can also get the number of free memory blocks in current memory partition by calling CoGetFreeBlockNum().

Code 2 The creation and use of fixed-length partition
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

###  Variable-length partition

From the implement of conventional dynamic memory shown above, we can see that it need to operate the free list and the allocated list at the same time while release the memory, which require a long operation time and has impaction to the CPU. For these reasons, CooCox CoOS redesigns the list to ensure that both the allocation and release of memory only require to search just one list.

![9](images/9.jpg)

From the figure we can see that all the free memory in the system can separately form a one-way list so that it is more convenient to look up the list while allocate the memory. To all the memory blocks, whether they are free or have been allocated, CooCox CoOS will link them through a doubly linked list. Thus, when an allocated memory releases, there is no need to find the insertion point of the memory from the beginning of the free list. You only need to find the former free block in the doubly linked list and then insert the memory into the free list, which has greatly improved the speed of memory releasing.

In CooCox CoOS, since all the memory is 4-byte alignment (if the space that needs to allocate is not 4-byte alignment, force it to be), the last two bits of the previous and next memory address that saved in the head of the memory are all invalid. Therefore, CooCox CoOS determines whether the memory is free through the least significant bit: bit 0 refers to the free block, otherwise refers to the allocated ones. If the memory list points to the free block, get the list address directly. If it points to the allocated one, decrease it by one.

Code 3 The head of allocated memory block 

~~~.C
typedef struct UsedMemBlk 
{
    void* nextMB;  
    void* preMB;     
}UMB,*P_UMB; 
~~~

Code 4 The head of free memory block

~~~.C
typedef struct FreeMemBlk
{
       struct FreeMemBlk* nextFMB;
       struct UsedMemBlk* nextUMB;
       struct UsedMemBlk* preUMB;
}FMB,*P_FMB;
~~~

For memory block 1 and 2, the memory block itself preserves the address of the previous free memory block when released, so it is very easy to plug back to the free list. You only need to decide whether to merge them according to if the memory block’s next block is free.

For memory block 3 and 4, its previous memory block is not a free memory block when released. It is essential to get the previous free memory address through two-way list when plug it back to the free list.

Code 5 To get the address of the previous free memory block

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

In the file OsConfig.h, you can determine whether it is essential to add variable-length partition to the kernel, and set the size of the memory partition at the same time.

Code 6 config.h file 
~~~.C
config.h file
Config.h
#define CFG_KHEAP_EN            (1)                
#if CFG_KHEAP_EN >0
#define KHEAP_SIZE               (50)                // size(word)
#endif 
~~~

You could implement the allocation and release of the memory by calling CoKmalloc() and CoKfree() respectively after ensuring enabling the variable-length partition. The memory size that CoKmalloc() applied is in bytes.

Code 7 The use of the variable-length partition

~~~.C
void myTask (void* pdata)
{
      void* data;
      data = CoKmalloc(100);
      ......
      CoKfree(data );
}
~~~


## Stack Overflow Check
     
Stack Overflow refers to that the size of the stack used when a task is running exceeds the size that assigned to the task, which results in writing data to the memory outside the stack. This may lead to the coverage of the system or other tasks’ data as well as the exception of memory access. The stack size assigned to each task is fixed in multi-tasking kernel. Once the stack overflow is not handled when the system is running, it may lead to system crashes.

When creating a task in CooCox CoOS, the system will save the stack bottom address in the task control block and write a special value into the memory block of the stack bottom address in order to judge whether the stack overflows. CooCox CoOS will check whether there is a stack overflow during each task scheduling.

Code 8 Stack overflow inspection
~~~.C
if((pCurTcb->stkPtr < pCurTcb->stack)||(*(U32*)(pCurTcb->stack) != MAGIC_WORD))       
{                                                             
       CoStkOverflowHook(pCurTcb->taskID);         /* Yes,call hander */      
}
~~~

When stack overflow in a task, the system will call CoStkOverflowHook(taskID) automatically. You can add the handling of stack overflow in the function. The parameter of this function is the ID of the task which has stack overflow.

Code 9 The handling function of stack overflow
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