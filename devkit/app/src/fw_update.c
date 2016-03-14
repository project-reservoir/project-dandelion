#include "fw_update.h"
#include "cmsis_os.h"
#include "flash.h"
#include "app_header.h"
#include "debug.h"
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
        
        taskENTER_CRITICAL();
        // Buffer is full, start a page program
        flash_erase(update_current_address);
        
        flash_program(update_current_address, update_buffer);
        update_current_address += (FLASH_PAGE_SIZE/2)*4;
        flash_program(update_current_address, &update_buffer[FLASH_PAGE_SIZE/2]);
        update_current_address += (FLASH_PAGE_SIZE/2)*4;
        taskEXIT_CRITICAL();
        
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
    
    // If there are bytes in the update buffer...
    if(update_buffer_pos != 0)
    {
        // Fill the rest of the update buffer with 0's. The last call to FwUpdateWriteWord
        for(i = update_buffer_pos; i < FLASH_PAGE_SIZE; i++)
        {
            FwUpdateWriteWord(0x00000000, (update_current_address - FwUpdateGetBaseAddress()) + (i * 4));
        }
    }
    
    update_in_progress = false;
    update_current_address = 0x00000000;
    
    // Take both semaphores
    osSemaphoreWait(updateBufferSem, 0);
    osSemaphoreWait(memoryOpSem, 0);
    
    flash_eeprom_nvm_lock();
}

uint8_t FwUpdateWriteWord(uint32_t word, uint32_t offset)
{
    if(!update_in_progress)
    {
        // Error: cannot write words if update not started
        ERR("Update Failed: no update in progress\r\n");
        return 1;
    }
    
    // If the provided offset is not the expected offset, abort the firmware upgrade
    if(offset != (update_current_address - FwUpdateGetBaseAddress() + (update_buffer_pos * 4)))
    {
        ERR("Update Failed, invalid address. Expected %d, got %d\r\n", (update_current_address - FwUpdateGetBaseAddress() + (update_buffer_pos * 4)), offset);
        return 1;
    }
    
    // Write the word into the update buffer
    update_buffer[update_buffer_pos++] = word;
    
    // If the buffer is full...
    if(update_buffer_pos >= FLASH_PAGE_SIZE)
    {
        // Tell the FwUpdateTask to wakeup, which causes a flash write.
        osSemaphoreRelease(updateBufferSem);
        
        // And don't return until the flash write completes. This blocks the calling task.
        osSemaphoreWait(memoryOpSem, osWaitForever);
    }
    
    return 0;
}

bool FwUpdateInProgress(void)
{
    return update_in_progress;
}

uint32_t crc32(uint32_t crc, uint8_t *buf, uint32_t len)
{
    crc = ~crc;
    while (len--) {
        crc ^= *buf++;
        crc = crc & 1 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
    }
    return ~crc;
}

bool FwUpdateBackupImageValid(void)
{
    APP_HEADER* dandelion = ((APP_HEADER*)BACKUP_APP_START);
    
    if(dandelion->image_size < APP_SIZE && dandelion->image_size != 0 && dandelion->image_size != 0xFFFFFFFF) {
        return crc32(0x00000000, (uint8_t*)(BACKUP_APP_START + 4), dandelion->image_size - 4) == dandelion->crc32;
    }
    return false;
}

bool FwUpdateMainImageValid(void)
{
    APP_HEADER* dandelion = ((APP_HEADER*)MAIN_APP_START);
    
    if(dandelion->image_size < APP_SIZE && dandelion->image_size != 0 && dandelion->image_size != 0xFFFFFFFF) {
        return crc32(0x00000000, (uint8_t*)(MAIN_APP_START + 4), dandelion->image_size - 4) == dandelion->crc32;
    }
    return false;
}
