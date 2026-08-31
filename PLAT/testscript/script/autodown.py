# coding:utf-8
import os
import time
import serial

ser_hold = serial.Serial("COM29", 115200, bytesize=8, parity='N', stopbits=1, timeout=5)

ser_hold.close()
ser_hold.open()
time.sleep(1)
ser_hold.rts=1
time.sleep(1)
ser_hold.rts=0
time.sleep(1)

os.system('D:/Jenkins/script/jflashdown.bat')

time.sleep(1)
#ser_hold.close()

#ser_hold.rts=1
#time.sleep(20)
#ser_hold.rts=0


