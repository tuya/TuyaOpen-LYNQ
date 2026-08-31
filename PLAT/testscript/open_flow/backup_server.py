import os
import subprocess
import multiprocessing


from fastapi import FastAPI
from common import util
from fastapi.staticfiles import StaticFiles
from fastapi.middleware.cors import CORSMiddleware

git_server_ip = "10.10.60.69"
git_server_ip = "127.0.0.1"

logger = util.Util().get_logger("backup_server_log.txt")


archives_dir = "/mnt/data/archives"
repo_url = "ssh://git@{}:2222/open_eigencomm/ec718_716_sdk_release.git".format(
    git_server_ip)
repo_dir_name = "ec718_716_sdk_release"


app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

os.makedirs(archives_dir, exist_ok=True)
app.mount("/archives",
          StaticFiles(directory=archives_dir), name="archives")


def checkout(repo: str, commit_id: str, backup_server_log_file) -> int:
    global repo_url, repo_dir_name
    os.makedirs(os.path.join(archives_dir, "release"), exist_ok=True)
    os.makedirs(os.path.join(archives_dir, "original"), exist_ok=True)

    clone_cmd = "cd {} && rm -rf {} && git clone {} && cd {} && git checkout {}".format(
        os.path.join(archives_dir, repo), repo_dir_name, repo_url, repo_dir_name, commit_id)

    logger.debug("clone_cmd = {}".format(clone_cmd))
    logger.debug("clone repo...")
    p = subprocess.run(clone_cmd, shell=True,
                       text=True, stdout=backup_server_log_file, stderr=backup_server_log_file, encoding="utf-8", errors="ignore")

    return p.returncode


def backup_process(repo: str, commit_id: str):
    global repo_url, repo_dir_name
    with open("/mnt/data/archives/backup_server_log.txt", mode='a', encoding="utf-8", errors="ignore") as backup_server_log_file:
        if repo == "release":
            repo_url = "ssh://git@{}:2222/open_eigencomm/ec718_716_sdk_release.git".format(
                git_server_ip)
            repo_dir_name = "ec718_716_sdk_release"
        elif repo == "original":
            repo_url = "ssh://git@{}:2222/open_eigencomm/ec7xx_sdk_org.git".format(
                git_server_ip)
            repo_dir_name = "ec7xx_sdk_org"
        else:
            logger.debug("Only support release and original repo")
            return

        if checkout(repo, commit_id, backup_server_log_file) != 0:
            logger.debug("checkout fail")
            return
        else:
            logger.debug("checkout success")

        backup_cmd = "cd {} && rm -rf {}/.git && zip -r {}.zip {} && rm -rf {}".format(
            os.path.join(archives_dir, repo), repo_dir_name, commit_id, repo_dir_name, repo_dir_name)
        logger.debug("backup_cmd = {}".format(backup_cmd))
        logger.debug("backup repo...")
        p = subprocess.run(backup_cmd, shell=True,
                           text=True, stdout=backup_server_log_file, stderr=backup_server_log_file, encoding="utf-8", errors="ignore")
        if p.returncode != 0:
            logger.debug("backup fail")
            return
        else:
            logger.debug("backup success")


@app.get("/backup/{repo}/{commit_id}")
def backup(repo: str, commit_id: str):
    p = multiprocessing.Process(target=backup_process, args=(repo, commit_id))
    p.start()
    # p.join()
    return "backup start"


@app.get("/backup_collection")
def backup_collection():

    original_dir = '/mnt/data/archives/original'
    release_dir = '/mnt/data/archives/release'

    original_files = [os.path.join(original_dir, file)
                      for file in os.listdir(original_dir)]

    release_files = [os.path.join(release_dir, file)
                     for file in os.listdir(release_dir)]

    return original_files + release_files
