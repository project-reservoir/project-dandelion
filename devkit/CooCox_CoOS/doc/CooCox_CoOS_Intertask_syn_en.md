@defgroup CoOS_User_Manaul_Intertask_syn_en CoOS Intertask Synchronization & Communication

CooCox CoOS Intertask Synchronization & Communication {#CooCox_CoOS_Intertask_syn_en}
============= 


## Intertask Synchronization

Intertask synchronization refers to that one task can only keep on executing after it has got the synchronization signal sent by another task or the ISR. There are semaphores, mutexes and flags to implement intertask synchronization in CooCox CoOS.

### Semaphores
     
Semaphores provide an effective mechanism for the system to handle the critical section and implement intertask synchronization.

The action of semaphore can be described as the classical PV operation:
~~~.C
P Operation: while( s==0); s--;
V Operation: s++;
~~~
     
You can create a semaphore by calling CoCreateSem() in CooCox CoOS. After the semaphore has been created successfully, you can obtain it by calling CoPendSem() or CoAcceptSem(). If there is no free semaphore, CoPendSem() will wait for a semaphore to be released while CoAcceptSem() will return the error immediately. You can also call CoPostSem() in the task or isr_PostSem() in the ISR to release a semaphore for the purpose of achieving synchronization.

Code 1 The creation of the semaphore
~~~.C
ID0 = CoCreateSem(0,1,EVENT_SORT_TYPE_FIFO); // initCnt=0,maxCnt=1,FIFO
ID1 = CoCreateSem(2,5,EVENT_SORT_TYPE_PRIO); // initCnt=2,maxCnt=5,PRIO
~~~

Code 2 The use of the semaphore
~~~.C
void myTaskA(void* pdata)
{
       ..........
       semID = CoCreateSem(0,1,EVENT_SORT_TYPE_FIFO); 
       CoPendSem(semID,0);
       ..........
}
void myTaskB(void* pdata)
{
       ......
       CoPostSem(semID); 
       ......
}
void myISR(void)
{
       CoEnterISR();
       ......
       isr_PostSem(semID);
       CoExitISR();
}
~~~

###  Mutexes
     
Mutexes have solved the "mutually exclusion" problem in CooCox CoOS. It forbids multiple tasks to enter the critical code section at the same time. Therefore, only one task can enter it at any time.

In CooCox CoOS, the mutex section has also considered the issue of  priority inversion. CooCox CoOS has solved this issue by the method of priority inheritance.

Priority inversion refers to that the high-priority task is waiting for the low-priority task to release resources, at the same time the low-priority task is waiting for the middle priority task’s.

There are two classical methods to prevent the inversion at present:
- 1) The priority inheritance strategy: The task which is possessing the critical section inherits the highest priority of all the tasks that request for this critical section. When the task exits from the critical section, it will restore to its original priority.
- 2) The ceiling priority strategy: Upgrade the priority of the task which requests a certain resource to the highest priority of all the tasks that be likely to access this resource (and the highest priority is called the ceiling priority of this resource).

The priority inheritance strategy has a less impact to the flow of the task execution since it only upgrades the priority of the low-priority task when a high-priority task is requesting for the critical resource that being occupied by the low-priority task. However, the ceiling priority strategy upgrades one task’s priority to the highest when the task is occupying the critical resource.

CooCox CoOS prevents the priority inversion by the method of priority inheritance.

The following figure describes the task scheduling of three tasks when there are mutex sections in CooCox CoOS. TaskA has the highest priority while TaskC has the lowest. The blue boxes refer to the mutex sections.
![10](images/10.jpg)
     
You can create a mutex section by calling CoCreateMutex(). Calling CoEnterMutexSection() and CoLeaveMutexSection() to enter or leave the mutex section so that we can protect the codes in critical section.

Code 3 The use of the mutex section  

