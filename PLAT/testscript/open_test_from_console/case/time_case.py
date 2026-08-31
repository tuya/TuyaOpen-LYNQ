def sync_match(response):
    pass


case_map = {"sync": {"cmd": b'time sync\r\n', "match": "0", "re": False},
            "get_gmtime_from_unix_time": {"cmd": b'time get_gmtime_from_unix_time 1708911000\r\n', "match": "2024-2-26 1:30:0 1", "re": False}, "get_local_time_from_unix_time": {"cmd": b'time get_local_time_from_unix_time 1708911000\r\n', "match": "2024-2-26 9:30:0 1", "re": False}, "get_diff_time_between_unix_time": {"cmd": b'time get_diff_time_between_unix_time 1 100\r\n', "match": "99", "re": False}}
