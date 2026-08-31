import serial.tools.list_ports as list_ports
import serial
import time
import logging
from logging import handlers


class Util:
    def __init__(self) -> None:
        self.ser = None
        self.logger = None

    def setup_port(self, port_num, baud):
        if not self.ser:
            ser = serial.Serial(port_num, baud)
            self.ser = ser
        return self.ser

    def find_console_port(self):
        if not self.ser:
            ports = list_ports.comports()
            # print("ports = {}".format(ports))
            for port in ports:
                ser = serial.Serial(port.device, 115200)
                ser.write(b'\r\n')
                time.sleep(0.1)
                res = ser.read(ser.in_waiting)
                if b'C:\\' in res:
                    self.ser = ser
                else:
                    ser.close()
        return self.ser

    def get_logger(self):
        if not self.logger:
            LOG_FORMAT = "%(asctime)s - %(levelname)s - %(filename)s:%(lineno)d - %(message)s"
            DATE_FORMAT = "%Y/%m/%d %H:%M:%S"

            format_str = logging.Formatter(fmt=LOG_FORMAT, datefmt=DATE_FORMAT)

            log = logging.getLogger("logger")
            log.setLevel(logging.DEBUG)
            sh = logging.StreamHandler()
            sh.setFormatter(format_str)

            # th = handlers.TimedRotatingFileHandler(
            #     filename="log.txt", when="D", encoding='utf-8')
            # th.setFormatter(format_str)

            fh = handlers.RotatingFileHandler(
                filename="log.txt", mode="a", maxBytes=1024 * 1024 * 512, backupCount=10, encoding="utf-8")
            fh.setFormatter(format_str)
            log.addHandler(sh)
            # log.addHandler(th)
            log.addHandler(fh)
            self.logger = log

        return self.logger

    def exec_case(self, module, case_name):
        def exec_case_info(case_info):
            if callable(case_info.get("cmd")):
                case_info.get("cmd")(self.ser, self.logger)
            else:
                self.ser.write(case_info.get("cmd"))
                self.logger.debug("发->:{}".format(case_info.get("cmd")))
                time.sleep(0.1)
                res = self.ser.read(self.ser.in_waiting).decode("utf-8")
                self.logger.debug("收<-:{}".format(res))
            if callable(case_info.get("match")):
                case_info.get("match")(res)
            elif case_info.get("match") is None:
                pass
            else:
                if case_info.get("re"):
                    pass
                else:
                    assert case_info.get("match") in res

        def exec_filesystem_case(case_name):
            from case import filesystem_case
            case_info = filesystem_case.case_map.get(case_name)
            exec_case_info(case_info)

        def exec_speaker_case(case_name):
            from case import speaker_case
            case_info = speaker_case.case_map.get(case_name)
            exec_case_info(case_info)

        def exec_phone_case(case_name):
            from case import phone_case
            case_info = phone_case.case_map.get(case_name)
            exec_case_info(case_info)

        def exec_time_case(case_name):
            from case import time_case
            case_info = time_case.case_map.get(case_name)
            exec_case_info(case_info)

        def exec_gpio_case(case_name):
            from case import gpio_case
            case_info = gpio_case.case_map.get(case_name)
            exec_case_info(case_info)

        module_map = {"filesystem": exec_filesystem_case,
                      "speaker": exec_speaker_case, "phone": exec_phone_case, "time": exec_time_case, "gpio": exec_gpio_case}
        self.logger.debug("exec case : {} - {}".format(module, case_name))
        module_map.get(module)(case_name)