~~~.C
void myTaskA(void* pdata)
{
       mutexID = CoCreateMutex();
       CoEnterMutexSection(mutexID );   // enter the mutex section
       ...........                                            // critical codes
       CoLeaveMutexSection(mutexID );  // leave the mutex section
}
void myTaskB(void* pdata)
{ 
        CoEnterMutexSection(mutexID );   // enter the mutex section
        ...........                                            // critical codes
        CoLeaveMutexSection(mutexID );   // leave the mutex section
}
~~~
### Flags

When a task wants to synchronize with a number of events, flags are needed. If the task synchronizes with a single event, it can be called independent synchronization (logical OR relationship). If it synchronizes with a number of events, then called associated synchronization (logical AND relationship).

CooCox CoOS supports 32 flags to the maximum at the same time. It supports that multiple tasks waiting for a single event or multiple events. When the flags that the waiting tasks waiting for are in the not-ready state, these tasks can not be scheduled. However, once the flags turn to the ready state, they will be resumed soon.

According to the types of the flags, the side effects are different when the tasks have waited for the flags successfully. There are two kinds of flags in CooCox CoOS: the ones reset manually and the ones reset automatically. When a task has waited for a flag which reset automatically, the system will convert the flag to not-ready state. On the contrary, if the flag is reset manually, there won’t be any side effect. Therefore, when a flag which reset manually converts to the ready state, all the tasks which waiting for this event will convert to the ready state as far as you call CoClearFlag() to reset the flag to the not-ready state. When a flag which reset automatically converts to the ready state, only one task which waiting for this event will convert to the ready state. Since the waiting list of the event flags is ordered by the principle of FIFO, towards the event which reset automatically only the first task of the waiting list converts to the ready state and others that waiting for this flag are still in the waiting state.

Suppose there are three tasks (A, B, C) waiting for the same flag I which reset manually. When I is ready, all the tasks will be converted (A, B, C) to the ready state and then inserted into the ready list. Suppose I is a flag which reset automatically and the tasks (A, B, C) are listed in sequence in the waiting list. When I is ready, it will inform task A. Then I will be converted to the not-ready state. Therefore B and C will keep waiting for the next ready state of flag I in the waiting list. 

You can create a flag by calling CoCreateFlag() in CooCox CoOS. After being created, you can call CoWaitForSingleFlag() and CoWaitForMultipleFlags() to wait for a single flag or multiple flags.

Code 4 Wait for a single flag

~~~.C
void myTaskA(void* pdata)
{
       ..........
       flagID = CoCreateFlag(0,0);      // Reset manually, the original state is not-ready
       CoWaitForSingleFlag(flagID,0);    
      ..........
}
void myTaskB(void* pdata)
{
       ......
       CoSetFlag(flagID); 
     ......
}
~~~
Code5 Wait for multiple flags
~~~.C
void myTaskA(void* pdata)
{
      U32 flag;
      StatusType err;
      ..........
      flagID1 = CoCreateFlag(0,0);    // Reset manually, the original state is not-ready
      flagID2 = CoCreateFlag(0,0);    // Reset manually, the original state is not-ready
      flagID3 = CoCreateFlag(0,0);    // Reset manually, the original state is not-ready
      flag = (1<<flagID1) | (1<<flagID2) | (1<<flagID3);
      CoWaitForMultipleFlags(flag,OPT_WAIT_ANY,0,&err);    
      ..........
}
void myTaskB(void* pdata)
{
      ......
      CoSetFlag(flagID1); 
      ......
}
void myISR(void)
{
      CoEnterISR();
      ......
      isr_SetFlag(flagID2);
      CoExitISR();
}
~~~


## Intertask Communication

Information transfer is sometimes needed among tasks or between the task and the ISR. Information transfer can be also called intertask communication.
     
There are two ways to implement it: through the global variable or by sending messages.

