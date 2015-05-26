@defgroup CoOS_HelloWorld_STM32F103RB STM32F103RB Hello World

基于ST STM32F103RB的CoOS示例  {#STM32F103RB_ARMCC}
============= 

### 设备:
- ST STM32F103RBT6

### 评估板:
- Embest STM32R100

### 编译器:
- ARM ARMCC (MDK3.80 或者更高版本)

### 任务:
- task_init()	初始化目标板的资源，并创建其它任务，然后自我删除并退出调度
- lcd_display_adc()	在LCD1602上面显示当前ADC的值
- uart_print()	通过串口打印当前ADC的值
- led_blink()	控制LED灯的闪烁
- time_display()	显示时间，格式为 xx:xx:xx
- time_set()	设置时间
- lcd_blink()	在LCD1602上闪烁即将设置的时、分或秒的值，并在设置时显示时间

### 描述:
- 1 该示例在LCD1602上模拟了一个实时时钟，用户可以按照如下操作来设置时间:
首先通过按下WKUP(S2)按钮来选择设置时、分或秒（在第四次按下该按钮时退出时间设置）；
然后通过按下TAMP(S3)按钮来增大上一步选中的时、分或秒的值。
- 2 系统每隔一定时间采样AD值，并在LCD1602上通过直方图来显示，同时在串口打印该值。
可以通过评估板上面的变阻器来调整AD转换的输入电压值，同时可以在LCD1602上或者串口打印中查看AD值的变化。
- 3 8个LED灯按照程序中设置的模式闪烁。

### 系统配置:
- XTAL   freq         = 8.00 MHz
- SYSCLK freq       = 60.00 MHz
- System Tick freq = 100Hz (10ms)

### 下载文件
	- ST STM32F103RB示例（ARMCC）