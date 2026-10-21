
#!/usr/bin/python
# -*- coding: UTF-8 -*-

import sys
import binascii
import os
import sys
import inspect
import serial.tools.list_ports
import time
import random

import struct
import windnd

from tkinter.ttk import *
from tkinter import *

import tkinter as tk

DEFAULT_BAUD='115200'
DEFAULT_PORT='COM3'

ser = serial.Serial(DEFAULT_PORT, DEFAULT_BAUD, bytesize=8, parity='N', stopbits=1,rtscts = 0,timeout=1)
    
#SEND_CMD = b'send c:/default.info\r\n'
#SEND_CMD = b'send c:/NetConnetOk_44100_705600.wav\r\n'
SEND_DUMMY = b'\r\n'
SEND_ACK = b'\00\00\00\00\11\11\11\11'
class FatalError(RuntimeError):
    def __init__(self, message):
        RuntimeError.__init__(self, message)

    @staticmethod
    def WithResult(message, result):
        """
        Return a fatal error object that appends the hex values of
        'result' as a string formatted argument.
        """
        message += " (result was %s)" % hex(result)
        return FatalError(message)
        
def readInChunks(fileObj, chunkSize=4096):
    while 1:
        data = fileObj.read(chunkSize)
        if not data:
            break
        yield data

def crc2hex(fileName): 
    res = 0xFFFFFFFF
    res = 0
    for line in open(fileName,"rb"):
        res = binascii.crc32(line,res)
    return '%08x' % (res & 0xffffffff) 

def datacrc32(data,lens): 
    res = 0xFFFFFFFF
    res = 0
    counter = 0
    for d in data:
        # print(d.to_bytes(1,byteorder='little'))
        res = binascii.crc32(d.to_bytes(1,byteorder='little'),res)
        counter = counter+1
        if counter == lens:
            break
    return '%08x' % (res & 0xffffffff) 
    
def unload_packet(packet):
    msg_id, msg_size = struct.unpack_from('>HI', packet, 0)
    msg_payload = struct.unpack_from('>%dH' % (int(msg_size-6)/2), packet, 6)
    return msg_id, msg_size, msg_payload

def get_file(arg):
    print("* Target Connected.       *")
    str_name = os.path.basename(arg)
    GET_CMD = b'get c:/'+str_name.encode('utf-8')+b'\r\n'
    promt_lens = 10
    cmd_lens = len(GET_CMD)
    cmd_length=cmd_lens+promt_lens
    print("cmd length",cmd_length)
    print("cmd length",GET_CMD)
    
    data_packet_len = 1000
    packet_size = data_packet_len+20;
    
    localfile = open(arg,mode='rb')
    stats = os.stat(arg)
    file_size = stats.st_size
    print("file_size:",file_size)
    pak_count = file_size/data_packet_len
    pak_leave = file_size%data_packet_len

    print(type(pak_count))
    print(int(pak_count))
    
    ser.write(GET_CMD)
    #get cmd header ack
    s1=ser.read(cmd_length)
    print(len(s1))
    print(s1)
    
    str_bytes=bytes(128)
    str_bytes = str_name.encode('UTF-8')
    #send file header
    file_hdr = struct.pack('<'+'I128sII',file_size,str_bytes,packet_size,int(pak_count))
    ser.write(file_hdr)
    print(file_hdr)
    print(len(file_hdr))
    #get file header ack
    ack=ser.read(8)
    loop = int(pak_count)+1
    #send file data
    data_bytes=bytes(1000)
    pack_flag = b'\ee\ee\ee\ee'
    
    for i in range(loop):
        #print(i)
        cur = i+1
        if cur > loop:
            break
        #print(loop)
        if cur ==loop:
            print("last")
            file_data = localfile.read(pak_leave)
            print(pak_leave)
            data_bytes = file_data
            packet_data_size = int(pak_leave)
            packet_reserve = b'\00\00'
            datacrc = datacrc32(data_bytes,pak_leave)
            headcrc=b'\00\00\00\00'
            print("calc data crc:",datacrc)
            print(type(datacrc))
            datacrc_ba= bytearray.fromhex(datacrc)
            print(datacrc_ba)
            file_hdr = struct.pack('<'+'I4sH2s4s4s1000s',int(i),pack_flag,packet_data_size,packet_reserve,datacrc_ba,headcrc,data_bytes)
            print(file_hdr)
            #file_hdr = struct.pack('<'+'2I2H2I4096s',int(i),pack_flag,packet_data_size,packet_reserve,111,222,data_bytes)
            ser.write(file_hdr)
        else:
            print("mid data")
            file_data = localfile.read(data_packet_len)
            print(data_packet_len)
            data_bytes = file_data
            packet_data_size = int(data_packet_len)
            packet_reserve = b'\00\00'
            datacrc = datacrc32(data_bytes,data_packet_len)
            headcrc=b'\00\00\00\00'
            print("calc data crc:",datacrc)
            print(type(datacrc))
            datacrc_ba= bytearray.fromhex(datacrc)
            print(datacrc_ba)
            file_hdr = struct.pack('<'+'I4sH2s4s4s1000s',int(i),pack_flag,packet_data_size,packet_reserve,datacrc_ba,headcrc,data_bytes)
            print(file_hdr)
            #file_hdr = struct.pack('<'+'2I2H2I4096s',int(i),pack_flag,packet_data_size,packet_reserve,111,222,data_bytes)
            ser.write(file_hdr)
        #get data ack
        ack=ser.read(8)
    #send file header
    print("data over")

    
