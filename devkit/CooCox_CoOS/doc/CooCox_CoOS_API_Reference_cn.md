@defgroup CoOS_User_Manaul_API_Reference_cn CoOS接口函数参考


CooCox CoOS API Reference  {#CooCox_CoOS_API_Reference_cn}
============= 

## 系统管理

### CoInitOS()

- 函数原型：
void CoInitOS(void); 
- 功能描述：
系统初始化。
- 函数参数：
无
- 返回值：
无
- 示例：
~~~.C 
#include "CoOS.h"
#define TASK0PRIO   10
OS_STK Task0Stk[100];
OS_TID Task0Id;
void Task0 (void *pdata);
int main(void)
{
    System_init ( );
    CoInitOS ( );                          // Initialize CoOS
    …       
Task0Id = CoCreateTask (Task0, (void *)0, TASK0PRIO , &Task0Stk[99], 100);
    …
    CoStartOS ( );                        // Start CoOS  
}
void Task0 (void *pdata)
{
   ...
   for(;;) 
  {
      ...        
  }
} 
~~~

- 备注：
  - 1)	CooCox CoOS要求用户在调用CooCox CoOS任何其他服务之前首先调用系统初始化函数OsInit()。
  - 2)	在调用 OsInit()之前需要设定好 CPU 时钟，并做好 OS 的配置选项。
  - 3)	在OS 初始化时锁定调度器，并创建第一个任务 CoIdleTask。
  
  
### CoStartOS()

- 函数原型：
void CoStartOS(void);
- 功能描述：
系统开始运行。
- 函数参数：
无
- 返回值：
无
- 示例：
~~~.C
#include "CoOS.h"
#define TASK0PRIO   10
OS_STK Task0Stk[100];
OS_TID Task0Id;
void Task0 (void *pdata);
int main(void)
{
    System_init ( );
    CoInitOS ( );                              // Initialize CoOS
    …       
Task0Id = CoCreateTask (Task0, (void *)0, TASK0PRIO , &Task0Stk[99], 100);
    …
    CoStartOS ( );                           // Start CoOS 
}
void Task0 (void *pdata)
{
    ...
    for(;;) 
   {
        ...        
   }
}
~~~
- 备注：
  - 1)	在 CoOsStart()之前，用户必须先创建一个应用任务，否则 OS 将一直处于 IdleTask()函数体内。
  - 2)	OS启动过程中，任务调度器解锁。
  
### CoEnterISR()

- 函数原型：
void CoEnterISR(void);
- 功能描述：
系统进入中断。
- 函数参数：
无
- 返回值：
无
- 示例：
~~~.C
#include "CCRTOS.h"
void EXTI0_IRQHandler(void)
{
      CoEnterISR ( );	                	// Enter ISR 
      …
      /* Process interrupt here */
      …
      CoExitISR ( );                   // Exit ISR
} 
~~~
- 备注：
  - 1)	系统进入中断。中断嵌套层数值 OSIntNesting 相应加1。
  - 2)	与 CoExitISR()成对使用。
  
### CoExitISR()

- 函数原型：
void CoExitISR(void);
- 功能描述：
系统退出中断。
- 函数参数：
无
- 返回值：
无
- 示例：
~~~.C
#include "CCRTOS.h"
void EXTI0_IRQHandler(void)
{
      CoEnterISR ( );	                           	// Enter ISR 
      …                           
      /* Process interrupt here */
      …
      CoExitISR ( );                               // Exit ISR
} 
~~~
- 备注：
  - 1)	若在中断服务程序里调用了系统的 API 函数，则在所有中断完全退出后需要调用CoExitISR ( )进行一次任务调度。
  - 2)	当系统退出中断时，中断嵌套计数器-OSIntNesting减1。当OSIntNesting减到0时，进行一次任务调度。
  - 3)	与CoEnterISR()成对使用。
  
### CoSchedLock()

