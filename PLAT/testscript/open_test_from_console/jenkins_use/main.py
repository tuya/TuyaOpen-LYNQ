import subprocess
import sys
import os
import datetime
import git
import glob
import time
import shutil
import serial
import serial.tools.list_ports as list_ports


class JenkinsTest():
    def __init__(self, build_target: str, latest_commit_id: str, return_code: int, case_name: str, control_com: str, download_com: str, start_time: str) -> None:
        self.build_target = build_target
        self.latest_commit_id = latest_commit_id
        self.return_code = return_code
        self.case_name = case_name
        self.control_com = control_com
        self.download_com = download_com
        self.start_time = start_time

    def replace_text_in_file(self, file_path, old_text, new_text):
        with open(file_path, 'r') as file:
            file_content = file.read()

        updated_content = file_content.replace(old_text, new_text)

        with open(file_path, 'w') as file:
            file.write(updated_content)

    def create_directory_if_not_exists(self, directory):
        if not os.path.exists(directory) or not os.path.isdir(directory):
            os.makedirs(directory)
            print("目录 '{}' 创建成功.".format(directory))
        else:
            print("目录 '{}' 已经存在，忽略创建.".format(directory))

    def find_binpkg_files(self, directory):
        binpkg_files = glob.glob(os.path.join(
            directory, '**', '*.binpkg'), recursive=True)

        res = []

        for file_path in binpkg_files:
            res.append(os.path.abspath(file_path))

        return res

    def zip(self):
        # zip
        if self.return_code == 0:
            p = subprocess.run("cd PLAT && 7z.exe a -tzip {commit_id}_{build_target}_{case_name}_{time}.zip ./gccout && move {commit_id}_{build_target}_{case_name}_{time}.zip ../../{commit_id}".format(build_target=self.build_target, time=self.start_time, commit_id=self.latest_commit_id, case_name=self.case_name),
                               shell=True)

            self.return_code = p.returncode
            print("zip_return_code : {}".format(self.return_code))

    def download(self):

        # download
        # FlashToolCLI.exe --cfgfile config_pkg_product_usb.ini pkg2img
        # FlashToolCLI.exe --port="COM5" --cfgfile config_pkg_product_usb.ini probe
        # FlashToolCLI.exe --skipconnect 1 --port="COM5" --cfgfile config_pkg_product_usb.ini runtmcfg
        # FlashToolCLI.exe --skipconnect 1 --cfgfile config_pkg_product_usb.ini --verbose 1 --port="COM5" burnbatch --imglist bootloader cp_system system
        # FlashToolCLI.exe --skipconnect 1 --cfgfile config_pkg_product_usb.ini --verbose 1 --port="COM5" burn_pkgflxs
        # FlashToolCLI.exe --skipconnect 1 --cfgfile config_pkg_product_usb.ini --port COM5 sysreset
        if return_code == 0:

            directory_to_search = './PLAT/gccout'
            files = self.find_binpkg_files(directory_to_search)
            print("Find binpkg files: {}".format(files))
            target_path = "../res.binpkg"
            for file in files:
                shutil.copy(file, target_path)

            p = subprocess.run("cd ../../tools/FlashToolCLI_V4.1.11p01_240401 && FlashToolCLI.exe --cfgfile config_pkg_product_usb.ini pkg2img",
                               shell=True)

            self.return_code = p.returncode
            print("pkg2img return_code : {}".format(self.return_code))

        while True:
            try:
                ser = serial.Serial(self.control_com, 115200)
                break
            except serial.serialutil.SerialException:
                print("open control com fail retry...")
                time.sleep(1)

        while True:
            ports = list_ports.comports()
            # print("ports = {}".format(ports))
            ports_names = [port.name for port in ports]

            print(ports_names)

            if self.download_com in ports_names:
                print("go to download mode ok")
                break

            print("go to download mode start")
            ser.dtr = True
            time.sleep(1)
            ser.rts = True
            time.sleep(1)
            ser.rts = False
            time.sleep(1)
            ser.dtr = False

        if self.return_code == 0:
            p = subprocess.run('cd ../../tools/FlashToolCLI_V4.1.11p01_240401 && FlashToolCLI.exe --port="{}" --cfgfile config_pkg_product_usb.ini probe'.format(self.download_com),
                               shell=True)

            self.return_code = p.returncode
            print("probe return_code : {}".format(self.return_code))

        if self.return_code == 0:
            p = subprocess.run('cd ../../tools/FlashToolCLI_V4.1.11p01_240401 && FlashToolCLI.exe --skipconnect 1 --port="{}" --cfgfile config_pkg_product_usb.ini runtmcfg'.format(self.download_com),
                               shell=True)

            self.return_code = p.returncode
            print("runtmcfg return_code : {}".format(self.return_code))

        if self.return_code == 0:
            p = subprocess.run('cd ../../tools/FlashToolCLI_V4.1.11p01_240401 && FlashToolCLI.exe --skipconnect 1 --cfgfile config_pkg_product_usb.ini --verbose 1 --port="{}" burnbatch --imglist bootloader cp_system system'.format(self.download_com),
                               shell=True)

            self.return_code = p.returncode
            print("burnbatch return_code : {}".format(self.return_code))

        if self.return_code == 0:
            p = subprocess.run('cd ../../tools/FlashToolCLI_V4.1.11p01_240401 && FlashToolCLI.exe --skipconnect 1 --cfgfile config_pkg_product_usb.ini --verbose 1 --port="{}" burn_pkgflxs'.format(self.download_com),
                               shell=True)

            self.return_code = p.returncode
            print("burn_pkgflxs return_code : {}".format(self.return_code))

        if self.return_code == 0:
            p = subprocess.run('cd ../../tools/FlashToolCLI_V4.1.11p01_240401 && FlashToolCLI.exe --skipconnect 1 --cfgfile config_pkg_product_usb.ini --port {} sysreset'.format(self.download_com),
                               shell=True)

            self.return_code = p.returncode
            print("sysreset return_code : {}".format(self.return_code))

        ser.close()

    def test(self):

        repo = git.Repo("./")
        self.latest_commit_id = repo.head.commit.hexsha
        print("commit id : {}".format(self.latest_commit_id))

        start_time = datetime.datetime.now()
        self.start_time = start_time.strftime("%Y_%m_%d_%H_%M_%S")
        print("start at : {}".format(self.start_time))

        build_output_collect_dir = "../{}".format(self.latest_commit_id)

        self.create_directory_if_not_exists(build_output_collect_dir)

        if self.build_target == "ref_app":
            try:
                os.remove("{}.txt".format(self.latest_commit_id))
                print("pipeline_result 已清除历史")
            except FileNotFoundError:
                print("pipeline_result 不存在, 无需清除")

        if self.build_target.startswith("speaker"):
            _, chip, case_name = self.build_target.split("_")
            self.case_name = "{}_{}".format(chip, case_name)
            # build
            if chip == "718p":
                p = subprocess.run("cd PLAT && ec718p_openbuild_speaker_1h00.bat clall && ec718p_openbuild_speaker_1h00.bat {}".format(case_name),
                                   shell=True)
            elif chip == "716e":
                p = subprocess.run("cd PLAT && ec716e_openbuild_speaker_1h00.bat clall && ec716e_openbuild_speaker_1h00.bat {}".format(case_name),
                                   shell=True)

            self.return_code = p.returncode
            print("build_return_code : {}".format(self.return_code))

            self.zip()
            self.download()

            self.case_name = "default"

        elif self.build_target == "driver_example":
            self.case_name = sys.argv[4]
            print("build driver_example {}".format(self.case_name))

            path = "./PLAT/project/ec7xx_ref_1h00/ap/apps/driver_example/src/app.c"
            self.replace_text_in_file(path, "static example_id_t exampleId = CAMERA_EX;",
                                      "static example_id_t exampleId = {};".format(self.case_name))

            # build
            p = subprocess.run("cd PLAT && ec718p_openbuild_ref_1h00.bat clall && ec718p_openbuild_ref_1h00.bat driver_example-merge",
                               shell=True)

            self.return_code = p.returncode
            print("build_return_code : {}".format(self.return_code))

            self.zip()
            self.download()

            self.case_name = "default"

        elif self.build_target == "lcd_demo":
            self.case_name = sys.argv[4]
            print("build lcd_demo {}".format(self.case_name))

            if self.case_name == "rgb":
                path = "./PLAT/project/ec7xx_ref_1h00/ap/apps/lcd_demo/GCC/Makefile"
                self.replace_text_in_file(path, "# CFLAGS += -DFEATURE_LCD_TEST_RGB",
                                          "CFLAGS += -DFEATURE_LCD_TEST_RGB")

            # build
            p = subprocess.run("cd PLAT && ec718p_openbuild_ref_1h00.bat clall && ec718p_openbuild_ref_1h00.bat lcd_demo",
                               shell=True)

            self.return_code = p.returncode
            print("build_return_code : {}".format(self.return_code))

            self.zip()
            self.download()

            self.case_name = "default"

        else:

            # build
            p = subprocess.run("cd PLAT && ec718p_openbuild_ref_1h00.bat clall && ec718p_openbuild_ref_1h00.bat {build_target}".format(build_target=self.build_target),
                               shell=True)

            self.return_code = p.returncode
            print("build_return_code : {}".format(self.return_code))

            self.zip()
            self.download()

        with open("{}.txt".format(self.latest_commit_id), "a") as f:
            f.write("{} {} {}\r\n".format(
                self.build_target, self.case_name, self.return_code))

        sys.exit(self.return_code)


if __name__ == "__main__":
    latest_commit_id = ""
    return_code = 0
    case_name = "default"
    start_time = ""
    print("args = {}".format(sys.argv))

    if len(sys.argv) > 3:
        control_com = sys.argv[1]
        download_com = sys.argv[2]
        build_target = sys.argv[3]

    JenkinsTest(build_target, latest_commit_id, return_code,
                case_name, control_com, download_com, start_time).test()
