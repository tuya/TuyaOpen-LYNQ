import typer
import requests
import json
import sys
import datetime
import traceback
import build
import os
import subprocess

from prettytable import PrettyTable


app = typer.Typer()

server_ip = "10.10.60.111"
# server_ip = "127.0.0.1"


def green_print(content: str):
    print("\033[0;32;40m{}\033[0m".format(content))


def yellow_print(content: str):
    print("\033[0;33;40m{}\033[0m".format(content))


def red_print(content: str):
    print("\033[0;31;40m{}\033[0m".format(content))


def green_format(content: str) -> str:
    return ("\033[0;32;40m{}\033[0m".format(content))


def yellow_format(content: str) -> str:
    return ("\033[0;33;40m{}\033[0m".format(content))


def red_format(content: str) -> str:
    return ("\033[0;31;40m{}\033[0m".format(content))


class OpenFlow(object):
    def __init__(self) -> None:
        self.main_repo_url = "http://10.10.60.68:3000/open_eigencomm/ec718_716_sdk_main.git"
        self.release_repo_url = "http://10.10.60.68:3000/open_eigencomm/ec718_716_sdk_release.git"
        self.repo_url = self.main_repo_url

    def check_commit_exist_in_remote_repo(self, commit_id: str, repo: str) -> bool:
        if repo == "main":
            self.repo_url = self.main_repo_url
        elif repo == "release":
            self.repo_url = self.release_repo_url
        else:
            red_print("Bad repo selected!")
            return False

        try:
            result = subprocess.run(
                ['git', 'ls-remote', self.repo_url],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            return commit_id in result.stdout
        except subprocess.CalledProcessError as e:
            red_print("An error occurred: {}".format(e.stderr))
            return False

    def get_status(self, commit_id: str, i: int):
        if commit_id is None and i is None:
            response = requests.get(
                "http://{}:8000/core_test/test_result/all".format(server_ip))

            format_text = response.text
            all_status = json.loads(format_text)
            show_status = {}
            for key in all_status.keys():
                show_status[key] = {}
                show_status[key]["submit_time"] = datetime.datetime.fromtimestamp(
                    all_status[key]["submit_time"]).strftime("%Y-%m-%d %H:%M:%S")
                if all_status[key]["start_time"] != 0:
                    show_status[key]["start_time"] = datetime.datetime.fromtimestamp(
                        all_status[key]["start_time"]).strftime("%Y-%m-%d %H:%M:%S")
                else:
                    show_status[key]["start_time"] = all_status[key].get(
                        "status")
                show_status[key]["cost_time"] = all_status[key].get(
                    "cost_time")
                show_status[key]["repo"] = all_status[key].get(
                    "repo")
                show_status[key]["status"] = all_status[key].get("status")
            table = PrettyTable(
                ["index", 'submit_time', 'start_time', "cost_time", 'repo', 'commit', 'status'], align="l")
            index = 1
            for key in show_status.keys():
                render_row = []
                render_row.append(index)
                index += 1
                render_row.append(show_status[key]["submit_time"])
                render_row.append(show_status[key]["start_time"])
                if show_status[key]["cost_time"] == 0 or show_status[key]["cost_time"] is None:
                    render_row.append(show_status[key]["status"])
                else:
                    mins = int(show_status[key]["cost_time"] / 60)
                    secs = show_status[key]["cost_time"] % 60
                    render_row.append("{}:{}".format(mins, secs))
                render_row.append(show_status[key]["repo"])
                render_row.append(key)
                if show_status[key]["status"] == "PASS":
                    render_row.append(green_format(show_status[key]["status"]))
                elif show_status[key]["status"] == "FAIL":
                    render_row.append(red_format(show_status[key]["status"]))
                else:
                    render_row.append(yellow_format(
                        show_status[key]["status"]))
                table.add_row(render_row)
            print(table)
        elif commit_id is None and i:
            response = requests.get(
                "http://{}:8000/core_test/test_result/all".format(server_ip))

            format_text = response.text
            all_status = json.loads(format_text)
            show_status = {}
            index = 1
            for commit_id in all_status.keys():
                if index == i:
                    response = requests.get(
                        "http://{}:8000/core_test/test_result/{}".format(server_ip, commit_id))
                    format_text = response.text
                    if format_text == "null":
                        red_print(
                            "Error: Can not find test of commit_id = {}".format(commit_id))
                        return
                    status = json.loads(format_text)
                    if isinstance(status, str):
                        print("{}".format(status))
                        return
                    if status.get("status") == "PASS":
                        print("STATUS : {}".format(
                            green_format(status.get("status"))))
                    elif status.get("status") == "FAIL":
                        print("STATUS : {}".format(
                            red_format(status.get("status"))))
                    else:
                        print("STATUS : {}".format(
                            yellow_format(status.get("status"))))
                    print("SUBMIT TIME : {}".format(datetime.datetime.fromtimestamp(
                        status["submit_time"]).strftime("%Y-%m-%d %H:%M:%S")))
                    if status["start_time"] != 0:
                        print("START TIME : {}".format(datetime.datetime.fromtimestamp(
                            status["start_time"]).strftime("%Y-%m-%d %H:%M:%S")))
                    else:
                        print("START TIME : WAIT")
                    print(
                        "BUILD_LOG : http://{}:8000/build_out/{}.txt".format(server_ip, commit_id))
                    if status.get("status") == "PASS" or status.get("status") == "FAIL":
                        print(
                            "BUILD_OUT : http://{}:8000/build_out/{}.zip".format(server_ip, commit_id))
                    table = PrettyTable(
                        ['test', "msmb", "psram", "unzip_bin", "zip_bin", "total", "available", "cost_time", "build_status", "test_status", 'status'], align="l")
                    for key in status["targets"].keys():
                        render_row = []
                        render_row.append(key)
                        if status["targets"][key]["ram_useage"] == "":
                            render_row.append("")
                            render_row.append("")
                        else:
                            ram_useage = status["targets"][key]["ram_useage"].split(
                                " ")
                            render_row.append(ram_useage[0])
                            render_row.append(ram_useage[1])
                        if status["targets"][key]["flash_useage"] == "":
                            render_row.append("")
                            render_row.append("")
                            render_row.append("")
                            render_row.append("")
                        else:
                            flash_useage = status["targets"][key]["flash_useage"].split(
                                " ")
                            render_row.append(flash_useage[0])
                            render_row.append(flash_useage[1])
                            render_row.append(flash_useage[2])
                            render_row.append(flash_useage[3])

                        if status["targets"][key]["cost_time"] == 0:
                            render_row.append(yellow_format(
                                status["targets"][key]["status"]))
                        else:
                            mins = int(status["targets"][key]
                                       ["cost_time"] / 60)
                            secs = status["targets"][key]["cost_time"] % 60
                            render_row.append("{}:{}".format(mins, secs))

                        if status["targets"][key]["build_status"] == "PASS":
                            render_row.append(green_format(
                                status["targets"][key]["build_status"]))
                        elif status["targets"][key]["build_status"] == "FAIL":
                            render_row.append(red_format(
                                status["targets"][key]["build_status"]))
                        else:
                            render_row.append(yellow_format(
                                status["targets"][key]["build_status"]))

                        if status["targets"][key]["test_status"] == "PASS":
                            render_row.append(green_format(
                                status["targets"][key]["test_status"]))
                        elif status["targets"][key]["test_status"] == "FAIL":
                            render_row.append(red_format(
                                status["targets"][key]["test_status"]))
                        else:
                            render_row.append(yellow_format(
                                status["targets"][key]["test_status"]))

                        if status["targets"][key]["status"] == "PASS":
                            render_row.append(green_format(
                                status["targets"][key]["status"]))
                        elif status["targets"][key]["status"] == "FAIL":
                            render_row.append(red_format(
                                status["targets"][key]["status"]))
                        else:
                            render_row.append(yellow_format(
                                status["targets"][key]["status"]))

                        table.add_row(render_row)
                    print(table)
                index += 1

        elif commit_id and i is None:
            response = requests.get(
                "http://{}:8000/core_test/test_result/{}".format(server_ip, commit_id))
            format_text = response.text
            if format_text == "null":
                red_print(
                    "Error: Can not find test of commit_id = {}".format(commit_id))
                return
            status = json.loads(format_text)
            if isinstance(status, str):
                print("{}".format(status))
                return
            if status.get("status") == "PASS":
                print("STATUS : {}".format(green_format(status.get("status"))))
            elif status.get("status") == "FAIL":
                print("STATUS : {}".format(red_format(status.get("status"))))
            else:
                print("STATUS : {}".format(yellow_format(status.get("status"))))
            print("SUBMIT TIME : {}".format(datetime.datetime.fromtimestamp(
                status["submit_time"]).strftime("%Y-%m-%d %H:%M:%S")))
            if status["start_time"] != 0:
                print("START TIME : {}".format(datetime.datetime.fromtimestamp(
                    status["start_time"]).strftime("%Y-%m-%d %H:%M:%S")))
            else:
                print("START TIME : WAIT")
            print(
                "BUILD_LOG : http://{}:8000/build_out/{}.txt".format(server_ip, commit_id))
            if status.get("status") == "PASS" or status.get("status") == "FAIL":
                print(
                    "BUILD_OUT : http://{}:8000/build_out/{}.zip".format(server_ip, commit_id))
            table = PrettyTable(['test', "msmb", "psram", "unzip_bin", "zip_bin", "total",
                                "available", "cost_time", "build_status", "test_status", 'status'], align="l")
            for key in status["targets"].keys():
                render_row = []
                render_row.append(key)
                if status["targets"][key]["ram_useage"] == "":
                    render_row.append("")
                    render_row.append("")
                else:
                    ram_useage = status["targets"][key]["ram_useage"].split(
                        " ")
                    render_row.append(ram_useage[0])
                    render_row.append(ram_useage[1])
                if status["targets"][key]["flash_useage"] == "":
                    render_row.append("")
                    render_row.append("")
                    render_row.append("")
                    render_row.append("")
                else:
                    flash_useage = status["targets"][key]["flash_useage"].split(
                        " ")
                    render_row.append(flash_useage[0])
                    render_row.append(flash_useage[1])
                    render_row.append(flash_useage[2])
                    render_row.append(flash_useage[3])
                if status["targets"][key]["cost_time"] == 0:
                    render_row.append(yellow_format(
                        status["targets"][key]["status"]))
                else:
                    mins = int(status["targets"][key]["cost_time"] / 60)
                    secs = status["targets"][key]["cost_time"] % 60
                    render_row.append("{}:{}".format(mins, secs))

                if status["targets"][key]["build_status"] == "PASS":
                    render_row.append(green_format(
                        status["targets"][key]["build_status"]))
                elif status["targets"][key]["build_status"] == "FAIL":
                    render_row.append(red_format(
                        status["targets"][key]["build_status"]))
                else:
                    render_row.append(yellow_format(
                        status["targets"][key]["build_status"]))

                if status["targets"][key]["test_status"] == "PASS":
                    render_row.append(green_format(
                        status["targets"][key]["test_status"]))
                elif status["targets"][key]["test_status"] == "FAIL":
                    render_row.append(red_format(
                        status["targets"][key]["test_status"]))
                else:
                    render_row.append(yellow_format(
                        status["targets"][key]["test_status"]))

                if status["targets"][key]["status"] == "PASS":
                    render_row.append(green_format(
                        status["targets"][key]["status"]))
                elif status["targets"][key]["status"] == "FAIL":
                    render_row.append(red_format(
                        status["targets"][key]["status"]))
                else:
                    render_row.append(yellow_format(
                        status["targets"][key]["status"]))

                table.add_row(render_row)
            print(table)

    def stop(self, commit_id: str):
        response = requests.get(
            "http://{}:8000/core_test/stop/{}".format(server_ip, commit_id))
        print(response.text)


@app.command()
def core_test(commit_id: str, repo: str = "main", subsys_test: bool = False):
    if len(commit_id) != 40:
        red_print("Error: Invalid commit id, sha1 lenth is 40")
        return
    if OpenFlow().check_commit_exist_in_remote_repo(commit_id, repo):
        response = requests.get(
            "http://{}:8000/core_test/commit_id/{}/{}/{}".format(server_ip, repo, commit_id, subsys_test))
        print(response.text)
    else:
        red_print(
            "Error: Can not find commit id {} in remote {}_repo's branch or merge request, please push your commit to remote repo".format(commit_id, repo))


@app.command()
def status(i: int = None, commit_id: str = None):
    if commit_id and len(commit_id) != 40:
        red_print("Error: Invalid commit id, sha1 lenth is 40")
        return
    OpenFlow().get_status(commit_id, i)


@app.command()
def stop(commit_id: str):
    if commit_id and len(commit_id) != 40:
        red_print("Error: Invalid commit id, sha1 lenth is 40")
        return
    OpenFlow().stop(commit_id)


@app.command()
def check_queue():
    response = requests.get(
        "http://{}:8000/core_test/check_queue".format(server_ip))
    print(response.text)


@app.command()
def check_collection():
    response = requests.get(
        "http://{}:8000/core_test/check_collection".format(server_ip))
    print(json.dumps(json.loads(response.text), indent=4))


@app.command()
def build_target(repo: str = "main", commit_id: str = "latest", board: str = "1h00", target: str = None) -> None:
    build_tools = build.BuildTools()
    if target:
        build_tools.target_list = target.split(",")
    os.makedirs(build_tools.build_dir, exist_ok=True)
    log_path = os.path.join(build_tools.build_dir,
                            "{}_{}_{}.txt".format(repo, board, commit_id))
    with open(log_path, "w") as log_file:
        print("build log in {}".format(log_path))
        build_tools.log_file = log_file
        build_tools.repo = repo
        build_tools.board = board
        build_tools.build(commit_id)


@app.command()
def backup(repo: str, commit_id: str):
    requests.get(
        "http://10.10.60.68:8000/backup/{}/{}".format(repo, commit_id))


if __name__ == "__main__":
    try:
        app()
    except requests.exceptions.ConnectionError as ce:
        red_print("Error: Can not connect to remote server {}!".format(server_ip))
        sys.exit(1)
    except Exception as e:
        traceback.print_exc()
        sys.exit(1)
