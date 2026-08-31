#ifndef __OL_SMS_API__
#define __OL_SMS_API__

#define OL_SMS_MAX_LENGTH_OF_ADDRESS_VALUE  40

typedef struct
{
    unsigned char used;
    unsigned char total;
} ol_sms_mem_storage;

typedef struct
{
    ol_sms_mem_storage mem1;
    ol_sms_mem_storage mem2;
    ol_sms_mem_storage mem3;
} ol_sms_store;

typedef enum
{
    SMS_STATUS_REC_UNREAD = 0,  /* Received unread message, i.e new message */
    SMS_STATUS_REC_READ   = 1,  /* Received read message */
    SMS_STATUS_STO_UNSENT = 2,  /* Stored unsent message only applicable to SMs */
    SMS_STATUS_STO_SENT   = 3,  /* Stored sent message only applicable to SMs */
    SMS_STATUS_ALL        = 4,  /* All message, only applicable to +CGML command */
    SMS_STATUS_END
} ol_sms_sta;

typedef struct
{
    unsigned char decorde_type;//not used
    unsigned char msg_data[500];
    unsigned int msg_len;
} ol_sms_msg;

typedef struct
{
    unsigned char     tsYear;
    unsigned char     tsMonth;
    unsigned char     tsDay;
    unsigned char     tsHour;
    unsigned char     tsMinute;
    unsigned char     tsSecond;
    unsigned char     tsTimezone;
    unsigned char     tsZoneSign;
} ol_sms_timestamp;

typedef struct
{
    ol_sms_sta stat;
    unsigned char da[40];
    ol_sms_timestamp timestamp;
    ol_sms_msg msg;
} ol_sms_info;

typedef enum
{
    OL_SMS_FORMAT_PDU_MODE = 0,
    OL_SMS_FORMAT_TXT_MODE = 1,
    OL_SMS_FORMAT_NUM_MODE
} ol_sms_format_mode;

typedef enum
{
    OL_SMS_STOR_MEM_TYPE_ME   = 1,      /* ME message storage */
    OL_SMS_STOR_MEM_TYPE_SM   = 2,      /* (U)SIM message storage */
    OL_SMS_STOR_MEM_TYPE_END
} ol_sms_store_mem_type;

typedef enum
{
    OL_SMS_FORMAT_GSM_7BIT          = 0,
    OL_SMS_FORMAT_UNICODE           = 1,//Only supports UTF-8 format
    OL_SMS_FORMAT_ASCII_HEX         = 2,
}ol_sms_format_type_ext;

typedef enum
{
    OL_SMS_TOA_NUMBER_RESTRICTED       = 0x80,   /* 128, Unknown type, unknown number format */
    OL_SMS_TOA_NUMBER_UNKNOWN          = 0x81,   /* 129, Unknown type, IDSN format number */
    OL_SMS_TOA_NUMBER_INTERNATIONAL    = 0x91,   /* 145, International number type, ISDN format */
    OL_SMS_TOA_NUMBER_NATIONAL         = 0xA1,   /* 161, National number type, IDSN format */
    OL_SMS_TOA_NUMBER_NETWORK_SPECIFIC = 0xB1,   /* 177, Network specific number, ISDN format */
    OL_SMS_TOA_NUMBER_DEDICATED        = 0xC1,   /* 193, Subscriber number, ISDN format */
    OL_SMS_TOA_NUMBER_EXTENSION        = 0xF1,   /* 241, extension, ISDN format */
    OL_SMS_TOA_NUMBER_INVALID
}ol_sms_type_of_address;

typedef struct
{
    ol_sms_format_mode messageFormat;
    ol_sms_store_mem_type mem_type[3];
} mbtk_ol_sms_config;

typedef void (*ol_sms_id_report_cb)(unsigned int index);
typedef void (*ol_sms_msg_report_cb)(ol_sms_info* msg_info);

/*****************************************************************************
 * DESCRIPTION
 *    This API is used to read the message
 *
 * PARAMETERS
 *    index       : [IN] the message index
 *    msg_info    : [OUT] the pointer to the struct ol_sms_info
 *
 * RETURN
 *    =0 : success
 *    <0 : fails, error code defined in _CmsRetId
*****************************************************************************/
extern int ol_sms_read(UINT8 index, ol_sms_info* msg_info);

