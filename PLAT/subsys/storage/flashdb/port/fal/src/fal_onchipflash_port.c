/*
 * Copyright (c) 2020, Armink, <armink.ztl@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <fal.h>
#include <stdlib.h>
#include "flash_rt.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif

#ifdef ON_CHIP_FLASH_DB_ADDRESS
static int init(void)
{
    /* do nothing now */
    SYSLOG_DEBUG("name=%s, addr=0x%x, len=0x%x, blk_size=0x%x\r\n", onchipFlashDb.name, onchipFlashDb.addr, onchipFlashDb.len, onchipFlashDb.blk_size);
    return 1;
}


static int read(long offset, uint8_t *buf, size_t size)
{
    int32_t retVal = -1;
    uint8_t resVal = 0;

    if (((offset + size) > onchipFlashDb.len) || (size == 0) || (buf == NULL))
    {
        SYSLOG_DEBUG("Param error: offset=0x%x, buf=%d, size=%d\r\n", offset, (uint32_t)buf, size);
        goto labelEnd;
    }

    resVal = FLASH_readSafe(buf, onchipFlashDb.addr + offset, (uint32_t)size);
    if (resVal != QSPI_OK)
    {
        SYSLOG_DEBUG("Read failed: offset=0x%x, size=%d, resVal=%d\r\n", offset, size, resVal);
        goto labelEnd;
    }

    retVal = 0;

labelEnd:
    return ((retVal == 0) ? size : 0);
}


static int write(long offset, const uint8_t *buf, size_t size)
{
    int32_t   retVal  = -1;
    uint8_t   resVal  = 0;
    uint32_t  address = 0;
    uint32_t  count   = 0;
    uint16_t  remain  = 0;
    uint32_t  index   = 0;
    uint8_t  *buffer  = NULL;

    if (((offset + size) > onchipFlashDb.len) || (size == 0) || (buf == NULL))
    {
        SYSLOG_DEBUG("Param error: offset=0x%x, buf=%d, size=%d\r\n", offset, (uint32_t)buf, size);
        goto labelEnd;
    }

    if ((offset % ON_CHIP_FLASH_PAGE_SIZE) == 0)
    {
        address = onchipFlashDb.addr + offset;
        if (size <= ON_CHIP_FLASH_PAGE_SIZE)
        {
            resVal = FLASH_writeSafe((uint8_t *)buf, address, (uint32_t)size);
            if (resVal != QSPI_OK)
            {
                SYSLOG_DEBUG("Write failed: offset=0x%x, size=%d, resVal=%d\r\n", offset, size, resVal);
                goto labelEnd;
            }
        }
        else
        {
            count  = size / ON_CHIP_FLASH_PAGE_SIZE;
            remain = size % ON_CHIP_FLASH_PAGE_SIZE;
            for (index=0; index<count; index++)
            {
                resVal = FLASH_writeSafe((uint8_t *)&buf[index * ON_CHIP_FLASH_PAGE_SIZE], address + index * ON_CHIP_FLASH_PAGE_SIZE, ON_CHIP_FLASH_PAGE_SIZE);
                if (resVal != QSPI_OK)
                {
                    SYSLOG_DEBUG("Write failed: offset=0x%x, size=%d, resVal=%d\r\n", offset, size, resVal);
                    goto labelEnd;
                }
            }

            if (remain > 0)
            {
                resVal = FLASH_writeSafe((uint8_t *)&buf[index * ON_CHIP_FLASH_PAGE_SIZE], address + index * ON_CHIP_FLASH_PAGE_SIZE, remain);
                if (resVal != QSPI_OK)
                {
                    SYSLOG_DEBUG("Write failed: offset=0x%x, size=%d, resVal=%d\r\n", offset, size, resVal);
                    goto labelEnd;
                }
            }
        }
    }
    else
    {
        uint32_t inPageOffset    = offset % ON_CHIP_FLASH_PAGE_SIZE;
        uint32_t pageAlignOffset = offset - offset % ON_CHIP_FLASH_PAGE_SIZE;
        uint32_t length          = inPageOffset + size;

        buffer = malloc(length);
        if (buffer == NULL)
        {
            SYSLOG_DEBUG("Failed to malloc %d bytes for buffer\r\n", length);
            goto labelEnd;
        }

        memset(buffer, 0, length);
        memcpy(&buffer[inPageOffset], buf, size);
        address = onchipFlashDb.addr + pageAlignOffset;
        FLASH_readSafe(buffer, address, inPageOffset);
        if (length <= ON_CHIP_FLASH_PAGE_SIZE)
        {
            resVal = FLASH_writeSafe(buffer, address, length);
            if (resVal != QSPI_OK)
            {
                SYSLOG_DEBUG("Write failed: offset=0x%x, size=%d, resVal=%d\r\n", offset, size, resVal);
                goto labelEnd;
            }
        }
        else
        {
            count  = length / ON_CHIP_FLASH_PAGE_SIZE;
            remain = length % ON_CHIP_FLASH_PAGE_SIZE;
            for (index=0; index<count; index++)
            {
                resVal = FLASH_writeSafe(&buffer[index * ON_CHIP_FLASH_PAGE_SIZE], address + index * ON_CHIP_FLASH_PAGE_SIZE, ON_CHIP_FLASH_PAGE_SIZE);
                if (resVal != QSPI_OK)
                {
                    SYSLOG_DEBUG("Write failed: offset=0x%x, size=%d, resVal=%d\r\n", offset, size, resVal);
                    goto labelEnd;
                }
            }

            if (remain > 0)
            {
                resVal = FLASH_writeSafe(&buffer[index * ON_CHIP_FLASH_PAGE_SIZE], address + index * ON_CHIP_FLASH_PAGE_SIZE, remain);
                if (resVal != QSPI_OK)
                {
                    SYSLOG_DEBUG("Write failed: offset=0x%x, size=%d, resVal=%d\r\n", offset, size, resVal);
                    goto labelEnd;
                }
            }
        }
    }

    retVal = 0;

labelEnd:
    if (buffer != NULL)
    {
        free(buffer);
        buffer = NULL;
    }
    return ((retVal == 0) ? size : 0);
}


static int erase(long offset, size_t size)
{
    int32_t retVal = -1;
    uint8_t resVal = 0;

    if (((offset % onchipFlashDb.blk_size) != 0) || ((offset + size) > onchipFlashDb.len) || (size < onchipFlashDb.blk_size) || ((size % onchipFlashDb.blk_size) != 0))
    {
        SYSLOG_DEBUG("Param error: offset=0x%x, size=%d\r\n", offset, size);
        goto labelEnd;
    }

    resVal = FLASH_eraseSafe(onchipFlashDb.addr + offset, size);
    if (resVal != QSPI_OK)
    {
        SYSLOG_DEBUG("Erase failed: offset=0x%x, size=%d, resVal=%d\r\n", offset, size, resVal);
        goto labelEnd;
    }

    retVal = 0;

labelEnd:
    return ((retVal == 0) ? size : 0);
}

const struct fal_flash_dev onchipFlashDb =
{
    .name       = ON_CHIP_FLASH_DB,
    .addr       = ON_CHIP_FLASH_DB_ADDRESS,
    .len        = ON_CHIP_FLASH_DB_SIZE,
    .blk_size   = ON_CHIP_FLASH_SECTOR_SIZE,
    .ops        = {init, read, write, erase},
    .write_gran = 1
};
#endif
