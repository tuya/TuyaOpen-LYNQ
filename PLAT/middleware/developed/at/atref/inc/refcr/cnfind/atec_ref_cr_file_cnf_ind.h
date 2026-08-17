/******************************************************************************

*(C) Copyright 2018 EIGENCOMM International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename:
*
*  Description:
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef __ATEC_FILE_CNF_IND_H__
#define __ATEC_FILE_CNF_IND_H__

#include "at_util.h"

#define ATC_FILE_RESP_MAX_STR_LEN        4096

CmsRetId crFileCfgCnf(UINT16 reqHandle, UINT16 rc, void *paras);
CmsRetId crFilePutCnf(UINT16 reqHandle, UINT16 rc, void *paras);
CmsRetId crFileGetCnf(UINT16 reqHandle, UINT16 rc, void *paras);
CmsRetId crFileOpenCnf(UINT16 reqHandle, UINT16 rc, void *paras);
CmsRetId crFileReadCnf(UINT16 reqHandle, UINT16 rc, void *paras);
CmsRetId crFileWriteCnf(UINT16 reqHandle, UINT16 rc, void *paras);
CmsRetId crFileSyncCnf(UINT16 reqHandle, UINT16 rc, void *paras);
CmsRetId crFileSeekCnf(UINT16 reqHandle, UINT16 rc, void *paras);
CmsRetId crFileTruncCnf(UINT16 reqHandle, UINT16 rc, void *paras);
CmsRetId crFileCloseCnf(UINT16 reqHandle, UINT16 rc, void *paras);
CmsRetId crFileDeleteCnf(UINT16 reqHandle, UINT16 rc, void *paras);
CmsRetId crFileMoveCnf(UINT16 reqHandle, UINT16 rc, void *paras);
CmsRetId crFileCheckCnf(UINT16 reqHandle, UINT16 rc, void *paras);
CmsRetId crFileEraseCnf(UINT16 reqHandle, UINT16 rc, void *paras);

void atApplCrFileProcCmsCnf(CmsApplCnf *pCmsCnf);

#endif

/* END OF FILE */

