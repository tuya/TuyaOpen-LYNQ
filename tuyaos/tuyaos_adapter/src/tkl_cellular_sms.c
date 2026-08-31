#include "tuya_iot_config.h"

#if defined(ENABLE_CATX_SMS) && (ENABLE_CATX_SMS == 1)

#include "tkl_cellular_sms.h"
#include "vlog.h"
#include "string.h"
#include "osasys.h"
#include "cmsis_os2.h"
#include "commontypedef.h"
#include "ol_sms_api.h"
#include "ol_log.h"
#include "tkl_mutex.h"
#include "ml.h"

#define DEBUG_PDU_PARSE 0

#define SMS_SEND_MAX_DELAY_MS            0xEFFFFFFF  //without timeout

static TUYA_CELLULAR_SMS_CB gSmsReadCb = NULL;
static TKL_MUTEX_HANDLE tkl_sms_mutex = NULL;    
static TUYA_CELLULAR_SMS_ENCODE_E tkl_sms_format_type = TUYA_SMS_UTF8;      //默认UTF8编码

extern int decode_pdu_msg(ol_sms_info* msg);

/* Max binary PDU length (SMSC + TPDU) for single SMS */
#define SMS_PDU_BINARY_MAX    256U

/**
 * @brief Pack GSM 7-bit default alphabet septets into octets (TS 23.038)
 * @param[in]  septets   one byte per septet (0..127)
 * @param[in]  n_septets TP-UDL (septet count)
 * @param[out] out       packed octets
 * @param[in]  out_cap   capacity of out
 * @return packed byte count, or -1 on overflow
 */
static int gsm_7bit_pack(const uint8_t *septets, unsigned int n_septets, uint8_t *out, size_t out_cap)
{
    uint32_t bit_buffer = 0U;
    unsigned int bits = 0U;
    size_t out_len = 0U;
    unsigned int i;

    for (i = 0U; i < n_septets; i++) {
        bit_buffer |= ((uint32_t)(septets[i] & 0x7FU)) << bits;
        bits += 7U;
        while (bits >= 8U && out_len < out_cap) {
            out[out_len++] = (uint8_t)(bit_buffer & 0xFFU);
            bit_buffer >>= 8;
            bits -= 8U;
        }
    }
    if (bits > 0U && out_len < out_cap) {
        out[out_len++] = (uint8_t)(bit_buffer & 0xFFU);
    }
    if (out_len > out_cap) {
        return -1;
    }
    return (int)out_len;
}

/**
 * @brief Encode destination address digits into semi-octets (BCD swapped)
 * @param[out] num_digits_out TP-DA length in digits (excluding skipped non-digits)
 * @return number of BCD octets written, 0 on error
 */
static size_t sms_phone_to_bcd(const CHAR_T *phone, uint8_t *bcd, size_t bcd_cap, uint8_t *ton_out,
    uint8_t *num_digits_out)
{
    char digits[TUYA_SMS_PHONENUM_LEN_MAX];
    size_t n = 0U;
    size_t i;
    int is_intl = 0;

    if (phone == NULL || bcd == NULL || ton_out == NULL || num_digits_out == NULL) {
        return 0U;
    }

    while (*phone == ' ' || *phone == '-') {
        phone++;
    }
    if (*phone == '+') {
        is_intl = 1;
        phone++;
    }
    while (*phone != '\0' && n < sizeof(digits)) {
        if (*phone >= '0' && *phone <= '9') {
            digits[n++] = *phone;
        }
        phone++;
    }
    if (n == 0U) {
        return 0U;
    }

    *num_digits_out = (uint8_t)n;
    *ton_out = 0x91U;
    {
        size_t bcd_len = (n + 1U) / 2U;
        if (bcd_len > bcd_cap) {
            return 0U;
        }
        for (i = 0U; i < n; i += 2U) {
            unsigned char d0 = (unsigned char)(digits[i] - '0');
            unsigned char d1 = (i + 1U < n) ? (unsigned char)(digits[i + 1U] - '0') : 0x0FU;
            bcd[i / 2U] = (uint8_t)((d1 << 4) | d0);
        }
        return bcd_len;
    }
}

