import os
import sys
import re
import time
import datetime
import json
import subprocess
import glob
import shutil
import multiprocessing
import requests
import prettytable
import serial
import serial.tools.list_ports as list_ports
import serial.serialutil


from common import util
from logging import Logger


class CoreTestCli(object):
    def __init__(self, control_com, download_com) -> None:
        self.core_test_dir = "C:\\core_test"
        self.repo_url = "ssh://git@10.10.60.68:2222/open_eigencomm/ec718_716_sdk_main.git"
        self.repo_url_template = "ssh://git@10.10.60.68:2222/open_eigencomm/ec718_716_sdk_{}.git"

        self.target_list_716e = ["speaker_7111", "speaker_pwm"]

        self.target_list_718p = ["speaker_8311", "speaker_7149", "speaker_pwm", "watch", "phone", "ref_app", "ref_min", "alipay_demo", "audio_demo", "camera_demo", "fs_demo", "socket_demo", "slp_demo", "time_demo", "volte_demo", "http_demo", "jpeg_demo", "lcd_demo_lvgl", "lcd_demo_rgb", "driver_example_USART_EX", "driver_example_SPI_EX", "driver_example_I2C_EX", "driver_example_GPIO_EX", "driver_example_TIMER_EX",
                                 "driver_example_WDT_EX", "driver_example_DMA_EX", "driver_example_ADC_EX", "driver_example_KPC_EX", "driver_example_AUDIO_EX", "driver_example_TLS_EX", "driver_example_CAMERA_EX", "driver_example_LCD_EX", "driver_example_ONEW_EX", "subsys_test_min", "subsys_test_flashex", "subsys_test_jsondb", "subsys_test_socket", "subsys_test_http", "subsys_test_mobile", "subsys_test_audio", "subsys_test_systime", "subsys_test_gpio"]

        self.target_list_718u = ["watch", "phone", "ref_app", "ref_min", "alipay_demo", "audio_demo", "camera_demo", "fs_demo", "socket_demo", "slp_demo", "time_demo", "volte_demo", "http_demo", "jpeg_demo", "lcd_demo_lvgl", "lcd_demo_rgb", "driver_example_USART_EX", "driver_example_SPI_EX", "driver_example_I2C_EX", "driver_example_GPIO_EX", "driver_example_TIMER_EX",
                                 "driver_example_WDT_EX", "driver_example_DMA_EX", "driver_example_ADC_EX", "driver_example_KPC_EX", "driver_example_AUDIO_EX", "driver_example_TLS_EX", "driver_example_CAMERA_EX", "driver_example_LCD_EX", "driver_example_ONEW_EX", "subsys_test_min", "subsys_test_flashex", "subsys_test_jsondb", "subsys_test_socket", "subsys_test_http", "subsys_test_mobile", "subsys_test_audio", "subsys_test_systime", "subsys_test_gpio"]

        self.target_list_718pm = ["speaker_8311", "speaker_7149", "speaker_pwm", "watch", "phone", "ref_app", "ref_min", "alipay_demo", "audio_demo", "camera_demo", "fs_demo", "socket_demo", "slp_demo", "time_demo", "volte_demo", "http_demo", "jpeg_demo", "lcd_demo_lvgl", "lcd_demo_rgb", "driver_example_USART_EX", "driver_example_SPI_EX", "driver_example_I2C_EX", "driver_example_GPIO_EX", "driver_example_TIMER_EX",
                                  "driver_example_WDT_EX", "driver_example_DMA_EX", "driver_example_ADC_EX", "driver_example_KPC_EX", "driver_example_AUDIO_EX", "driver_example_TLS_EX", "driver_example_CAMERA_EX", "driver_example_LCD_EX", "driver_example_ONEW_EX", "subsys_test_min", "subsys_test_flashex", "subsys_test_jsondb", "subsys_test_socket", "subsys_test_http", "subsys_test_mobile", "subsys_test_audio", "subsys_test_systime", "subsys_test_gpio"]

        self.target_list_718um = ["watch", "phone", "ref_app", "ref_min", "alipay_demo", "audio_demo", "camera_demo", "fs_demo", "socket_demo", "slp_demo", "time_demo", "volte_demo", "http_demo", "jpeg_demo", "lcd_demo_lvgl", "lcd_demo_rgb", "driver_example_USART_EX", "driver_example_SPI_EX", "driver_example_I2C_EX", "driver_example_GPIO_EX", "driver_example_TIMER_EX",
                                  "driver_example_WDT_EX", "driver_example_DMA_EX", "driver_example_ADC_EX", "driver_example_KPC_EX", "driver_example_AUDIO_EX", "driver_example_TLS_EX", "driver_example_CAMERA_EX", "driver_example_LCD_EX", "driver_example_ONEW_EX", "subsys_test_min", "subsys_test_flashex", "subsys_test_jsondb", "subsys_test_socket", "subsys_test_http", "subsys_test_mobile", "subsys_test_audio", "subsys_test_systime", "subsys_test_gpio"]
        self.commit_info = "null"
        self.commit_id = "null"
        self.repo = "main"
        self.subsys_test_enable = False
        self.ec716e_enable = False
        self.ec718p_enable = False
        self.ec718pm_enable = False
        self.ec718u_enable = False
        self.ec718um_enable = False
        self.repo_dir_name = "ec718_716_sdk_{}".format(self.repo)
        self.log_file = None
        self.return_code = 1
        self.download_return_code = 1
        self.flash_tools_dir = "C:\\FlashTools\\FlashToolCLI_V4.1.11p01_240401"
        self.binpkg_dir = os.path.join(self.core_test_dir, "binpkg")
        self.control_com = control_com
        self.download_com = download_com
        self.test_util = util.Util()
        self.logger = self.test_util.get_logger("main_log.txt")
        self.zip_file_name = ""

    def replace_text_in_file(self, file_path: str, old_text: str, new_text: str):
        with open(file_path, 'r') as file:
            file_content = file.read()

        updated_content = file_content.replace(old_text, new_text)

        with open(file_path, 'w') as file:
            file.write(updated_content)

    def find_size_files(self) -> list:
        size_files = glob.glob(os.path.join(
            self.core_test_dir, self.repo_dir_name, "PLAT/gccout", '**', '*.size'), recursive=True)

        res = []

        for file_path in size_files:
            res.append(os.path.abspath(file_path))

        return res

    def find_binpkg_files(self):
        binpkg_files = glob.glob(os.path.join(
            self.core_test_dir, self.repo_dir_name, "PLAT/gccout", '**', '*.binpkg'), recursive=True)

        res = []

        for file_path in binpkg_files:
            res.append(os.path.abspath(file_path))

        return res

    def find_comdb_files(self):
        comdb_files = glob.glob(os.path.join(
            self.core_test_dir, self.repo_dir_name, "PLAT/gccout", '**', 'comdb.txt'), recursive=True)

        res = []

        for file_path in comdb_files:
            res.append(os.path.abspath(file_path))

        return res

    def find_elf_files(self):
        elf_files = glob.glob(os.path.join(
            self.core_test_dir, self.repo_dir_name, "PLAT/gccout", '**', '*.elf'), recursive=True)

        res = []

        for file_path in elf_files:
            res.append(os.path.abspath(file_path))

        return res

    def find_collect_files(self):
        collect_files = []
        collect_files.extend(self.find_binpkg_files())
        collect_files.extend(self.find_comdb_files())
        collect_files.extend(self.find_elf_files())
        return collect_files

    def collect_build_out(self, chip: str, target: str, logger: Logger) -> str:
        if self.return_code == 0:
            os.makedirs(os.path.join(self.core_test_dir,
                        self.commit_id), exist_ok=True)
            collect_files = [
                file for file in self.find_collect_files() if os.path.basename(file) != "ap_bootloader.elf"]
            files_to_compress = ' '.join(collect_files)
            zip_file_name = "{chip}_{build_target}_{time}.zip".format(
                chip=chip, build_target=target, time=datetime.datetime.now().strftime("%Y_%m_%d_%H_%M_%S"))
            zip_cmd = "cd {core_test_dir}/{repo_dir_name}/PLAT && 7z.exe a -tzip {zip_file_name} {files_to_compress} && move {zip_file_name} ../../{commit_id}".format(
                core_test_dir=self.core_test_dir, repo_dir_name=self.repo_dir_name, commit_id=self.commit_id, files_to_compress=files_to_compress, zip_file_name=zip_file_name)
            # logger.debug("zip_cmd = {}".format(zip_cmd))

            p = subprocess.run(zip_cmd,
                               shell=True, stdout=self.log_file, stderr=self.log_file)

            self.return_code = p.returncode
            logger.debug("zip_return_code = {}".format(self.return_code))
            return zip_file_name

    def download(self, chip: str, target: str, logger: Logger) -> bool:
        # download
        # FlashToolCLI.exe --cfgfile config_pkg_product_usb.ini pkg2img
        # FlashToolCLI.exe --port="COM5" --cfgfile config_pkg_product_usb.ini probe
        # FlashToolCLI.exe --skipconnect 1 --port="COM5" --cfgfile config_pkg_product_usb.ini runtmcfg
        # FlashToolCLI.exe --skipconnect 1 --cfgfile config_pkg_product_usb.ini --verbose 1 --port="COM5" burnbatch --imglist bootloader cp_system system
        # FlashToolCLI.exe --skipconnect 1 --cfgfile config_pkg_product_usb.ini --verbose 1 --port="COM5" burn_pkgflxs
        # FlashToolCLI.exe --skipconnect 1 --cfgfile config_pkg_product_usb.ini --port COM5 sysreset
        if self.return_code == 0:

            binpkg_files = self.find_binpkg_files()
            logger.debug("Find binpkg files: {}".format(binpkg_files))
            target_path = os.path.join(self.binpkg_dir, "res.binpkg")
            os.makedirs(os.path.join(self.binpkg_dir), exist_ok=True)
            for file in binpkg_files:
                shutil.copy(file, target_path)

            p = subprocess.run("cd {} && FlashToolCLI.exe --cfgfile config_pkg_product_usb.ini pkg2img".format(self.flash_tools_dir),
                               shell=True, text=True, stdout=self.log_file, stderr=self.log_file, encoding="utf-8", errors="ignore")

            self.download_return_code = p.returncode
            logger.debug("pkg2img return_code = {}".format(
                self.download_return_code))
            if self.download_return_code != 0:
                return False

            open_control_com_res = False

            for i in range(10):
                try:
                    ser = serial.Serial(self.control_com, 115200)
                    open_control_com_res = True
                    break
                except serial.serialutil.SerialException:
                    logger.debug("open control com fail retry...")
                    time.sleep(1)

            go_to_download_mode_res = False

            if open_control_com_res == False:
                return False

            for i in range(10):
                ports = list_ports.comports()
                # logger.debug("ports = {}".format(ports))
                ports_names = [port.name for port in ports]

                logger.debug("ports = {}".format(ports_names))

                if self.download_com in ports_names:
                    logger.debug("go to download mode ok")
                    go_to_download_mode_res = True
                    break

                logger.debug("go to download mode start")
                ser.dtr = True
                time.sleep(1)
                ser.rts = True
                time.sleep(1)
                ser.rts = False
                time.sleep(1)
                ser.dtr = False

            if go_to_download_mode_res == False:
                ser.close()
                return False

            if self.download_return_code == 0:
                p = subprocess.run('cd {} && FlashToolCLI.exe --port="{}" --cfgfile config_pkg_product_usb.ini probe'.format(self.flash_tools_dir, self.download_com),
                                   shell=True, text=True, stdout=self.log_file, stderr=self.log_file, encoding="utf-8", errors="ignore")

                self.download_return_code = p.returncode
                logger.debug("probe return_code = {}".format(
                    self.download_return_code))
            else:
                ser.close()
                return False

            if self.download_return_code == 0:
                p = subprocess.run('cd {} && FlashToolCLI.exe --skipconnect 1 --port="{}" --cfgfile config_pkg_product_usb.ini runtmcfg'.format(self.flash_tools_dir, self.download_com),
                                   shell=True, text=True, stdout=self.log_file, stderr=self.log_file, encoding="utf-8", errors="ignore")

                self.download_return_code = p.returncode
                logger.debug(
                    "runtmcfg return_code = {}".format(self.download_return_code))
            else:
                ser.close()
                return False

            if self.download_return_code == 0:
                p = subprocess.run('cd {} && FlashToolCLI.exe --skipconnect 1 --cfgfile config_pkg_product_usb.ini --verbose 1 --port="{}" burnbatch --imglist bootloader cp_system system'.format(self.flash_tools_dir, self.download_com),
                                   shell=True, text=True, stdout=self.log_file, stderr=self.log_file, encoding="utf-8", errors="ignore")

                self.download_return_code = p.returncode
                logger.debug(
                    "burnbatch return_code = {}".format(self.download_return_code))
            else:
                ser.close()
                return False

            if self.download_return_code == 0:
                p = subprocess.run('cd {} && FlashToolCLI.exe --skipconnect 1 --cfgfile config_pkg_product_usb.ini --verbose 1 --port="{}" burn_pkgflxs'.format(self.flash_tools_dir, self.download_com),
                                   shell=True, text=True, stdout=self.log_file, stderr=self.log_file, encoding="utf-8", errors="ignore")

                self.download_return_code = p.returncode
                logger.debug(
                    "burn_pkgflxs return_code = {}".format(self.download_return_code))
            else:
                ser.close()
                return False

            if self.download_return_code == 0:
                p = subprocess.run('cd {} && FlashToolCLI.exe --skipconnect 1 --cfgfile config_pkg_product_usb.ini --port {} sysreset'.format(self.flash_tools_dir, self.download_com),
                                   shell=True, text=True, stdout=self.log_file, stderr=self.log_file, encoding="utf-8", errors="ignore")

                self.download_return_code = p.returncode
                logger.debug("sysreset return_code = {}".format(
                    self.download_return_code))
            else:
                ser.close()
                return False

            if self.download_return_code != 0:
                ser.close()
                return False

            ser.close()
            return True

    def subsys_test(self, chip: str, target: str, logger: Logger):
        if self.download_return_code == 0:
            case_name = target.replace("subsys_test_", "")
            ser = serial.Serial(self.control_com, 115200)
            ser.dtr = False
            ser.rts = True
            time.sleep(1)
            ser.rts = False
            time.sleep(5)
            ser.write(b"help\r\n")
            time.sleep(1)
            logger.debug(ser.read(ser.in_waiting).decode(
                encoding="utf-8", errors="ignore"))
            ser.write(b"help\r\n")
            time.sleep(1)
            logger.debug(ser.read(ser.in_waiting).decode(
                encoding="utf-8", errors="ignore"))

            exec_case_res = self.test_util.exec_case(case_name, ser)
            ser.close()
            return exec_case_res

    def build_target(self, chip: str, target: str, logger: Logger):
        case_name = "default"
        if target.startswith("speaker_"):
            case_name = target.replace("speaker_", "")
            logger.debug("build speaker {} {}".format(chip, case_name))
            # build
            if chip == "718p":
                p = subprocess.run("cd {}/{}/PLAT && ec718p_openbuild_speaker_1h00.bat clall && ec718p_openbuild_speaker_1h00.bat {}".format(self.core_test_dir, self.repo_dir_name, case_name),
                                   shell=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, encoding="utf-8", errors="ignore")

            elif chip == "718pm":
                p = subprocess.run("cd {}/{}/PLAT && ec718pm_openbuild_speaker_1h00.bat clall && ec718pm_openbuild_speaker_1h00.bat {}".format(self.core_test_dir, self.repo_dir_name, case_name),
                                   shell=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, encoding="utf-8", errors="ignore")
            elif chip == "716e":
                p = subprocess.run("cd {}/{}/PLAT && ec716e_openbuild_speaker_1h00.bat clall && ec716e_openbuild_speaker_1h00.bat {}".format(self.core_test_dir, self.repo_dir_name, case_name),
                                   shell=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, encoding="utf-8", errors="ignore")

        elif target.startswith("driver_example_"):
            case_name = target.replace("driver_example_", "")
            logger.debug("build driver_example {} {}".format(chip, case_name))

            path = "{}/{}/PLAT/project/ec7xx_ref_1h00/ap/apps/driver_example/src/app.c".format(
                self.core_test_dir, self.repo_dir_name)
            self.replace_text_in_file(path, "static example_id_t exampleId = CAMERA_EX;",
                                      "static example_id_t exampleId = {};".format(case_name))

            # build
            if chip == "718p":
                p = subprocess.run(
                    "cd {}/{}/PLAT && ec718p_openbuild_ref_1h00.bat clall && ec718p_openbuild_ref_1h00.bat driver_example-merge".format(self.core_test_dir, self.repo_dir_name), shell=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, encoding="utf-8", errors="ignore")
            elif chip == "718pm":
                p = subprocess.run(
                    "cd {}/{}/PLAT && ec718pm_openbuild_ref_1h00.bat clall && ec718pm_openbuild_ref_1h00.bat driver_example-merge".format(self.core_test_dir, self.repo_dir_name), shell=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, encoding="utf-8", errors="ignore")
            elif chip == "718u":
                p = subprocess.run(
                    "cd {}/{}/PLAT && ec718u_openbuild_ref_1h00.bat clall && ec718u_openbuild_ref_1h00.bat driver_example-merge".format(self.core_test_dir, self.repo_dir_name), shell=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, encoding="utf-8", errors="ignore")
            elif chip == "718um":
                p = subprocess.run(
                    "cd {}/{}/PLAT && ec718um_openbuild_ref_1h00.bat clall && ec718um_openbuild_ref_1h00.bat driver_example-merge".format(self.core_test_dir, self.repo_dir_name), shell=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, encoding="utf-8", errors="ignore")

        elif target.startswith("lcd_demo"):
            case_name = target.replace("lcd_demo_", "")
            logger.debug("build lcd_demo {} {}".format(chip, case_name))

            if case_name == "rgb":
                path = "{}/{}/PLAT/project/ec7xx_ref_1h00/ap/apps/lcd_demo/GCC/Makefile".format(
                    self.core_test_dir, self.repo_dir_name)
                self.replace_text_in_file(path, "# CFLAGS += -DFEATURE_LCD_TEST_RGB",
                                          "CFLAGS += -DFEATURE_LCD_TEST_RGB")

            # build
            if chip == "718p":
                p = subprocess.run("cd {}/{}/PLAT && ec718p_openbuild_ref_1h00.bat clall && ec718p_openbuild_ref_1h00.bat lcd_demo".format(self.core_test_dir, self.repo_dir_name),
                                   shell=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, encoding="utf-8", errors="ignore")
            elif chip == "718pm":
                p = subprocess.run("cd {}/{}/PLAT && ec718pm_openbuild_ref_1h00.bat clall && ec718pm_openbuild_ref_1h00.bat lcd_demo".format(self.core_test_dir, self.repo_dir_name),
                                   shell=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, encoding="utf-8", errors="ignore")
            elif chip == "718u":
                p = subprocess.run("cd {}/{}/PLAT && ec718u_openbuild_ref_1h00.bat clall && ec718u_openbuild_ref_1h00.bat lcd_demo".format(self.core_test_dir, self.repo_dir_name),
                                   shell=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, encoding="utf-8", errors="ignore")
            elif chip == "718um":
                p = subprocess.run("cd {}/{}/PLAT && ec718um_openbuild_ref_1h00.bat clall && ec718um_openbuild_ref_1h00.bat lcd_demo".format(self.core_test_dir, self.repo_dir_name),
                                   shell=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, encoding="utf-8", errors="ignore")
        elif target.startswith("subsys_test"):
            case_name = target.replace("subsys_test_", "")
            logger.debug("build subsys_test {} {}".format(chip, case_name))
            path = "{}/{}/PLAT/project/ec7xx_ref_1h00/ap/apps/subsys_test/GCC/Makefile".format(
                self.core_test_dir, self.repo_dir_name)

            if case_name == "flashex":
                self.replace_text_in_file(path, "SUBSYS_STORAGE_TEST_ENABLE      = n",
                                          "SUBSYS_STORAGE_TEST_ENABLE      = y")
            elif case_name == "jsondb":
                self.replace_text_in_file(path, "SUBSYS_JSONDB_TEST_ENABLE       = n",
                                          "SUBSYS_JSONDB_TEST_ENABLE       = y")
            elif case_name == "socket":
                self.replace_text_in_file(path, "SUBSYS_SOCKET_TEST_ENABLE       = n",
                                          "SUBSYS_SOCKET_TEST_ENABLE       = y")
            elif case_name == "http":
                self.replace_text_in_file(path, "SUBSYS_HTTP_TEST_ENABLE         = n",
                                          "SUBSYS_HTTP_TEST_ENABLE         = y")
            elif case_name == "mobile":
                self.replace_text_in_file(path, "SUBSYS_MOBILE_TEST_ENABLE       = n",
                                          "SUBSYS_MOBILE_TEST_ENABLE       = y")
            elif case_name == "audio":
                self.replace_text_in_file(path, "SUBSYS_AUDIO_TEST_ENABLE        = n",
                                          "SUBSYS_AUDIO_TEST_ENABLE        = y")
            elif case_name == "systime":
                self.replace_text_in_file(path, "SUBSYS_SYSTIME_TEST_ENABLE      = n",
                                          "SUBSYS_SYSTIME_TEST_ENABLE      = y")
            elif case_name == "gpio":
                self.replace_text_in_file(path, "SUBSYS_GPIO_TEST_ENABLE         = n",
                                          "SUBSYS_GPIO_TEST_ENABLE         = y")

            # build
            if chip == "718p":
                p = subprocess.run("cd {}/{}/PLAT && ec718p_openbuild_ref_1h00.bat clall && ec718p_openbuild_ref_1h00.bat subsys_test".format(self.core_test_dir, self.repo_dir_name),
                                   shell=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, encoding="utf-8", errors="ignore")
            elif chip == "718pm":
                p = subprocess.run("cd {}/{}/PLAT && ec718pm_openbuild_ref_1h00.bat clall && ec718pm_openbuild_ref_1h00.bat subsys_test".format(self.core_test_dir, self.repo_dir_name),
                                   shell=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, encoding="utf-8", errors="ignore")
            elif chip == "718u":
                p = subprocess.run("cd {}/{}/PLAT && ec718u_openbuild_ref_1h00.bat clall && ec718u_openbuild_ref_1h00.bat subsys_test".format(self.core_test_dir, self.repo_dir_name),
                                   shell=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, encoding="utf-8", errors="ignore")
            elif chip == "718um":
                p = subprocess.run("cd {}/{}/PLAT && ec718um_openbuild_ref_1h00.bat clall && ec718um_openbuild_ref_1h00.bat subsys_test".format(self.core_test_dir, self.repo_dir_name),
                                   shell=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, encoding="utf-8", errors="ignore")
        else:
            logger.debug("build {} {}".format(chip, target))
            # build
            if chip == "718p":
                p = subprocess.run("cd {}/{}/PLAT && ec718p_openbuild_ref_1h00.bat clall && ec718p_openbuild_ref_1h00.bat {}".format(self.core_test_dir, self.repo_dir_name, target),
                                   shell=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, encoding="utf-8", errors="ignore")
            elif chip == "718pm":
                p = subprocess.run("cd {}/{}/PLAT && ec718pm_openbuild_ref_1h00.bat clall && ec718pm_openbuild_ref_1h00.bat {}".format(self.core_test_dir, self.repo_dir_name, target),
                                   shell=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, encoding="utf-8", errors="ignore")
            elif chip == "718u":
                p = subprocess.run("cd {}/{}/PLAT && ec718u_openbuild_ref_1h00.bat clall && ec718u_openbuild_ref_1h00.bat {}".format(self.core_test_dir, self.repo_dir_name, target),
                                   shell=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, encoding="utf-8", errors="ignore")
            elif chip == "718um":
                p = subprocess.run("cd {}/{}/PLAT && ec718um_openbuild_ref_1h00.bat clall && ec718um_openbuild_ref_1h00.bat {}".format(self.core_test_dir, self.repo_dir_name, target),
                                   shell=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, encoding="utf-8", errors="ignore")
        self.return_code = p.returncode
        logger.debug("build_return_code = {}".format(self.return_code))
        self.log_file.write(p.stdout)
        self.log_file.flush()

        self.flash_useage = "{} {}KB {}KB {}KB"

        flash_area_numbers = re.findall(
            r"FLASH_AREA:\s+(\d+\s+\w+)\s+(\d+)\s+\w+\s+(\d+.\d+)%", p.stdout)
        if len(flash_area_numbers) == 2:
            if flash_area_numbers[-1][0].endswith("KB"):
                unzip_bin_size = flash_area_numbers[-1][0].replace(" ", "")
            else:
                unzip_bin_size = "{}KB".format(
                    int(int(flash_area_numbers[-1][0].split(" ")[0])/1024))
        else:
            unzip_bin_size = "0KB"

        total_numbers = re.findall(
            r"Total\s+\|\s+\|\s+\|\s+(\d+)\s+\|\s+(\d+)", p.stdout)
        if len(total_numbers) == 2:
            total_size = int(int(total_numbers[-1][0])/1024)
        else:
            total_size = 0

        image_file_numbers = re.findall(
            r"available size: (\d+), real image size\(aligned\):(\d+)", p.stdout)
        if len(image_file_numbers) == 2:
            available_size = int(int(image_file_numbers[-1][0])/1024)
            zip_bin_size = int(int(image_file_numbers[-1][1])/1024)
        else:
            available_size = 0
            zip_bin_size = 0

        self.flash_useage = self.flash_useage.format(
            unzip_bin_size, zip_bin_size, total_size, available_size)

        self.ram_useage = "{}KB {}"

        msmb_area_numbers = re.findall(
            r"MSMB_AREA:\s+(\d+)\s+\w+\s+(\d+)\s+\w+\s+(\d+.\d+)%", p.stdout)
        if len(msmb_area_numbers) == 2:
            msmb_size = int(int(msmb_area_numbers[-1][0])/1024)
        else:
            msmb_size = 0

        psram_area_numbers = re.findall(
            r"PSRAM_AREA:\s+(\d+\s+\w+)\s+(\d+)\s+\w+\s+(\d+.\d+)%", p.stdout)
        if len(psram_area_numbers) == 1:
            if psram_area_numbers[-1][0].endswith("KB"):
                psram_size = psram_area_numbers[-1][0].replace(" ", "")
            else:
                psram_size = "{}KB".format(
                    int(int(psram_area_numbers[-1][0].split(" ")[0])/1024))
        else:
            psram_size = "0KB"
        self.ram_useage = self.ram_useage.format(msmb_size, psram_size)

        size_files = self.find_size_files()
        analysis_size_file = None
        for file in size_files:
            if os.path.basename(
                    file) != "ap_bootloader.size":
                analysis_size_file = file
                break
        if analysis_size_file != None:
            with open(analysis_size_file) as f:
                lib_file_useage = {}
                lib_file_name = ""
                for l in f.readlines():
                    match = re.search(r".*/(\w+\.a)\)", l)
                    if match:
                        if match.groups()[0] not in lib_file_useage:
                            lib_file_useage[match.groups()[0]] = ""
                            lib_file_name = match.groups()[0]
                        continue

                    match = re.search(
                        r"\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+\(TOTALS\)", l)
                    if match:
                        lib_file_useage[lib_file_name] = match.groups()
                        lib_file_name = ""
                if "" in lib_file_useage:
                    del lib_file_useage[""]
                table = prettytable.PrettyTable(
                    ["lib_file", "text", "data", "bss", "total"], title="Lib file useage", align="l")

                for key in lib_file_useage.keys():
                    render_row = []
                    render_row.append(key)
                    render_row.append(lib_file_useage.get(key)[0])
                    render_row.append(lib_file_useage.get(key)[1])
                    render_row.append(lib_file_useage.get(key)[2])
                    render_row.append(lib_file_useage.get(key)[3])
                    table.add_row(render_row)

                self.log_file.write(table.get_string())
                self.log_file.flush()

        self.zip_file_name = self.collect_build_out(chip, target, logger)

    def checkout(self, logger: Logger):
        os.makedirs(self.core_test_dir, exist_ok=True)

        rmdir_cmd = "rmdir /s /Q {}".format(os.path.join(self.core_test_dir,
                                                         self.repo_dir_name))
        clone_cmd = "cd {} && git clone {} && cd {} && git checkout {}".format(
            self.core_test_dir, self.repo_url, self.repo_dir_name, self.commit_id)

        logger.debug("rmdir_cmd = {}".format(rmdir_cmd))
        logger.debug("clone_cmd = {}".format(clone_cmd))
        if os.path.exists(os.path.join(self.core_test_dir, self.repo_dir_name)):
            logger.debug("rmdir repo...")
            p = subprocess.run(rmdir_cmd, shell=True,
                               stdout=self.log_file, stderr=self.log_file)
        logger.debug("clone repo...")
        p = subprocess.run(clone_cmd, shell=True,
                           stdout=self.log_file, stderr=self.log_file)

    def prepare_repo(self, logger: Logger):
        self.checkout(logger)
        if os.path.exists(os.path.join(self.core_test_dir, "{}_backup".format(self.repo_dir_name))):
            rm_backup_dir_cmd = "rmdir /s /Q {}".format(os.path.join(self.core_test_dir,
                                                                     "{}_backup".format(self.repo_dir_name)))
            logger.debug("rm_backup_dir_cmd = {}".format(rm_backup_dir_cmd))
            logger.debug("rm_backup_dir repo...")
            p = subprocess.run(rm_backup_dir_cmd, shell=True,
                               stdout=self.log_file, stderr=self.log_file)
        shutil.copytree(os.path.join(self.core_test_dir, self.repo_dir_name), os.path.join(
            self.core_test_dir, "{}_backup".format(self.repo_dir_name)))

    def get_new_repo(self, logger: Logger):
        rmdir_cmd = "rmdir /s /Q {}".format(os.path.join(self.core_test_dir,
                                                         self.repo_dir_name))
        logger.debug("rmdir_cmd = {}".format(rmdir_cmd))
        if os.path.exists(os.path.join(self.core_test_dir, self.repo_dir_name)):
            logger.debug("rmdir repo...")
            p = subprocess.run(rmdir_cmd, shell=True,
                               stdout=self.log_file, stderr=self.log_file)
        shutil.copytree(os.path.join(
            self.core_test_dir, "{}_backup".format(self.repo_dir_name)), os.path.join(self.core_test_dir, self.repo_dir_name))

    def commit_id_handle(self):
        logger = util.Util().get_logger("main_log.txt")
        os.makedirs(self.core_test_dir, exist_ok=True)

        core_test_start_time = int(time.time())

        status = {"pid": os.getpid(), "status": "WAIT",
                  "cost_time": 0, "targets": {}}

        if self.subsys_test_enable:
            pass
        else:
            self.target_list_718p = ["speaker_8311", "speaker_7149", "speaker_pwm", "watch", "phone", "ref_app", "ref_min", "alipay_demo", "audio_demo", "camera_demo", "fs_demo", "socket_demo", "slp_demo", "time_demo", "volte_demo", "http_demo", "jpeg_demo", "lcd_demo_lvgl", "lcd_demo_rgb", "driver_example_USART_EX", "driver_example_SPI_EX", "driver_example_I2C_EX", "driver_example_GPIO_EX", "driver_example_TIMER_EX",
                                     "driver_example_WDT_EX", "driver_example_DMA_EX", "driver_example_ADC_EX", "driver_example_KPC_EX", "driver_example_AUDIO_EX", "driver_example_TLS_EX", "driver_example_CAMERA_EX", "driver_example_LCD_EX", "driver_example_ONEW_EX"]
            self.target_list_718u = ["watch", "phone", "ref_app", "ref_min", "alipay_demo", "audio_demo", "camera_demo", "fs_demo", "socket_demo", "slp_demo", "time_demo", "volte_demo", "http_demo", "jpeg_demo", "lcd_demo_lvgl", "lcd_demo_rgb", "driver_example_USART_EX", "driver_example_SPI_EX", "driver_example_I2C_EX", "driver_example_GPIO_EX", "driver_example_TIMER_EX",
                                     "driver_example_WDT_EX", "driver_example_DMA_EX", "driver_example_ADC_EX", "driver_example_KPC_EX", "driver_example_AUDIO_EX", "driver_example_TLS_EX", "driver_example_CAMERA_EX", "driver_example_LCD_EX", "driver_example_ONEW_EX"]
            self.target_list_718pm = ["speaker_8311", "speaker_7149", "speaker_pwm", "watch", "phone", "ref_app", "ref_min", "alipay_demo", "audio_demo", "camera_demo", "fs_demo", "socket_demo", "slp_demo", "time_demo", "volte_demo", "http_demo", "jpeg_demo", "lcd_demo_lvgl", "lcd_demo_rgb", "driver_example_USART_EX", "driver_example_SPI_EX", "driver_example_I2C_EX", "driver_example_GPIO_EX", "driver_example_TIMER_EX",
                                      "driver_example_WDT_EX", "driver_example_DMA_EX", "driver_example_ADC_EX", "driver_example_KPC_EX", "driver_example_AUDIO_EX", "driver_example_TLS_EX", "driver_example_CAMERA_EX", "driver_example_LCD_EX", "driver_example_ONEW_EX"]
            self.target_list_718um = ["watch", "phone", "ref_app", "ref_min", "alipay_demo", "audio_demo", "camera_demo", "fs_demo", "socket_demo", "slp_demo", "time_demo", "volte_demo", "http_demo", "jpeg_demo", "lcd_demo_lvgl", "lcd_demo_rgb", "driver_example_USART_EX", "driver_example_SPI_EX", "driver_example_I2C_EX", "driver_example_GPIO_EX", "driver_example_TIMER_EX",
                                      "driver_example_WDT_EX", "driver_example_DMA_EX", "driver_example_ADC_EX", "driver_example_KPC_EX", "driver_example_AUDIO_EX", "driver_example_TLS_EX", "driver_example_CAMERA_EX", "driver_example_LCD_EX", "driver_example_ONEW_EX"]

        for target in self.target_list_716e:
            status["targets"]["716e {}".format(target)] = {"build_status": "WAIT", "test_status": "WAIT",
                                                           "status": "WAIT", "cost_time": 0, "flash_useage": "", "ram_useage": ""}

        for target in self.target_list_718p:
            status["targets"]["718p {}".format(target)] = {"build_status": "WAIT", "test_status": "WAIT",
                                                           "status": "WAIT", "cost_time": 0, "flash_useage": "", "ram_useage": ""}

        for target in self.target_list_718pm:
            status["targets"]["718pm {}".format(target)] = {"build_status": "WAIT", "test_status": "WAIT",
                                                            "status": "WAIT", "cost_time": 0, "flash_useage": "", "ram_useage": ""}

        for target in self.target_list_718u:
            status["targets"]["718u {}".format(target)] = {"build_status": "WAIT", "test_status": "WAIT",
                                                           "status": "WAIT", "cost_time": 0, "flash_useage": "", "ram_useage": ""}

        for target in self.target_list_718um:
            status["targets"]["718um {}".format(target)] = {"build_status": "WAIT", "test_status": "WAIT",
                                                            "status": "WAIT", "cost_time": 0, "flash_useage": "", "ram_useage": ""}

        with open(os.path.join(self.core_test_dir, "{}.txt".format(self.commit_id)), mode='w', encoding="utf-8", errors="ignore") as log_file:
            self.log_file = log_file

            start_time = int(time.time())
            self.prepare_repo(logger)
            cost_time = int(time.time()) - start_time
            logger.debug("prepare repo cost {}s".format(cost_time))

            target_list_dict = {}
            if self.ec716e_enable:
                target_list_dict["716e"] = self.target_list_716e
            if self.ec718p_enable:
                target_list_dict["718p"] = self.target_list_718p
            if self.ec718pm_enable:
                target_list_dict["718pm"] = self.target_list_718pm
            if self.ec718u_enable:
                target_list_dict["718u"] = self.target_list_718u
            if self.ec718um_enable:
                target_list_dict["718um"] = self.target_list_718um

            if not target_list_dict:
                target_list_dict["718p"] = self.target_list_718p

            first_build = True
            for chip in target_list_dict.keys():
                for _, target in enumerate(target_list_dict[chip]):
                    self.logger.debug(
                        "\r\n-------------------------------\r\n{} start\r\n-------------------------------\r\n".format(target))
                    build_target_start_time = int(time.time())
                    if first_build == False:
                        start_time = int(time.time())
                        self.get_new_repo(logger)
                        cost_time = int(time.time()) - start_time
                        logger.debug("get new repo cost {}s".format(cost_time))

                    status["targets"]["{} {}".format(
                        chip, target)]["status"] = "RUNNING"
                    status["targets"]["{} {}".format(
                        chip, target)]["build_status"] = "RUNNING"
                    status["status"] = "RUNNING"
                    try:
                        report_req = "http://127.0.0.1:8000/core_test/report/{}/{}".format(
                            self.commit_id, json.dumps(status))
                        # logger.debug("send report = {}".format(report_req))
                        response = requests.get(report_req)
                        res = response.text
                        # logger.debug("上报请求结果 = {}".format(res))

                    except requests.exceptions.ConnectionError as ce:
                        logger.debug(ce)
                        logger.debug("上报测试结果失败")

                    log_file.write(
                        "\r\n-------------------------------\r\nBuild {} {} start\r\n-------------------------------\r\n".format(chip, target))
                    log_file.flush()

                    self.build_target(chip, target, logger)
                    first_build = False

                    log_file.write(
                        "\r\n-------------------------------\r\nBuild {} {} end\r\n-------------------------------\r\n".format(chip, target))
                    log_file.flush()

                    if chip == "718p" and target.startswith("subsys_test"):
                        if self.return_code != 0:
                            status["targets"]["{} {}".format(
                                chip, target)]["build_status"] = "FAIL"
                        else:
                            status["targets"]["{} {}".format(
                                chip, target)]["build_status"] = "PASS"
                        status["targets"]["{} {}".format(
                            chip, target)]["test_status"] = "RUNNING"
                        try:
                            report_req = "http://127.0.0.1:8000/core_test/report/{}/{}".format(
                                self.commit_id, json.dumps(status))
                            # logger.debug("send report = {}".format(report_req))
                            response = requests.get(report_req)
                            res = response.text
                            # logger.debug("上报请求结果 = {}".format(res))

                        except requests.exceptions.ConnectionError as ce:
                            logger.debug(ce)
                            logger.debug("上报测试结果失败")
                        test_res = False
                        download_res = self.download(chip, target, logger)
                        if download_res:
                            test_res = self.subsys_test(chip, target, logger)
                        if download_res == False or test_res == False:
                            status["targets"]["{} {}".format(
                                chip, target)]["test_status"] = "FAIL"
                        else:
                            status["targets"]["{} {}".format(
                                chip, target)]["test_status"] = "PASS"
                    else:
                        if self.return_code != 0:
                            status["targets"]["{} {}".format(
                                chip, target)]["build_status"] = "FAIL"
                        else:
                            status["targets"]["{} {}".format(
                                chip, target)]["build_status"] = "PASS"
                        status["targets"]["{} {}".format(
                            chip, target)]["test_status"] = "None"

                    status["targets"]["{} {}".format(
                        chip, target)]["zip_file_name"] = self.zip_file_name

                    try:
                        report_req = "http://127.0.0.1:8000/core_test/report/{}/{}".format(
                            self.commit_id, json.dumps(status))
                        # logger.debug("send report = {}".format(report_req))
                        response = requests.get(report_req)
                        res = response.text
                        # logger.debug("上报请求结果 = {}".format(res))

                    except requests.exceptions.ConnectionError as ce:
                        logger.debug(ce)
                        logger.debug("上报测试结果失败")

                    build_target_end_time = int(time.time())

                    build_target_cost_time = build_target_end_time-build_target_start_time

                    status["targets"]["{} {}".format(
                        chip, target)]["cost_time"] = build_target_cost_time

                    status["targets"]["{} {}".format(
                        chip, target)]["flash_useage"] = self.flash_useage
                    self.flash_useage = ""

                    status["targets"]["{} {}".format(
                        chip, target)]["ram_useage"] = self.ram_useage
                    self.ram_useage = ""

                    if self.return_code != 0:
                        status["targets"]["{} {}".format(
                            chip, target)]["status"] = "FAIL"
                    else:
                        status["targets"]["{} {}".format(
                            chip, target)]["status"] = "PASS"

                    try:
                        report_req = "http://127.0.0.1:8000/core_test/report/{}/{}".format(
                            self.commit_id, json.dumps(status))
                        # logger.debug("send report = {}".format(report_req))
                        response = requests.get(report_req)
                        res = response.text
                        # logger.debug("上报请求结果 = {}".format(res))

                    except requests.exceptions.ConnectionError as ce:
                        logger.debug(ce)
                        logger.debug("上报测试结果失败")

                    self.logger.debug(
                        "\r\n-------------------------------\r\n{} end\r\n-------------------------------\r\n".format(target))

            p = subprocess.run("cd {core_test_dir} && 7z.exe a -tzip {commit_id}.zip ./{commit_id}".format(core_test_dir=self.core_test_dir, commit_id=self.commit_id),
                               shell=True, stdout=self.log_file, stderr=self.log_file)

            self.return_code = p.returncode
            logger.debug("zip_return_code = {}".format(self.return_code))

        for key in status["targets"].keys():
            if status["targets"][key]["status"] == "FAIL":
                status["status"] = "FAIL"
                break
        else:
            status["status"] = "PASS"
        core_test_end_time = int(time.time())
        core_test_cost_time = core_test_end_time-core_test_start_time
        status["cost_time"] = core_test_cost_time

        try:
            report_req = "http://127.0.0.1:8000/core_test/report/{}/{}".format(
                self.commit_id, json.dumps(status))
            # logger.debug("send report = {}".format(report_req))
            response = requests.get(report_req)
            res = response.text
            # logger.debug("上报请求结果 = {}".format(res))

        except requests.exceptions.ConnectionError as ce:
            logger.debug(ce)
            logger.debug("上报测试结果失败")

        try:
            report_req = "http://127.0.0.1:8000/core_test/report/{}/finish".format(
                self.commit_id)
            # logger.debug("send finish report = {}".format(report_req))
            response = requests.get(report_req)
            res = response.text
            # logger.debug("上报请求结果 = {}".format(res))
        except requests.exceptions.ConnectionError as ce:
            logger.debug(ce)
            logger.debug("上报测试结果失败")

    def start(self):
        while True:
            time.sleep(1)
            try:
                response = requests.get(
                    "http://127.0.0.1:8000/core_test/get_commit_id")
                self.commit_info = response.text
            except requests.exceptions.ConnectionError as ce:
                self.logger.debug(ce)
                self.logger.debug("请求 commit_id 连接失败")
                continue
            if self.commit_info != "null":
                self.commit_info = json.loads(self.commit_info)
                print(self.commit_info)
                self.commit_id = self.commit_info["commit_id"]
                self.repo = self.commit_info["repo"]
                self.subsys_test_enable = self.commit_info["subsys_test"]
                self.ec716e_enable = self.commit_info["ec716e"]
                self.ec718p_enable = self.commit_info["ec718p"]
                self.ec718pm_enable = self.commit_info["ec718pm"]
                self.ec718u_enable = self.commit_info["ec718u"]
                self.ec718um_enable = self.commit_info["ec718um"]
                self.repo_dir_name = "ec718_716_sdk_{}".format(self.repo)
                self.repo_url = self.repo_url_template.format(self.repo)
                self.logger.debug("get {} commit_id = {}".format(
                    self.repo, self.commit_id))
                p = multiprocessing.Process(target=self.commit_id_handle)
                p.start()
                p.join()
                self.commit_id = "null"


if __name__ == "__main__":
    # print(sys.argv)
    try:
        CoreTestCli(sys.argv[1], sys.argv[2]).start()
    except KeyboardInterrupt:
        print("Aborted!")
        sys.exit(0)
