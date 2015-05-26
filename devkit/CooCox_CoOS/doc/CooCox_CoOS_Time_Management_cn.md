@defgroup CoOS_User_Manaul_Time_Management_cn CoOS时间管理

CooCox CoOS时间管理  {#CooCox_CoOS_Time_Management_cn}
============= 

## 系统节拍

CooCox CoOS用 systick 中断实现系统节拍，用户需要在config.h文件中配置系统节拍的频率。CFG_CPU_FREQ用于表示CPU的时钟频率，在配置systick中断时，系统需要通过CPU时钟频率来确定具体的参数；CFG_SYSTICK_FREQ表示用户所需的系统节拍频率，CooCox CoOS支持1Hz到1000Hz的频率，实际值要由具体的应用来确定，系统默认为100Hz（即时间间隔为10ms）。
     
CooCox CoOS在每一次系统节拍中断服务程序中对系统时间进行加一操作，用户可以通过调用CoGetOSTime()来获得当前的系统时间。
     
除了对系统时间进行加一处理外，CooCox CoOS还会在系统节拍中断中检查延时链表和定时器链表是否为空，若链表不为空，则对链表首项的延时时间进行减一操作，并判断链表首项的等待时间是否到期，若到期，则调用相关的操作函数；否则跳过进行下一步操作。

CooCox CoOS在系统节拍中断退出时，调用任务调度函数来判断当前系统内是否需要进行任务调度。

程序1 系统节拍中断处理

~~~.C
void SysTick_Handler(void)
{
     OSSchedLock++;                             /* Lock the scheduler. */
     OSTickCnt++;                                 /* Increment system time. */
     if(DlyList != NULL)                            /* Have task in delayed list? */
     {
          DlyList->delayTick--;                  /* Decrease delay time of the list head. */
          if(DlyList->delayTick == 0)           /* Delay time == 0? */
          {
               isr_TimeDispose();                 /* Call hander for delay time list */
          }
     }
     #if CFG_TMR_EN > 0   
     if(TmrList != NULL)                           /* Have timer be in working? */
     {
          TmrList->tmrCnt--;                    /* Decrease timer time of the list head. */
          if(TmrList->tmrCnt == 0)             /* Timer time == 0? */
          {
               isr_TmrDispose();                  /* Call hander for timer list. */
          }
     }     
     #endif
     OSSchedLock--;                             /* Unlock scheduler. */
     if(OSSchedLock==0)
     {
          Schedule();                              /* Call task scheduler */
     }
}
~~~

## 延时管理

CooCox CoOS通过延时链表管理所有任务的延时和超时。用户调用 CoTickDelay() 、 CoTimeDelay() 、 CoResetTaskDelayTick() 等函数向操作系统申请延时或者调用其他带超时的等待服务时，操作系统将这些延时的时间从短到长排序，然后插入到延时链表中。任务控制块中的delayTick域保存的是本任务与前一项任务延时时间之间的差值。链表第一项为时间延时或超时值，后续链表项中的值均为与前一项的差值。如任务A/B/C分别延时10/18/5，则在延时链表中为如下排序：
![deley](images/5.jpg) 

系统在每次系统节拍中断时会对延时链表的首项进行减一操作，直至其变为0后就将它从延时链表中移入就绪链表。在系统节拍中断中将时间到期的任务移出链表的同时，需要判断该任务是一次延时操作还是超时操作。对于延时操作的任务，CooCox CoOS在其被移出延时链表的同时将其插入就绪链表。而对于超时操作的任务，CooCox CoOS首先判断是由哪一事件所引发的超时，然后将任务从该事件的等待链表中移入就绪链表。

CooCox CoOS 的系统延时在以下条件下不能保证延时的精确性：
- 1) 在任务 A 延时的过程中，有高优先级的任务抢占运行，延时时间将取决于高优先级任务的运行时间；
- 2) 在任务 A 延时的过程中，有与任务 A 同一优先级的任务 B 抢占运行，延时时间将取决于就绪链表中与任务 A 同一优先级任务的任务数和它们各自的时间片长度，以及延时时间到期的时间点。


## 软件定时器

软件定时器是CooCox CoOS以系统节拍为基准时钟源的一个高精度定时器。CooCox CoOS最多支持32个软件定时器，每一个定时器都可将其工作模式设定为循环运行模式或单次运行模式。

用户可以调用 CoCreateTmr() 创建一个软件定时器。创建一个软件定时器时，系统会分配一个相应的定时器控制块来描述定时器当前的状态，创建成功后，定时器默认为停止状态，需要用户调用 CoStartTmr() 后，定时器才能正常工作。对于正常工作的定时器，CooCox CoOS通过定时器链表进行管理，和延时链表一样，定时链表同样以到期时间的长短进行排序：早到期的任务排在链表前，并将所有定时器的到期时间与链表前一个定时器的到期时间作差后存入定时器控制块。如定时器A/B/C分别定时为10/18/5，则在定时器链表中为如下排序：
![P3.3.1 定时器链表](images/12.jpg)
     
软件定时器启动后就完全独立于其他模块，只与系统节拍中断有关。CooCox CoOS将用户工作的定时器都按照到期时间的长短链入定时器链表。在每次系统节拍中断时，对定时器链表的首项进行减一操作（直至其变为0）。

当定时器等待时间到期时，对于循环运行的定时器，CooCox CoOS会根据用户给定的tmrReload重新设定下一次的定时后再链入定时器链表；而对于单次运行的定时器，则将其移出链表，并将其运行状态设定为停止态。

由上可知，定时器的到期时间只取决于系统节拍的个数，而与是否有高优先级任务在运行或创建该定时器的任务是否处于就绪状态等无关。

软件定时器提供了一个供用户在 systick 中断内部进行操作的函数入口（软件定时器的回调函数）。但是还有一些API是不能调用的，它们的调用会引起一些错误。下面列出不能调用的API及不能调用原因：

- 1.不能调用功能不相符的API，例如 CoEnterISR() 、 CoExitISR() ，因为软件定时器的回调函数不是一个ISR。
- 2.因为软件定时器的回调函数不是一个任务，而是一个可能在所有任务中被调用的函数，故会改变当前任务状态的API是不能调用的，例如 CoExitTask() \ CoEnterMutexSection() \ CoLeaveMutexSection() \ CoAcceptSem() \ CoPendSem() \ CoAcceptMail() \ CoPendMail() \ CoAcceptQueueMail() \ CoPendQueueMail() \ CoAcceptSingleFlag() \ CoAcceptMultipleFlags() \ CoWaitForSingleFlag() \ CoWaitForMultipleFlags()。
- 3.由于每一个软件定时器到期时的回调函数均在 systick 中断内部执行，这就要求软件定时器的代码必须精简，不能长时间运行而影响systick 中断的精度，故不能调用  CoTimeDelay() 和 CoTickDelay()，不但会影响精度还可能会导致内核错误。


