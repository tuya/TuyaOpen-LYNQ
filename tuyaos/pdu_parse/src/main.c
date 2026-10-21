/**
 * @file main.c
 * @brief Host-side GSM SMS PDU hex parser with ml-library charset conversion
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "ml.h"

#define LOGE(fmt, ...) printf("ERROR: " fmt "\n", ##__VA_ARGS__)
#define LOGI(fmt, ...) printf("INFO: " fmt "\n", ##__VA_ARGS__)
#define LOGD(fmt, ...) printf("DEBUG: " fmt "\n", ##__VA_ARGS__)

typedef enum {
    SMS_STATUS_REC_UNREAD = 0,
    SMS_STATUS_REC_READ   = 1,
    SMS_STATUS_STO_UNSENT = 2,
    SMS_STATUS_STO_SENT   = 3,
    SMS_STATUS_ALL        = 4,
    SMS_STATUS_END
} ol_sms_sta;

typedef struct {
    unsigned char tsYear;
    unsigned char tsMonth;
    unsigned char tsDay;
    unsigned char tsHour;
    unsigned char tsMinute;
    unsigned char tsSecond;
    unsigned char tsTimezone;
    unsigned char tsZoneSign;
} ol_sms_timestamp;

typedef struct {
    unsigned char decorde_type;
    unsigned char msg_data[500];
    unsigned int msg_len;
} ol_sms_msg;

typedef enum {
    PDU_TYPE_DELIVER = 0,
    PDU_TYPE_SUBMIT  = 1,
    PDU_TYPE_UNKNOWN = 0xFF
} pdu_msg_type_e;

typedef struct {
    ol_sms_sta stat;
    unsigned char da[40];
    ol_sms_timestamp timestamp;
    ol_sms_msg msg;

    unsigned char dcs;

    unsigned char submit_rd;
    unsigned char submit_vpf;
    unsigned char submit_srr;
    unsigned char smsc_present;
    char smsc[40];
    char recipient[40];
    unsigned char message_ref;
    char validity[64];
} ol_sms_info;

/* ---------------------------------------------------------------------------
 * PDU helper functions
 * --------------------------------------------------------------------------- */

static void decode_address(const unsigned char *address_bytes, size_t addr_byte_len,
    unsigned char ton_npi_byte, char *out, size_t out_sz)
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

static void decode_vp(unsigned char vp_byte, char *out, size_t out_sz)
{
    if (out == NULL || out_sz == 0U) {
        return;
    }
    if (vp_byte <= 143U) {
        unsigned int minutes = (unsigned int)(vp_byte + 1U) * 5U;
        (void)snprintf(out, out_sz, "%uh%um", (unsigned int)(minutes / 60U), (unsigned int)(minutes % 60U));
    } else if (vp_byte <= 167U) {
        unsigned int days = (unsigned int)(vp_byte - 143U) * 12U * 60U / 60U / 24U;
        (void)snprintf(out, out_sz, "%u days", days);
    } else {
        (void)snprintf(out, out_sz, "unknown VP format");
    }
}

static int get_hex_val(char ch)
{
    if (ch >= 'a' && ch <= 'f') {
        return (ch - 'a') + 10;
    }
    if (ch >= 'A' && ch <= 'F') {
        return (ch - 'A') + 10;
    }
    if (ch >= '0' && ch <= '9') {
        return (ch - '0');
    }
    return 0;
}

static const char *chset_name(unsigned chset)
{
    switch (chset) {
    case ML_ISO8859_1: return "ISO8859-1";
    case ML_UTF8:      return "UTF-8";
    case ML_UTF16BE:   return "UTF-16BE";
    case ML_UTF16LE:   return "UTF-16LE";
    case ML_GSM:       return "GSM";
    case ML_CP936:     return "CP936";
    default:           return "unknown";
    }
}


/* ---------------------------------------------------------------------------
 * PDU decode + ml charset conversion
 * --------------------------------------------------------------------------- */

/**
 * @brief Decode PDU hex string and convert user data to target charset via ml
 * @param[in,out] msg         on input msg_data holds PDU hex; on output holds decoded text
 * @param[in]     to_chset    target charset (ML_UTF8, ML_GSM, ML_UTF16BE, etc.)
 * @return 0 on success
 */
