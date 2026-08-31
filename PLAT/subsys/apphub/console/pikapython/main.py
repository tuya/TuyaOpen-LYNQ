import PikaStdLib
import PikaStdData
import PikaStdTask
import ec7xx

# print('Pika Main')

# time = ec7xx.Time()

led = ec7xx.GPIO()
led.setPin(2)
led.setMode('out')
led.getMode()
led.enable()
led.high()
led.read()
led.low()
led.read()

uar1 = ec7xx.UART()
uar1.setId(1)
uar1.setBaudRate(115200)
uar1.enable()
uar1.writeBytes(b'\xfa\xfe\xfd\xfc', 4)
uar1.write("test\r\n")
uar1.readBytes(5)

uar2 = ec7xx.UART()
uar2.setId(2)
uar2.setBaudRate(115200)
uar2.enable()
uar2.write("test\r\n")
uar2.writeBytes(b'\xfa\xfe\xfd\xfc', 4)

# uar2.readBytes(5)
# readBuff = uar2.read(2)
# print('read 2 char:')
# print(readBuff)

uar3 = ec7xx.UART()
uar3.setId(3)
uar3.setBaudRate(115200)
uar3.enable()
uar3.write("test\r\n")
uar3.writeBytes(b'\xff\xfe\xfd\xfc', 4)

spi = ec7xx.SPI()
spi.setId(2)
spi.enable()

i2c = ec7xx.IIC()
i2c.enable()

adc = ec7xx.ADC()
adc.enable()

pwm = ec7xx.PWM()
pwm.enable()

task = ec7xx.Task()

# print('mem used max:')
# mem.max()

# def print_task():
#     print('test PikaStdTask')
#     mem.now()

# def led_task():
#     if led.read():
#         led.low()
#     else:
#         led.high()


# task.call_period_ms(print_task, 1000)
# task.call_period_ms(led_task, 500)
# task.run_forever()