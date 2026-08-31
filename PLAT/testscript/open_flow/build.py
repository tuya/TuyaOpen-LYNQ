import subprocess
import os
import glob
import datetime
import sys
import shutil
import time
from prettytable import PrettyTable


class BuildTools():
    def __init__(self) -> None:
        self.repo = "main"
        self.board = "1h00"
        self.build_bat = "ec718p_openbuild_ref_1h00.bat"
        self.log_file = None
        self.repo_dir_name = "ec718_716_sdk_main"
        self.repo_url = "ssh://git@10.10.60.68:2222/open_eigencomm/ec718_716_sdk_main.git"
        self.main_repo_url = "ssh://git@10.10.60.68:2222/open_eigencomm/ec718_716_sdk_main.git"
        self.release_repo_url = "ssh://git@10.10.60.68:2222/open_eigencomm/ec718_716_sdk_release.git"
        self.target_list = []
        self.target_list_0h00 = ["driver_example_USART_EX", "driver_example_SPI_EX", "driver_example_I2C_EX", "driver_example_GPIO_EX", "driver_example_TIMER_EX",
                                 "driver_example_WDT_EX", "driver_example_DMA_EX", "driver_example_ADC_EX", "driver_example_KPC_EX", "driver_example_AUDIO_EX", "driver_example_TLS_EX", "driver_example_CAMERA_EX", "driver_example_LCD_EX", "driver_example_ONEW_EX", "driver_example_PSRAM_EX"]
        self.target_list_1h00 = ["speaker", "phone", "ref_app", "ref_min", "audio_demo", "camera_demo", "fs_demo", "socket_demo", "lcd_demo_lvgl", "lcd_demo_rgb", "slp_demo", "time_demo", "volte_demo", "driver_example_USART_EX", "driver_example_SPI_EX", "driver_example_I2C_EX", "driver_example_GPIO_EX", "driver_example_TIMER_EX",
                                 "driver_example_WDT_EX", "driver_example_DMA_EX", "driver_example_ADC_EX", "driver_example_KPC_EX", "driver_example_AUDIO_EX", "driver_example_TLS_EX", "driver_example_CAMERA_EX", "driver_example_LCD_EX", "driver_example_ONEW_EX", "driver_example_PSRAM_EX"]

        self.build_return_code = 0
        self.zip_return_code = 0
        self.build_dir = "C:\\build_collect"
        self.commit_id = "latest"
        self.collect_dir = "{}_{}_{}".format(
            self.repo, self.board, self.commit_id)
        self.build_res_dict = {}
        self.build_res_table = None

    def replace_text_in_file(self, file_path, old_text, new_text):
        with open(file_path, 'r') as file:
            file_content = file.read()

        updated_content = file_content.replace(old_text, new_text)

        with open(file_path, 'w') as file:
            file.write(updated_content)

    def find_binpkg_files(self):
        binpkg_files = glob.glob(os.path.join(
            self.build_dir, self.repo_dir_name, "PLAT/gccout", '**', '*.binpkg'), recursive=True)

        res = []

        for file_path in binpkg_files:
            res.append(os.path.abspath(file_path))

        return res

    def find_comdb_files(self):
        comdb_files = glob.glob(os.path.join(
            self.build_dir, self.repo_dir_name, "PLAT/gccout", '**', 'comdb.txt'), recursive=True)

        res = []

        for file_path in comdb_files:
            res.append(os.path.abspath(file_path))

        return res

    def find_elf_files(self):
        elf_files = glob.glob(os.path.join(
            self.build_dir, self.repo_dir_name, "PLAT/gccout", '**', '*.elf'), recursive=True)

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

    def collect_build_out(self, target, case_name):
        self.collect_dir = "{}_{}_{}".format(
            self.repo, self.board, self.commit_id)
        if self.build_return_code == 0:
            os.makedirs(os.path.join(self.build_dir,
                        self.collect_dir), exist_ok=True)
            collect_files = [
                file for file in self.find_collect_files() if os.path.basename(file) != "ap_bootloader.elf"]
            files_to_compress = ' '.join(collect_files)
            p = subprocess.run("cd {build_dir}/{repo_dir_name}/PLAT && 7z.exe a -tzip {build_target}_{case_name}_{time}.zip {files_to_compress} && move {build_target}_{case_name}_{time}.zip ../../{collect_dir}".format(build_dir=self.build_dir, repo_dir_name=self.repo_dir_name, build_target=target, time=datetime.datetime.now().strftime("%Y_%m_%d_%H_%M_%S"), files_to_compress=files_to_compress, collect_dir=self.collect_dir, case_name=case_name),
                               shell=True, stdout=self.log_file, stderr=self.log_file)

            self.zip_return_code = p.returncode
            print("zip_return_code = {}".format(self.zip_return_code))

    def prepare_repo(self):
        self.checkout()
        if os.path.exists(os.path.join(self.build_dir, "{}_backup".format(self.repo_dir_name))):
            rm_backup_dir_cmd = "rmdir /s /Q {}".format(os.path.join(self.build_dir,
                                                                     "{}_backup".format(self.repo_dir_name)))
            print("rm_backup_dir_cmd = {}".format(rm_backup_dir_cmd))
            print("rm_backup_dir repo...")
            p = subprocess.run(rm_backup_dir_cmd, shell=True,
                               stdout=self.log_file, stderr=self.log_file)
        shutil.copytree(os.path.join(self.build_dir, self.repo_dir_name), os.path.join(
            self.build_dir, "{}_backup".format(self.repo_dir_name)))

    def get_new_repo(self):
        rmdir_cmd = "rmdir /s /Q {}".format(os.path.join(self.build_dir,
                                                         self.repo_dir_name))
        print("rmdir_cmd = {}".format(rmdir_cmd))
        if os.path.exists(os.path.join(self.build_dir, self.repo_dir_name)):
            print("rmdir repo...")
            p = subprocess.run(rmdir_cmd, shell=True,
                               stdout=self.log_file, stderr=self.log_file)
        shutil.copytree(os.path.join(
            self.build_dir, "{}_backup".format(self.repo_dir_name)), os.path.join(self.build_dir, self.repo_dir_name))

    def checkout(self):
        print("build_dir = {}".format(self.build_dir))
        os.makedirs(self.build_dir, exist_ok=True)
        self.repo_dir_name = "ec718_716_sdk_{}".format(self.repo)
        rmdir_cmd = "rmdir /s /Q {}".format(os.path.join(self.build_dir,
                                                         self.repo_dir_name))
        if self.repo == "main":
            self.repo_url = self.main_repo_url
        elif self.repo == "release":
            self.repo_url = self.release_repo_url
        else:
            print("Bad repo selected: {}".format(self.repo))
            sys.exit(1)

        if self.commit_id != None and self.commit_id != "latest":
            clone_cmd = "cd {} && git clone {} && cd {} && git checkout {}".format(
                self.build_dir, self.repo_url, self.repo_dir_name, self.commit_id)
        else:
            clone_cmd = "cd {} && git clone {}".format(
                self.build_dir, self.repo_url)
        print("rmdir_cmd = {}".format(rmdir_cmd))

        print("clone_cmd = {}".format(clone_cmd))
        if os.path.exists(os.path.join(self.build_dir, self.repo_dir_name)):
            print("rmdir repo...")
            p = subprocess.run(rmdir_cmd, shell=True,
                               stdout=self.log_file, stderr=self.log_file)
        print("clone repo...")
        p = subprocess.run(clone_cmd, shell=True,
                           stdout=self.log_file, stderr=self.log_file)

    def build_target(self, target: str):
        case_name = "default"
        if self.board == "1h00":
            self.build_bat = "ec718p_openbuild_ref_1h00.bat"
        elif self.board == "0h00":
            self.build_bat = "GccBuild_ec718p.bat"
        else:
            print("Bad board selected: {}".format(self.board))
            sys.exit(1)
        if target == "speaker":
            print("build speaker...")
            # build
            p = subprocess.run("cd {}/{}/PLAT && ec718p_openbuild_speaker_1h00.bat clall && ec718p_openbuild_speaker_1h00.bat".format(self.build_dir, self.repo_dir_name),
                               shell=True, stdout=self.log_file, stderr=self.log_file)

            self.build_return_code = p.returncode
            print("build_return_code = {}".format(self.build_return_code))

        elif "driver_example" in target:
            case_name = target.replace("driver_example_", "")
            print("build driver_example {}...".format(case_name))

            if self.board == "1h00":
                path = "{}/{}/PLAT/project/ec7xx_ref_1h00/ap/apps/driver_example/src/app.c".format(
                    self.build_dir, self.repo_dir_name)
                self.replace_text_in_file(path, "static example_id_t exampleId = CAMERA_EX;",
                                          "static example_id_t exampleId = {};".format(case_name))
            elif self.board == "0h00":
                path = "{}/{}/PLAT/project/ec7xx_0h00/ap/apps/driver_example/src/app.c".format(
                    self.build_dir, self.repo_dir_name)
                self.replace_text_in_file(path, "static example_id_t exampleId = LCD_EX;",
                                          "static example_id_t exampleId = {};".format(case_name))

            # build
            p = subprocess.run(
                "cd {}/{}/PLAT && {} clall && {} driver_example-merge".format(self.build_dir, self.repo_dir_name, self.build_bat, self.build_bat), shell=True, stdout=self.log_file, stderr=self.log_file)

            self.build_return_code = p.returncode
            print("build_return_code = {}".format(self.build_return_code))

        elif "lcd_demo" in target:
            case_name = target.replace("lcd_demo_", "")
            print("build lcd_demo {}...".format(case_name))

            if case_name == "rgb":
                path = "{}/{}/PLAT/project/ec7xx_ref_1h00/ap/apps/lcd_demo/GCC/Makefile".format(
                    self.build_dir, self.repo_dir_name)
                self.replace_text_in_file(path, "# CFLAGS += -DFEATURE_LCD_TEST_RGB",
                                          "CFLAGS += -DFEATURE_LCD_TEST_RGB")

            # build
            p = subprocess.run("cd {}/{}/PLAT && ec718p_openbuild_ref_1h00.bat clall && ec718p_openbuild_ref_1h00.bat lcd_demo".format(self.build_dir, self.repo_dir_name),
                               shell=True, stdout=self.log_file, stderr=self.log_file)

            self.build_return_code = p.returncode
            print("build_return_code = {}".format(self.build_return_code))

        else:
            print("build {}...".format(target))
            # build
            p = subprocess.run("cd {}/{}/PLAT && ec718p_openbuild_ref_1h00.bat clall && ec718p_openbuild_ref_1h00.bat {}".format(self.build_dir, self.repo_dir_name, target),
                               shell=True, stdout=self.log_file, stderr=self.log_file)

            self.build_return_code = p.returncode
            print("build_return_code = {}".format(self.build_return_code))

        self.collect_build_out(target, case_name)
        self.build_res_table.add_row(
            [target, self.build_return_code, self.zip_return_code])

    def build(self, commit_id: str):
        self.build_res_table = PrettyTable(
            ['target', 'build_res', 'zip_res'])
        self.commit_id = commit_id

        if len(self.target_list) == 0:
            if self.board == "0h00":
                self.target_list = self.target_list_0h00
            elif self.board == "1h00":
                self.target_list = self.target_list_1h00
            else:
                print("Bad board selected: {}".format(self.board))
                sys.exit(1)

        start_time = int(time.time())
        self.prepare_repo()
        cost_time = int(time.time()) - start_time
        print("prepare repo cost {}s".format(cost_time))

        for index, target in enumerate(self.target_list):
            if index != 0:
                start_time = int(time.time())
                self.get_new_repo()
                cost_time = int(time.time()) - start_time
                print("get new repo cost {}s".format(cost_time))
            self.build_target(target)

        print("REPO: {}".format(self.repo))
        print("REPO_URL: {}".format(self.repo_url))
        print("BOARD: {}".format(self.board))
        print("COMMIT: {}".format(self.commit_id))
        print(self.build_res_table)


if __name__ == "__main__":
    build_tools = BuildTools()
    commit_id = sys.argv[1]
    if len(sys.argv) == 3:
        target = sys.argv[2]
        build_tools.target_list = [target,]
    with open(os.path.join(build_tools.build_dir, "{}.txt".format(commit_id)), "w") as log_file:
        build_tools.log_file = log_file
        build_tools.build(commit_id)
