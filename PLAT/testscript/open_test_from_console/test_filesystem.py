import allure
import pytest
import serial
import os
import sys
import common.util as util
import serial.serialutil
import time


# ls cat catx cd send get dbgswi echo rm


class TestFileSystem:

    def setup_class(self):
        self.test_util = util.Util()
        self.logger = self.test_util.get_logger()
        self.logger.debug("Find test port")
        try:
            self.ser = self.test_util.find_console_port()
            self.logger.debug(self.ser)
            if not self.ser:
                pytest.exit("Can not find test port")
        except serial.serialutil.SerialException as se:
            self.logger.error("Can not open test port")
            pytest.exit("Can not open test port")

    def teardown_class(self):
        self.ser.close()

    @allure.title('test_ls')
    def test_ls(self):
        self.test_util.exec_case("filesystem", "ls")

    @allure.title('test_cat')
    def test_cat(self):
        self.test_util.exec_case("filesystem", "cat")

    @allure.title('test_catx')
    def test_catx(self):
        self.test_util.exec_case("filesystem", "catx")

    @allure.title('test_cdD')
    def test_cdD(self):
        self.test_util.exec_case("filesystem", "cdD")

    @allure.title('test_cdC')
    def test_cdC(self):
        self.test_util.exec_case("filesystem", "cdC")

    @allure.title('test_echo_new')
    def test_echo_new(self):
        self.test_util.exec_case("filesystem", "echo_new")

    @allure.title('test_echo_old')
    def test_echo_old(self):
        self.test_util.exec_case("filesystem", "echo_old")

    @allure.title('test_rm')
    def test_rm(self):
        self.test_util.exec_case("filesystem", "rm")


# if __name__ == '__main__':
#     pytest.main(['-s', 'main.py',
#                 '--clean-alluredir', '--alluredir=allure-results'])
    # os.system(r"D:\allure-2.27.0\bin\allure generate -c -o allure-report")