/**
 * @brief Build SMS-SUBMIT PDU (default SMSC: length 0), GSM 7-bit or UCS2 (UTF-16BE) only.
 * @note GSM: sms_msg->msg_len = septet count; raw septets in first msg_len bytes of msg (cast to bytes).
 *       UCS2: sms_msg->msg_len = UTF-16 code unit count (max 70); sms_msg->msg[i] emitted big-endian.
 * @param[out] pdu_msg_len total binary length; 0 on error or unsupported encoding
 */
static OPERATE_RET encode_pdu_msg(TUYA_CELLULAR_SMS_SEND_T *sms_msg, uint8_t *pdu_msg, uint32_t *pdu_msg_len)
{
    uint8_t da_bcd[16];
    size_t da_bcd_len;
    uint8_t ton;
    uint32_t pos = 0U;
    uint8_t dcs = 0U;
    uint8_t udl = 0U;
    uint8_t ud_buf[160];
    uint8_t gsm_septets[160];
    int ud_packed_len = 0;
    unsigned int i;
    uint8_t num_digits = 0U;
    TUYA_CELLULAR_SMS_ENCODE_E to_chset = TUYA_SMS_UTF16BE;

    if (sms_msg == NULL || pdu_msg == NULL || pdu_msg_len == NULL) {
        return OPRT_INVALID_PARM;
    }

    LOGD("tkl_cellular_sms_send %d phone: %s msg_len: %d sms_encode: %d", sms_msg->sms_encode, sms_msg->phone, sms_msg->msg_len,sms_msg->sms_encode);

    *pdu_msg_len = 0U;


    if(sms_msg->sms_encode == TUYA_SMS_UTF8 || sms_msg->sms_encode == TUYA_SMS_UTF16BE || sms_msg->sms_encode == TUYA_SMS_UTF16LE || sms_msg->sms_encode == TUYA_SMS_CP936) {
        to_chset = TUYA_SMS_UTF16BE;

    }
    else if (sms_msg->sms_encode == TUYA_SMS_GSM || sms_msg->sms_encode == TUYA_SMS_ISO8859_1) {
        to_chset = TUYA_SMS_GSM;
    }
    else
    {
        LOGE("sms_encode is invalid, sms_encode: %d", sms_msg->sms_encode);
        return OPRT_INVALID_PARM;
    }

    /* UTF-16LE and UTF-16BE data must have even byte count (each character is 2 bytes) */
    if ((sms_msg->sms_encode == TUYA_SMS_UTF16LE || sms_msg->sms_encode == TUYA_SMS_UTF16BE || sms_msg->sms_encode == TUYA_SMS_UTF8)
    && (sms_msg->msg_len % 2U) != 0U) {
    LOGE("UTF-16 data length must be even, got: %u, encode: %d", sms_msg->msg_len, sms_msg->sms_encode);
    return OPRT_INVALID_PARM;
    }

    if (sms_msg->sms_encode != to_chset) {
        int out_bytes = 0;
        VOID *converted = mlConvertStr(sms_msg->msg, (int)sms_msg->msg_len, sms_msg->sms_encode, to_chset, &out_bytes);
        if (converted != NULL) {
            size_t msg_cap = sizeof(sms_msg->msg);

            if ((size_t)out_bytes > msg_cap) {
                LOGE("mlConvertStr output too long: %d cap %u", out_bytes, (unsigned int)msg_cap);
                free(converted);
                return OPRT_COM_ERROR;
            }
            memcpy(sms_msg->msg, converted, (size_t)out_bytes);
            sms_msg->msg_len = (unsigned int)out_bytes;
            free(converted);
        } else {
            LOGE("mlConvertStr failed (from=%u to=%u)", sms_msg->sms_encode, to_chset);
            return OPRT_COM_ERROR;
        }
    }
    LOGD("sms_msg->sms_encode: %d", sms_msg->sms_encode);

    da_bcd_len = sms_phone_to_bcd(sms_msg->phone, da_bcd, sizeof(da_bcd), &ton, &num_digits);
    if (da_bcd_len == 0U || num_digits == 0U) {
        LOGE("sms_phone_to_bcd failed, da_bcd_len: %d, num_digits: %d", da_bcd_len, num_digits);
        return OPRT_INVALID_PARM;
    }

    /* Default SMSC: modem/SIM uses stored SC (TS 23.040: length 0) */
    if (pos + 1U > SMS_PDU_BINARY_MAX) {
        return OPRT_COM_ERROR;
    }
    pdu_msg[pos++] = 0x00U;

    /* SMS-SUBMIT: MTI=01, no VP (VPF=00) */
    if (pos + 1U > SMS_PDU_BINARY_MAX) {
        return OPRT_COM_ERROR;
    }
    pdu_msg[pos++] = 0x01U;

    /* TP-Message-Reference */
    if (pos + 1U > SMS_PDU_BINARY_MAX) {
        return OPRT_COM_ERROR;
    }
    pdu_msg[pos++] = 0x00U;

    /* TP-Destination-Address */
    if (pos + 2U + da_bcd_len > SMS_PDU_BINARY_MAX) {
        return OPRT_COM_ERROR;
    }
    pdu_msg[pos++] = num_digits;
    pdu_msg[pos++] = ton;
    memcpy(&pdu_msg[pos], da_bcd, da_bcd_len);
    pos += (uint32_t)da_bcd_len;

    /* TP-Protocol-Identifier */
    if (pos + 1U > SMS_PDU_BINARY_MAX) {
        return OPRT_COM_ERROR;
    }
    pdu_msg[pos++] = 0x00U;

    if (to_chset == TUYA_SMS_GSM) {
        const uint8_t *septets = (const uint8_t *)sms_msg->msg;

        if (sms_msg->msg_len == 0U || sms_msg->msg_len > 160U) {
            return OPRT_INVALID_PARM;
        }
        dcs = 0x00U;
        udl = (uint8_t)sms_msg->msg_len;
        for (i = 0U; i < (unsigned int)sms_msg->msg_len; i++) {
            gsm_septets[i] = (uint8_t)(septets[i] & 0x7FU);
        }
        ud_packed_len = gsm_7bit_pack(gsm_septets, (unsigned int)sms_msg->msg_len, ud_buf, sizeof(ud_buf));
        if (ud_packed_len < 0) {
            return OPRT_INVALID_PARM;
        }
    } else {
        /* UCS2: DCS 0x08; mlConvertStr(…, UTF16BE) sets msg_len to UTF-16BE byte count (even, max 140). */
        unsigned int n_bytes = (unsigned int)sms_msg->msg_len;

        if (n_bytes == 0U || n_bytes > 140U || ((n_bytes % 2U) != 0U)) {
            LOGE("UCS2 user data invalid, msg_len(bytes): %u", n_bytes);
            return OPRT_INVALID_PARM;
        }
        dcs = 0x08U;
        udl = (uint8_t)n_bytes;
        memcpy(ud_buf, (uint8_t *)sms_msg->msg, (size_t)n_bytes);
        ud_packed_len = (int)n_bytes;
    }

    if ((uint32_t)ud_packed_len + pos + 2U > SMS_PDU_BINARY_MAX) {
        return OPRT_INVALID_PARM;
    }

    if (pos + 1U > SMS_PDU_BINARY_MAX) {
        return OPRT_INVALID_PARM;
    }
    pdu_msg[pos++] = dcs;

    if (pos + 1U > SMS_PDU_BINARY_MAX) {
        return OPRT_INVALID_PARM;
    }
    pdu_msg[pos++] = udl;

    if (pos + (uint32_t)ud_packed_len > SMS_PDU_BINARY_MAX) {
        return OPRT_INVALID_PARM;
    }
    memcpy(&pdu_msg[pos], ud_buf, (size_t)ud_packed_len);
    pos += (uint32_t)ud_packed_len;

    *pdu_msg_len = pos;

    return OPRT_OK;
}

