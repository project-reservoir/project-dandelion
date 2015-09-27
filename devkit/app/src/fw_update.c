#include "fw_update.h"
#include "cmsis_os.h"
#include "flash.h"
#include "app_header.h"
#include <stdbool.h>

osSemaphoreId updateBufferSem;
osSemaphoreId memoryOpSem;

uint32_t update_buffer[FLASH_PAGE_SIZE];
uint8_t  update_buffer_pos = 0;

uint32_t update_current_address = 0x00000000;

bool update_in_progress = false;

// Detect which bank this code is running in and return the base address of the other bank
uint32_t FwUpdateGetBaseAddress(void)
{
    // If this function is in the backup region, update the main region
    if((uint32_t)FwUpdateGetBaseAddress >= BACKUP_APP_START)
    {
        return MAIN_APP_START;
    }
    
    return BACKUP_APP_START;
}

uint32_t FwUpdateGetCurrentRegion(void)
{
    // If this function is in the backup region, we're running in the backup region
    if((uint32_t)FwUpdateGetBaseAddress >= BACKUP_APP_START)
    {
        return BACKUP_APP_START;
    }
    
    return MAIN_APP_START;
}

void FwUpdateHwInit(void)
{
    
}

void FwUpdateOsInit(void)
{
    osSemaphoreDef(updateBuffer);
    osSemaphoreDef(memOp);
        
    updateBufferSem = osSemaphoreCreate(osSemaphore(updateBuffer), 1);
    memoryOpSem = osSemaphoreCreate(osSemaphore(memOp), 1);
}

void FwUpdateTask(void)
{
    // Take both semaphores
    osSemaphoreWait(updateBufferSem, 0);
    osSemaphoreWait(memoryOpSem, 0);
    
    // Let other tasks warmup before starting the FwUpdate main loop
    osDelay(4000);
    
    while(1)
    {
        // Wait to be given access to the updateBuffer
        osSemaphoreWait(updateBufferSem, osWaitForever);
        
        // Buffer is full, start a page program
        flash_erase(update_current_address);
        
        flash_program(update_current_address, update_buffer);
        update_current_address += (FLASH_PAGE_SIZE/2)*4;
        flash_program(update_current_address, &update_buffer[FLASH_PAGE_SIZE/2]);
        update_current_address += (FLASH_PAGE_SIZE/2)*4;
        
        update_buffer_pos = 0;
        
        osSemaphoreRelease(memoryOpSem);
    }
}

void FwUpdateStart(void)
{
    uint8_t i;
    
    update_in_progress = true;
    update_current_address = FwUpdateGetBaseAddress();
    
    update_buffer_pos = 0;
    
    for(i = 0; i < FLASH_PAGE_SIZE; i++)
    {
        update_buffer[i] = 0;
    }
    
    // Take both semaphores
    osSemaphoreWait(updateBufferSem, 0);
    osSemaphoreWait(memoryOpSem, 0);
    
    flash_eeprom_unlock();
    flash_nvm_unlock();
}

void FwUpdateEnd(void)
{
    uint8_t i  = 0;
    
    if(update_buffer_pos != 0)
    {
        for(i = update_buffer_pos; i < FLASH_PAGE_SIZE; i++)
        {
            FwUpdateWriteWord(0x00000000);
        }
    }
    
    update_in_progress = false;
    update_current_address = 0x00000000;
    
    // Take both semaphores
    osSemaphoreWait(updateBufferSem, 0);
    osSemaphoreWait(memoryOpSem, 0);
    
    flash_eeprom_nvm_lock();
}

uint8_t FwUpdateWriteWord(uint32_t word)
{
    if(!update_in_progress)
    {
        // Error: cannot write words if update not started
        return 1; 
    }
    
    update_buffer[update_buffer_pos++] = word;
    
    if(update_buffer_pos >= FLASH_PAGE_SIZE)
    {
        // Tell the FwUpdateTask to wakeup
        osSemaphoreRelease(updateBufferSem);
        // And don't return until the memory operation completes
        osSemaphoreWait(memoryOpSem, osWaitForever);
    }
    
    return 0;
}

bool FwUpdateInProgress(void)
{
    return update_in_progress;
}
