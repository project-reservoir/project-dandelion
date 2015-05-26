@defgroup CoOS_User_Manaul_Time_Management_en CoOS Time Management

CooCox CoOS Time Management  {#CooCox_CoOS_Time_Management_en}
============= 

## System Ticks

CooCox CoOS uses interrupt systick to implement system tick. You need to configure the frequency of system tick in config.h file. CFG_CPU_FREQ is used for CPU’s clock frequency. The system needs to determine the specific parameters through CPU’s clock frequency while configuring systick. CFG_SYSTICK_FREQ is used for the frequency of system tick that users need. CooCox CoOS supports the frequency from 1 to 1000Hz. The actual value is determined by the specific application while the default value is 100Hz (that is, the time interval is 10ms).

CooCox CoOS increases the system time by 1 in every system tick interrupt service routine, you can get the current system time by calling CoGetOSTime().

CooCox CoOS will also check whether the delayed list and the timer list is empty in system tick interrupt service routine except increasing the system time by 1.If the list is not empty, decrease the delayed time of the first item in the list by 1, and judge whether the waiting time of the first item in the list is due. If it is due, call the corresponding operation function, otherwise, skip to the next step.

CooCox CoOS calls the task scheduling function to determine whether the current system needs to run a task scheduling when exits from the system tick interrupt service.

Code 1 System tick interrupt handling

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

## Delay Management
     
CooCox CoOS manages all the tasks’ delay and timeout through delayed list. When you call CoTickDelay(), CoTimeDelay(), CoResetTaskDelayTick() or other API functions to apply for delay. CooCox CoOS will sort the delay time from short to long, and then insert them into the delayed list. The delayTick item in the task control block preserves the difference value of the delay time between the current task and the previous task. The first item in the list is the value of the delay time or the timeout value, while the subsequent item is the difference value with the former. For example, Task A, B, C delay 10,18,5 respectively, then they will be sequenced as follows in the delayed list:
![deley](images/5.jpg)     


System will decrease the first item of the delayed list by 1 in every system tick interrupt, and then move it from the delayed list to the ready list until it becomes 0. When moving the tasks whose time is due out of the list in system tick interrupt, system should determine whether the task is a delay operation or a timeout operation. Towards the tasks with a delay operation, CooCox CoOS moves it to the ready list after it being moved out from the delayed list. Towards the tasks with a timeout operation, CooCox CoOS will judge which event leads to the overtime first, and then move the task from the waiting list to the ready list.

CooCox CoOS’s system delay can’t guarantee the accuracy under the following conditions:

- 1) There is a higher priority task preempting to run when task A is delayed, the delay time will be determined by the running time of the higher priority task;
- 2) There is a task whose priority is the same to task A preempting to run when task A is delayed, the delay time will be determined by the number of the tasks whose priority are the same to task A in the ready list and the length of their timeslice, as well as the time that the delay time is due.


## Software Timer

Software timer is a high precision timer that CooCox CoOS takes the system tick as the benchmark clock source. CooCox CoOS software supports up to 32 software timers, the operation mode of each timer can be set to the periodic mode or the one-shot mode.

You can create a software timer by calling CoCreateTmr(). When creating a software timer, the system will assign a corresponding timer control block to describe the current state of the timer. After being created successfully, the timer’s default state is stopping. It won’t work normally until you call CoStartTmr(). To the timer which works normally, CooCox CoOS manages them through the timer list. The same as the delayed list, the timer list is also sorted by the length of the time that is due: the task whose time is due earlier will be in front of the list, and subtract expiration time of the previous timer in the list from expiration time of all the timers, and then save the result into the timer control block. For example, Task A, B, C are set to 10,18,5 respectively, then they will be sequenced as follows in the timer list:

![P3.3.1](images/12.jpg)
     
Once the software timer starts, it will be independent to other modules completely, and only be related to the system tick interrupt. CooCox CoOS takes all the timers into the timer list by the length of the time that is due. Decrease the first item of the timer list by 1(until it becomes 0) in every sysytem tick interrupt.

When the waiting time of the timer is due, towards the periodic timer CooCox CoOS will reset the next timing according to the tmrReload that you set and then put it into the timer list. However, to the one-shot timer, it will be moved out of the list, and its state will be set to stop.

From the above, we know that the expiration time of the timer is only determined by the number of system ticks, but has no relation to whether there is a higher priority task running or whether the task which created the timer is in the ready state.

Software timer provides a function entrance (software timer callback function) for your operation inside the systick interrupt. But some of the APIs which will cause some errors can't be called. Many APIs can not be called in the following situation:

- 1.Should not call the APIs whose functions do not match, such as CoEnterISR()\CoExitISR(), because the software timer callback function is not a ISR.
- 2.Software timer callback function is not a task but a function may be called in all tasks, so the APIs which will change the state of the current task (running task) should not be called, such as CoExitTask() \ CoEnterMutexSection() \ CoLeaveMutexSection() \ CoAcceptSem() \ CoPendSem() \ CoAcceptMail() \ CoPendMail() \ CoAcceptQueueMail() \ CoPendQueueMail() \ CoAcceptSingleFlag() \ CoAcceptMultipleFlags() \ CoWaitForSingleFlag() \ CoWaitForMultipleFlags().
- 3.Each timer’s callback function is implemented inside the systick interrupt when it is due, which requires the code of the software timer must be simplified and should not run a long time to affect the precision of the systick interrupt. So users should not call CoTimeDelay() and CoTickDelay() which will not only affect the precision of the systick interrupt but also cause the kernel error.


