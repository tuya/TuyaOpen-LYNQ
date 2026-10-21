import sys
import binascii
import os
import sys
import serial
import time
import struct
import typer
import sys
import traceback
import os

from prettytable import PrettyTable


app = typer.Typer()


def green_print(content: str):
    print("\033[0;32;40m{}\033[0m".format(content))


def yellow_print(content: str):
    print("\033[0;33;40m{}\033[0m".format(content))


def red_print(content: str):
    print("\033[0;31;40m{}\033[0m".format(content))


def green_format(content: str) -> str:
    return ("\033[0;32;40m{}\033[0m".format(content))


def yellow_format(content: str) -> str:
    return ("\033[0;33;40m{}\033[0m".format(content))


def red_format(content: str) -> str:
    return ("\033[0;31;40m{}\033[0m".format(content))


SEND_DUMMY = b'\r\n'
SEND_ACK = b'\00\00\00\00\11\11\11\11'


def datacrc32(data, lens):
    res = 0xFFFFFFFF
    res = 0
    counter = 0
    for d in data:
        # print(d.to_bytes(1,byteorder='little'))
        res = binascii.crc32(d.to_bytes(1, byteorder='little'), res)
        counter = counter+1
        if counter == lens:
            break
    return '%08x' % (res & 0xffffffff)


def push_file(ser: serial.Serial, drive_letter: str, file_path: str):
    str_name = os.path.basename(file_path)
    PUSH_CMD = 'get {}:/{}\r\n'.format(drive_letter, str_name).encode()
    # print("PUSH_CMD", PUSH_CMD)

    data_packet_len = 1000
    packet_size = data_packet_len+20

    localfile = open(file_path, mode='rb')
    stats = os.stat(file_path)
    file_size = stats.st_size
    # print("file_size:", file_size)
    pak_count = file_size/data_packet_len
    pak_leave = file_size % data_packet_len

    # print(type(pak_count))
    # print(int(pak_count))

    ser.write(PUSH_CMD)
    time.sleep(0.1)
    s1 = ser.read(ser.in_waiting)

    # print(len(s1))
    # print(s1)

    # str_bytes = bytes(128)
    str_bytes = str_name.encode('UTF-8')
    # send file header
    file_hdr = struct.pack('<'+'I128sII', file_size,
                           str_bytes, packet_size, int(pak_count))
    ser.write(file_hdr)
    # print(file_hdr)
    # print(len(file_hdr))
    # get file header ack
    ack = ser.read(8)
    loop = int(pak_count)+1
    # send file data
    data_bytes = bytes(1000)
    pack_flag = b'\xee\xee\xee\xee'

    for i in range(loop):
        # print(i)
        cur = i+1
        if cur > loop:
            break
        # print(loop)
        if cur == loop:
            # print("last")
            file_data = localfile.read(pak_leave)
            # print(pak_leave)
            data_bytes = file_data
            packet_data_size = int(pak_leave)
            packet_reserve = b'\00\00'
            datacrc = datacrc32(data_bytes, pak_leave)
            headcrc = b'\00\00\00\00'
            # print("calc data crc:", datacrc)
            # print(type(datacrc))
            datacrc_ba = bytearray.fromhex(datacrc)
            # print(datacrc_ba)
            file_hdr = struct.pack('<'+'I4sH2s4s4s1000s', int(i), pack_flag,
                                   packet_data_size, packet_reserve, datacrc_ba, headcrc, data_bytes)
            # print(file_hdr)
            # file_hdr = struct.pack('<'+'2I2H2I4096s',int(i),pack_flag,packet_data_size,packet_reserve,111,222,data_bytes)
            ser.write(file_hdr)
        else:
            # print("mid data")
            file_data = localfile.read(data_packet_len)
            # print(data_packet_len)
            data_bytes = file_data
            packet_data_size = int(data_packet_len)
            packet_reserve = b'\00\00'
            datacrc = datacrc32(data_bytes, data_packet_len)
            headcrc = b'\00\00\00\00'
            # print("calc data crc:", datacrc)
            # print(type(datacrc))
            datacrc_ba = bytearray.fromhex(datacrc)
            # print(datacrc_ba)
            file_hdr = struct.pack('<'+'I4sH2s4s4s1000s', int(i), pack_flag,
                                   packet_data_size, packet_reserve, datacrc_ba, headcrc, data_bytes)
            # print(file_hdr)
            # file_hdr = struct.pack('<'+'2I2H2I4096s',int(i),pack_flag,packet_data_size,packet_reserve,111,222,data_bytes)
            ser.write(file_hdr)
        # get data ack
        ack = ser.read(8)
    # send file header
    # print("data over")


