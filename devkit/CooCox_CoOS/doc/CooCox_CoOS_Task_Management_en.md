@defgroup CoOS_User_Manaul_Task_Management_en CoOS Task Management

CooCox CoOS Task Management  {#CooCox_CoOS_Task_Management_en}
=============
 
## Task   

During OS-based application development, an application is usually seperated into a number of tasks. In CooCox CoOS, a task is a C function whose inside is a infinite loop, it also has the return values and parameters. However, since a task will never return, the returned type must be defined as void. Code 1 shows a typical task structure.

- Code 1 An infinite loop task
~~~.c
void  myTask (void* pdata)
{
     for(;;)
    {
     }
}
~~~

Which is different from the C function, the quit of a task is achieved by calling system API function. Once you quit the task only through the ending of the code execution, the system would breakdown.

You can delete a task by calling CoExitTask() or CoDelTask(taskID) in CooCox CoOS. CoExitTask() is called to delete the current running task while CoDelTask(taskID) to delete others. If the incoming parameter is the current task ID, CoDelTask(taskID) can also delete the current task. The concrete use is shown in Code 2.

- Code 2 Deleting a task
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


## Task  State

A task can exist in one of the following states in CooCox CoOS.
- *Ready State(TASK_READY)*:Ready tasks are those that are able to execute (they are not waiting or dormant) but are not currently executing because a different task of equal or higher priority is already in the Running state. A task will be in this state after being created.
- *Running State(TASK_RUNNING)*:When a task is actually executing it is said to be in the Running state. It is currently occupying the processor.
- *Waiting State(TASK_WAITING)*:Wait for an event to occur. A task will be in the waiting state if it is currently waiting for a certain event in CooCox CoOS.
- *The Dormant State(TASK_DORMANT)*:The task has been deleted and is not available for scheduling. The dormant state is not the same as the waiting state. Tasks in the waiting state will be reactivated and be available for the scheduling when its waiting events have satisfied. However, tasks in the dormant state will never be reactivated.
      
      
 The state of a task can be changed among the above four states. You can call CoSuspendTask() to convert a task which in the running or ready state to the waiting state. By calling CoAwakeTask() you can also convert the state of a task from the waiting state to the ready state(as shown in Figure 2.2.1).
![状态转换图](images/1.jpg)

## Task control block

Task control block is a data structure used to save the state of a task in CooCox CoOS. Once a task has been created, CooCox CoOS will assign a task control block to describe it (as shown in code 3). This ensure the task can execute accurately when it obtains the CPU runtime again.
     
Task control block always accompanies with the task as a description snapshot. It will not be recovered by the system until the task being deleted.

     
### Code 3 Task control block
~~~.C
typedef struct TCB
{
    OS_STK *stkPtr;           /*!< The current point of task. */
    U8 prio;                  /*!< Task priority. */
    U8 state;                 /*!< TaSk status. */
    OS_TID taskID;            /*!< Task ID. */

#if CFG_MUTEX_EN > 0
    OS_MutexID mutexID;       /*!< Mutex ID. */
#endif

#if CFG_EVENT_EN > 0
    OS_EventID eventID;       /*!< Event ID. */
#endif

#if CFG_ROBIN_EN >0
    U16 timeSlice;            /*!< Task time slice */
#endif

#if CFG_STK_CHECKOUT_EN >0
    OS_STK *stack;            /*!< The top point of task. */
#endif

#if CFG_EVENT_EN > 0
    void* pmail;              /*!< Mail to task. */
    struct TCB *waitNext;     /*!< Point to next TCB in the Event waitting list.*/
    struct TCB *waitPrev;     /*!< Point to prev TCB in the Event waitting list.*/
#endif 
#if CFG_TASK_SCHEDULE_EN == 0
FUNCPtr     taskFuc;
OS_STK     *taskStk;
#endif  

#if CFG_FLAG_EN > 0
    void* pnode;                       /*!< Pointer to node of event flag. */
#endif 
#if CFG_TASK_WAITTING_EN >0
    U32 delayTick;                     /*!< The number of ticks which delay. */ 
#endif
    struct TCB *TCBnext;               /*!< The pointer to next TCB. */
    struct TCB *TCBprev;               /*!< The pointer to prev TCB. */
}OSTCB,*P_OSTCB;
~~~

- stkPtr:	A pointer to the current task's top of stack. CooCox CoOS allows each task to have its own stack of any size. During every task switches, CoOS saves the current CPU running state through the stack that stkPtr pointed to so that the task can come back to the previous running state when it gets the CPU runtime again. Since Cortex-M3 has 16 32-bit general-purpose registers to describe the CPU states, the minimum size of stack for a task is 68 bytes(other 4 bytes are used to checking stack overflow)
- prio:	The task priority that you assigned. Multiple tasks can share the same priority in CooCox CoOS.
state	The state of the task
- taskID	The task ID that system assigned. Since multiple tasks can share the same priority, the priority can not be used as the unique identifier. We use task ID to distinguish different tasks in CooCox CoOS
- mutexID	The mutex ID that the task waiting for
- eventID	The event ID that the task waiting for.
- timeSlice	The time slice of the task
- Stack	 A pointer to the bottom of a stack. It can be used to check the stack overflow
- Pmail	The message pointer sent to the task
- waitNext	The TCB of the next task in the event waiting list.
- waitPrev	The TCB of the previous task in the event waiting list
- taskFuc	Task function pointer, to active task
- taskStk	A pointer to the current task's top of stack, to active task
- Pnode	The pointer to the node of the event flag
- delayTick	The time difference between the previous delayed event and the task when it is in the delayed state
- TCBnext	The next TCB when a task is in the ready list / delayed list / mutex waiting list. Which list the task belongs to is determined by the task state and the item of the TCB. If the current task is in the ready state, it is in the ready list. If it is in the waiting state, then judged by the mutexID and delayTick: If mutexID is not 0xFFFFFFFF, it is in the mutex waiting list; else if delayTick is not 0xFFFFFFFF, it is in the delayed list.
- TCBprev	The previous TCB when a task is in the ready list /delayed list /mutex waiting list. Which list the task belongs to is determined by the task state and the relevant event flag


System will assign a block to the current task from the current free TCB list while creating a task. The free TCB pointer is designated by FreeTCB in CooCox CoOS. If the FreeTCB is NULL, there is no TCB to assign and the task will fail to create.

While system initializing, CoOS will sort all the assignable TCB resources and then reflect the current state of the TCB through the forms of lists, as follows:
![TCB](images/2.jpg)

Every time you create a task successfully, the FreeTCB will be assigned to this task and its next item will be the new FreeTCB until it equals NULL. When a task is deleted or exited, the system will recover the TCB which had been assigned to this task when it was created and then assign it as the FreeTCB of the next time so as to reuse the resources of the deleted task.

## Task Ready List

- CooCox CoOS links all the TCB of ready tasks together according to the level of the priority through two-way linked list. This ensures that the first item of the list is always the one which has the highest priority and is the most in need of task scheduling.

- CooCox CoOS allows multiple tasks to share the same priority level. Therefore, tasks with the same priority will inevitably occur in the ready list. CooCox CoOS follows the principle "first-in-first out (FIFO)": put the latest task in the last of the tasks which share the same priority so that all of them can obtain its own CPU runtime. 

- TCBRdy is the beginning of the ready list in CooCox CoOS. In other words, TCBRdy is the TCB of the task which has the highest priority in the ready list. Therefore, when starting a task scheduling, which only need to be checked is whether the priority of the task that TCBRdy pointed to is higher than the current running one. In this way, the efficiency of the task scheduling can be improved to the maximum.
![TCB](images/3.jpg)

## Task Scheduling
     
CooCox CoOS supports two kinds of scheduling mode, preemptive priority and round-robin. The former is used among tasks of different priority, while the latter among tasks of the same priority.

CooCox CoOS will start a task scheduling in the following three situations:
     
- 1) A task whose priority is higher than the current running one is converting to the ready state
- 2) The current running task is changing from the running state to the waiting or dormant state
- 3) A task sharing the same priority with the current running task is in the ready state, and meanwhile the time slice of the current task runs out