/*****************************************************************************
 * DESCRIPTION
 *    This API is used to send message
 *
 * PARAMETERS
 *    num         : [IN] the phone number
 *    num_len     : [IN] the length of phone number
 *    data        : [IN] the send data
 *    data_len    : [IN] the size of dend data
 *    timeout_ms  : [IN] the timeout of send message, 0 will be set as default
 *                           CMS_MAX_DELAY_MS
 * RETURN
 *    =0 : success
 *    <0 : fails, error code defined in _CmsRetId
*****************************************************************************/
extern int ol_sms_send(char* num, int num_len, char* data, int data_len, UINT32 timeout_ms);


/*****************************************************************************
 * DESCRIPTION
 *    This API is used to get sms memory information
 *
 * PARAMETERS
 *    mem         : [OUT] pointer to the strcut ol_mem_store
 *
 * RETURN
 *    =0 : success
 *    <0 : fails, error code defined in _CmsRetId
*****************************************************************************/
extern int ol_sms_mem(ol_sms_store* mem);


/*****************************************************************************
* DESCRIPTION
*    This API is used to delete message
*
* PARAMETERS
*    index       : [IN] the message index
*
* RETURN
*    =0 : success
*    <0 : fails, error code defined in _CmsRetId
*****************************************************************************/
extern int ol_sms_delete(int index);


/*****************************************************************************
 * DESCRIPTION
 *    This API is used to register the callback of the sms id when recieve message
 *    The received message is stored for later reading
 *
 * PARAMETERS
 *    cb_func     : [IN] the callback function
 *
 * RETURN
 *    =0 : success
 *    <0 : fails, error code defined in _CmsRetId
*****************************************************************************/
extern int ol_sms_id_report_register(ol_sms_id_report_cb cb_func);


/*****************************************************************************
 * DESCRIPTION
 *    This API is used to set sms configuration about sms format and sms store mem
 *
 * PARAMETERS
 *    sms_config      : [IN] the sturct mbtk_ol_sms_config
 *
 * RETURN
 *    =0 : success
 *    <0 : fails, error code defined in _CmsRetId
*****************************************************************************/
extern int ol_sms_config(mbtk_ol_sms_config sms_config);

/*****************************************************************************
 * DESCRIPTION
 *    This API is used to register the callback of the sms content when recieve message
 *    The received SMS message is not stored
 *
 * PARAMETERS
 *    cb_func     : [IN] the callback function
 *
 * RETURN
 *    =0 : success
 *    <0 : fails, error code defined in _CmsRetId
*****************************************************************************/
extern int ol_sms_msg_report_register(ol_sms_msg_report_cb cb_func);

/*****************************************************************************
 * DESCRIPTION
 *    This extend API is used to send message
 *
 * PARAMETERS
 *    num           : [IN] the phone number
 *    numLen        : [IN] the length of phone number
 *    smscNum       : [IN] the smsc number
 *    smscNumLen    : [IN] the length of smsc number
 *    formatType    : [IN] the send data encoding format
 *    data          : [IN] the send data
 *    dataLen       : [IN] the size of send data
 *    timeout_ms    : [IN] the timeout of send message, 0 will be set as default
 *                           CMS_MAX_DELAY_MS
 * RETURN
 *    =0 : success
 *    <0 : fails, error code defined in _CmsRetId
*****************************************************************************/
extern int ol_sms_send_ext(char* num, int numLen, char* smscNum, int smscNumLen, 
    ol_sms_format_type_ext formatType, char* data, int dataLen, UINT32 timeout_ms);

/*****************************************************************************
 * DESCRIPTION
 *    This API is used to set sms configuration about sms format and sms store mem
 *
 * PARAMETERS
 *    sca      : [IN] the string of Service-Centre-Address
 *	  tosca    : [IN] Type of SMSC Address
 *
 * RETURN
 *    =0 : success
 *    <0 : fails, error code defined in _CmsRetId
*****************************************************************************/

int ol_sms_set_sca(const char *sca,ol_sms_type_of_address tosca);


#endif

