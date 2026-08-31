#ifndef OPEN_MEM_H
#define OPEN_MEM_H

//#ifdef MBTK_OPENCPU_SUPPORT

/*
0x00000000      |---------------------------------|
                |      header1 4KB                |
                |---------------------------------|
                |      header2 4KB                |
                |---------------------------------|
                |      fuse mirror 4KB            |
                |---------------------------------|
                |      BL 88KB                    |
                |---------------------------------|
                |      rel (factory) 20KB         |
                |---------------------------------|
                |      cp img                     |
                |---------------------------------|
                |      ap img                     |----Can be resized
                |---------------------------------|
                |      hib backup                 |----Can be resized
                |---------------------------------|
                |      lfs                        |----Can be resized
                |---------------------------------|
                |      fota rsvd                  |----Can be resized
*/


#if defined TYPE_EC718PM

//AP region
#define AP_FLASH_LOAD_ADDR              (0x008Be000)
#define AP_FLASH_LOAD_SIZE              (0x2D7000-0x33000-0x7000+0xC000)//2860KB + 48KB -204KB -28KB+48KB
#define AP_FLASH_LOAD_UNZIP_SIZE        (0x325000-0x33000-0x7000+0xC000)//3172KB ,for ld + 48KB -204KB -28KB
//HIB region
#define FLASH_HIB_BACKUP_EXIST          (0)
#define FLASH_MEM_BACKUP_ADDR           (AP_FLASH_XIP_ADDR+FLASH_MEM_BACKUP_NONXIP_ADDR)
#define FLASH_MEM_BACKUP_NONXIP_ADDR    (0x389000)
#define FLASH_MEM_BACKUP_SIZE           (0x18000)//96KB
//FS region
#define FLASH_FS_REGION_START           (0x395000-0x33000-0x7000+0xC000) //0x367000
#define FLASH_FS_REGION_SIZE            (FLASH_FS_REGION_END-FLASH_FS_REGION_START)
#define FLASH_FS_REGION_END             (0x3ad000-0x7000)  // 48KB + 48KB + 204KB-48KB = 252KB
//FOTA region
#define FLASH_FOTA_REGION_START         (0x3ad000-0x7000)
#define FLASH_FOTA_REGION_LEN           (0x44000+0x7000)//272KB + 28KB = 300KB
#define FLASH_FOTA_REGION_END           (0x3f1000)


#else
    #error "Need define chip type"
#endif

//#endif /*MBTK_OPENCPU_SUPPORT*/
#endif /*OPEN_MEM_H*/