def send(arg):
    #open port
    print("* Target Connected.       *")
    SEND_CMD = b'send c:/'+arg+b'\r\n'
    cmd_lens = len(SEND_CMD)
    promt_lens = 4
    file_pkt_header_lens=140
    cmd_length=cmd_lens+promt_lens
    print("cmd length",cmd_length)
    ser.write(SEND_CMD)
    s1=ser.read(cmd_lens+promt_lens+file_pkt_header_lens)
    print(len(s1))
    print(s1)
    prot,filesize,file_name,packet_size,filecounter= struct.unpack('<'+str(cmd_length)+'sI128s2I', s1)
    print('file_hdr:',prot)
    print('filesize:',filesize)
    real_file_name = str(file_name,'utf8').strip('\00')
    print('file_name:', real_file_name[3:])
    print('length of file_name:', len(real_file_name))
    print('packet_size:',packet_size)
    print('filecounter:',filecounter)
    
    loop = filecounter + 1
    data_packet_len = packet_size-20;
    localfile = open(real_file_name[3:],mode='wb')
    for i in range(loop):
        #ack
        cur = i+1
        if cur > loop:
            break
        ser.write(SEND_ACK)
        s2=ser.read(packet_size)
        # print(len(s2))
        # print(s2)
        #parse
        packet_counter,packet_flag,packet_data_size,packet_reserve,data_crc,header_crc,data= struct.unpack('<'+'2I2H2I'+str(data_packet_len)+'s', s2)
        print("packet_flag:%08x"%packet_flag)
        print("packet_data_size:",packet_data_size)
        print("packet_reserve:",packet_reserve)
        print("data_crc:%08x"%data_crc)
        print("header_crc:%08x"%header_crc)
        print("packet_counter:",packet_counter)
        real_data_size = packet_data_size -20
        if cur ==loop:
            print("calc last data crc:",datacrc32(data,filesize%data_packet_len))
            localfile.write(data[0:packet_data_size])
        else:
            print("calc data crc:",datacrc32(data,data_packet_len))
            localfile.write(data)
        print("***************************")
        localfile.flush()
    localfile.close()
    ser.write(SEND_DUMMY)
    time.sleep(1)
    print("download finished")

def dele(arg):
    #open port
    DEL_CMD = b'rm c:/'+arg+b'\r\n'
    ser.write(DEL_CMD)
    time.sleep(3)
    print("delete finished")

window = Tk()
window.title("文件浏览器")
window.geometry('640x320')
list1 = Listbox(window,width=80,height=10,selectmode = SINGLE)
list1.place(x=10,y=5,width=500,height=260)

def click_button():
    val = list1.get(list1.curselection())
    send(val)

btn1 = tk.Button(window, text='下载', command=click_button)
btn1.place(x=520,y=5,width=100,height=30)

lb1 =Label(window, text='上传文件路径：',fg='red')
lb1.place(x=0,y=280,width=100,height=30)
var_str = StringVar()
lb2 =Entry(window,textvariable=var_str,width=80, bd=2)
lb2.place(x=90,y=280,width=420,height=30)

def ls_dir():
    ser.rts = 0
    print("* Target Connected.       *")
    s1=ser.read(4096)
    print(" ser.read(1024)")
    print(s1)
    time.sleep(2) 
    ser.write(b'\r\n')
    s1=ser.read(4096)
    print(" ser.read(2048)")
    print(s1)
    print(" time.sleep(5) ")
    ser.write(b'ls\n\r')
    s1=ser.read(4096)
    print(" ser.read(4096)")
    print(s1)
    s2=s1.split(b'\n\r')
    print(s2)
    print(s2[6])
    for s in s2[5:-2]:
        dir_list = s.split(b'\t')[0]
        print(dir_list)
        list1.insert(0,dir_list)
       
        
def click_up_button():
    print(var_str.get())
    get_file(var_str.get())
    list1.delete(0, tk.END)
    ls_dir()
    
btn2 = tk.Button(window, text='上传', command=click_up_button)
btn2.place(x=520,y=280,width=100,height=30)

def click_del_button():
    val = list1.get(list1.curselection())
    dele(val)
    list1.delete(0, tk.END)
    ls_dir()
    
btn3 = tk.Button(window, text='删除', command=click_del_button)
btn3.place(x=520,y=75,width=100,height=30)

def click_fresh_button():
    list1.delete(0, tk.END)
    ls_dir()
    
btn4 = tk.Button(window, text='刷新', command=click_fresh_button)
btn4.place(x=520,y=40,width=100,height=30)


def dragged_files(files):
    msg = '\n'.join((item.decode('gbk') for item in files))
    print("拖拽文件路径:",msg)
    var_str.set(msg)


        
def main():
    ls_dir()
    # windnd.hook_dropfiles(window,func=dragged_files)
    window.mainloop()
    
def _main():
    try:
        main()
    except FatalError as e:
        print('\n A fatal error occurred: %s' % e)
        sys.exit(2)

if __name__ == '__main__':
    _main()
    