/**
 * @brief 发送短信
 *
 * @param simId sim卡ID
 * @param smsMsg 发送的短信
 *
 * @return  0 发送成功 其它 发送失败
 */
OPERATE_RET tkl_cellular_sms_send(UINT8_T sim_id, TUYA_CELLULAR_SMS_SEND_T* sms_msg)
{
    if(NULL == sms_msg || NULL == sms_msg->phone || NULL == sms_msg->msg || sms_msg->msg_len == 0) {
        return OPRT_INVALID_PARM;
    }
    OPERATE_RET ret = OPRT_NOT_SUPPORTED;

    uint8_t pdu_msg[SMS_PDU_BINARY_MAX];
    uint8_t pdu_msg_print[SMS_PDU_BINARY_MAX*2];
    uint32_t pdu_msg_len = 0;


    ret = encode_pdu_msg(sms_msg, pdu_msg, &pdu_msg_len);
    if(ret != OPRT_OK) {
        LOGE("encode_pdu_msg failed, ret: %d", ret);
        return ret;
    }
    for(int i = 0; i < pdu_msg_len; i++) {
        sprintf(pdu_msg_print + i*2, "%02X", pdu_msg[i]);
    }
    pdu_msg_print[pdu_msg_len*2] = '\0';
    LOGD("pdu_msg: %s, pdu_msg_len: %d", pdu_msg_print, strlen(pdu_msg_print));

    ret = ol_sms_send(NULL, 0, (char*)pdu_msg_print, strlen(pdu_msg_print), SMS_SEND_MAX_DELAY_MS);

    if(ret) {
        LOGE("tkl_cellular_sms_send %d failed, ret: %d", sms_msg->sms_encode, ret);
    } else {
        LOGD("tkl_cellular_sms_send %d success", sms_msg->sms_encode);
    }
    return ret;
}

