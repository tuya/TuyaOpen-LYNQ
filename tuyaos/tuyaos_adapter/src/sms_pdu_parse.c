
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

extern TUYA_CELLULAR_SMS_ENCODE_E tkl_cellular_sms_get_charactor(void);

//hexstr 转 hex
static int get_hex_val(char ch)
{
	int val;

	if (ch >= 'a' && ch <= 'f')
		val = (ch - 'a') + 10;
	else if (ch >= 'A' && ch <= 'F')
		val = (ch - 'A') + 10;
	else if (ch >= '0' && ch <= '9')
		val = (ch - '0');
	else
		val = 0;
	
	return val;
}

//电话号码解析
static void decode_address(const unsigned char *address_bytes, size_t addr_byte_len, unsigned char ton_npi_byte, char *out, size_t out_sz)
{
    size_t i;
    size_t pos = 0;

    if (out == NULL || out_sz == 0U) {
        return;
    }
    out[0] = '\0';
    for (i = 0; i < addr_byte_len; i++) {
        unsigned char b = address_bytes[i];
        unsigned char low = (unsigned char)(b & 0x0FU);
        unsigned char high = (unsigned char)((b >> 4) & 0x0FU);
        if (low != 0x0FU && pos + 2U < out_sz) {
            out[pos++] = (char)('0' + low);
        }
        if (high != 0x0FU && pos + 2U < out_sz) {
            out[pos++] = (char)('0' + high);
        }
    }
    out[pos] = '\0';
    {
        unsigned char ton = (unsigned char)((ton_npi_byte >> 4) & 0x07U);
        if (ton == 1U && pos + 2U < out_sz) {
            memmove(out + 1, out, pos + 1U);
            out[0] = '+';
        }
    }
}

static unsigned char swap_bcd(unsigned char b)
{
    return (unsigned char)((b & 0x0FU) * 10U + ((b >> 4) & 0x0FU));
}

//时间戳解析
static void fill_timestamp_from_scts(const unsigned char ts_bytes[7], ol_sms_timestamp *ts)
{
    unsigned char tz_byte = ts_bytes[6];
    int tz_sign = (tz_byte & 0x08U) ? -1 : 1;
    unsigned int tz_val = (unsigned int)((tz_byte & 0x07U) * 10U + ((tz_byte >> 4) & 0x0FU));

    if (ts == NULL) {
        return;
    }
    ts->tsYear = swap_bcd(ts_bytes[0]);
    ts->tsMonth = swap_bcd(ts_bytes[1]);
    ts->tsDay = swap_bcd(ts_bytes[2]);
    ts->tsHour = swap_bcd(ts_bytes[3]);
    ts->tsMinute = swap_bcd(ts_bytes[4]);
    ts->tsSecond = swap_bcd(ts_bytes[5]);
    /* Quarter-hour steps from PDU (0..48); actual offset minutes = tsTimezone * 15 */
    ts->tsTimezone = (unsigned char)tz_val;
    ts->tsZoneSign = (tz_sign < 0) ? 1U : 0U;
}

/**
 * @brief Unpack GSM 7-bit packed data into one-byte-per-septet array
 * @param[in]  packed      packed 7-bit bytes from PDU
 * @param[in]  packed_len  byte count of packed data
 * @param[in]  num_chars   expected septet count (from TP-UDL)
 * @param[out] out         output: each byte is a GSM septet index (0~127)
 * @param[in]  out_cap     output buffer capacity
 * @return number of septets written
 */
static int gsm_7bit_unpack(const unsigned char *packed, size_t packed_len,
    unsigned int num_chars, uint8_t *out, size_t out_cap)
{
    uint32_t bit_buffer = 0U;
    unsigned int bits_remaining = 0U;
    unsigned int count = 0U;
    size_t pi = 0U;

    while (pi < packed_len && count < num_chars && count < out_cap) {
        bit_buffer |= ((uint32_t)packed[pi] << bits_remaining);
        bits_remaining += 8U;
        pi++;
        while (bits_remaining >= 7U && count < num_chars && count < out_cap) {
            out[count++] = (uint8_t)(bit_buffer & 0x7FU);
            bit_buffer >>= 7;
            bits_remaining -= 7U;
        }
    }
    if (count < num_chars && count < out_cap && bits_remaining > 0U) {
        out[count++] = (uint8_t)(bit_buffer & 0x7FU);
    }
    return (int)count;
}

