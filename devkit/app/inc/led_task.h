#ifndef __LED_TASK_H
#define __LED_TASK_H

// DEFINES

#ifdef DEVKIT

    #define LED_GPIO_MODE                           GPIO_MODE_OUTPUT_PP
    #define LED_GPIO_PULL                           GPIO_PULLUP

	#define LEDG_PIN                                GPIO_PIN_4
	#define LEDG_GPIO_PORT                          GPIOB
	#define LEDG_GPIO_CLK_ENABLE()                  __GPIOB_CLK_ENABLE()
	#define LEDG_GPIO_CLK_DISABLE()                 __GPIOB_CLK_DISABLE()

	#define LEDR_PIN                                GPIO_PIN_5
	#define LEDR_GPIO_PORT                          GPIOA
	#define LEDR_GPIO_CLK_ENABLE()                  __GPIOA_CLK_ENABLE()
	#define LEDR_GPIO_CLK_DISABLE()                 __GPIOA_CLK_DISABLE()

#elif DANDELION

    #define LED_GPIO_MODE                           GPIO_MODE_OUTPUT_PP
    #define LED_GPIO_PULL                           GPIO_NOPULL
    
	#define LEDG_PIN                                GPIO_PIN_3
	#define LEDG_GPIO_PORT                          GPIOA
	#define LEDG_GPIO_CLK_ENABLE()                  __GPIOA_CLK_ENABLE()
	#define LEDG_GPIO_CLK_DISABLE()                 __GPIOA_CLK_DISABLE()

	#define LEDB_PIN                                GPIO_PIN_4
	#define LEDB_GPIO_PORT                          GPIOA
	#define LEDB_GPIO_CLK_ENABLE()                  __GPIOA_CLK_ENABLE()
	#define LEDB_GPIO_CLK_DISABLE()                 __GPIOA_CLK_DISABLE()

	#define LEDR_PIN                                GPIO_PIN_5
	#define LEDR_GPIO_PORT                          GPIOA
	#define LEDR_GPIO_CLK_ENABLE()                  __GPIOA_CLK_ENABLE()
	#define LEDR_GPIO_CLK_DISABLE()                 __GPIOA_CLK_DISABLE()

#else
	#error "Device type not selected. Define DEVKIT or DANDELION in project settings"
#endif


void LedBlinkTaskHwInit(void);
void LedBlinkTask(const void *argument);

#endif // __LED_TASK_H
