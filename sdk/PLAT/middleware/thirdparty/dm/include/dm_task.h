/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    app.h
 * Description:  EC618 at command demo entry header file
 * History:      Rev1.0   2018-07-12
 *
 ****************************************************************************/
#ifndef _DM_TASK_H
#define _DM_TASK_H

#define DISABLE_CMCC_REGISTER       0x0
#define ENABLE_CMCC_REGISTER        0x1

#define CCID_LEN           24
#define IMEI_LEN           18
#define IMSI_LEN           18

#define SRC_DATA_LEN        300
#define DST_DATA_LEN        384

#define EC_AUTO_REG_TASK_STACK_SIZE   1700 

#define REG_OK             1
#define REG_FAIL           2

typedef struct
{
    int ret;
}dmMessage;

typedef enum applDmPrimId_Enum
{
    APPL_DM_PRIM_ID_BASE = 0,

	APPL_DM_CTCC_OPEN_IND,
    APPL_DM_CUCC_OPEN_IND,
    APPL_DM_CMCC_OPEN_IND,
	
    APPL_DM_PRIM_ID_END = 0xFF
}applDmPrimId;



void ecAutoRegisterInit(void);

#endif
