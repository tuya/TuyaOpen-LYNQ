/******************************************************************************

*(C) Copyright 2018 EIGENCOMM International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: atec_adc.h
*
*  Description:
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef __ATCR_FILE_H__
#define __ATCR_FILE_H__

#include "at_util.h"

#define CR_FILE_NAME_OFFSET               12
#define CR_FILE_NAME_HEAD_OFFSET          5

#define CR_FINFO_PRINT_BUF_LEN     (128)

#define CR_FOPEN_0_NAME_STR_LEN             63
#define CR_FOPEN_0_NAME_STR_DEF             NULL
#define CR_FOPEN_1_MODE_MIN               0
#define CR_FOPEN_1_MODE_MAX               4
#define CR_FOPEN_1_MODE_DEF               0

#define CR_FREAD_0_HANDLE_LEN_MIN               1
#define CR_FREAD_0_HANDLE_LEN_MAX               33
#define CR_FREAD_0_HANDLE_LEN_DEF               0
#define CR_FREAD_1_LEN_MIN               0
#define CR_FREAD_1_LEN_MAX               6000
#define CR_FREAD_1_LEN_DEF               0

#define CR_FWRITE_0_HANDLE_LEN_MIN               1
#define CR_FWRITE_0_HANDLE_LEN_MAX               33
#define CR_FWRITE_0_HANDLE_LEN_DEF               0
#define CR_FWRITE_1_LEN_MIN                   0
#define CR_FWRITE_1_LEN_MAX                   (10*1024)
#define CR_FWRITE_1_LEN_DEF                   (1000)
#define CR_FWRITE_2_DATA_STR_LEN             8192
#define CR_FWRITE_2_DATA_STR_DEF             NULL


#define CR_FSEEK_0_HANDLE_LEN_MIN               1
#define CR_FSEEK_0_HANDLE_LEN_MAX               33
#define CR_FSEEK_0_HANDLE_LEN_DEF               0
#define CR_FSEEK_1_LEN_MIN               0
#define CR_FSEEK_1_LEN_MAX               0x7fffffff
#define CR_FSEEK_1_LEN_DEF               0
#define CR_FSEEK_2_POSITION_MIN               0
#define CR_FSEEK_2_POSITION_MAX               8192
#define CR_FSEEK_2_POSITION_DEF               0

#define CR_FPOSITION_0_HANDLE_LEN_MIN               1
#define CR_FPOSITION_0_HANDLE_LEN_MAX               33
#define CR_FPOSITION_0_HANDLE_LEN_DEF               0

#define CR_FTUCAT_0_HANDLE_LEN_MIN               1
#define CR_FTUCAT_0_HANDLE_LEN_MAX               33
#define CR_FTUCAT_0_HANDLE_LEN_DEF               0

#define CR_FCLOSE_0_HANDLE_LEN_MIN               1
#define CR_FCLOSE_0_HANDLE_LEN_MAX               33
#define CR_FCLOSE_0_HANDLE_LEN_DEF               0

#define CR_FERASE_0_HANDLE_LEN_MIN               1
#define CR_FERASE_0_HANDLE_LEN_MAX               33
#define CR_FERASE_0_HANDLE_LEN_DEF               0
#define CR_FERASE_0_NAME_STR_LEN             81
#define CR_FERASE_0_NAME_STR_DEF             NULL

#define CR_FDELETE_0_HANDLE_LEN_MIN               1
#define CR_FDELETE_0_HANDLE_LEN_MAX               33
#define CR_FDELETE_0_HANDLE_LEN_DEF               0
#define CR_FDELETE_0_NAME_STR_LEN             81
#define CR_FDELETE_0_NAME_STR_DEF             NULL

#define CR_FRENAME_0_NAME_STR_LEN             81
#define CR_FRENAME_0_NAME_STR_DEF             NULL
#define CR_FRENAME_1_NAME_STR_LEN             81
#define CR_FRENAME_1_NAME_STR_DEF             NULL

#define CR_FDEL_0_NAME_STR_LEN             81
#define CR_FDEL_0_NAME_STR_DEF             NULL

#define CR_FLDS_0_NAME_STR_LEN             81
#define CR_FDEL_0_NAME_STR_DEF             NULL

#define CR_FLST_0_NAME_STR_LEN             81
#define CR_FLST_0_NAME_STR_DEF             NULL

#define CR_FUPL_0_NAME_STR_LEN             81
#define CR_FUPL_0_NAME_STR_DEF             NULL
#define CR_FUPL_1_SIZE_MIN               0
#define CR_FUPL_1_SIZE_MAX               0xffff
#define CR_FUPL_1_SIZE_DEF               0
#define CR_FUPL_2_TIMEOUT_MIN               0
#define CR_FUPL_2_TIMEOUT_MAX               2
#define CR_FUPL_2_TIMEOUT_DEF               0
#define CR_FUPL_3_ACK_MIN               0
#define CR_FUPL_3_ACK_MAX               0xffff
#define CR_FUPL_3_ACK_DEF               0

#define CR_FDWL_0_NAME_STR_LEN             81
#define CR_FDWL_0_NAME_STR_DEF             NULL


CmsRetId crFileCFG(const AtCmdInputContext *pAtCmdReq);
CmsRetId crFileINFO(const AtCmdInputContext *pAtCmdReq);
CmsRetId crFileLIST(const AtCmdInputContext *pAtCmdReq);
CmsRetId crFileSIZE(const AtCmdInputContext *pAtCmdReq);
CmsRetId crFilePUT(const AtCmdInputContext *pAtCmdReq);
CmsRetId crFileGET(const AtCmdInputContext *pAtCmdReq);
CmsRetId crFileOPEN(const AtCmdInputContext *pAtCmdReq);
CmsRetId crFileREAD(const AtCmdInputContext *pAtCmdReq);
CmsRetId crFileREADHEX(const AtCmdInputContext *pAtCmdReq);
CmsRetId crFileWRITE(const AtCmdInputContext *pAtCmdReq);
CmsRetId crFileWRITEHEX(const AtCmdInputContext *pAtCmdReq);
CmsRetId crFileWriteInputData(UINT8 chanId, UINT8 *pData, INT16 dataLength);
CmsRetId crFileWriteCancel(void);
CmsRetId crFileSYNC(const AtCmdInputContext *pAtCmdReq);
CmsRetId crFileSEEK(const AtCmdInputContext *pAtCmdReq);
CmsRetId crFilePOSITION(const AtCmdInputContext *pAtCmdReq);
CmsRetId crFileTRUNC(const AtCmdInputContext *pAtCmdReq);
CmsRetId crFileCLOSE(const AtCmdInputContext *pAtCmdReq);
CmsRetId crFileDELETE(const AtCmdInputContext *pAtCmdReq);
CmsRetId crFileMOVE(const AtCmdInputContext *pAtCmdReq);
CmsRetId crFileCHECK(const AtCmdInputContext *pAtCmdReq);

#endif

/* END OF FILE */