- 函数原型：
void CoSchedLock(void);
- 功能描述：
锁定任务调度器。
- 函数参数：
无
- 返回值：
无
- 示例：
~~~.C
#include "CCRTOS.c"
void Task0 (void *pdata) 
{ 
    	.....	
     CoSchedLock ( );
     …
     /* Process critical resources	here */
     …
     CoSchedUnlock ( );
    	.....
}
~~~
- 备注：
  - 1)	任务调度被关闭，OSSchedLock 相应加1。任务调度器被锁期间，被保护的共享数据和资源只能被当前任务使用，这在一定程序上保证了代码的顺序执行。
  - 2)	与CoOsSchedUnlock()成对使用。
  
### CoSchedUnlock()

- 函数原型：
void CoSchedUnlock(void);
- 功能描述：
解锁任务调度器。
- 函数参数：
无
- 返回值：
无
- 示例：
~~~.C
#include "CCRTOS.c"
void Task0 (void *pdata) 
{
    	.....	
     CoSchedLock ( );
     …
     /* Process critical resources	here */
     …
     CoSchedUnlock ( );
    	.....
}
~~~
- 备注：
  - 1)	任务调度被打开，OSSchedLock 相应减1。当 OSSchedLock减至 0 时，运行任务调度。
  - 2)	与 OsSchedLock()成对使用。
  
### CoGetOSVersion()

- 函数原型：
OS_VER CoGetOSVersion(void);
- 功能描述：
获得当前CoOS版本号。
- 函数参数：
无
- 返回值：
CoOS版本号
- 示例：
~~~.C
#include "CCRTOS.H"
void TaskN (void *pdata)
{
     	U16 version;
     	U8 Major, Minor;	
     	....
     	version = CoGetOSVersion ( );
     	// Get Major Version
     	Major = ((version>>12)&0xF) * 10 + (version>>8)&0xF;
     	// Get Minor Version	
     	Minor = ((version>>4)&0xF) * 10 + version&0xF;
     	printf("Current OS Version: %d.%02d\n",Major, Minor);
     	....
}
~~~
- 备注：
  - 1) 函数返回值是一个 16 位的二进制数，需要右移 8 位才能获得实际版本。例如，0x0101表示当前CoOS版本号为 1.01。
  
## 任务管理

### CoCreateTask()
### CoCreateTaskEx()
### CoExitTask()
### CoDelTask()
### CoGetCurTaskID()
### CoSetPriority()
### CoSuspendTask()
### CoAwakeTask()
### CoActivateTask()

## 时间管理

### CoGetOSTime()
### CoTickDelay()
### CoResetTaskDelayTick()
### CoTimeDelay()

## 软件定时器

### CoCreateTmr()
### CoStartTmr()
### CoStopTmr()
### CoDelTmr()
### CoGetCurTmrCnt()
### CoSetTmrCnt()

## 内存管理

### CoKmalloc()
### CoKfree()
### CoCreateMemPartition()
### CoDelMemoryPartition()
### CoGetMemoryBuffer()
### CoFreeMemoryBuffer()
### CoGetFreeBlockNum()

## 互斥体

### CoCreateMutex()
### CoEnterMutexSection()
### CoLeaveMutexSection()

## 信号量

### CoCreateSem()
### CoDelSem()
### CoAcceptSem()
### CoPendSem()
### CoPostSem()
### isr_PostSem()

## 邮箱

### CoCreateMbox()
### CoDelMbox()
### CoAcceptMail()
### CoPendMail()
### CoPostMail()
### isr_PostMail()

## 消息队列

### CoCreateQueue()
### CoDelQueue()
### CoAcceptQueueMail()
### CoPendQueueMail()
### CoPostQueueMail()
### isr_PostQueueMail()

## 事件标志

### CoCreateFlag()
### CoDelFlag()
### CoAcceptSingleFlag()
### CoAcceptMultipleFlags()
### CoWaitForSingleFlag()
### CoWaitForMultipleFlags()
### CoClearFlag()
### CoSetFlag()
### isr_SetFlag()

## 系统工具

### CoTickToTime()
### CoTimeToTick()

## 其他

### CoIdleTask()
### CoStkOverflowHook()
