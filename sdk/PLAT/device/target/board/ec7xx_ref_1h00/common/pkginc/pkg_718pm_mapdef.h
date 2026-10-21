#ifndef __PKG_718PM_MAPDEF_H__
#define __PKG_718PM_MAPDEF_H__

#define AP_FLASH_BASE_LNA 0x800000

//For 718PM, CP image in AP Flash
#define CP_PKGIMG_LNA (0x0081e000)

#define BL_PKGIMG_LNA (0x00803000)

#define BOOTLOADER_PKGIMG_LIMIT_SIZE (0x16000)

#if defined (FEATURE_AMR_CP_ENABLE) || defined (FEATURE_VEM_CP_ENABLE)
#define AP_PKGIMG_LNA (0x008Be000)

#define AP_PKGIMG_LIMIT_SIZE (0x2D7000-0x33000-0x7000+0xC000) //-204KB -28KB + 48KB

#define CP_PKGIMG_LIMIT_SIZE (0xA0000)

#else

#define AP_PKGIMG_LNA (0x00882000)

#define AP_PKGIMG_LIMIT_SIZE (0x313000)

#define CP_PKGIMG_LIMIT_SIZE (0x64000)

#endif

#define XPKGDBLSH_VIRTIMG_MERGE_LNA    (0x0)
#define XPKGDSYSH_VIRTIMG_MERGE_LNA    (0x1000)
#define XPKGDPRM_VIRTIMG_MERGE_LNA  (0x0)
#define XPKGDCMN_VIRTIMG_MERGE_LNA  (0x0)
#define XPKG_VIRTIMG_LOAD_SIZE      (0x100000)

#define EF_IMG_GUI_FONT_LNA         (0x80000000)
#define EF_IMG_GUI_FONT_SIZE        (         0)

#define EFID01_IMG_EXLFS_LNA        (0x80000000)
#define EF_IMG_LFS_LNA              (0x80000000)
#define EF_IMG_LFS_SIZE             (0x2000000)

#define PKGFLXRAW0_LFS_LNA          (0x00B95000)
#define PKGFLXRAW0_LFS_SIZE         (   0x18000)

#define PKGFLXRAW1_LOGO_LNA         (0x00BAD000)
#define PKGFLXRAW1_LOGO_SIZE        (   0x44000)
#endif

