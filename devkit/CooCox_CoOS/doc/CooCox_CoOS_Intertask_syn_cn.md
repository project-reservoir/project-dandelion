@defgroup CoOS_User_Manaul_Intertask_syn_cn CoOS任务间同步通讯

CooCox CoOS 任务间同步通讯  {#CooCox_CoOS_Intertask_syn_cn}
============= 


## 任务间的同步

任务同步是指一个任务需要等待另一个任务或中断服务程序发送相应的同步信号后才能继续执行。在CooCox CoOS中，提供了信号量、互斥区域和事件标志来实现任务间的同步。

### 信号量
     
信号量为系统处理临界区和实现任务间同步的问题提供了一种有效的机制。
     
信号量的行为可以用经典的PV 操作来描述：
~~~.C
P Operation: while( s==0); s--;
V Operation: s++;
~~~
     
在CooCox CoOS中，用户可以调用 CoCreateSem() 来创建一个信号量，成功创建一个信号量之后，用户就可通过调用 CoPendSem() 、 CoAcceptSem() 来获得一个信号量，两者不同的是，对于 CoPendSem() ，如果当前没有信号量空闲，则将超时等待到该信号量被释放，而对于 CoAcceptSem() 则立刻返回错误。用户也可以在任务体内调用 CoPostSem() 或者中断服务程序内调用 isr_PostSem() 来释放一个信号量，以实现彼此同步。

程序1 信号量创建
~~~.C
ID0 = CoCreateSem(0,1,EVENT_SORT_TYPE_FIFO); // initCnt=0,maxCnt=1,FIFO
ID1 = CoCreateSem(2,5,EVENT_SORT_TYPE_PRIO); // initCnt=2,maxCnt=5,PRIO
~~~

程序2 信号量的使用
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
       CoEnterISR ( );
       ......
       isr_PostSem(semID);
       CoExitISR ( );
}
~~~

### 互斥区域
     
在CooCox CoOS中，互斥区域解决了"互相排斥"的问题。互斥区域禁止多个任务同时进入受保护的代码“临界区”（critical section）。因此，在任意时刻，只能有一个任务进入这样的代码保护区。
     
在CooCox CoOS中，互斥区域还考虑了优先级反转问题，并通过优先级继承的方法解决了有可能出现优先级反转的问题。
     
优先级反转是指高优先级任务等待低优先级任务释放资源，而低优先级任务又正在等待中等优先级任务的现象。
     
目前两种经典的防止反转的方法：
- 1) 优先级继承策略:当前占有临界资源的任务继承所有申请该临界资源任务的最高优先级作为其优先级，当其退出临界区时，恢复至初始优先级。
- 2) 优先级天花板策略:将申请（占有）某资源的任务的优先级提升至可能访问该资源的所有任务中优先级最高的任务的优先级。(这个优先级称为该资源的优先级天花板)

优先级继承策略对任务执行流程的影响相对较小，因为只有当高优先级任务申请已被低优先级任务占有的临界资源这一事实发生时，才抬升低优先级任务的优先级。而天花板策略是谁占有就直接升到最高。
     
CooCox CoOS通过优先级继承的方法来防止优先级的反转。
     
下图描述了三个任务在CooCox CoOS中有互斥区域时的任务调度，TaskA拥有最高优先级，TaskC拥有最低优先级，蓝框表示互斥区域。
![互斥](images/10.jpg)
     
用户在CooCox CoOS中，可以调用 CoCreateMutex() 来创建一个互斥区域，调用 CoEnterMutexSection() 和 CoLeaveMutexSection() 进入和离开互斥区域，以实现对临界区代码的保护。

程序3 互斥区域的使用 

