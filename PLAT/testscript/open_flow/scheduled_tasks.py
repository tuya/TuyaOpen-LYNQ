import schedule
import time
import requests
import subprocess

server_ip = "10.10.60.111"
# server_ip = "127.0.0.1"


def job():
    p = subprocess.run(
        "git ls-remote ssh://git@10.10.60.68:2222/open_eigencomm/ec718_716_sdk_release.git origin main", shell=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, encoding="utf-8", errors="ignore")
    print(p.stdout[:40])

    response = requests.get(
        "http://{}:8000/core_test/commit_id/{}/{}/{}".format(server_ip, "release", p.stdout[:40], True))

    print(response.text)


# schedule.every(1).seconds.do(job)

schedule.every().day.at("01:00").do(job)


while True:
    schedule.run_pending()
    time.sleep(1)
