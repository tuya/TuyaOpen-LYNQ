import allure
import pytest
import serial
import os
import sys
import common.util as util
import serial.serialutil
import time


# minus_volume


class TestPhone:

    def setup_class(self):
        self.test_util = util.Util()
        self.logger = self.test_util.get_logger()
        self.logger.debug("Find test port")
        try:
            self.ser = self.test_util.setup_port("COM16", 115200)
            self.ser.rts = True
            time.sleep(1)
            self.ser.rts = False
            time.sleep(30)
            # self.ser = self.test_util.find_console_port()
            # self.logger.debug(self.ser)
            if not self.ser:
                pytest.exit("Can not find test port")
        except serial.serialutil.SerialException as se:
            self.logger.error("Can not open test port")
            self.logger.error(se)
            pytest.exit("Can not open test port")

    def teardown_class(self):
        self.ser.close()

    @allure.title('test_monkey')
    def test_monkey(self):
        self.test_util.exec_case("phone", "monkey")


# if __name__ == '__main__':
#     pytest.main(['-s', 'main.py',
#                 '--clean-alluredir', '--alluredir=allure-results'])
    # os.system(r"D:\allure-2.27.0\bin\allure generate -c -o allure-report")