/**
 * @brief 注册短信接收回调函数
 * @note 该函数需在SIM卡激活前注册，否则可能引起异常或短信丢失。
 *
 * @param callback 短信接收回调函数
 *
 * @return  0 注册成功 其它 注册失败
 */
OPERATE_RET tkl_cellular_sms_recv_cb_register(TUYA_CELLULAR_SMS_CB callback)
{
    gSmsReadCb = callback;
    return OPRT_OK;
}

/**
 * @brief 设置短信接收时静音
 * @note 默认情况下，收到短信时，会通过扬声器播放提示音。通过该函数，可设置接收
 *       短信时，不播放提示音。
 *
 * @param mute TRUE 静音 FALSE 短信接收时打开提示音
 *
 * @return 0 设置成功 其它 设置失败
 */
OPERATE_RET tkl_cellular_sms_mute(BOOL_T mute)
{
    return OPRT_NOT_SUPPORTED;
}

/**
 * 短信内容编码转换
 *
 * When \a from_charset or \a to_charset is unknown or unsupported,
 * return NULL.
 * The returned pointer is allocated inside, caller should free it.
 *
 * At most \a from_size byte count will be processed. When \a from_size
 * is -1, process till null chracter.
 *
 * null character will always be inserted into the output string.
 * Invalid characters are replaced silently.
 *
 * \a to_size can be NULL in case that the output byte count is not needed.
 *
 * @param from          input string
 * @param from_size     input string byte count
 * @param from_chset    input string charset
 * @param to_chset      output string charset
 * @param to_size       output string byte count
 * @return
 *      - NULL: invalid parameters
 *      - output string
 */
VOID *tkl_cellular_sms_convert_str(CONST VOID *from, INT_T from_size, TUYA_CELLULAR_SMS_ENCODE_E from_chset, TUYA_CELLULAR_SMS_ENCODE_E to_chset, INT_T *to_size)
{
    return NULL;
}

/**
 * @brief 设置短信默认接收短信编码
 * @param chset TUYA_CELLULAR_SMS_ENCODE_E类型
 *
 * @return 0 设置成功 其它 设置失败
 */
