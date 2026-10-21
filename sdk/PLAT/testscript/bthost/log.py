import serial
import time

# 设置串口参数
port = 'COM64'
ser = serial.Serial()
ser.baudrate = 115200
ser.port = port
ser.timeout = 1
ser.open()

# 打开文件准备写入数据
filename = port + '-EC-BLE.csv'
f = open(filename, 'a')
count = 0
# cmd = 'AT+CEREG?\n'
# ser.write(cmd.encode())
# 循环读取串口数据并写入文件
while True:
    data = ser.readline().decode().strip()
    
    # if 'ECRDY' in data:
    #     count += 1 and count > 0
    if len(data) > 3 :
        # 添加时间戳
        timestamp = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime())
        line = '{},{}\n'.format(timestamp, data)
        f.write(line)
        print(line.strip())

    count += 1
    # if count > 10 :
    #     ser.write(cmd.encode())
    #     print(cmd)
    #     count = 0
# 关闭文件和串口连接
f.close()
ser.close()