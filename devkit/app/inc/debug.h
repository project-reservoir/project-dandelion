#ifndef __DEBUG_H
#define __DEBUG_H

#include "stm32l0xx_hal.h"

/*
    General purpose static assert.
    Works in/out -side of scope:
        STATIC_ASSERT(sizeof(long)==8);
        int main()
        {
            STATIC_ASSERT(sizeof(int)==4);
        }
*/
#define STATIC_ASSERT(X)            STATIC_ASSERT2(X,__LINE__)

/*
    These macros are required by STATIC_ASSERT to make token pasting work.
    Not really useful by themselves.
*/
#define STATIC_ASSERT2(X,L)         STATIC_ASSERT3(X,L)
#define STATIC_ASSERT3(X,L)         STATIC_ASSERT_MSG(X,at_line_##L)

/*
    Static assertion with special error message.
    Note: It depends on compiler whether message is visible or not!
    STATIC_ASSERT_MSG(sizeof(long)==8, long_is_not_eight_bytes);
*/
#define STATIC_ASSERT_MSG(COND,MSG) \
    typedef char static_assertion_##MSG[(!!(COND))*2-1]

void ERR(char* a);
void WARN(char* a);
void INFO(char* a);
void DEBUG(char* a);

void ToggleInfo(void);
void ToggleWarn(void);
void ToggleError(void);
void ToggleDebug(void);

#endif //__DEBUG_H
