@defgroup CoOS_User_Manaul_Task_Management_cn CoOS任务管理

CooCox CoOS任务管理  {#CooCox_CoOS_Task_Management_cn}
=============
 
## 任务  

在基于OS的应用开发中，一个应用程序通常由若干个任务组成。在CooCox CoOS中，任务通常是一个内部无限循环的C函数，同样有返回值和参数，由于任务永远不会返回，所以任务的返回类型必须定义为void。程序1为一个典型的任务体。

- 程序1 一个无限循环的任务体
~~~.c
void  myTask (void* pdata)
{
     for(;;)
    {
     }
}
~~~
与普通的C函数不同，任务退出是通过调用系统退出的API函数来实现的。若只是通过代码执行结束来表示任务退出，这样将会会导致系统崩溃。
在CooCox CoOS中，可以调用 CoExitTask() 和 CoDelTask(taskID) 来删除一个任务。 CoExitTask() 删除当前正在运行的任务； CoDelTask(taskID) 可以删除其他任务，若参数为当前任务ID，则同 CoExitTask() 一样删除当前任务。具体用法如程序2所示：

- 程序2 删除任务
~~~.c
void  myTask0 (void* pdata)
{
      CoExitTask ( );
} 
void  myTask1 (void* pdata)
{
      CoDelTask(taskID);
}
~~~


## 任务状态

在CooCox CoOS中，一个任务可以存在于以下几种状态之一：
- 就绪态（TASK_READY）：就绪态任务是指那些可以执行（它们不是处于等待态或休眠态）但因当前有一个同等优先级或更高优先级的任务在运行状态而不能运行的任务。任务一旦创建完成就处于此状态。
- 运行态（TASK_RUNNING）：当一个任务真正在运行的时候它就处于运行态。它占用当前CPU。
- 等待态（TASK_WAITING）：等待某个事件发生。在CooCox CoOS中，等待任一事件都将使任务变为等待状态。
- 休眠态（TASK_DORMANT）：任务已被删除，不再参与任务调度。休眠态不同于等待态：处于等待态中的任务当等待的事件满足时会被重新激活，参与任务调度；而处于休眠态的任务则永远不会被再次激活。
      
      
各任务间状态可以进行转换，如下图所示：在CooCox  CoOS中，调用 CoSuspendTask() 可使任务从运行态、就绪态变为等待态，调用 CoAwakeTask() 则可使任务从等待态恢复至就绪态。
![状态转换图](images/1.jpg)

## 任务控制块
任务控制块是CooCox CoOS用来保存任务状态的一种数据结构。CooCox CoOS每创建一个任务，就会分配一个任务控制块来描述该任务（如程序3），以确保该任务的CPU使用权被剥夺后又重新获得时能丝毫不差地继续执行。


任务控制块作为任务的描述快照一直伴随任务存在，直到任务被删除时才被系统收回。
     
### 程序3 任务控制块
~~~.C
typedef struct TCB
{
    OS_STK *stkPtr;                    /*!< The current point of task. */
    U8 prio;                                /*!< Task priority. */
    U8 state;                              /*!< TaSk status. */
    OS_TID taskID;                      /*!< Task ID. */

#if CFG_MUTEX_EN > 0
    OS_MutexID mutexID;              /*!< Mutex ID. */
#endif

#if CFG_EVENT_EN > 0
    OS_EventID eventID;               /*!< Event ID. */
#endif

#if CFG_ROBIN_EN >0
    U16 timeSlice;                         /*!< Task time slice */
#endif

#if CFG_STK_CHECKOUT_EN >0
    OS_STK *stack;                       /*!< The top point of task. */
#endif

#if CFG_EVENT_EN > 0
    void* pmail;                           /*!< Mail to task. */
    struct TCB *waitNext;     /*!< Point to next TCB in the Event waitting list.*/
    struct TCB *waitPrev;     /*!< Point to prev TCB in the Event waitting list.*/
#endif 
#if CFG_TASK_SCHEDULE_EN == 0
FUNCPtr     taskFuc;
OS_STK     *taskStk;
#endif  

#if CFG_FLAG_EN > 0
    void* pnode;                           /*!< Pointer to node of event flag. */
#endif 
#if CFG_TASK_WAITTING_EN >0
    U32 delayTick;                         /*!< The number of ticks which delay. */ 
#endif
    struct TCB *TCBnext;               /*!< The pointer to next TCB. */
    struct TCB *TCBprev;               /*!< The pointer to prev TCB. */
}OSTCB,*P_OSTCB;
~~~

- stkPtr:	指向当前任务的栈顶指针。CooCox CoOS允许每个任务拥有自己的栈，且大小任意。在每次任务切换时，CoOS通过stkPtr所指定的栈来保存CPU的当前运行状态，以便在再次获得CPU运行时间时，能恢复至上一次的运行状态。由于Cortex-M3拥有16个32位通用寄存器来描述CPU的状态，故在CoOS中，任务的栈最小为68字节（最后4个字节用于检查堆栈溢出）。
- prio:	用户指定的任务优先级，CooCox CoOS支持多个任务享有同一优先级。
- state	任务状态。
- taskID	系统分配的任务ID。在CooCox CoOS中，多个任务可以共享同一优先级，优先级并不能作为任务的唯一标识，因此在系统内用任务ID来区分不同的任务。
- mutexID	任务等待的互斥体ID。
- eventID	任务等待的事件ID。
- timeSlice	任务时间片的时间。
- Stack	堆栈的栈底指针，用于进行堆栈溢出检查。
- Pmail	发送给任务的消息指针。
- waitNext	事件等待链表的下一个任务TCB。
- waitPrev	事件等待链表的前一个任务TCB。
- taskFuc	任务的执行函数指针，用于重新激活任务。
- taskStk	任务的起始堆栈指针，用于重新激活任务。
- Pnode	 事件标志的结点指针。
- delayTick	任务处于延时状态时与前一个延时事件的时间差值。
- TCBnext	任务在就绪链表/延时链表/互斥体等待链表中的下一个任务TCB。具体处于哪一链表由用户的任务状态及TCB中相关项决定。若当前任务为就绪态，则任务处于就绪链表。若为等待状态，则通过mutexID来判断：若mutexID不为0xFFFFFFFF，则处于互斥链表中，否则处于延时链表中。
- TCBprev	任务在就绪链表/延时链表/互斥体等待链表中的前一个任务TCB。具体处于哪一链表由用户的任务状态及相关事件标志决定。


每一次创建任务，系统都要从当前空闲的任务控制块链表中分配一项给当前任务。在CooCox CoOS中，系统通过FreeTCB来指定系统的空闲TCB指针，若FreeTCB为NULL，则说明系统没有可分配的TCB，这将导致创建任务失败。
     
在系统初始化时，CoOS将系统内部所有可分配的TCB资源进行管理排序，通过链表的形式来反映当前的TCB状态，如下图所示：
![TCB](images/2.jpg)

每成功创建一个任务，就将FreeTCB作为该任务的TCB，而将FreeTCB的下一项作为新的FreeTCB，直到TCBnext==NULL为止。删除任务和退出任务时，系统回收创建任务时分配出去的TCB，并将其作为下一次分配的FreeTCB，由此保证对已删除任务资源的重新利用。

## 任务就绪链表
- CooCox CoOS将所有就绪态任务的TCB以优先级高低的顺序通过双向链表链接在一起，保证了链表的第一项总是优先级最高、最需要调度的就绪态任务。

- CooCox CoOS允许多个任务共享同一优先级，因此就绪链表中不可避免地会出现相同优先级的任务。对此CooCox CoOS遵循“先入先出（FIFO）”的原则，将迟来的任务排在同一优先级的最后，这样所有处于同一优先级的任务就都能获得相应时间片的CPU运行时间。 
- CooCox CoOS用TCBRdy表示就绪链表的开始，即TCBRdy为就绪链表中优先级最高的任务TCB，故每一个任务调度只需要检查TCBRdy所指任务的优先级是否大于当前正在运行任务的优先级，这样可以最大限度地提升任务调度的效率。
![TCB](images/3.jpg)

## 任务调度

CooCox CoOS 支持时间片轮转和优先级抢占两种任务调度机制。不同优先级任务间为优先级抢占调度，同级优先级任务间为时间片轮转调度。
     
     
CooCox CoOS在以下三种情况下会发生任务调度：
- 1) 比当前运行的任务拥有更高优先级的任务转为就绪态时
- 2) 正在运行的任务状态发生改变，即从运行态变为等待态或休眠态时
- 3) 与当前任务是同一优先级的任务处于就绪态，且当前任务的时间片已到时
     