def pull_file(ser: serial.Serial, file_path: str):
    drive_letter = file_path.split(":/")[0]
    if drive_letter == "c":
        ser.write(b'\r\n')
        ser.write(b'\r\n')
        ser.write(b'cd C:\r\n')
        time.sleep(0.2)
        s1 = ser.read(ser.in_waiting)
    elif drive_letter == "d":
        ser.write(b'\r\n')
        ser.write(b'\r\n')
        ser.write(b'cd D:\r\n')
        time.sleep(0.2)
        s1 = ser.read(ser.in_waiting)
    PULL_CMD = 'send {}'.format(file_path).encode()
    ser.write(PULL_CMD+b"\r\n")
    time.sleep(1)
    # s1 = ser.read(cmd_lens+promt_lens+file_pkt_header_lens)
    s1 = ser.read(ser.in_waiting)
    # print(len(s1))
    # print(s1)
    # send c: / test.txt\n\rC:\\>
    s1 = s1.replace(PULL_CMD, b"")
    s1 = s1.replace(b"\r\n", b"")
    s1 = s1.replace(b"\n\r", b"")
    s1 = s1.replace("{}:\\>".format(drive_letter.upper()).encode(), b"")
    # print(s1)

    filesize, file_name, packet_size, filecounter = struct.unpack(
        '<I128s2I', s1)
    # print('file_hdr:', prot)
    # print('filesize:', filesize)
    real_file_name = str(file_name, 'utf8').strip('\00')
    # print('file_name:', real_file_name[3:])
    # print('length of file_name:', len(real_file_name))
    # print('packet_size:', packet_size)
    # print('filecounter:', filecounter)

    loop = filecounter + 1
    data_packet_len = packet_size-20
    localfile = open(real_file_name[3:], mode='wb')
    for i in range(loop):
        # ack
        cur = i+1
        if cur > loop:
            break
        ser.write(SEND_ACK)
        s2 = ser.read(packet_size)
        # print(len(s2))
        # print(s2)
        # parse
        packet_counter, packet_flag, packet_data_size, packet_reserve, data_crc, header_crc, data = struct.unpack(
            '<'+'2I2H2I'+str(data_packet_len)+'s', s2)
        # print("packet_flag:%08x" % packet_flag)
        # print("packet_data_size:", packet_data_size)
        # print("packet_reserve:", packet_reserve)
        # print("data_crc:%08x" % data_crc)
        # print("header_crc:%08x" % header_crc)
        # print("packet_counter:", packet_counter)
        real_data_size = packet_data_size - 20
        if cur == loop:
            # print("calc last data crc:", datacrc32(
            #     data, filesize % data_packet_len))
            localfile.write(data[0:packet_data_size])
        else:
            # print("calc data crc:", datacrc32(data, data_packet_len))
            localfile.write(data)
        localfile.flush()
    localfile.close()
    ser.write(SEND_DUMMY)
    time.sleep(0.2)
    print("download ok in {}".format(real_file_name[3:]))


def ls_dir(ser: serial.Serial, drive_letter: str):
    file_list_table = PrettyTable(['name', 'size'], align="l")
    if drive_letter == "c":
        ser.write(b'\r\n')
        ser.write(b'\r\n')
        ser.write(b'cd C:\r\n')
        time.sleep(0.2)
        s1 = ser.read(ser.in_waiting)
    elif drive_letter == "d":
        ser.write(b'\r\n')
        ser.write(b'\r\n')
        ser.write(b'cd D:\r\n')
        time.sleep(0.2)
        s1 = ser.read(ser.in_waiting)

    ser.write(b'ls\r\n')
    time.sleep(0.5)
    s1 = ser.read(ser.in_waiting)
    # print(s1)
    s2 = s1.split(b'\n\r')
    # print(s2)

    for s in s2[s2.index(b'file name\tsize\r\n')+1:-2]:
        file_info = s.split(b'\t')
        row = [file_info[0].decode(), file_info[1].decode()]
        file_list_table.add_row(row)

    print(file_list_table)
    print(s2[-2].decode())


def setup_port(port: str) -> serial.Serial:
    ser = serial.Serial(port, 115200)
    ser.write(b'\r\n')
    time.sleep(0.1)
    ser.write(b'\r\n')
    time.sleep(0.1)
    s1 = ser.read(ser.in_waiting)
    return ser


@app.command()
def ls(port: str, drive_letter: str):
    ser = setup_port(port)
    ls_dir(ser, drive_letter)


@app.command()
def push(port: str, drive_letter: str, file_path: str):
    ser = setup_port(port)
    push_file(ser, drive_letter, file_path)


@app.command()
def pull(port: str, file_path: str):
    ser = setup_port(port)
    pull_file(ser, file_path)


@app.command()
def rm(port: str, file_path: str):
    ser = setup_port(port)
    DEL_CMD = 'rm '+file_path+'\r\n'
    ser.write(DEL_CMD.encode())
    time.sleep(0.5)
    print(ser.read(ser.in_waiting).decode())


if __name__ == "__main__":
    try:
        app()
    except serial.serialutil.SerialException as se:
        red_print("Can not open port")
        sys.exit(1)
    except Exception as e:
        traceback.print_exc()
        sys.exit(1)
