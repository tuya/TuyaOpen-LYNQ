/*************************************************************
Description:
    MBTK Open Voice Call Internal API
*************************************************************/
#ifndef __OL_CALL_API_H__
#define __OL_CALL_API_H__

#ifndef TRUE
#define  TRUE (1)
#endif

#ifndef FALSE
#define  FALSE (0)
#endif

#define CALL_SEMPHR_MAX_NUMB   10 
#define CALL_TASK_STACK_SIZE   6600
#define CALL_MSG_TIMEOUT       1000



typedef struct
{
    int    cmd_type;
    char *dial_digits;
}call_msg;

enum CALL_MSG_CMD
{
    MSG_RESERVE,
    CALL_MSG_DIAL,
    CALL_MSG_ANSWER,
    CALL_MSG_HANGUP,
    CALL_MSG_URC, 
};

typedef enum 
{
    VCALL_NO_CARRIER,
    VCALL_NO_ANSWER,
    VCALL_RING,
    VCALL_CLIP,
    VCALL_COLP,
    VCALL_OTHER,
}mbtk_vc_event_id_enum;

typedef void (*mbtk_vc_event_cb_f)(int event_id,void *msg);




/*************************************************************
    Function Declaration
*************************************************************/
extern int ol_call_dial(char *dial_digits);
extern int ol_call_answer(void);
extern int ol_call_hangup(void);
extern int ol_call_event_register(mbtk_vc_event_cb_f cb_func);
extern int ol_call_urc_callback(UINT8 chanId, const CHAR *pStr, UINT32 strLen);
extern int ol_call_task_Init(void);

#endif /*__OL_CALL_API_H__*/
