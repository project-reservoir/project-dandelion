#include "console.h"
#include "debug.h"
#include "FreeRTOS.h"
#include "xprintf.h"
#include <stdarg.h>

static uint8_t debug_enabled = 0;
static uint8_t info_enabled = 1;
static uint8_t warn_enabled = 1;
static uint8_t error_enabled = 1;

void ERR(const char *fmt, ...) {
    if(error_enabled)
    {
        xprintf("ERROR: ");
        va_list args;
        va_start(args, fmt);
        xvprintf(fmt, args);
        va_end(args);
    }
}

void WARN(const char *fmt, ...) {
    if(warn_enabled)
    {
        xprintf("WARN: ");
        va_list args;
        va_start(args, fmt);
        xvprintf(fmt, args);
        va_end(args);
    }
}

void INFO(const char *fmt, ...) {
    if(info_enabled)
    {
        xprintf("INFO: ");
        va_list args;
        va_start(args, fmt);
        xvprintf(fmt, args);
        va_end(args);
    }
}

void DEBUG(const char *fmt, ...) {
    if(debug_enabled)
    {
        xprintf("DEBUG: ");
        va_list args;
        va_start(args, fmt);
        xvprintf(fmt, args);
        va_end(args);
    }
}

void ToggleDebug(void)
{
    debug_enabled = !debug_enabled;
}

void ToggleWarn(void)
{
    warn_enabled = !warn_enabled;
}

void ToggleError(void)
{
    error_enabled = !error_enabled;
}

void ToggleInfo(void)
{
    info_enabled = !info_enabled;
}

uint8_t DebugEnabled(void)
{
    return debug_enabled;
}

uint8_t WarnEnabled(void)
{
    return warn_enabled;
}

uint8_t ErrorEnabled(void)
{
    return error_enabled;
}

uint8_t InfoEnabled(void)
{
    return info_enabled;
}