~~~.C
void myTaskA(void* pdata)
{
       mutexID = CoCreateMutex ( );
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
### 事件标志

当某一任务要与多个事件同步时，需要使用事件标志。若该任务仅与任一个事件同步，称为独立型同步(逻辑或关系)；若与多个事件都发生同步，则称之为关联型同步(逻辑与关系)。

在CooCox CoOS中，最多支持32个事件标志同时存在。CooCox CoOS支持多个任务等待单个事件或多个事件的发生。在CooCox CoOS中，当等待任务所等待的事件标志处于未就绪状态时，这些任务处于不可调度状态。但一旦等待对象变成就绪状态，任务将很快恢复运行。

一个任务成功等待到事件标志后，根据事件标志的类型不同，会有不同的成功等待副作用。在CooCox CoOS中，事件标志有两种类型，人工重置和自动重置。当一个任务成功等待到自动重置事件标志，系统会自动将该事件标志变为未就绪状态；而对于人工重置事件标志，则无副作用。故当人工重置事件就绪后，等待该事件的所有任务均可变为就绪状态，直到用户调用CoClearFlag()将事件标志设置为非就绪态。当一个自动重置事件得到通知，等待该事件标志的任务只有一个变成可调度状态。而且因为事件标志的等待列表按照FIFO的原则排列，因此对于自动重置事件而言，只有等待列表的第一个任务变为就绪状态，其它等待该事件标志的任务仍处于等待状态。
     
如任务A/B/C同时等待事件标志Ⅰ，若该事件标志为人工重置事件，那么当事件标志Ⅰ就绪时，同时会通知所有的等待任务，即将任务A/B/C均从等待状态变为就绪态，且插入就绪链表。若等待的事件标志Ⅰ为自动重置事件，且任务A/B/C按先后顺序排列于等待链表，则当事件标志Ⅰ就绪时，等待其通知任务A后，就变为未就绪状态，所以任务B/C仍在等待链表中，等待下一次的事件标志Ⅰ就绪。

在CooCox CoOS中，用户可以调用 CoCreateFlag() 来创建一个事件标志，创建完事件标志，用户可以通过 CoWaitForSingleFlag() 、 CoWaitForMultipleFlags() 来等待单个或多个事件标志。

程序4 等待单个事件标志

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
程序5 等待多个事件标志
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


## 任务间的通信

任务间或任务与中断服务程序间有时需要进行信息的传递，这种信息传递即为任务间的通信。
     
任务间的通信有两种途径：通过全局变量来实现或发消息给另一个任务。
     
用全局变量时，必须保证每个任务或中断服务程序独享该变量。在中断服务中保证独享的唯一办法是关中断。如果两个任务共享某变量，各任务要独享该变量可以先关中断再开中断或使用信号量(见5.1节)。请注意，任务只能通过全局变量与中断服务程序通信，而且任务并不知道全局变量什么时候被中断服务程序修改了（除非中断程序以信号量方式向任务发信号或者是该任务以查询方式不断周期性地查询变量的值）。在这种情况下，CooCox CoOS提供了邮箱和消息队列来避免以上问题。

### 邮箱
     
系统或用户代码可以通过内核服务来给任务发送消息。典型的消息邮箱也称作交换消息，是指一个任务或一个中断服务程序利用一个指针型变量，通过内核服务来把一则消息(即一个指针)放入邮箱。同样，一个或多个任务可以通过内核服务来接收这则消息。发送消息的任务和接收消息的任务约定，该指针指向的内容就是那则消息。
![M_list](images/11.jpg)

CooCox CoOS的邮箱就是一个典型的消息邮箱。在CooCox CoOS中，邮箱由两部分组成：一个是邮箱的信息，用一个void指针来表示；另一个是由等待该邮箱的任务组成的等待链表。邮箱的等待链表支持两种排序方式，FIFO和优先级抢占，具体选择哪一种方式，由用户在创建邮箱的时候决定。
     
在CooCox CoOS中，用户可以调用 CoCreateMbox() 来创建一个邮箱，成功创建一个邮箱后，邮箱内并无消息存在，用户可在任务体或中断服务程序中调用 CoPostMail() 或 isr_PostMail() 向邮箱发送一则消息，也可以通过 CoPendMail() 或 CoAcceptMail() 从邮箱中获得一则消息。

程序6 邮箱的使用
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
        CoEnterISR ( );
        ......
       isr_PostMail(mboxID,"hello,CooCox");
       CoExitISR ( );
}
~~~

### 消息队列

消息队列实际上就是邮箱阵列，用于给任务发送消息。通过内核提供的服务，任务或中断服务子程序可以将多个消息(该消息的指针)放入消息队列。同样，一个或多个任务可以通过内核服务从消息队列中取出消息。发送和接收消息的任务约定，传递的消息实际上就是指针所指向的内容。
     
消息队列不同于邮箱的一点是，邮箱只能存放一个消息，而消息队列则可存放多个消息。在CooCox CoOS里，一个队列可容纳的消息个数是在创建该队列时决定的。
     
在CooCox CoOS中，消息队列由两部分组成：一个是指示该消息队列的结构体；另一个是由等待该消息队列的任务组成的等待任务表。消息队列的等待链表支持两种排序方式，FIFO和优先级抢占，具体选择哪一种方式，由用户在创建消息队列的时候决定。.
     
在CooCox CoOS中，用户可以调用 CoCreateQueue() 来创建一个消息队列，成功创建消息队列后，消息队列内并无消息存在，用户可在任务体或中断服务程序中调用 CoPostQueueMail() 或  isr_PostQueueMail() 向消息队列发送一则消息。亦可以通过 CoPendQueueMail() 或 CoAcceptQueueMail() 从消息队列中获得一则消息。

程序7 消息队列的使用
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
       CoEnterISR ( );
       ......
       isr_PostQueueMail(queueID ,"hello,CooCox");
       CoExitISR ( );
}
~~~