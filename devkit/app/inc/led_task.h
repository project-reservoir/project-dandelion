#ifndef __LED_TASK_H
#define __LED_TASK_H

// DEFINES
#define LED3_PIN                                GPIO_PIN_4
#define LED3_GPIO_PORT                          GPIOB
#define LED3_GPIO_CLK_ENABLE()                  __GPIOB_CLK_ENABLE()
#define LED3_GPIO_CLK_DISABLE()                 __GPIOB_CLK_DISABLE()

#define LED4_PIN                                GPIO_PIN_5
#define LED4_GPIO_PORT                          GPIOA
#define LED4_GPIO_CLK_ENABLE()                  __GPIOA_CLK_ENABLE()
#define LED4_GPIO_CLK_DISABLE()                 __GPIOA_CLK_DISABLE()

void LedBlinkTaskHwInit(void);
void LedBlinkTask(const void *argument);

#endif // __LED_TASK_H
