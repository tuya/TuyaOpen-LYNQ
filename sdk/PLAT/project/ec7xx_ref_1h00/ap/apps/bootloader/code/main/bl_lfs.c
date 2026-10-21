#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "lfs_port.h"
#include "lfs_util.h"
#include "bl_lfs.h"
#include "common.h"


#if defined CHIP_EC718 || defined CHIP_EC716
extern uint8_t FLASH_XIPRead(uint8_t* pData, uint32_t ReadAddr, uint32_t Size);
extern uint8_t FLASH_write(uint8_t* pData, uint32_t WriteAddr, uint32_t Size);
extern uint8_t FLASH_eraseSectorSafe(uint32_t SectorAddress);
extern uint8_t FLASH_erase32KBlkSafe(uint32_t SectorAddress);

#define LFS_FLASH_ERASE(addr)              FLASH_eraseSectorSafe(addr)
#define LFS_FLASH_ERASE_32K(addr)          FLASH_erase32KBlkSafe(addr)
#define LFS_FLASH_WRITE(buf, addr, size)   FLASH_write(buf, addr, size)
#define LFS_FLASH_READ(buf, addr, size)    FLASH_XIPRead(buf, addr, size)
#endif

extern lfs_t littlefs;

/***************************************************
 ***************       MACRO      ******************
 ***************************************************/
#define EXTERNAL_FLASH_LFS_REGION_START      FLASH_FS_REGION_START

#define LITTLEFS_BLOCK_DEVICE_READ_SIZE      (256)
#define LITTLEFS_BLOCK_DEVICE_PROG_SIZE      (256)
#define LITTLEFS_BLOCK_DEVICE_CACHE_SIZE     (256)
#define LITTLEFS_BLOCK_DEVICE_ERASE_SIZE     (4096)       // one sector 4KB
#define LITTLEFS_BLOCK_DEVICE_TOTOAL_SIZE    FLASH_FS_REGION_SIZE
#define LITTLEFS_BLOCK_DEVICE_LOOK_AHEAD     (16)

/***************************************************
 *******    FUNCTION FORWARD DECLARTION     ********
 ***************************************************/

// Read a block
static int bd_read(const struct lfs_config *cfg, lfs_block_t block,
        lfs_off_t off, void *buffer, lfs_size_t size);

// Program a block
//
// The block must have previously been erased.
static int bd_prog(const struct lfs_config *cfg, lfs_block_t block,
        lfs_off_t off, const void *buffer, lfs_size_t size);

// Erase a block
//
// A block must be erased before being programmed. The
// state of an erased block is undefined.
static int bd_erase(const struct lfs_config *cfg, lfs_block_t block);

// Sync the block device
static int bd_sync(const struct lfs_config *cfg);

// utility functions for traversals
static int littlefs_statfs_count(void *p, lfs_block_t b);

/***************************************************
 ***************  GLOBAL VARIABLE  *****************
 ***************************************************/

// variables used by the filesystem
lfs_t littlefs;

static char littlefs_read_buf[256];
static char littlefs_prog_buf[256];
static __ALIGNED(4) char littlefs_lookahead_buf[LITTLEFS_BLOCK_DEVICE_LOOK_AHEAD];


// configuration of the filesystem is provided by this struct
static struct lfs_config littlefs_cfg =
{
    .context = NULL,
    // block device operations
    .read  = bd_read,
    .prog  = bd_prog,
    .erase = bd_erase,
    .sync  = bd_sync,

    // block device configuration
    .read_size = LITTLEFS_BLOCK_DEVICE_READ_SIZE,
    .prog_size = LITTLEFS_BLOCK_DEVICE_PROG_SIZE,
    .block_size = LITTLEFS_BLOCK_DEVICE_ERASE_SIZE,
    .block_count = LITTLEFS_BLOCK_DEVICE_TOTOAL_SIZE / LITTLEFS_BLOCK_DEVICE_ERASE_SIZE,
    .block_cycles = 200,
    .cache_size = LITTLEFS_BLOCK_DEVICE_CACHE_SIZE,
    .lookahead_size = LITTLEFS_BLOCK_DEVICE_LOOK_AHEAD,

    .read_buffer = littlefs_read_buf,
    .prog_buffer = littlefs_prog_buf,
    .lookahead_buffer = littlefs_lookahead_buf,
    .name_max = 0,
    .file_max = 0,
    .attr_max = 0
};

