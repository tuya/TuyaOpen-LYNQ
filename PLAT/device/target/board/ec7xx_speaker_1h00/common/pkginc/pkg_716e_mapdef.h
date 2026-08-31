#ifndef __PKG_716E_MAPDEF_H__
#define __PKG_716E_MAPDEF_H__

#define AP_FLASH_BASE_LNA 0x800000



//For 716E, CP image in AP Flash
#define CP_PKGIMG_LNA (0x0081a000)

#define BL_PKGIMG_LNA (0x00803000)

#define BOOTLOADER_PKGIMG_LIMIT_SIZE (0x12000)

#define AP_PKGIMG_LNA (0x0087e000)

#define AP_PKGIMG_LIMIT_SIZE (0x277000)

#define CP_PKGIMG_LIMIT_SIZE (0x64000)

#define PKGFLXAPP_APP0_LNA      (0x00B81000)
#define PKGFLXAPP_APP0_SIZE     (0x8000)

#define EF_IMG_TTS_LNA          (0x80100000)
#define EF_IMG_TTS_SIZE         (0x96000)
#define EF_IMG_LFS_LNA          (0x80200000)
#define EF_IMG_LFS_SIZE         (0xA2800)

#define PKGFLXTTS_LNA           (0xAF5000)
#define PKGFLXTTS_SIZE          (0x8C000)
#define PKGFLXLFS_LNA           (0xBA1000)
#define PKGFLXLFS_SIZE          (0x3E000)

#endif

