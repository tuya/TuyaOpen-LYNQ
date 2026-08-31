/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-05-17     armink       the first version
 */

#ifndef _FAL_CFG_H_
#define _FAL_CFG_H_

#define NOR_FLASH_DEV_NAME             "sdcard"

#define FAL_PART_TABLE_FLASH_DEV_NAME NOR_FLASH_DEV_NAME
#define FAL_PART_TABLE_END_OFFSET      65536

#define FAL_PART_HAS_TABLE_CFG

#define ON_CHIP_FLASH_DB            "onchipFlashDb"
#define ON_CHIP_FLASH_DB_PARTITION  "onchipFlashDbPartition"
#define ON_CHIP_FLASH_DB_ADDRESS    0x704000
#define ON_CHIP_FLASH_DB_SIZE       (32 * 1024)
#define ON_CHIP_FLASH_PAGE_SIZE     256
#define ON_CHIP_FLASH_SECTOR_SIZE   4096

#define SPI_FLASH_DB                "spiFlashDb"
#define SPI_FLASH_DB_PARTITION      "spiFlashDbPartition"
#define SPI_FLASH_DB_ADDRESS        0x178000
#define SPI_FLASH_DB_SIZE           (32 * 1024)

/* ===================== Flash device Configuration ========================= */
extern const struct fal_flash_dev sdcard_flash;
extern struct fal_flash_dev nor_flash0;

#ifdef ON_CHIP_FLASH_DB_ADDRESS
extern const struct fal_flash_dev onchipFlashDb;
#endif
#ifdef SPI_FLASH_DB_ADDRESS
extern const struct fal_flash_dev spiFlashDb;
#endif

/* flash device table */
#if (defined(ON_CHIP_FLASH_DB_ADDRESS) && defined(SPI_FLASH_DB_ADDRESS))
#define FAL_FLASH_DEV_TABLE     {&onchipFlashDb, &spiFlashDb}
#elif defined(ON_CHIP_FLASH_DB_ADDRESS)
#define FAL_FLASH_DEV_TABLE     {&onchipFlashDb}
#elif defined(SPI_FLASH_DB_ADDRESS)
#define FAL_FLASH_DEV_TABLE     {&spiFlashDb}
#else
#define FAL_FLASH_DEV_TABLE                                          \
{                                                                    \
    &sdcard_flash,                                                   \
    &nor_flash0,                                                     \
}
#endif
/* ====================== Partition Configuration ========================== */
#ifdef FAL_PART_HAS_TABLE_CFG
/* partition table */
#if (defined(ON_CHIP_FLASH_DB_ADDRESS) && defined(SPI_FLASH_DB_ADDRESS))
#define FAL_PART_TABLE                                                                                \
{                                                                                                     \
    {FAL_PART_MAGIC_WORD, ON_CHIP_FLASH_DB_PARTITION, ON_CHIP_FLASH_DB, 0, ON_CHIP_FLASH_DB_SIZE, 0}, \
    {FAL_PART_MAGIC_WORD, SPI_FLASH_DB_PARTITION,     SPI_FLASH_DB,     0, SPI_FLASH_DB_SIZE,     0}  \
}
#elif defined(ON_CHIP_FLASH_DB_ADDRESS)
#define FAL_PART_TABLE      {{FAL_PART_MAGIC_WORD, ON_CHIP_FLASH_DB_PARTITION, ON_CHIP_FLASH_DB, 0, ON_CHIP_FLASH_DB_SIZE, 0}}
#elif defined(SPI_FLASH_DB_ADDRESS)
#define FAL_PART_TABLE      {{FAL_PART_MAGIC_WORD, SPI_FLASH_DB_PARTITION,     SPI_FLASH_DB,     0, SPI_FLASH_DB_SIZE,     0}}
#else
#define FAL_PART_TABLE                                                               \
{                                                                                    \
    {FAL_PART_MAGIC_WORD,        "bl",     "stm32_onchip",         0,   64*1024, 0}, \
    {FAL_PART_MAGIC_WORD,       "app",     "stm32_onchip",   64*1024,  704*1024, 0}, \
    {FAL_PART_MAGIC_WORD, "easyflash", NOR_FLASH_DEV_NAME,         0, 1024*1024, 0}, \
    {FAL_PART_MAGIC_WORD,  "download", NOR_FLASH_DEV_NAME, 1024*1024, 1024*1024, 0}, \
}
#endif
#endif /* FAL_PART_HAS_TABLE_CFG */

#endif /* _FAL_CFG_H_ */
