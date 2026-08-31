import allure
import pytest
import serial
import os
import sys
import common.util as util
import serial.serialutil
import time


# minus_volume plus_volume min_volume max_volume short_menu long_menu


class TestSpeaker:

    def setup_class(self):
        self.test_util = util.Util()
        self.logger = self.test_util.get_logger()
        self.logger.debug("Find test port")
        try:
            self.ser = self.test_util.setup_port("COM8", 9600)
            # self.logger.debug(self.ser)
            if not self.ser:
                pytest.exit("Can not find test port")
        except serial.serialutil.SerialException as se:
            self.logger.error("Can not open test port")
            self.logger.error(se)
            pytest.exit("Can not open test port")

    def teardown_class(self):
        self.ser.close()

    @allure.title('test_minus_volume')
    def test_minus_volume(self):
        self.test_util.exec_case("speaker", "minus_volume")

    # @allure.title('test_plus_volume')
    # def test_plus_volume(self):
    #     self.test_util.exec_case("speaker", "plus_volume")

    @allure.title('test_min_volume')
    def test_min_volume(self):
        self.test_util.exec_case("speaker", "min_volume")

    @allure.title('test_max_volume')
    def test_max_volume(self):
        self.test_util.exec_case("speaker", "max_volume")

    @allure.title('test_short_menu')
    def test_short_menu(self):
        self.test_util.exec_case("speaker", "short_menu")

    @allure.title('test_long_menu')
    def test_long_menu(self):
        self.test_util.exec_case("speaker", "long_menu")


# if __name__ == '__main__':
#     pytest.main(['-s', 'main.py',
#                 '--clean-alluredir', '--alluredir=allure-results'])
    # os.system(r"D:\allure-2.27.0\bin\allure generate -c -o allure-report")
