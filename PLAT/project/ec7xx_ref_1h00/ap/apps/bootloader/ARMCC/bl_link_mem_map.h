
#ifndef BL_LINK_MEM_MAP_H
#define BL_LINK_MEM_MAP_H
//included in bootloader ec718_0h00_flash.sct for link only


#define AP_FLASH_XIP_ADDR                  0x00800000

#define BOOTLOADER_FLASH_LOAD_ADDR      (AP_FLASH_XIP_ADDR+0x3000)
#define BOOTLOADER_FLASH_LOAD_SIZE      0x20000//128KB
#define XP_DBGRESERVED_BASE_ADDR        (0x0053EF00)

//plat config addr and size
#define FLASH_MEM_PLAT_INFO_ADDR        (AP_FLASH_XIP_ADDR+0x3fc000)
#define FLASH_MEM_PLAT_INFO_SIZE        (0x4000)//16KB
#define FLASH_MEM_PLAT_INFO_NONXIP_ADDR (FLASH_MEM_PLAT_INFO_ADDR - AP_FLASH_XIP_ADDR)

#endif
