#ifndef __MW_NVM_AI_H__
#define __MW_NVM_AI_H__
/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    mw_nvm_ai.h
 * Description:  middleware NVM ai header file
 * History:      2024/07/08, Originated by yflu
 ****************************************************************************/
#include "osasys.h"
#include "mw_common.h"
#ifdef FEATURE_SUBSYS_AI_ENABLE

/******************************************************************************
 *****************************************************************************
 * MARCO/MARCO
 *****************************************************************************
******************************************************************************/
/*
 *
*/
#define MID_WARE_NVM_AI_FILE_NAME					"aiCfg.nvm"
#define MID_WARE_NVM_AI_CUR_VER						0x0

#define	MID_WARE_NVM_AI_DEF_UID						"user*****"
#define MID_WARE_NVM_AT_CZ_DEF_BID					"pat_*****"
#define MID_WARE_NVM_AT_CZ_DEF_TOKEN				"**********"

#define MID_WARE_NVM_AI_UID_LEN_MAX					(64)
#define MID_WARE_NVM_AT_CZ_BID_LEN_MAX				(64)
#define MID_WARE_NVM_AT_CZ_TOKEN_LEN_MAX			(72)

/******************************************************************************
 *****************************************************************************
 * STRUCT
 *****************************************************************************/
 #pragma pack(1)

typedef struct _ecAiCzCfg_tag{
	UINT8 uid[MID_WARE_NVM_AI_UID_LEN_MAX];
	UINT8 bid[MID_WARE_NVM_AT_CZ_BID_LEN_MAX];
	UINT8 token[MID_WARE_NVM_AT_CZ_TOKEN_LEN_MAX];
}ecAiCzCfg_t;

typedef struct _ecAiHsCfg_tag{
}ecAiHsCfg_t;

typedef struct _ecAiCfgStore_tag{
	ecAiCzCfg_t czInfo;
	ecAiHsCfg_t hsInfo;
}ecAiCfgStore_t;

#pragma pack()
/******************************************************************************
 *****************************************************************************
 * API
 *****************************************************************************/
BOOL 	mwNvmAiCfgFormat(ecAiCfgStore_t *pAiCfg);
UINT32 	mwNvmAiCfgInit(void);

BOOL	mwNvmAiCfgRead(ecAiCfgStore_t *pAiCfg);
void	mwNvmAiCfgSave(ecAiCfgStore_t *pAiCfg);

UINT32	mwNvmAiCzUidCfgGet(char *uid);
BOOL	mwNvmAiCzUidCfgSet(char *uid);

UINT32	mwNvmAiCzBidCfgGet(char *bid);
BOOL	mwNvmAiCzBidCfgSet(char *bid);

UINT32	mwNvmAiCzTokenCfgGet(char *token);
BOOL	mwNvmAiCzTokenCfgSet(char *token);


#endif/*FEATURE_SUBSYS_AI_ENABLE*/
#endif/*__MW_NVM_AI_H__*/

