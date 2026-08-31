# EC718 Console


## mshell

通过串口指令扩展功能，实现测试和数据采集

* 实现通过LVGL输出到LCD屏幕

## python

* 将python组件分离为单独的subsys便于迁移，subsys只是用于建立快速任务，属于APP，而组件才是核心依赖文件

适配过程是通过参考其他平台的package结构，驱动大致包括open/close/ioctl_config/enable/disable/write/read等封装函数

* open/close用于初始化相关数据堆栈
* enable/disable用于使能配置，init和deinit
* config用于配置相关参数

再根据不同平台和不同SDK，适配出对应的底层C调用，相关的API头文件可以放在pikascript-api路径下

```
time = ec7xx.Time()
```

```
pin = ec7xx.GPIO()
pin.setId(2)
pin.getId()
pin.setMode('out')
pin.setPull('up')
pin.getMode()

// pin.setPin(1)
// pin.getPin()
// pin.enable()
// pin.high()
// pin.low()
// pin.read()
```

```
uart = ec7xx.UART()
uart.setId(2)
uart.setPinTX(23)
uart.setBaudRate(115200)
uart.enable()
uart.readBytes(5)
uart.writeBytes(b'\xff\xfe\xfd\xfc', 4)

readBuff = uart.read(2)
print('read 2 char:')
print(readBuff)
```

 
```
def uart():
    """
    uart 例程
    通过 uart1 进行自发自收实验，你可以将 P23 和 P24 用杜邦线连接起来
    """
    uart1 = ec7xx.UART()
    uart1.setId(1)
    uart1.setPinTX(23)
    uart1.setPinRX(24)
    uart1.setBaudRate(115200)
    uart1.enable()
    print('Waiting for data from uart1')
    for i in range(10):
        uart1.write('Hello!')
        data = uart1.read(256)
        time.sleep(0.1)
        if data:
            print('Received: %r' % data)
        time.sleep(0.1)

def _uart_receive_callback(signal):
    """
    UART 回调例程的回调函数
    """
    global uart_rx_cnt
    data = uart1.read(256)
    print('Received:', data, '(%d/%d)' % (uart_rx_cnt + 1, 3))
    uart_rx_cnt += 1

def uart_callback():
    """
    uart 回调例程
    通过 uart1 与电脑通信, 使用回调方式接收数据
    电脑需要连接到 uart1 的 TX 和 RX 引脚，波特率为 115200
    例程开始后，你需要在电脑上输出三行数据
    TX引脚是 P23
    RX引脚是 P24
    """
    global uart1
    global uart_rx_cnt
    uart_rx_cnt = 0
    uart1 = machine.UART()
    uart1.setId(1)
    uart1.setPinTX(23)
    uart1.setPinRX(24)
    uart1.setBaudRate(115200)
    uart1.enable()
    uart1.setCallBack(_uart_receive_callback, uart1.SIGNAL_RX)
    print('Waiting for data from uart1 (0/3)')
    while True:
        if uart_rx_cnt == 3:
            break
        time.sleep(0.1)
    uart1.disable()

```

* 008b691c <Driver_USART0>
* 008b695c <Driver_USART1>
* 008b699c <Driver_USART2>
* 008b69dc <Driver_USART3>


### UART2

* RX GPIO12 - 27 - PAD_MUX_ALT3 - UART1-RTS
* TX GPIO13 - 28 - PAD_MUX_ALT3 - UART1-CTS

### UART3

* RX GPIO14 - 29 - PAD_MUX_ALT3 - SDA
* TX GPIO15 - 30 - PAD_MUX_ALT3 - SCL


### SPI

```
>>>spi.enable()
>>>Error: abstract method `PikaStdDevice_SPI_platformEnable()` need override.
 -> RUN spi.enable              (#1)

```

