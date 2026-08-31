case_map = {
    "sync": {"cmd": b"time sync\r\n", "match": None, "re": False, "wait": 10},
    "set_time_zone": {"cmd": b"time set_time_zone 8\r\n", "match": None, "re": False},
    "get_time_zone": {"cmd": b"time get_time_zone\r\n", "match": "8", "re": False},
    "mktime": {"cmd": b"time mktime 2024 9 25 14 30 00\r\n", "match": "1727245800", "re": False},
    "get_current_unix_time": {"cmd": b"time get_current_unix_time\r\n", "match": None, "re": False},
    "get_current_ctime": {"cmd": b"time get_current_ctime\r\n", "match": None, "re": False},
    "get_current_gmtime": {"cmd": b"time get_current_gmtime\r\n", "match": None, "re": False},
    "get_current_local_time": {"cmd": b"time get_current_local_time\r\n", "match": None, "re": False},
    "get_gmtime_from_unix_time": {"cmd": b"time get_gmtime_from_unix_time 1727248438\r\n", "match": "2024-9-25 7:13:58 3", "re": False},
    "get_local_time_from_unix_time": {"cmd": b"time get_local_time_from_unix_time 1727248438\r\n", "match": "2024-9-25 15:13:58 3", "re": False},
    "get_diff_time_between_unix_time": {"cmd": b"time get_diff_time_between_unix_time 1727248438 1727248439\r\n", "match": "1", "re": False},
}
