# coding:utf-8
import os
import time
import serial

ser_hold = serial.Serial("COM29", 9600, bytesize=8, parity='N', stopbits=1, rtscts =1,timeout=5)
#ser_reset = serial.Serial("COM32", 9600, bytesize=8, parity='N', stopbits=1,rtscts =0, timeout=5)

#ser_reset.close()
ser_hold.close()
ser_hold.rts=1
#ser_reset.rts=1
ser_hold.dtr=1
ser_hold.open()
#ser_reset.open()
time.sleep(2)
#ser_reset.close()
ser_hold.close()
#ser_reset.rts=0
ser_hold.dtr=0
ser_hold.rts=1
time.sleep(5)

os.system('D:/Jenkins/script/jflashdown.bat')
time.sleep(5)
#os.system('burn_RfCaliTB.bat')

#ser_reset.close()
ser_hold.close()
ser_hold.rts=0
#ser_reset.rts=1
ser_hold.dtr=1
ser_hold.open()
#ser_reset.open()
time.sleep(2)
#ser_reset.close()
ser_hold.close()
#ser_reset.rts=0
ser_hold.dtr=0
ser_hold.rts=0
time.sleep(25)