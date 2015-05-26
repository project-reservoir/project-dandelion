
@if (DOXYGEN_PROJECT)
@defgroup CooCox_CoOS
@endif

# CooCox CoOS  {#CooCox_CoOS_Chinese}
CooCox CoOS 是一款针对ARM Cortex-M系列芯片而设计的实时系统内核。[Click to enter English version](@ref CooCox_CoOS_English)
## CoOS特征：
- 免费并开源
- ARM Cortex M系列定制操作系统
- 高度可裁剪性，最小系统内核仅974Byte
- 支持优先级抢占和时间片轮转
- 自适应任务调度算法
- 中断延时时间趋近于零
- 堆栈溢出检测
- 信号量、邮箱、队列、事件标志、互斥等同步通信方式
- 支持多种编译器：ICCARM、ARMCC、GCC

## 技术数据：
### 时间特性：

功能                              |  时间 (无时间片轮转/有时间片轮转)
--------------------------------- | -------------
创建已定义的任务（无任务切换）    |	 5.3us / 5.8us
创建已定义的任务（有任务切换）    |	 7.5us / 8.6us
删除任务 (退出任务)	              |  4.8us / 5.2us
任务切换 (切换内容)	              |  1.5us / 1.5 us
任务切换 (在设置事件标志的情况下) |	 7.5us / 8.1us
任务切换 (在发送信号量的情况下)   |	 6.3us / 7.0us
任务切换 (在发送邮件的情况下)     |	 6.1us / 7.1us
任务切换 (在发送队列的情况下)     |	 7.0us / 7.6us
设置事件标志（无任务切换）	      |  1.3us / 1.3us
发送信号量（无任务切换）	      |  1.6us / 1.6us
发送邮件（无任务切换）	          |  1.5us / 1.5us
发送队列（无任务切换）	          |  1.8us / 1.8us
IRQ中断服务程序的最大中断延迟时间 |	 0 / 0

### 空间特性：

描述	                 | 空间
------------------------ | ------------ 
内核占RAM空间	         |  168 Bytes
内核占代码空间	         |  974 Bytes
一个任务占RAM空间	     |  TaskStackSize + 24 Bytes(MIN) TaskStackSize + 48 Bytes(MAX)
一个邮箱占RAM空间	     |  16 Bytes
一个信号量占RAM空间	     |  16 Bytes
一个队列占RAM空间	     |  32 Bytes
一个互斥体占RAM空间	     |  8   Bytes
一个用户定时器占RAM空间	 |  24 Bytes

> 测试条件：STM32F103RB，工作于72 MHz，使用内部flash，使能预取指缓冲。 

## 用户手册：
我们为您提供了CooCox CoOS的使用手册，让您更轻松方便地使用CooCox CoOS，其内容包括：
- [第一章 CooCox CoOS概述](@ref CooCox_CoOS_gettingstart_cn)
- [第二章 任务管理](@ref CooCox_CoOS_Task_Management_cn)
- [第三章 时间管理](@ref CooCox_CoOS_Time_Management_cn)
- [第四章 内存管理](@ref CooCox_CoOS_Memory_Management_cn)
- [第五章 任务间的同步和通信](@ref CooCox_CoOS_Intertask_syn_cn)
- [第六章 API手册](@ref CooCox_CoOS_API_Reference_cn)

要获得详细信息，请下载 [CooCox CoOS用户手册](assets/pdf/CoOS_User_Guide_CH.pdf).

## 示例：
CoOS 也提供一些例子来帮助您更容易地开发基于CoOS的嵌入式应用程序，如下：

### 使用ARMCC编译器：
- ST [STM32F103RB示例代码](@ref STM32F103RB_ARMCC)
- Atmel [SAM3U4E示例代码](@ref SAM3U4E_ARMCC)
- NXP LPC1766示例代码
- NXP LPC1766 LwIP示例代码
- NXP LPC1114 示例代码
- TI LM3S8962示例代码

### 使用ICCARM编译器：
- ST STM32F103RB示例代码
- NXP LPC1766示例代码
- NXP LPC1114示例代码

### 使用 GCC 编译器（CoBuilder/CoIDE） ：
- ST STM32F103RB示例代码
- Atmel SAM3U4E示例代码
- NXP LPC1766示例代码
- NXP LPC1766 LwIP示例代码
- NXP LPC1114示例代码
- TI LM3S8962示例代码
- Nuvoton NUC100LE3AN示例代码
- Nuvoton NUC140示例代码
- Nuvoton NUC140 CAN示例代码
- Nuvoton NUC140 CAN Master示例代码
- Nuvoton M0516示例代码
- Nuvoton NUC122RD2AN示例代码
- Nuvoton Mini54LAN示例代码
- Nuvoton NUC123SD4AN0 示例代码
- Nuvoton NUC123SD4AN0 UART 示例代码
- Nuvoton NANO130KE3BN 示例代码
- Nuvoton NUC472HI8AE 示例代码
- Nuvoton M058SSAN 示例代码
- Nuvoton AU9110 示例代码 
- Nuvoton M0516LDN 示例代码 
- Nuvoton NUC472HI8AE 示例代码
- Holtek HT32F1253示例代码
- Holtek HT32F1755示例代码
- Freescale KL25Z示例代码

## 支持设备：
CoOS支持所有ARM Cortex MCU器件，以下仅列出一些最常用的芯片：
- Atmel ATSAM3U系列
- Energy Micro EFM32系列
- Luminary LM3S系列
- Nuvoton Numicro系列
- NXP LPC17xx LPC13xx LPC12xx LPC11xx系列
- ST STM32F10x系列
- Toshiba TMPM330系列
- Holtek HT32系列