import allure
import pytest
import serial
import os
import sys
import common.util as util
import serial.serialutil
import time


# create delete create_all delete_all write read write_all interrupt


class TestGpio:

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

    @allure.title('test create')
    def test_create(self):
        self.test_util.exec_case("gpio", "create")

    @allure.title('test delete')
    def test_delete(self):
        self.test_util.exec_case("gpio", "delete")

    @allure.title('test create_all')
    def test_create_all(self):
        self.test_util.exec_case("gpio", "delete_all")

    @allure.title('test delete_all')
    def test_delete_all(self):
        self.test_util.exec_case("gpio", "delete_all")

    @allure.title('test write')
    def test_write(self):
        self.test_util.exec_case("gpio", "write")

    @allure.title('test read')
    def test_read(self):
        self.test_util.exec_case("gpio", "read")

    @allure.title('test write_all')
    def test_write_all(self):
        self.test_util.exec_case("gpio", "write_all")


# if __name__ == '__main__':
#     pytest.main(['-s', 'main.py',
#                 '--clean-alluredir', '--alluredir=allure-results'])
    # os.system(r"D:\allure-2.27.0\bin\allure generate -c -o allure-report")
