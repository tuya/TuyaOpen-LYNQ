import json
import time
import os
import psutil
import subprocess

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from common import util
from fastapi.staticfiles import StaticFiles
from redis import StrictRedis, ConnectionPool

logger = util.Util().get_logger("server_log.txt")


pool = ConnectionPool(host='10.10.60.68', port=6379, db=0)
redis = StrictRedis(connection_pool=pool)

get_test_collection = redis.get("test_collection")
if get_test_collection:
    test_collection = json.loads(get_test_collection)
else:
    test_collection = {}


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
            return False


app = FastAPI()
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

test_queue = []

os.makedirs("C:\\core_test", exist_ok=True)

app.mount("/build_out",
          StaticFiles(directory=os.path.join("C:\\core_test")), name="build_out")


@app.get("/core_test/check_queue")
def check_queue():
    return test_queue


@app.get("/core_test/check_collection")
def check_collection():
    return test_collection


@app.get("/core_test/commit_id/{repo}/{commit_id}/{subsys_test}/{ec716e}/{ec718p}/{ec718pm}/{ec718u}/{ec718um}")
def commit_id(repo: str, commit_id: str, subsys_test: bool, ec716e: bool, ec718p: bool, ec718pm: bool, ec718u: bool, ec718um: bool):
    logger.debug("receive core test {} commit id = {}".format(repo, commit_id))
    if len(commit_id) != 40:
        return "Error: Invalid commit id, sha1 lenth is 40"

    if not OpenFlow().check_commit_exist_in_remote_repo(commit_id, repo):
        return "Error: Can not find commit id {} in remote {}_repo's branch or merge request, please push your commit to remote repo".format(commit_id, repo)

    for item in test_queue:
        if commit_id == item:
            return "Commit id {} core test already exist".format(commit_id)
    for key in test_collection.keys():
        if commit_id == key:
            if test_collection.get(key).get("status") == "KILLED":
                break
            return "Commit id {} core test already commit".format(commit_id)
    test_queue.append(commit_id)
    test_collection[commit_id] = {
        "repo": repo, "subsys_test": subsys_test, "ec716e": ec716e, "ec718p": ec718p, "ec718pm": ec718pm, "ec718u": ec718u, "ec718um": ec718um, "submit_time": int(time.time()), "start_time": 0, "status": "WAIT", "pid": 0, "cost_time": 0, "targets": {}}
    redis.set("test_collection", json.dumps(test_collection))
    return "Commit id {} core test submit ok".format(commit_id)


@app.get("/core_test/get_commit_id")
def get_commit_id():
    if len(test_queue) > 0:
        for item in test_queue:
            if test_collection[item]["status"] == "WAIT":
                logger.debug("Send commit id = {}".format(item))
                test_collection[item].update({"start_time": int(time.time())})
                redis.set("test_collection", json.dumps(test_collection))
                return {"commit_id": item, "repo": test_collection[item]["repo"], "subsys_test": test_collection[item]["subsys_test"], "ec716e": test_collection[item]["ec716e"], "ec718p": test_collection[item]["ec718p"], "ec718pm": test_collection[item]["ec718pm"], "ec718u": test_collection[item]["ec718u"], "ec718um": test_collection[item]["ec718um"]}
        else:
            return None
    else:
        return None


@app.get("/core_test/stop/{commit_id}")
def stop(commit_id: str):
    logger.debug("Stop commit id = {}".format(commit_id))
    if test_collection.get(commit_id) is None:
        return "Can not find task with commit id {}".format(commit_id)
    stop_commit_id_status = test_collection.get(commit_id).get("status")
    if stop_commit_id_status == "PASS" or stop_commit_id_status == "FAIL":
        return "Task with commit id {} already finished".format(commit_id)
    prepare_kill_pid = test_collection[commit_id]["pid"]
    logger.debug("Kill pid = {}".format(prepare_kill_pid))
    if prepare_kill_pid > 0:
        try:
            parent = psutil.Process(prepare_kill_pid)
            children = parent.children(recursive=True)
            for child in children:
                child.terminate()
            parent.terminate()
        except Exception as e:
            logger.debug(e)
            logger.error("Kill pid {} fail".format(prepare_kill_pid))
        for target in test_collection[commit_id]["targets"]:
            if test_collection[commit_id]["targets"][target]["status"] == "WAIT" or test_collection[commit_id]["targets"][target]["status"] == "RUNNING":
                test_collection[commit_id]["targets"][target]["status"] = "KILLED"
            if test_collection[commit_id]["targets"][target]["build_status"] == "WAIT" or test_collection[commit_id]["targets"][target]["build_status"] == "RUNNING":
                test_collection[commit_id]["targets"][target]["build_status"] = "KILLED"
            if test_collection[commit_id]["targets"][target]["test_status"] == "WAIT" or test_collection[commit_id]["targets"][target]["test_status"] == "RUNNING":
                test_collection[commit_id]["targets"][target]["test_status"] = "KILLED"
    elif prepare_kill_pid == 0:
        logger.debug("Need stop commit do not running")
    test_collection[commit_id].update({"status": "KILLED"})
    test_collection[commit_id].update({"cost_time": "KILLED"})
    test_queue.remove(commit_id)
    redis.set("test_collection", json.dumps(test_collection))
    return "Stop task with commit id {} ok".format(commit_id)


@app.get("/core_test/report/{commit_id}/{status}")
def report(commit_id: str, status: str):
    # logger.debug("commit_id = {}, status = {}".format(
    #     commit_id, status))
    if status == "finish":
        try:
            test_queue.remove(commit_id)
        except Exception as e:
            return "Finish report commit_id not in queue"
        return "Commit id {} finish ok".format(commit_id)
    if commit_id in test_queue:
        # logger.debug("before test_collection = {}".format(test_collection))
        test_collection[commit_id].update(json.loads(status))
        # logger.debug("after test_collection = {}".format(test_collection))
        redis.set("test_collection", json.dumps(test_collection))
        return "Status report ok"
    else:
        return "Status report commit id not in queue"


@app.get("/core_test/test_result/{commit_id}")
def test_result(commit_id: str):
    logger.debug("Check commit id = {}".format(commit_id))
    if commit_id == "all":
        # logger.debug(test_collection)
        return test_collection
    else:
        status = test_collection.get(commit_id)
        # logger.debug("status = {}".format(status))
        if status == None and commit_id in test_queue:
            return "Commit id {} in test queue wait for test...".format(commit_id)
        if status == None and commit_id not in test_queue:
            return "Can not find commit id {}".format(commit_id)
        return status


@app.get("/core_test/delete/{commit_id}")
def delete(commit_id: str):
    logger.debug("Delete commit id = {}".format(commit_id))
    del test_collection[commit_id]
    redis.set("test_collection", json.dumps(test_collection))
    return "Delete ok"
