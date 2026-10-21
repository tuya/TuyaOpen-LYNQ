#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
GSM SMS PDU 解析器
支持 SMS-DELIVER / SMS-SUBMIT
支持 GSM 7-bit / UCS2 / 8-bit 编码
"""

import sys
import re
from datetime import datetime, timedelta, timezone

# ---------- GSM 03.38 默认字母表映射 ----------
GSM_7BIT_TABLE = [
    '@', '£', '$', '¥', 'è', 'é', 'ù', 'ì', 'ò', 'Ç', '\n', 'Ø', 'ø', '\r', 'Å', 'å',
    'Δ', '_', 'Φ', 'Γ', 'Λ', 'Ω', 'Π', 'Ψ', 'Σ', 'Θ', 'Ξ', '\x1b', 'Æ', 'æ', 'ß', 'É',
    ' ', '!', '"', '#', '¤', '%', '&', "'", '(', ')', '*', '+', ',', '-', '.', '/',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',
    '¡', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'Ä', 'Ö', 'Ñ', 'Ü', '§',
    '¿', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'ä', 'ö', 'ñ', 'ü', 'à'
]

def gsm_7bit_decode(packed_data, num_chars):
    """将 GSM 7-bit 打包字节解包为字符索引列表"""
    bit_buffer = 0
    bits_remaining = 0
    indices = []
    for b in packed_data:
        bit_buffer |= (b << bits_remaining)
        bits_remaining += 8
        while bits_remaining >= 7:
            idx = bit_buffer & 0x7F
            indices.append(idx)
            bit_buffer >>= 7
            bits_remaining -= 7
            if len(indices) == num_chars:
                break
        if len(indices) == num_chars:
            break
    if len(indices) < num_chars and bits_remaining > 0:
        idx = bit_buffer & 0x7F
        indices.append(idx)
    return indices

def decode_gsm_text(indices):
    """将索引列表转换为 GSM 03.38 字符"""
    text = []
    for idx in indices:
        if idx < len(GSM_7BIT_TABLE):
            text.append(GSM_7BIT_TABLE[idx])
        else:
            text.append('?')
    return ''.join(text)

def decode_address(address_bytes, ton_npi_byte):
    """解码地址字段（半字节交换 BCD + TON/NPI）"""
    digits = []
    for b in address_bytes:
        low = b & 0x0F
        high = (b >> 4) & 0x0F
        if low != 0x0F:
            digits.append(str(low))
        if high != 0x0F:
            digits.append(str(high))
    number = ''.join(digits)
    ton = (ton_npi_byte >> 4) & 0x07
    if ton == 1:
        return '+' + number
    else:
        return number

def swap_bcd(b):
    """将半字节交换的 BCD 字节还原为十进制数值"""
    return (b & 0x0F) * 10 + ((b >> 4) & 0x0F)

def decode_timestamp(ts_bytes):
    """解码时间戳 (SMS-DELIVER 的 TP-SCTS, 7 字节半字节交换 BCD)"""
    year   = 2000 + swap_bcd(ts_bytes[0])
    month  = swap_bcd(ts_bytes[1])
    day    = swap_bcd(ts_bytes[2])
    hour   = swap_bcd(ts_bytes[3])
    minute = swap_bcd(ts_bytes[4])
    second = swap_bcd(ts_bytes[5])
    tz_byte = ts_bytes[6]
    tz_sign = -1 if (tz_byte & 0x08) else 1
    tz_val = ((tz_byte & 0x07) * 10 + ((tz_byte >> 4) & 0x0F))
    tz_offset = timedelta(minutes=tz_sign * tz_val * 15)
    tz_info = timezone(tz_offset)
    return datetime(year, month, day, hour, minute, second, tzinfo=tz_info)

def decode_vp(vp_byte):
    """解码有效期（SMS-SUBMIT 的 TP-VP）简单处理"""
    if vp_byte <= 143:
        minutes = (vp_byte + 1) * 5
        return f"{minutes // 60}h{minutes % 60}m"
    elif vp_byte <= 167:
        days = (vp_byte - 143) * 12 * 60 // 60 // 24
        return f"{days}天"
    else:
        return "未知有效期格式"

def parse_pdu(pdu_hex):
    """解析完整的PDU字符串"""
    pdu_bytes = bytes.fromhex(pdu_hex)
    offset = 0
    result = {}

    # ---- 1. 短信中心 (SCA) ----
    sca_len = pdu_bytes[offset]
    offset += 1
    if sca_len > 0:
        sca_ton_npi = pdu_bytes[offset]
        offset += 1
        sca_number_bytes = pdu_bytes[offset:offset+sca_len-1]
        result['smsc'] = decode_address(sca_number_bytes, sca_ton_npi)
        offset += sca_len - 1
    else:
        result['smsc'] = None

    # ---- 2. TPDU 首字节 (TP-MTI) ----
    tpdu_first = pdu_bytes[offset]
    offset += 1
    mti = tpdu_first & 0x03
    result['mti'] = mti
    if mti == 0x00:   # SMS-DELIVER
        result['type'] = 'SMS-DELIVER'
        tp_mms  = (tpdu_first >> 2) & 0x01
        tp_sri  = (tpdu_first >> 5) & 0x01
        tp_udhi = (tpdu_first >> 6) & 0x01
        tp_rp   = (tpdu_first >> 7) & 0x01
        result['flags'] = {'UDHI': tp_udhi, 'SRI': tp_sri, 'MMS': tp_mms, 'RP': tp_rp}
    elif mti == 0x01: # SMS-SUBMIT
        result['type'] = 'SMS-SUBMIT'
        tp_rd   = (tpdu_first >> 2) & 0x01
        tp_vpf  = (tpdu_first >> 3) & 0x03
        tp_srr  = (tpdu_first >> 5) & 0x01
        tp_udhi = (tpdu_first >> 6) & 0x01
        tp_rp   = (tpdu_first >> 7) & 0x01
        result['flags'] = {'RD': tp_rd, 'VPF': tp_vpf, 'UDHI': tp_udhi, 'SRR': tp_srr, 'RP': tp_rp}
    else:
        result['type'] = 'Unknown'
        result['error'] = 'Unsupported MTI'
        return result

    # ---- 3. 消息参考 (TP-MR) - only for SMS-SUBMIT ----
    if result['type'] == 'SMS-SUBMIT':
        result['message_ref'] = pdu_bytes[offset]
        offset += 1

    # ---- 4. 目标/源地址 (TP-DA / TP-OA) ----
    addr_len = pdu_bytes[offset]
    offset += 1
    ton_npi = pdu_bytes[offset]
    offset += 1
    bcd_len = (addr_len + 1) // 2
    addr_bytes = pdu_bytes[offset:offset+bcd_len]
    offset += bcd_len
    number = decode_address(addr_bytes, ton_npi)
    if result['type'] == 'SMS-DELIVER':
        result['sender'] = number
    else:
        result['recipient'] = number

    # ---- 5. TP-PID ----
    result['pid'] = pdu_bytes[offset]
    offset += 1

    # ---- 6. TP-DCS ----
    result['dcs'] = pdu_bytes[offset]
    offset += 1

    # ---- 7. 时间戳或有效期 ----
    if result['type'] == 'SMS-DELIVER':
        ts_bytes = pdu_bytes[offset:offset+7]
        offset += 7
        result['timestamp'] = decode_timestamp(ts_bytes)
    else:  # SMS-SUBMIT
        vp_byte = pdu_bytes[offset]
        offset += 1
        result['validity'] = decode_vp(vp_byte)

    # ---- 8. 用户数据长度 (TP-UDL) ----
    udl = pdu_bytes[offset]
    offset += 1
    result['udl'] = udl

    # ---- 9. 用户数据 (TP-UD) ----
    ud_bytes = pdu_bytes[offset:]

    dcs = result['dcs']
    encoding = dcs & 0x0F
    tp_udhi = result['flags'].get('UDHI', 0)

    udh_len = 0
    if tp_udhi:
        if len(ud_bytes) > 0:
            udhl = ud_bytes[0]
            udh_len = 1 + udhl

    text_bytes = ud_bytes[udh_len:]

    if encoding == 0x00:  # GSM 7-bit
        num_chars = udl
        indices = gsm_7bit_decode(text_bytes, num_chars)
        result['text'] = decode_gsm_text(indices)
    elif encoding == 0x08:  # UCS2
        result['text'] = text_bytes.decode('utf-16be', errors='replace')
    elif encoding == 0x04:  # 8-bit
        result['text'] = f"[二进制数据: {text_bytes.hex()}]"
    else:
        result['text'] = f"[未知编码 0x{dcs:02X}, 原始数据: {text_bytes.hex()}]"

    return result

def print_result(result):
    print("===== PDU 解析结果 =====")
    if result.get('smsc'):
        print(f"短信中心: {result['smsc']}")
    print(f"消息类型: {result.get('type')}")
    if 'sender' in result:
        print(f"发送方: {result['sender']}")
    if 'recipient' in result:
        print(f"接收方: {result['recipient']}")
    if 'timestamp' in result:
        print(f"时间戳: {result['timestamp']}")
    if 'validity' in result:
        print(f"有效期: {result['validity']}")
    print(f"数据编码: 0x{result['dcs']:02X}")
    print(f"用户数据长度: {result.get('udl')}")
    print(f"短信内容: {result.get('text', '')}")
    if result.get('flags'):
        print(f"标志位: {result['flags']}")
    if 'error' in result:
        print(f"错误: {result['error']}")

def main():
    if len(sys.argv) > 1:
        pdu_str = sys.argv[1]
    else:
        pdu_str = input("请输入PDU十六进制字符串: ").strip()
    pdu_str = re.sub(r'\s+', '', pdu_str)
    try:
        result = parse_pdu(pdu_str)
        print_result(result)
    except Exception as e:
        print(f"解析失败: {e}")
        import traceback
        traceback.print_exc()

if __name__ == '__main__':
    main()