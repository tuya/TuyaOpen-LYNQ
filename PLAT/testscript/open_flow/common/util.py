import time
import logging
from logging import handlers


class Util:
    def __init__(self) -> None:
        self.logger = None

    def get_logger(self, log_file_path: str = "log.txt"):
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
                filename=log_file_path, mode="a", maxBytes=1024 * 1024 * 512, backupCount=10, encoding="utf-8")
            fh.setFormatter(format_str)
            log.addHandler(sh)
            # log.addHandler(th)
            log.addHandler(fh)
            self.logger = log

        return self.logger

    def exec_case(self, module, ser) -> bool:
        def exec_case_map(case_map) -> bool:
            exec_case_res = True
            for k in case_map.keys():
                case_info = case_map[k]
                if callable(case_info.get("cmd")):
                    case_info.get("cmd")(ser, self.logger)
                else:
                    ser.write(case_info.get("cmd"))
                    self.logger.debug("发->:{}".format(case_info.get("cmd")))
                    if case_info.get("wait"):
                        time.sleep(case_info.get("wait"))
                    else:
                        time.sleep(0.1)
                    res = ser.read(ser.in_waiting).decode("utf-8")
                    self.logger.debug("收<-:{}".format(res))
                if "FAIL" in res:
                    self.logger.error("exec case {} FAIL".format(k))
                    continue
                if callable(case_info.get("match")):
                    case_info.get("match")(res)
                elif case_info.get("match") is None:
                    pass
                else:
                    if case_info.get("re"):
                        pass
                    else:
                        if case_info.get("match") in res:
                            self.logger.debug("exec case {} PASS".format(k))
                        else:
                            self.logger.error("exec case {} FAIL".format(k))
                            exec_case_res = False
            return exec_case_res

        def exec_min_case() -> bool:
            return True

        def exec_flashex_case() -> bool:
            from subsys_test_case import flashex_case
            case_map = flashex_case.case_map
            return exec_case_map(case_map)

        def exec_jsondb_case() -> bool:
            from subsys_test_case import jsondb_case
            case_map = jsondb_case.case_map
            return exec_case_map(case_map)

        def exec_socket_case() -> bool:
            from subsys_test_case import socket_case
            case_map = socket_case.case_map
            return exec_case_map(case_map)

        def exec_http_case() -> bool:
            from subsys_test_case import http_case
            case_map = http_case.case_map
            return exec_case_map(case_map)

        def exec_mobile_case() -> bool:
            from subsys_test_case import mobile_case
            case_map = mobile_case.case_map
            return exec_case_map(case_map)

        def exec_audio_case() -> bool:
            from subsys_test_case import audio_case
            case_map = audio_case.case_map
            return exec_case_map(case_map)

        def exec_systime_case() -> bool:
            from subsys_test_case import systime_case
            case_map = systime_case.case_map
            return exec_case_map(case_map)

        def exec_gpio_case() -> bool:
            from subsys_test_case import gpio_case
            case_map = gpio_case.case_map
            return exec_case_map(case_map)

        module_map = {"min": exec_min_case, "flashex": exec_flashex_case,
                      "jsondb": exec_jsondb_case, "socket": exec_socket_case, "http": exec_http_case, "mobile": exec_mobile_case, "audio": exec_audio_case, "systime": exec_systime_case, "gpio": exec_gpio_case}
        self.logger.debug("exec {} cases".format(module))
        return module_map.get(module)()
