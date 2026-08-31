/******************************************************************************
*
******************************************************************************/
#include <stdint.h>
#include "mem_map.h"

uint32_t mbtk_ap_flash_load_size(void)
{
	return AP_FLASH_LOAD_SIZE;
}

uint32_t mbtk_flash_mem_backup_addr(void)
{
	return FLASH_MEM_BACKUP_ADDR;
}

uint32_t mbtk_flash_fs_region_start(void)
{
	return FLASH_FS_REGION_START;
}

uint32_t mbtk_flash_fs_region_size(void)
{
	return FLASH_FS_REGION_SIZE;
}

uint32_t mbtk_flash_fota_region_start(void)
{
	return FLASH_FOTA_REGION_START;
}

uint32_t mbtk_flash_fota_region_len(void)
{
	return FLASH_FOTA_REGION_LEN;
}