When a system tick interrupt exits or some tasks’ states have changed, CooCox CoOS will call the task scheduling function to determine whether it is essential to start a task scheduling or not.

For the scheduling of tasks sharing the same priority, the system starts the rotation scheduling according to the time slice of each task. When the system has run out the time slice of the current task, it will give the right of control to the next task with the same priority. Figure 2.5.1 shows the system running state of the three tasks (A, B, C with their respective time slices 1, 2, 3) with the same priority when they are entering the ready state in turn.
![TCB](images/4.jpg)

In CooCox CoOS source codes, the implementing of task scheduling is shown as follows:

Code 4 Task with a higher priority is ready
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
Code 5 The state of the current task changes
~~~.C
/* Does Running task status change */
else if(pCurTcb->state != TASK_RUNNING)	
{
     	TCBNext           = pRdyTcb;	         /* Yes, set TCBNext and reorder ready list*/
     	pRdyTcb->state = TASK_RUNNING;
     	RemoveFromTCBRdyList(pRdyTcb);
} 
~~~
Code 6 The task scheduling among the same priority tasks 
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

## Critical Section

Different from other kernels, CooCox CoOS does not handle the critical code section by closing interrupts, but locking the scheduler. Therefore, CoOS has a shorter latency for interrupt compared with others.

