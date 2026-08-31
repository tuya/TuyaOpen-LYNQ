import random
import time
import sys


def monkey(ser, logger, counts=1000000):
    number_keys = ["0",
                   "1", "2", "3", "4", "5", "6", "7", "8", "9", "*", "#"]
    operation_keys = ["a", "b", "m", "n", "C", "U", "D", "L", "R"]
    # keys = ["a", "b", "m", "n", "C", "U", "D", "L", "R", "0",
    #         "1", "2", "3", "4", "5", "6", "7", "8", "9", "*", "#"]

    # keys = ["m", "m", "m", "n", "U", "D", "L", "R"]
    keys = number_keys + ["U", "D", "L", "R"]
    # keys = ["m", "m", "m", "n", "b", "U", "D", "L", "R"]
    # keys = number_keys
    keys = number_keys + ["m", "m", "m", "n", "b", "U", "D", "L", "R"]

    key_systest_cmd_template = '{{"name":"@key","param1":0,"param2":0,"param3":"{}"}}\r\n'
    for i in range(3):
        ser.write(b'\r\n')
        time.sleep(0.1)
        res = ser.read(ser.in_waiting).decode("utf-8", errors="ignore")
        logger.debug("收<-:{}".format(res))
    time.sleep(0.1)
    ser.write(b'dbgswi\r\n')
    time.sleep(0.1)
    for i in range(1, counts+1):
        random_key_index = random.randint(0, len(keys)-1)
        key = keys[random_key_index]
        logger.debug("monkey push key ({}/{}):{}".format(i, counts, key))
        monkey_push_cmd = key_systest_cmd_template.format(key).encode()
        # logger.debug("发->:{}".format(monkey_push_cmd))
        ser.write(monkey_push_cmd)
        time.sleep(0.3)
        res = ser.read(ser.in_waiting).decode("utf-8", errors="ignore")
        logger.debug("收<-:{}".format(res))


case_map = {"monkey": {"cmd": monkey, "match": None, "re": False}}