When using the global variable, it is important to ensure that each task or ISR possesses the variable alone. The only way to ensure it is enabling the interrupt. When two tasks share one variable, each task possesses the variable alone through firstly enabling then disabling the interrupt or by the semaphore (see chapter 5.1). Please note that a task can communicate with the ISR only through the global variable and the task won’t know when the global variable has been modified by the ISR (unless the ISR sends signals to the task in manner of semaphore or the task keeps searching the variable’s value). In this case, CooCox CoOS supplies the mailboxes and the message queues to avoid the problems above.

### Mailboxes
     
System or the user code can send a message by the core services. A typical mail message, also known as the exchange of information, refers to a task or an ISR using a pointer variable, through the core services to put a message (that is, a pointer) into the mailbox. Similarly, one or more tasks can receive this message by the core services. The tasks sending and receiving the message promise that the content that the pointer points to is just that piece of message.

![M_list](images/11.jpg)

The mailbox of CooCox CoOS is a typical message mailbox which is composed of two parts: one is the information which expressed by a pointer of void; the other is the waiting list which composed of the tasks waiting for this mailbox. The waiting list supports two kinds of sorting: FIFO and preemptive priority. The sorting mode is determined by the user when creating the mailbox.

You can create a mailbox by calling CoCreateMbox() in CooCox CoOS. After being created successfully, there won’t be any message inside. You can send a message to the mailbox by calling CoPostMail() or isr_PostMail() respectively in a task or the ISR. You can also get a message from the mailbox by calling CoPendMail() or CoAcceptMail().

Code 6 The use of the mailbox
~~~.C
void myTaskA(void* pdata)
{
       void* pmail;
       StatusType err;
       ..........
       mboxID = CoCreateMbox(EVENT_SORT_TYPE_PRIO); //Sort by preemptive          
       priority pmail = CoPendMail(mboxID,0,&err);    
       ..........
}
void myTaskB(void* pdata)
{
       ......
       CoPostMail(mboxID,"hello,world"); 
       ......
}
void myISR(void)
{
        CoEnterISR();
        ......
       isr_PostMail(mboxID,"hello,CooCox");
       CoExitISR();
}
~~~

### Message Queues

Message queue is just an array of mailboxes used to send messages to the task in fact. The task or the ISR can put multiple messages (that is, the pointers of the message) to the message queue through the core services. Similarly, one or more tasks can receive this message by the core services. The tasks sending and receiving the message promise that the content that the pointer points to is just that piece of message.

The difference between the mailbox and the message queue is that the former can store only one piece of message while the latter can store multiple of it. The maximum pieces of message stored in a queue are determined by the user when creating the queue in CooCox CoOS.

In CooCox CoOS, message queue is composed of two parts: one is the struct which pointed to the message queue; the other is the waiting list which composed of the tasks waiting for this message queue. The waiting list supports two kinds of sorting: FIFO and preemptive priority. The sorting mode is determined by the user when creating the message queue.

You can create a message queue by calling CoCreateQueue() in CooCox CoOS. After being created successfully, there won’t be any message inside. You can send a message to the message queue by calling CoPostQueueMail() or  isr_PostQueueMail() respectively in a task or the ISR. Similarly, you can also obtain a message from the message queue by calling CoPendQueueMail() or CoAcceptQueueMail().

Code 7 The use of the message queue
~~~.C
void myTaskA(void* pdata)
{
       void* pmail;
       Void* queue[5];
       StatusType err;
       ..........
       queueID = CoCreateQueue(queue,5,EVENT_SORT_TYPE_PRIO); 
       //5 grade, sorting by preemptive priority 
       pmail = CoPendQueueMail(queueID ,0,&err);    
       ..........
}
void myTaskB(void* pdata)
{
       ......
       CoPostQueueMail(queueID ,"hello,world"); 
       ......
}
void myISR(void)
{
       CoEnterISR();
       ......
       isr_PostQueueMail(queueID ,"hello,CooCox");
       CoExitISR();
}
~~~