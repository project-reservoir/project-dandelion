#include "console.h"
#include "debug.h"

static uint8_t debug_enabled = 0;
static uint8_t info_enabled = 1;
static uint8_t warn_enabled = 1;
static uint8_t error_enabled = 1;

void ERR(char* a) 
{
    if(error_enabled)
    {
        ConsolePrint("ERROR: ");
        ConsolePrint(a);
    }
}

void WARN(char* a) 
{
    if(warn_enabled)
    {
        ConsolePrint("WARN: ");
        ConsolePrint(a);
    }
}

void INFO(char* a) 
{
    if(info_enabled)
    {
        ConsolePrint("INFO: ");
        ConsolePrint(a);
    }
}

void DEBUG(char* a) 
{
    if(debug_enabled)
    {
        ConsolePrint("DEBUG: ");
        ConsolePrint(a);
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
