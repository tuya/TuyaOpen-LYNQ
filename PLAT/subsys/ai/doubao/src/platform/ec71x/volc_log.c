#include DEBUG_LOG_HEADER_FILE
#include <stdio.h>
#include "exception_process.h"
int volc_print(const char* str) {
	EC_ASSERT(str != NULL,str,0,0);
    ECPLAT_PRINTF(UNILOG_PLAT_VOLC,volc_print,P_INFO,"%s",str);
    return 0;
}