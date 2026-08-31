print('test Pika uart')

def _uart_receive_callback(signal):
    global uart_rx_cnt
    data = uart2.read(256)
    print('Received:', data, '(%d/%d)' % (uart_rx_cnt + 1, 3))
    uart_rx_cnt += 1

def uart_callback():
    global uart2
    global uart_rx_cnt
    uart_rx_cnt = 0
    uart2 = machine.UART()
    uart2.setId(1)
    uart2.setPinTX(23)
    uart2.setPinRX(24)
    uart2.setBaudRate(115200)
    uart2.enable()
    uart2.setCallBack(_uart_receive_callback, uart2.SIGNAL_RX)
    print('Waiting for data from uart2 (0/3)')
    while True:
        if uart_rx_cnt == 3:
            break
        time.sleep(0.1)
    uart2.disable()

uart2 = ec7xx.UART()
uart2.setId(2)
uart2.setPinTX(23)
uart2.setPinRX(24)
uart2.setBaudRate(115200)
uart2.enable()
print('Waiting for data from uart2')
for i in range(10):
    uart2.write('Hello!')
    data = uart2.read(256)
    time.sleep(0.1)
    if data:
        print('Received: %r' % data)
    time.sleep(0.1)