/***************************************************
 *******         INTERANL FUNCTION          ********
 ***************************************************/

static int bd_read(const struct lfs_config *cfg, lfs_block_t block,
        lfs_off_t off, void *buffer, lfs_size_t size)
{
    // Check if read is valid
    LFS_ASSERT(off  % cfg->read_size == 0);
    LFS_ASSERT(size % cfg->read_size == 0);
    LFS_ASSERT(block < cfg->block_count);

    LFS_FLASH_READ((uint8_t *)buffer, (EXTERNAL_FLASH_LFS_REGION_START + block * cfg->block_size + off), size);
    return 0;
}

static int bd_prog(const struct lfs_config *cfg, lfs_block_t block,
        lfs_off_t off, const void *buffer, lfs_size_t size)
{
    // Check if write is valid
    LFS_ASSERT(off  % cfg->prog_size == 0);
    LFS_ASSERT(size % cfg->prog_size == 0);
    LFS_ASSERT(block < cfg->block_count);
    LFS_FLASH_WRITE((uint8_t *)buffer,(EXTERNAL_FLASH_LFS_REGION_START + block * cfg->block_size + off), size);
    return 0;
}

static int bd_erase(const struct lfs_config *cfg, lfs_block_t block)
{
    // Check if erase is valid
    LFS_ASSERT(block < cfg->block_count);
    LFS_FLASH_ERASE(EXTERNAL_FLASH_LFS_REGION_START + block * cfg->block_size);
    return 0;
}

static int bd_sync(const struct lfs_config *cfg)
{
    return 0;
}

static int littlefs_statfs_count(void *p, lfs_block_t b)
{
    *(lfs_size_t *)p += 1;

    return 0;
}

/***************************************************
 *******      FUNCTION IMPLEMENTATION       ********
 ***************************************************/

// Initialize
int LFS_init(void)
{
    // mount the filesystem
    int err = lfs_mount(&littlefs, &littlefs_cfg);

    // reformat if we can't mount the filesystem
    // this should only happen on the first boot
    if (err)
    {
        err = lfs_format(&littlefs, &littlefs_cfg);
        if(err)
            return err;

        err = lfs_mount(&littlefs, &littlefs_cfg);
        if(err)
            return err;
    }

    return 0;
}

// Deinit
void LFS_deinit(void)
{
    // release any resources we were using
    lfs_unmount(&littlefs);
}

int LFS_format(void)
{
    return lfs_format(&littlefs, &littlefs_cfg);
}

int LFS_stat(const char *path, struct lfs_info *info)
{
    return lfs_stat(&littlefs, path, info);
}

int LFS_remove(const char *path)
{
    return lfs_remove(&littlefs, path);
}

int LFS_fileOpen(lfs_file_t *file, const char *path, int flags)
{
    return lfs_file_open(&littlefs, file, path, flags);
}

int LFS_fileClose(lfs_file_t *file)
{
    return lfs_file_close(&littlefs, file);
}

lfs_ssize_t LFS_fileRead(lfs_file_t *file, void *buffer, lfs_size_t size)
{
    return lfs_file_read(&littlefs, file, buffer, size);
}

lfs_ssize_t LFS_fileWrite(lfs_file_t *file, const void *buffer, lfs_size_t size)
{
    return lfs_file_write(&littlefs, file, buffer, size);
}

int LFS_fileSync(lfs_file_t *file)
{
	return lfs_file_sync(&littlefs, file);
}

lfs_soff_t LFS_fileSeek(lfs_file_t *file, lfs_soff_t off, int whence)
{
    return lfs_file_seek(&littlefs, file, off, whence);
}

int LFS_fileTruncate(lfs_file_t *file, lfs_off_t size)
{
    return lfs_file_truncate(&littlefs, file, size);
}

lfs_soff_t LFS_fileTell(lfs_file_t *file)
{
    return lfs_file_tell(&littlefs, file);
}

int LFS_fileRewind(lfs_file_t *file)
{
    return lfs_file_rewind(&littlefs, file);
}

lfs_soff_t LFS_fileSize(lfs_file_t *file)
{
    return lfs_file_size(&littlefs, file);
}

int LFS_statfs(lfs_status_t *status)
{
    status->total_block = littlefs_cfg.block_count;
    status->block_size = littlefs_cfg.block_size;

    return lfs_fs_traverse(&littlefs, littlefs_statfs_count, &(status->block_used));
}



