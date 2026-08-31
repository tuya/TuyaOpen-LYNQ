from PikaObj import *
import PikaStdDevice
import PikaStdTask


class GPIO(PikaStdDevice.GPIO):
    ...

class UART(PikaStdDevice.UART):
    ...

class SPI(PikaStdDevice.SPI):
    ...

class IIC(PikaStdDevice.IIC):
    ...

class ADC(PikaStdDevice.ADC):
    ...


class PWM(PikaStdDevice.PWM):


class Task(PikaStdTask.Task):
    ...
