#ifndef _CMCC_DM_H_
#define _CMCC_DM_H_
#include "slpman.h"

#include "cis_api.h"
#include "cis_internals.h"
#include "mw_nvm_config.h"

//dm task
#define DM_TASK_STACK_SIZE (1024*2+512)//add 512B to aovid stack overflow

#define DM_SAMPLE_OBJECT_MAX          2
#define DM_SAMPLE_OID_1               (1)
#define DM_SAMPLE_OID_2                (3)
#define DM_SAMPLE_OID_3                (6)
#define DM_SAMPLE_OID_WRITE               (668)
#define DM_SAMPLE_OID_READ                (669)
#define DM_SAMPLE_INSTANCE_COUNT        1
#define DM_SAMPLE_INSTANCE_BITMAP       "1"

typedef enum
{
    DM_INIT_STATE,
    DM_SIM_READY,
    DM_NOT_RIGHT_SIM,
    DM_IPREADY_STATE,
    DM_HEART_BEAT_ARRIVED,
    DM_HEART_BEAT_UPDATE,
    DM_IDLE_STATE
} dmStateMachine_t;

typedef enum
{
   DM_NO_REG_STATE,
   DM_REG_SUC_STATE,
   DM_REG_FAIL_STATE,
}dmRegState_t;

typedef struct dm_device_object
{
    cis_oid_t         oid;
    cis_instcount_t   instCount;
    const char*       instBitmap;
    const cis_rid_t*  attrListPtr;
    uint16_t          attrCount;
    const cis_rid_t*  actListPtr;
    uint16_t          actCount;
} dm_device_object;

//////////////////////////////////////////////////////////////////////////
//a object

typedef struct dm_object{
    int32_t intVal;
    float   floatVal;
    bool    boolVal;
    char    strVal[64];
    uint8_t update;
} dm_object;


typedef struct dm_instance
{
    cis_iid_t   instId;
    bool        enabled;
    dm_object instance;
} dm_instance;

enum{
    attributeWrite_fieldConfig     = 1,
    attributeWrite_ruleConfig      = 2,
    attributeWrite_addressConfig   = 3,
};

                        
void cmccAutoRegisterInit(MWNvmCfgDmCmccParam* pDmCmccParam, slpManSlpState_t slpState);
INT8 cmccDmReadRegstat(void);

#endif//_OBJECT_DEFS_H_