static int decode_pdu_msg(ol_sms_info *msg, unsigned to_chset)
{
    pdu_msg_type_e pdu_type;
    unsigned char flags_udhi;

    unsigned char pdu[512];
    size_t pdu_len = 0U;
    size_t off = 0U;
    unsigned char sca_len;
    unsigned char tpdu_first;
    unsigned char mti;
    unsigned char addr_len;
    unsigned char ton_npi;
    unsigned char bcd_len;
    unsigned char udl;
    unsigned char dcs;
    unsigned char encoding;
    size_t udh_len = 0U;
    const unsigned char *ud_bytes;
    size_t ud_len_remain;

    if (msg == NULL) {
        return -1;
    }
    msg->smsc[0] = '\0';
    msg->recipient[0] = '\0';
    msg->validity[0] = '\0';
    msg->smsc_present = 0U;

    {
        char *ptr = (char *)(msg->msg.msg_data);
        int cnt = 0;
        unsigned int i;
        for (i = 0; i + 1U < msg->msg.msg_len; i += 2U) {
            pdu[cnt++] = (unsigned char)((get_hex_val(ptr[i]) << 4) + get_hex_val(ptr[i + 1]));
        }
        pdu_len = (size_t)cnt;
    }

    sca_len = pdu[off++];
    if (sca_len != 0U) {
        if (off + (size_t)sca_len > pdu_len) {
            LOGE("SCA length overflow, sca_len=%u off=%u pdu_len=%u",
                (unsigned)sca_len, (unsigned)off, (unsigned)pdu_len);
            return -1;
        }
        off += (size_t)sca_len;
    }

    if (off >= pdu_len) {
        LOGE("missing TPDU, off=%u pdu_len=%u", (unsigned)off, (unsigned)pdu_len);
        return -1;
    }
    tpdu_first = pdu[off++];
    mti = (unsigned char)(tpdu_first & 0x03U);

    if (mti == 0U) {
        pdu_type = PDU_TYPE_DELIVER;
        flags_udhi = (unsigned char)((tpdu_first >> 6) & 0x01U);
        msg->stat = SMS_STATUS_REC_UNREAD;
    } else if (mti == 1U) {
        pdu_type = PDU_TYPE_SUBMIT;
        msg->submit_rd = (unsigned char)((tpdu_first >> 2) & 0x01U);
        msg->submit_vpf = (unsigned char)((tpdu_first >> 3) & 0x03U);
        msg->submit_srr = (unsigned char)((tpdu_first >> 5) & 0x01U);
        flags_udhi = (unsigned char)((tpdu_first >> 6) & 0x01U);
        msg->stat = SMS_STATUS_STO_UNSENT;
        if (off >= pdu_len) {
            LOGE("missing MR");
            return -1;
        }
        msg->message_ref = pdu[off++];
    } else {
        LOGE("unsupported MTI=%u", (unsigned)mti);
        return -1;
    }

    if (off + 2U > pdu_len) {
        LOGE("truncated address, off=%u pdu_len=%u", (unsigned)off, (unsigned)pdu_len);
        return -1;
    }
    addr_len = pdu[off++];
    ton_npi = pdu[off++];
    bcd_len = (unsigned char)((addr_len + 1U) / 2U);
    if (off + (size_t)bcd_len > pdu_len) {
        LOGE("address BCD overflow, bcd_len=%u off=%u pdu_len=%u",
            (unsigned)bcd_len, (unsigned)off, (unsigned)pdu_len);
        return -1;
    }
    if (pdu_type == PDU_TYPE_DELIVER) {
        decode_address(pdu + off, (size_t)bcd_len, ton_npi, (char *)msg->da, sizeof(msg->da));
    } else {
        decode_address(pdu + off, (size_t)bcd_len, ton_npi, msg->recipient, sizeof(msg->recipient));
        (void)snprintf((char *)msg->da, sizeof(msg->da), "%s", msg->recipient);
    }
    off += (size_t)bcd_len;

    if (off + 2U > pdu_len) {
        LOGE("truncated PID/DCS, off=%u pdu_len=%u", (unsigned)off, (unsigned)pdu_len);
        return -1;
    }
    (void)pdu[off++];
    dcs = pdu[off++];
    msg->dcs = dcs;

    if (pdu_type == PDU_TYPE_DELIVER) {
        if (off + 7U > pdu_len) {
            LOGE("truncated SCTS, off=%u pdu_len=%u", (unsigned)off, (unsigned)pdu_len);
            return -1;
        }
        fill_timestamp_from_scts(pdu + off, &msg->timestamp);
        off += 7U;
    } else {
        if (off >= pdu_len) {
            LOGE("truncated VP, off=%u pdu_len=%u", (unsigned)off, (unsigned)pdu_len);
            return -1;
        }
        decode_vp(pdu[off++], msg->validity, sizeof(msg->validity));
    }

    if (off >= pdu_len) {
        LOGE("missing UDL, off=%u pdu_len=%u", (unsigned)off, (unsigned)pdu_len);
        return -1;
    }
    udl = pdu[off++];

    ud_bytes = pdu + off;
    ud_len_remain = pdu_len - off;
    encoding = (unsigned char)(dcs & 0x0FU);

    udh_len = 0U;
    if (flags_udhi != 0U && ud_len_remain > 0U) {
        unsigned char udhl = ud_bytes[0];
        udh_len = 1U + (size_t)udhl;
        if (udh_len > ud_len_remain) {
            udh_len = 0U;
        }
    }

    {
        const unsigned char *text_bytes = ud_bytes + udh_len;
        size_t text_len = (udh_len <= ud_len_remain) ? (ud_len_remain - udh_len) : 0U;
        unsigned from_chset;
        const void *conv_src = NULL;
        int conv_src_len = 0;
        uint8_t gsm_unpacked[256];

        msg->msg.msg_data[0] = '\0';
        msg->msg.msg_len = 0U;

        if (encoding == 0x00U) {
            int n = gsm_7bit_unpack(text_bytes, text_len, (unsigned int)udl, gsm_unpacked, sizeof(gsm_unpacked));
            from_chset = ML_GSM;
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
            LOGE("unknown DCS 0x%02X", dcs);
            memcpy(msg->msg.msg_data, text_bytes, text_len);
            msg->msg.msg_len = (unsigned int)text_len;
            return -1;
        }

        {
            int out_size = 0;
            LOGI("conv from %s to %s\n", chset_name(from_chset), chset_name(to_chset));
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
        }
        msg->msg.decorde_type = encoding;
    }

    return 0;
}

