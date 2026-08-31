#include "string.h"
#include "osasys.h"
#include "cmsis_os2.h"
#include "commontypedef.h"
#include "ol_sms_api.h"
#include "ol_log.h"

static unsigned int read_index = 0;

//8613800280500 send hello word
#define PDU_MSG "0011000D91683108200805F00008011600680065006C006C006F00200077006F0072006C0064"
#define PDU_LEN strlen(PDU_MSG)

#define SMS_SEND_MAX_DELAY_MS            0xEFFFFFFF  //without timeout

#define REPORT_ID 0
#define TXT_MODE 0
#define OL_EXTEND 1

static void sms_msg_info_dump(ol_sms_info* msg_info)
{
    OL_LOG_PRINTF("[MSG]:stat: %d", msg_info->stat);
    OL_LOG_PRINTF("[MSG]:da: %s", msg_info->da);
    OL_LOG_PRINTF("[MSG]:timezone: %d sign %c", msg_info->timestamp.tsTimezone, msg_info->timestamp.tsZoneSign);
    OL_LOG_PRINTF("[MSG]:TIME: %d-%d-%d",msg_info->timestamp.tsYear,msg_info->timestamp.tsMonth,msg_info->timestamp.tsDay);
    OL_LOG_PRINTF("[MSG]:TIME: %d-%d-%d",msg_info->timestamp.tsHour,msg_info->timestamp.tsMinute,msg_info->timestamp.tsSecond);
    OL_LOG_PRINTF("[MSG]:coding type: %d", msg_info->msg.decorde_type);
    OL_LOG_PRINTF("[MSG]:len: %d", msg_info->msg.msg_len);
    OL_LOG_PRINTF("[MSG]:msg: %s", msg_info->msg.msg_data);
}

void sms_id_callback(unsigned int index)
{
    OL_LOG_PRINTF("sms_id_callback id = %d", index);
    read_index = index;
}

void sms_msg_callback(ol_sms_info* msg_info)
{
    OL_LOG_PRINTF("sms_msg_callback");
    sms_msg_info_dump(msg_info);
}

void sms_demo(void)
{
    int i = 0;
    ol_sms_store mem;
    ol_sms_info msg_info;
    mbtk_ol_sms_config sms_config = {0};

    memset(&sms_config, 0, sizeof(mbtk_ol_sms_config));
    memset(&mem, 0x0, sizeof(mem));
    memset(&msg_info, 0x0, sizeof(msg_info));

    #if(REPORT_ID)
    ol_sms_id_report_register(sms_id_callback);
    #else
    ol_sms_msg_report_register(sms_msg_callback);
    #endif

    #if(TXT_MODE)
    sms_config.mem_type[0] = OL_SMS_STOR_MEM_TYPE_SM;
    sms_config.messageFormat = OL_SMS_FORMAT_TXT_MODE;
    ol_sms_config(sms_config);
    for(i = 0; i < 5; i++)
    {
        memset(&msg_info, 0x0, sizeof(msg_info));
        ol_sms_read(i, &msg_info);
        sms_msg_info_dump(&msg_info);
    }
    ol_sms_send("8613800280500", 13, "hello", 5, SMS_SEND_MAX_DELAY_MS);
    #else
    sms_config.mem_type[0] = OL_SMS_STOR_MEM_TYPE_ME;
    sms_config.messageFormat = OL_SMS_FORMAT_PDU_MODE;
    ol_sms_config(sms_config);
    for(i = 0; i < 5; i++)
    {
        memset(&msg_info, 0x0, sizeof(msg_info));
        ol_sms_read(i, &msg_info);
        sms_msg_info_dump(&msg_info);
    }

    #if(OL_EXTEND)
    ol_sms_send_ext("8613800280500", 13, NULL, 0, OL_SMS_FORMAT_GSM_7BIT, "hello world", 11, SMS_SEND_MAX_DELAY_MS);
    #else
    ol_sms_send(NULL, 0, PDU_MSG, PDU_LEN, SMS_SEND_MAX_DELAY_MS);
    #endif //OL_EXTEND

    #endif //TXT_MODE
    ol_sms_mem(&mem);
    OL_LOG_PRINTF("mem1:%d", mem.mem1.used);
    ol_sms_delete(39);
}

void sms_read_recv(void)
{
    if(read_index >= 0)
    {
        ol_sms_info msg_info;

        memset(&msg_info, 0x0, sizeof(msg_info));
        ol_sms_read(read_index, &msg_info);
        sms_msg_info_dump(&msg_info);
        ol_sms_delete(read_index);
        read_index = 0;
    }
}