OPERATE_RET tkl_cellular_sms_set_charactor(TUYA_CELLULAR_SMS_ENCODE_E chset)
{   
    LOGI("tkl_cellular_sms_set_charactor %d", chset);
    tkl_sms_format_type = chset;      
    return OPRT_OK;
}

TUYA_CELLULAR_SMS_ENCODE_E tkl_cellular_sms_get_charactor(void)
{
    return tkl_sms_format_type;
}


static void decode_msg(UINT8_T* buf, UINT8_T* data, UINT32_T* data_len)
{
	*data_len = buf[0];
	memcpy(data, &(buf[1]), *data_len);
}

static void ol_sms_msg_report(ol_sms_info* msg_info)
{
    int ret = 0;
    TUYA_CELLULAR_SMS_RECV_T  sms_info_t;

    if (0 == strlen((char*)(msg_info->da))) {
        ret = decode_pdu_msg(msg_info);
        if (0 != ret) {
            LOGE("decode pdu msg failed, %d", ret);
            return ;
        } else {
            LOGI("decode pdu msg success");
        }
    }

    LOGI("sms_msg_callback %d", msg_info->stat);
    LOGD("[MSG]:stat: %d", msg_info->stat);
    LOGD("[MSG]:da: %s", msg_info->da);
    LOGD("[MSG]:timezone: %d sign %c", msg_info->timestamp.tsTimezone, msg_info->timestamp.tsZoneSign);
    LOGD("[MSG]:TIME: %d-%d-%d",msg_info->timestamp.tsYear,msg_info->timestamp.tsMonth,msg_info->timestamp.tsDay);
    LOGD("[MSG]:TIME: %d-%d-%d",msg_info->timestamp.tsHour,msg_info->timestamp.tsMinute,msg_info->timestamp.tsSecond);
    LOGD("[MSG]:coding type: %d", msg_info->msg.decorde_type);
    LOGD("[MSG]:len: %d", msg_info->msg.msg_len);
    LOGD("[MSG]:msg: %s", msg_info->msg.msg_data);

    if(gSmsReadCb) {
        memset(&sms_info_t, 0, sizeof(sms_info_t));
        strncpy(sms_info_t.phone, msg_info->da, sizeof(sms_info_t.phone));
        sms_info_t.date.year = msg_info->timestamp.tsYear;
        sms_info_t.date.month = msg_info->timestamp.tsMonth;
        sms_info_t.date.day = msg_info->timestamp.tsDay;
        sms_info_t.date.hour = msg_info->timestamp.tsHour;
        sms_info_t.date.minute = msg_info->timestamp.tsMinute;
        sms_info_t.date.second = msg_info->timestamp.tsSecond;
        sms_info_t.date.zone = msg_info->timestamp.tsTimezone;
        sms_info_t.msg_len = msg_info->msg.msg_len;
        memcpy(sms_info_t.msg, msg_info->msg.msg_data, msg_info->msg.msg_len);
        gSmsReadCb(0, &sms_info_t);
    }
}

#if defined(DEBUG_PDU_PARSE) && DEBUG_PDU_PARSE == 1
static void debug_pdu_parse(void) 
{
    // char *pdu = "099164000339541902F0240BA18152484803F40000624010816050230A31D98C56BBD970391A";
    // 短信内容 你好，短信测试abcdefghijklmnopqrstuvwxyz1234567890,.?!@#$%^&*(){}`[]
    // char *pdu = "4F60597DFF0C77ED4FE16D4B8BD5006100620063006400650066006700680069006A006B006C006D006E006F0070007100720073007400750076007700780079007A0031003200330034003500360037003800390030002C002E003F00210040002300240025005E0026002A00280029007B007D0060005B005D";
    char *pdu = "0001000B917118160161F900081C00680065006C006C006F00200077006F0072006C006400204F60597D";
    
    ol_sms_info msg_info;
    memset(&msg_info, 0, sizeof(msg_info));
    strncpy(msg_info.msg.msg_data, pdu, sizeof(msg_info.msg.msg_data));
    msg_info.msg.msg_len = strlen(pdu);
    // GSM -> GSM
    tkl_cellular_sms_set_charactor(TUYA_SMS_GSM);
    ol_sms_msg_report(&msg_info);
    // GSM -> UTF-8
    tkl_cellular_sms_set_charactor(TUYA_SMS_UTF8);
    ol_sms_msg_report(&msg_info);
    // GSM -> UTF-16BE
    tkl_cellular_sms_set_charactor(TUYA_SMS_UTF16BE);
    ol_sms_msg_report(&msg_info);
    // GSM -> UTF-16LE
    tkl_cellular_sms_set_charactor(TUYA_SMS_UTF16LE);
    ol_sms_msg_report(&msg_info);
    // GSM -> ISO8859-1
    tkl_cellular_sms_set_charactor(TUYA_SMS_ISO8859_1);
    ol_sms_msg_report(&msg_info);
    // GSM -> CP936
    tkl_cellular_sms_set_charactor(TUYA_SMS_CP936);
    ol_sms_msg_report(&msg_info);
}
#endif

