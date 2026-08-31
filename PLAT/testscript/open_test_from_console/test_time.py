import allure
import pytest
import serial
import os
import sys
import common.util as util
import serial.serialutil
import time


# minus_volume


class TestTime:

    def setup_class(self):
        self.test_util = util.Util()
        self.logger = self.test_util.get_logger()
        self.logger.debug("Find test port")
        try:
            self.ser = self.test_util.find_console_port()
            # self.logger.debug(self.ser)
            if not self.ser:
                pytest.exit("Can not find test port")
        except serial.serialutil.SerialException as se:
            self.logger.error("Can not open test port")
            self.logger.error(se)
            pytest.exit("Can not open test port")

    def teardown_class(self):
        self.ser.close()

    @allure.title('test sync')
    def test_sync(self):
        self.test_util.exec_case("time", "sync")

    @allure.title('test get_gmtime_from_unix_time')
    def test_get_gmtime_from_unix_time(self):
        self.test_util.exec_case("time", "get_gmtime_from_unix_time")

    @allure.title('test get_local_time_from_unix_time')
    def test_get_local_time_from_unix_time(self):
        self.test_util.exec_case("time", "get_local_time_from_unix_time")

    @allure.title('test get_diff_time_between_unix_time')
    def test_get_diff_time_between_unix_time(self):
        self.test_util.exec_case("time", "get_diff_time_between_unix_time")


# if __name__ == '__main__':
#     pytest.main(['-s', 'main.py',
#                 '--clean-alluredir', '--alluredir=allure-results'])
    # os.system(r"D:\allure-2.27.0\bin\allure generate -c -o allure-report")