CooCox CoOS会在每一个系统节拍中断退出及有任务状态改变时，调用任务调度函数，判断当前是否需要进行任务调度。

对于多个同一优先级任务间的调度，则根据其自身任务的时间片进行轮转调度，系统执行完当前任务的时间片长度，就将控制权交给下一个同一优先级任务。下图就描述了三个依次进入就绪状态的相同优先级任务A,B,C（时间片分别为1，2，3）的系统运行状态：
![TCB](images/4.jpg)

在CooCox源码中，各情况下任务调度的实现方式如下：
程序4 高优先级的任务就绪
~~~.C
/* Is higher PRI task coming in? */
if(RdyPrio < RunPrio )	
{
     	TCBNext           = pRdyTcb;	          /* Yes, set TCBNext and reorder ready list*/
     	pCurTcb->state = TASK_READY;
     	pRdyTcb->state = TASK_RUNNING;
     	InsertToTCBRdyList(pCurTcb);	
     	RemoveFromTCBRdyList(pRdyTcb);
}
~~~
程序5 当前任务状态改变
~~~.C
/* Does Running task status change */
else if(pCurTcb->state != TASK_RUNNING)	
{
     	TCBNext           = pRdyTcb;	         /* Yes, set TCBNext and reorder ready list*/
     	pRdyTcb->state = TASK_RUNNING;
     	RemoveFromTCBRdyList(pRdyTcb);
} 
~~~
程序6 相同优先级的任务调度
~~~.C
/* Is it the time for robinning */
else if((RunPrio == RdyPrio) && (OSCheckTime == OSTickCnt))
{
     	TCBNext           = pRdyTcb;       /* Yes, set TCBNext and reorder ready list*/
     	pCurTcb->state = TASK_READY;
     	pRdyTcb->state = TASK_RUNNING;
     	InsertToTCBRdyList(pCurTcb);	
     	RemoveFromTCBRdyList(pRdyTcb);
}
~~~