int decode_pdu_msg(ol_sms_info* msg)
{
	if (!msg->msg.msg_data)
		return -1;

    // LOGI("msg->msg.msg_data: %s", msg->msg.msg_data);

	int len = strlen((char*)(msg->msg.msg_data));
	if (len % 2) {
		LOGE("invalid msg len: %d", len);
		return -2;
	}

	char* ptr = (char*)(msg->msg.msg_data);
	UINT8_T* pdu = (UINT8_T*)malloc(len / 2);
	if (!pdu) {
		LOGE("malloc buf failed");
		return -3;
	}

	int pdu_len = 0;
	for (int i = 0; i < len; i += 2) {
		pdu[pdu_len++] = (get_hex_val(ptr[i]) << 4) + get_hex_val(ptr[i+1]);
	}

    // LOGI("pdu_len: %d", pdu_len);

    // 跳过sca解析
    int off = 0;
    int sca_len = pdu[off++];
    if(sca_len != 0) {
        off += sca_len;
    }

    uint8_t tpdu_first = pdu[off++];
    uint8_t mti = tpdu_first & 0x03U;
    uint8_t flags_udhi = (unsigned char)((tpdu_first >> 6) & 0x01U);
    int phone_len = pdu[off++];
    uint8_t ton_npi = pdu[off++];
    uint8_t bcd_len = (unsigned char)((phone_len + 1U) / 2U);
    decode_address(pdu + off, (size_t)bcd_len, ton_npi, (char *)msg->da, sizeof(msg->da));
    off += (size_t)bcd_len;
    LOGI("phone: %s", msg->da);

    off++;
    uint8_t dcs = pdu[off++];
    uint8_t encoding = (unsigned char)(dcs & 0x0FU);
    LOGI("DCS: %d encoding: %d", dcs, encoding);
    fill_timestamp_from_scts(pdu + off, &msg->timestamp);
    off += 7U;
    LOGI("timestamp: %d-%d-%d %d:%d:%d zone:%d sign:%c",
        2000U + (unsigned int)msg->timestamp.tsYear,
        (unsigned int)msg->timestamp.tsMonth,
        (unsigned int)msg->timestamp.tsDay,
        (unsigned int)msg->timestamp.tsHour,
        (unsigned int)msg->timestamp.tsMinute,
        (unsigned int)msg->timestamp.tsSecond,
        (unsigned int)msg->timestamp.tsTimezone,
        (unsigned int)msg->timestamp.tsZoneSign);

    uint8_t udl = pdu[off++];
    LOGI("UDL: %u", (unsigned int)udl);             //UDL：用户数据长度

    char *ud_bytes = pdu + off;
    int ud_len_remain = pdu_len - off;

    int udh_len = 0U;
    if (flags_udhi != 0U && ud_len_remain > 0U) {
        unsigned char udhl = ud_bytes[0];
        udh_len = 1U + (size_t)udhl;
        if (udh_len > ud_len_remain) {
            udh_len = 0U;
        }
    }

    unsigned char *text_bytes = ud_bytes + udh_len;
    size_t text_len = (udh_len <= ud_len_remain) ? (ud_len_remain - udh_len) : 0U;

    LOGI("text_len: %d", text_len);
    // uint8_t pdu_msg_print[256*2];
    // for(int i = 0; i < text_len; i++) {
    //     sprintf(pdu_msg_print + i*2, "%02X", ((uint8_t*)text_bytes)[i]);
    // }
    // pdu_msg_print[text_len*2] = '\0';
    // LOGD("no convert msg_hex: %s", pdu_msg_print);

    unsigned from_chset;
    unsigned to_chset = tkl_cellular_sms_get_charactor();
    const void *conv_src = NULL;
    int conv_src_len = 0;
    uint8_t gsm_unpacked[256];
    msg->msg.msg_data[0] = '\0';
    msg->msg.msg_len = 0U;

    if (encoding == 0U) {
        from_chset = ML_GSM;
        int n = gsm_7bit_unpack(text_bytes, text_len, (unsigned int)udl, gsm_unpacked, sizeof(gsm_unpacked));
        conv_src = gsm_unpacked;
        conv_src_len = n;
    } else if (encoding == 0x08U) {
        from_chset = ML_UTF16BE;
        conv_src = text_bytes;
        conv_src_len = (int)text_len;
    } else if (encoding == 0x04U) {
        memcpy(msg->msg.msg_data, text_bytes, text_len);
        msg->msg.msg_len = (unsigned int)text_len;
        msg->msg.decorde_type = encoding;
        return 0;
    } else {
        LOGE("unknown DCS 0x%X", dcs);
        memcpy(msg->msg.msg_data, text_bytes, text_len);
        msg->msg.msg_len = (unsigned int)text_len;
        return -1;
    }

    if(from_chset == to_chset) {
        LOGI("from_chset == to_chset, no convert");
        memcpy(msg->msg.msg_data, conv_src, conv_src_len);
        msg->msg.msg_len = (unsigned int)conv_src_len;
        return 0;
    }

    int out_size = 0;
    LOGI("conv from %d to %d\n", from_chset, to_chset);
    void *converted = mlConvertStr(conv_src, conv_src_len, from_chset, to_chset, &out_size);
    if (converted != NULL) {
        size_t copy_len = ((size_t)out_size < sizeof(msg->msg.msg_data) - 1U)
            ? (size_t)out_size : sizeof(msg->msg.msg_data) - 1U;
        memcpy(msg->msg.msg_data, converted, copy_len);
        msg->msg.msg_data[copy_len] = '\0';
        msg->msg.msg_len = (unsigned int)copy_len;
        free(converted);
    } else {
        LOGE("mlConvertStr failed (from=%u to=%u)", from_chset, to_chset);
        return -1;
    }

    // for(int i = 0; i < msg->msg.msg_len; i++) {
    //     sprintf(pdu_msg_print + i*2, "%02X", ((uint8_t*)msg->msg.msg_data)[i]);
    // }
    // pdu_msg_print[msg->msg.msg_len*2] = '\0';
    // LOGD("convert msg_hex: %s", pdu_msg_print);

	free(pdu);
	return 0;
}


#endif