/* ---------------------------------------------------------------------------
 * Debug print + main
 * --------------------------------------------------------------------------- */

static void debug_pdu_parse(const ol_sms_info *msg, unsigned to_chset)
{
    if (msg == NULL) {
        return;
    }
    printf("===== PDU parse (C + ml) =====\n");
    printf("phone: %s\n", (const char *)msg->da);

    {
        unsigned int year_full = 2000U + (unsigned int)msg->timestamp.tsYear;
        printf("timestamp: %04u-%02u-%02u %02u:%02u:%02u (zone %s%u min)\n",
            year_full,
            (unsigned int)msg->timestamp.tsMonth,
            (unsigned int)msg->timestamp.tsDay,
            (unsigned int)msg->timestamp.tsHour,
            (unsigned int)msg->timestamp.tsMinute,
            (unsigned int)msg->timestamp.tsSecond,
            (msg->timestamp.tsZoneSign != 0U) ? "-" : "+",
            (unsigned int)msg->timestamp.tsTimezone * 15U);
    }

    printf("DCS: 0x%02X (pdu encoding: %s)\n", (unsigned int)msg->dcs,
        (msg->msg.decorde_type == 0U) ? "GSM 7-bit" :
        (msg->msg.decorde_type == 8U) ? "UCS2" :
        (msg->msg.decorde_type == 4U) ? "8-bit" : "other");
    printf("target charset: %s\n", chset_name(to_chset));
    printf("msg_len: %u\n", msg->msg.msg_len);

    if (to_chset == ML_UTF8 || to_chset == ML_GSM || to_chset == ML_ISO8859_1 || to_chset == ML_CP936) {
        printf("text: %s\n", (const char *)msg->msg.msg_data);
    } else {
        printf("text: (non-printable charset)\n");
    }

    printf("hex dump (%u bytes):", msg->msg.msg_len);
    for (unsigned int i = 0; i < msg->msg.msg_len; i++) {
        if (i % 16U == 0U) {
            printf("\n  %04X: ", i);
        }
        printf("%02X ", msg->msg.msg_data[i]);
    }
    printf("\n");
}

static unsigned parse_chset_arg(const char *arg)
{
    if (arg == NULL) {
        return ML_UTF8;
    }
    if (strcmp(arg, "utf8") == 0 || strcmp(arg, "UTF8") == 0 || strcmp(arg, "utf-8") == 0) {
        return ML_UTF8;
    }
    if (strcmp(arg, "gsm") == 0 || strcmp(arg, "GSM") == 0) {
        return ML_GSM;
    }
    if (strcmp(arg, "utf16be") == 0 || strcmp(arg, "UTF16BE") == 0) {
        return ML_UTF16BE;
    }
    if (strcmp(arg, "utf16le") == 0 || strcmp(arg, "UTF16LE") == 0) {
        return ML_UTF16LE;
    }
    if (strcmp(arg, "cp936") == 0 || strcmp(arg, "CP936") == 0 || strcmp(arg, "gbk") == 0 || strcmp(arg, "GBK") == 0) {
        return ML_CP936;
    }
    if (strcmp(arg, "iso8859") == 0 || strcmp(arg, "ISO8859") == 0 || strcmp(arg, "latin1") == 0) {
        return ML_ISO8859_1;
    }
    fprintf(stderr, "unknown charset '%s', using UTF-8\n", arg);
    return ML_UTF8;
}

int main(int argc, char *argv[])
{
    ol_sms_info msg_info;
    unsigned to_chset;

    if (argc < 2 || argc > 3) {
        printf("Usage: %s <pdu_hex> [charset]\n", argv[0]);
        printf("  charset: utf8(default), gsm, utf16be, utf16le, cp936/gbk, iso8859/latin1\n");
        return -1;
    }

    mlInit();

    to_chset = parse_chset_arg((argc >= 3) ? argv[2] : NULL);

    memset(&msg_info, 0, sizeof(msg_info));
    (void)snprintf((char *)msg_info.msg.msg_data, sizeof(msg_info.msg.msg_data), "%s", argv[1]);
    msg_info.msg.msg_len = (unsigned int)strlen(argv[1]);

    if (decode_pdu_msg(&msg_info, to_chset) != 0) {
        debug_pdu_parse(&msg_info, to_chset);
        return 1;
    }

    debug_pdu_parse(&msg_info, to_chset);
    return 0;
}
