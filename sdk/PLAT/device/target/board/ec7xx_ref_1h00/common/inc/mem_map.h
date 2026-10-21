
#ifndef MEM_MAP_H
#define MEM_MAP_H

/*
718S: 2M flash only
718H/V: 4M ap flash+ 1M cp flash
718P: 4M flash+ 2M psram
718U: 8M flash+ 2M psram

*/

/* invalid base addr */
#define FLASH_FOTA_ADDR_UNDEF    0xF07ADEAD


#if defined TYPE_EC718S
    #include "mem_map_718s.h"
#elif defined TYPE_EC718H
    #include "mem_map_718h.h"
#elif defined TYPE_EC718P
    #include "mem_map_718p.h"
#elif defined TYPE_EC718U
    #include "mem_map_718u.h"
#elif defined TYPE_EC716S
    #include "mem_map_716s.h"
#elif defined TYPE_EC716E
    #include "mem_map_716e.h"
#elif defined TYPE_EC718PM
    #include "mem_map_718pm.h"
#elif defined TYPE_EC718UM
    #include "mem_map_718um.h"
#elif defined TYPE_EC718SM
    #include "mem_map_718sm.h"
#elif defined TYPE_EC718HM
    #include "mem_map_718hm.h"
#else
    #error "Need define chip type"
#endif


//mbtk add for build package version
#ifndef XPKGDPRM_VIRTIMG_MERGE_LNA
#define XPKGDPRM_VIRTIMG_MERGE_LNA  0x0
#endif
#ifndef XPKGDCMN_VIRTIMG_MERGE_LNA
#define XPKGDCMN_VIRTIMG_MERGE_LNA  0x0
#endif
#ifndef XPKG_VIRTIMG_LOAD_SIZE
#define XPKG_VIRTIMG_LOAD_SIZE  0x100000
#endif

#endif