Since the time of enabling the interrupt relates to system responsiveness towards the real-time events, it is one of the most important factors offered by the real-time kernel developers. By locking the scheduler we can improve system real-time feature to the maximum comparing to other approaches.

Since CooCox CoOS manages the critical section by forbidding to schedule task, user applications cannot call any API functions which will suspend the current running task in critical sections, such as CoExitTask(), CoSuspendTask(), CoTickDelay(), CoTimeDelay(), CoEnterMutexSection(), CoPendSem(), CoPendMail(), CoPendQueueMail(), CoWaitForSingleFlag(), CoWaitForMultipleFlags() and so on.
Code 7 Critical Section
~~~.C
void Task1(void* pdata)
{
      .....................
      CoSchedLock ( );                                 // Enter Critical Section
      ...............	                               // Critical Code 
      CoSchedUnlock ( );                               // Exit Critical Section
      .....................
}
~~~

## Interrupts

In CooCox CoOS, the interrupt is divided into two categories according to whether called the system API functions inside or not.

For the ISR which has nothing to do with OS, CooCox CoOS does not force it to do anything and you can operate just like there is not an OS.

However, for the ISR which called the system API functions inside, CooCox CoOS demands that you call the relevant functions when entering or exiting the interrupt (as shown in code 8).

Code 8 The interrupt handler which called the system API
~~~.C
void WWDG_IRQHandler(void)
{
      CoEnterISR ( );                               // Enter the interrupt
      isr_SetFlag(flagID);                          // API function 
     ..................;                            // Interrupt service routine 
      CoExitISR ( );                                // Exit the interrupt 
}
~~~

All the system API which can be called in the interrupt service routine begin with isr_, such as isr_PostSem(), isr_PostMail(), isr_PostQueueMail() and isr_SetFlag().The calling of any other API inside the ISR will lead to the system chaos.

When calling the corresponding API functions in the interrupt service routine, system need to determine whether the task scheduling is locked or not. If it is unlocked, system can call it normally. Otherwise, system will send a relevant service request to the service request list and then wait for the unlocking of the scheduler to respond it.