## 临界区

与其它内核不一样的是：CooCox CoOS并不是通过开关中断来处理临界代码段，而是通过锁定调度器来实现的。因此相对于其它内核，CoOS拥有较短的中断屏蔽时间。

关中断的时间是实时内核开发商应提供的最重要的指标之一，因为这个指标关系到系统对实时事件的响应性。相比其它处理方式，通过锁定调度器能最大程度地提升系统的实时性。
     
由于CooCox CoOS是通过禁止任务调度来管理临界区的，故在此临界代码段内，用户的应用程序不得使用任何能将现行任务挂起的API。也就是说，在临界区内，系统不能调用CoExitTask()、CoSuspendTask()、CoTickDelay()、CoTimeDelay()、CoEnterMutexSection()、CoPendSem()、CoPendMail()、CoPendQueueMail()、CoWaitForSingleFlag()、CoWaitForMultipleFlags()等函数。

程序7 临界区
~~~.C
void Task1(void* pdata)
{
      .....................
      CoSchedLock ( );                                   // Enter Critical Section
      ...............	                                         // Critical Code 
      CoSchedUnlock ( );                               // Exit Critical Section
      .....................
}
~~~

## 中断

在CooCox CoOS中，中断按照在其内部是否调用了系统API函数被划分为两类：调用了操作系统API的中断和与操作系统无关的中断。
     
对于与操作系统无关的中断，CooCox CoOS不强制其进行任何处理，用户所做的任何操作与没有操作系统时一样。
     
而调用了操作系统API的中断，CooCox CoOS在中断进入和退出时要求必须调用相关函数，如程序8：

程序8 调用系统API中断处理程序
~~~.C
void WWDG_IRQHandler(void)
{
      CoEnterISR ( );                               // Enter the interrupt
      isr_SetFlag(flagID);                       // API function 
     ..................;                              // Interrupt service routine 
      CoExitISR ( );                                // Exit the interrupt 
}
~~~

在CooCox CoOS中，所有可在中断服务程序里调用的系统API均以isr_开头，如isr_PostSem()、isr_PostMail()、isr_PostQueueMail()、isr_SetFlag()，其它任何API的调用，都有可能导致系统运行混乱。

在中断服务程序里调用相应的API函数，需要判断当前任务调度是否被锁定。若未被锁定，则正常调用。若被锁定，则需要发送一个相应的服务请求至服务请求列表，等待解锁调度器来响应请求列表里的服务请求。


