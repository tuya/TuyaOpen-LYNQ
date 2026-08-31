/*
 * Copyright (c) 2020, Armink, <armink.ztl@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <fal.h>
#include <stdlib.h>
#include "flashex.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif

#ifdef SPI_FLASH_DB_ADDRESS
static int init(void)
{
    /* do nothing now */
    SYSLOG_DEBUG("name=%s, addr=0x%x, len=0x%x, blk_size=0x%x\r\n", spiFlashDb.name, spiFlashDb.addr, spiFlashDb.len, spiFlashDb.blk_size);
    return 1;
}


static int read(long offset, uint8_t *buf, size_t size)
{
    if (((offset + size) > spiFlashDb.len) || (size == 0) || (buf == NULL))
    {
        SYSLOG_DEBUG("Param error: offset=0x%x, buf=%d, size=%d\r\n", offset, (uint32_t)buf, size);
        return 0;
    }

    spiFlashRead(spiFlashDb.addr + offset, buf, (uint32_t)size);

    return size;
}


static int write(long offset, const uint8_t *buf, size_t size)
{
    uint32_t address = 0;
    uint32_t count   = 0;
    uint16_t remain  = 0;
    uint32_t index   = 0;

    if (((offset + size) > spiFlashDb.len) || (size == 0) || (buf == NULL))
    {
        SYSLOG_DEBUG("Param error: offset=0x%x, buf=%d, size=%d\r\n", offset, (uint32_t)buf, size);
        return 0;
    }

    if ((offset % PAGE_SIZE) == 0)
    {
        address = spiFlashDb.addr + offset;
        if (size <= PAGE_SIZE)
        {
            spiFlashWritePage(address, (uint8_t *)buf, (uint16_t)size);
        }
        else
        {
            count  = size / PAGE_SIZE;
            remain = size % PAGE_SIZE;
            for (index=0; index<count; index++)
            {
                spiFlashWritePage(address + index * PAGE_SIZE, (uint8_t *)&buf[index * PAGE_SIZE], PAGE_SIZE);
            }

            if (remain > 0)
            {
                spiFlashWritePage(address + index * PAGE_SIZE, (uint8_t *)&buf[index * PAGE_SIZE], remain);
            }
        }
    }
    else
    {
        uint32_t  inPageOffset    = offset % PAGE_SIZE;
        uint32_t  pageAlignOffset = offset - offset % PAGE_SIZE;
        uint32_t  length          = inPageOffset + size;
        uint8_t  *buffer          = malloc(length);

        if (buffer == NULL)
        {
            SYSLOG_DEBUG("Failed to malloc %d bytes for buffer\r\n", length);
            return 0;
        }

        memset(buffer, 0, length);
        memcpy(&buffer[inPageOffset], buf, size);
        address = spiFlashDb.addr + pageAlignOffset;
        spiFlashRead(address, buffer, inPageOffset);
        if (length <= PAGE_SIZE)
        {
            spiFlashWritePage(address, buffer, (uint16_t)length);
        }
        else
        {
            count  = length / PAGE_SIZE;
            remain = length % PAGE_SIZE;
            for (index=0; index<count; index++)
            {
                spiFlashWritePage(address + index * PAGE_SIZE, &buffer[index * PAGE_SIZE], PAGE_SIZE);
            }

            if (remain > 0)
            {
                spiFlashWritePage(address + index * PAGE_SIZE, &buffer[index * PAGE_SIZE], remain);
            }
        }

        if (buffer != NULL)
        {
            free(buffer);
            buffer = NULL;
        }
    }

    return size;
}


static int erase(long offset, size_t size)
{
    uint32_t count = size / spiFlashDb.blk_size;

    if (((offset % spiFlashDb.blk_size) != 0) || ((offset + size) > spiFlashDb.len) || (size < spiFlashDb.blk_size) || ((size % spiFlashDb.blk_size) != 0))
    {
        SYSLOG_DEBUG("Param error: offset=0x%x, size=%d\r\n", offset, size);
        return 0;
    }

    for (uint32_t i=0; i<count; i++)
    {
        spiFlashEraseSector(spiFlashDb.addr + offset + i * spiFlashDb.blk_size);
    }

    return size;
}


const struct fal_flash_dev spiFlashDb =
{
    .name       = SPI_FLASH_DB,
    .addr       = SPI_FLASH_DB_ADDRESS,
    .len        = SPI_FLASH_DB_SIZE,
    .blk_size   = SECTOR_SIZE,
    .ops        = {init, read, write, erase},
    .write_gran = 1
};
#endif
