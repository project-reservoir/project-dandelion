@defgroup CoOS_User_Manaul_gettingstart_en CoOS Getting Start

CooCox CoOS快速入门  {#CooCox_CoOS_gettingstart_en}
============= 

本节介绍CooCox CoOS的使用，这里我们使用Keil RealView MDK 4.02开发工具和LPC1114做一个很简单的基于CoOS的demo。
     我们这里假定您已经会使用Keil RealView MDK进行简单的开发和基本的设置。下面为您介绍的是这样一个简单的实例，实例中包括三个任务：：
taskA	循环递增一个局部变量，并延时等待50个时钟节拍
taskB	循环递增一个局部变量，并延时等待50个时钟节拍
taskC	循环递增一个局部变量，并延时等待50个时钟节拍
    接下来介绍CoOS是怎样实现以上功能的。
Step 1 准备工作
1. 访问 www.coocox.org 网站下载 'The first CoOS Program' 源代码；
2. 首先创建一个文件夹，命名为getting_start_sample
3. 然后进入getting_start_sample,分别创建inc、src和ccrtos文件夹，用于存放头文件和源文件；
4. 复制文件到工程目录下：
(1) 把The first CoOS program的发布包中的Demo\Sample目录下的startup_LPC11xx.s复制到src文件夹中
(2) 把The first CoOS program的发布包中的Demo\Sample目录下的main.c复制到src文件夹中
(3) 把CoOS\kernel和CoOS\portable（除CoOS\portable\IAR、CoOS\portable\GCC）目录下的所有文件复制到ccrtos文件夹中

Step 2 创建工程
1. 用MDK软件创建一个空工程，器件选择NXP的LPC1114，内部没有任何文件（不用默认的启动代码）；
2. 添加应用驱动代码到工程：
       添加src文件夹下的所有源文件到工程中；
3. 添加CoOS源码到工程：
       添加ccrtos文件夹下的所有源文件到工程中(头文件不用包含)；
4. 配置工程
       对工程做适当的配置，在C/C++中添加包含路径：.\inc 。此后您应该已经能成功通过编译，如果不能，请仔细检查步骤和MDK设置是否正确。
(如果您想先使用，而不想做前面这些工作来节省您的时间，您可以直接使用我们为您准备的一个现成的工程，存放路径为 \Demo\getting_start_sample。)

Step 3 编写应用代码
     打开main.c，您可以发现，我们已经为您做了一部分工作，包括初始化时钟。我们需要做的就是一步步往里面添加任务代码和配置CoOS。
1. 包含CoOS头文件
    要使用CoOS，首先要将源代码加入您的工程，这个步骤在前面已经完成，接下来要做的就是在您的用户代码中包含CoOS的头文件，即在main.c中添加如下语句：
#include <CoOS.h>                  /*!< CoOS header file */
2. 编写任务代码
    任务在创建的时候要为它指定堆栈空间，对于CoOS，任务的堆栈指针是用户指定的，所以我们要定义三个数组用于三个任务的堆栈：
OS_STK     taskA_stk[128];         /*!< define "taskA" task stack  */
OS_STK     taskB_stk[128];         /*!< define "taskB" task stack  */
OS_STK     taskC_stk[128];         /*!< define "taskC" task stack  */
taskA：循环递增一个局部变量，并延时等待50个时钟节拍：
void taskA (void* pdata)
{
unsigned int led_num;
for (;;) 
{
led_num++; 
CoTickDelay (50);
}
}
taskB：循环递增一个局部变量，并延时等待50个时钟节拍：
void taskB (void* pdata)
{
unsigned int led_num;
for (;;) 
{
led_num++; 
CoTickDelay (50);
}
}
taskC：循环递增一个局部变量，并延时等待50个时钟节拍：
void taskC (void* pdata)
{
unsigned int led_num;
for (;;) 
{
led_num++; 
CoTickDelay (50);
}
}

Step 4 创建任务，起始多任务
    此刻我们已经完成了所有的任务代码，接下来应该初始化OS，创建任务，起始多任务调度。在使用CoOS之前，也就是调用任何OS 的API之前，必须首先对CoOS进行初始化，这个工作通过CoInitOS()函数来完成。初始化完成之后，就可以调用系统的API来创建任务，创建标志、互斥、信号量….，最后，通过CoStartOS()函数，系统进行第一次调度，系统正式启动。CoStartOS()之后的代码不会得到执行，因为OS在第一次调度之后不会返回。
    在main函数目标初始化的后面，添加以下代码：
CoInitOS ( );                            /*!< Initial CooCox CoOS          */
  /*!< Create three tasks  */
  CoCreateTask (taskA,0,0,&taskA_stk[128-1],128);
  CoCreateTask (taskB,0,1,&taskB_stk[128-1],128);
  CoCreateTask (taskC,0,2,&taskC_stk[128-1]  ,128);
  CoStartOS ( );                        /*!< Start multitask               */  

Step 5 配置、裁剪CoOS
    打开 OsConfig.h，这里包括了CoOS的所有可配置项和裁剪项目，在修改每一项之前，确保您已经了解了它的真实作用，文件里有详细的注释来解释每一项的用途。
    首先，我们来配置几个必须要核查或者修改的项目：
    CFG_CHIP_TYPE  
    此参数表明用户的所选用芯片的类型，1代表cortex M3，2代表cortex M0。对于LPC1114，我们选择cortex M0(2)。
    CFG_MAX_USER_TASKS  
    这个指示用户最多能创建多少个任务，这里我们只有三个任务，所以我们修改它的值为3以节省空间。
    CFG_CPU_FREQ 
    这个是您的系统所用的系统时钟，前面的SystemInit()函数把芯片初始化为50MHz，所以这里修改成50000000，对应实际的目标芯片的工作频率。
    CFG_SYSTICK_FREQ
    这个是系统节拍周期，我们设置为100，表示10ms、100Hz的节拍时钟。
    完成了以上工作，您的程序就能够正确运行了。编译您的工程，通过我们的配套的仿真器Colink下载到目标板，或者在MDK的模拟器上运行您的程序，就可以观察到前述的现象。
文件下载
	第一个CoOS程序