OPERATE_RET tkl_debug_pdu_parse(char *pdu, TUYA_CELLULAR_SMS_ENCODE_E format)
{    
    ol_sms_info msg_info;
    memset(&msg_info, 0, sizeof(msg_info));
    strncpy(msg_info.msg.msg_data, pdu, sizeof(msg_info.msg.msg_data));
    msg_info.msg.msg_len = strlen(pdu);
    // GSM -> GSM
    tkl_cellular_sms_set_charactor(format);
    ol_sms_msg_report(&msg_info);

    return OPRT_OK;
}

OPERATE_RET tkl_cellular_sms_init(TUYA_CELLULAR_SMS_CB callback)
{
    gSmsReadCb = callback;
    mlInit();
    mbtk_ol_sms_config sms_config = {0};
    sms_config.mem_type[0] = OL_SMS_STOR_MEM_TYPE_ME;
    sms_config.messageFormat = OL_SMS_FORMAT_PDU_MODE;
    ol_sms_config(sms_config);
    ol_sms_msg_report_register(ol_sms_msg_report);
    tkl_mutex_create_init(&tkl_sms_mutex);
#if defined(DEBUG_PDU_PARSE) && DEBUG_PDU_PARSE == 1
    debug_pdu_parse();
#endif
    ol_sms_store sms_mem;
	int ret = ol_sms_mem(&sms_mem);
	if (0!= ret) {
		LOGE("sms init failed, sms_mem ret: %d", ret);
		return OPRT_OK;
	}

	int num = sms_mem.mem1.used;
    LOGD("sms mem1 used: %d", num);
	ol_sms_info msg;
	if (num) {
		if (!gSmsReadCb)
			goto DELETE_ITEM_1;

		for (int i = 0; i < num; i++) {
			ret = ol_sms_read(i, &msg);
			if (0 == ret) {
				if (SMS_STATUS_REC_UNREAD != msg.stat) {
					continue;
				}
				ol_sms_msg_report(&msg);
			}
		}
DELETE_ITEM_1:
		for (int i = 0; i < num; i++) {
			ol_sms_delete(i);
		}
	}

	num = sms_mem.mem2.used;
    LOGD("sms mem2 used: %d", num);
	if (num) {
		for (int i = 0; i < num; i++) {
			ol_sms_delete(i);
		}
	}

	num = sms_mem.mem3.used;
    LOGD("sms mem3 used: %d", num);
	if (num) {
		for (int i = 0; i < num; i++){
			ol_sms_delete(i);
		}
	}

    return OPRT_OK;
}

#endif

//099164000339541902F0240BA18152484803F40000624010816050230A31D98C56BBD970391A   18258484304  1234576894


// GSM -> utf-8 
// GSM -> utf-16be
// GSM -> utf-16le
// UCS-2 -> GSM
// UCS-2 -> utf-8
// UCS-2 -> utf-16be
// UCS-2 -> utf-16le
// utf-16be -> utf-8
// utf-16le -> utf-8
// ISO8859-1 -> GSM
// GSM -> ISO8859-1
