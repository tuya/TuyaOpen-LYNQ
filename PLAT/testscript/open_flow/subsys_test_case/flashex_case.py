case_map = {
    "write": {"cmd": b"storage write d:/test.txt w+ 123456\r\n", "match": None, "re": False},
    "read": {"cmd": b"storage read d:/test.txt\r\n", "match": "read size = 6\r\n123456", "re": False},
    "ls": {"cmd": b"storage ls d\r\n", "match": "test.txt	6B", "re": False},
    "seek_write": {"cmd": b"storage seek_write d:/test.txt r+ 2 0 999999\r\n", "match": "file position = 2", "re": False},
    "seek_write_verify": {"cmd": b"storage read d:/test.txt\r\n", "match": "12999999", "re": False},
    "rewind_write": {"cmd": b"storage rewind_write d:/test.txt r+ zzz\r\n", "match": "file position = 0", "re": False},
    "rewind_write_verify": {"cmd": b"storage read d:/test.txt\r\n", "match": "zzz99999", "re": False},
    "fsstat": {"cmd": b"storage fsstat d:/test.txt\r\n", "match": "read size = 8", "re": False},
    "truncate": {"cmd": b"storage truncate d:/test.txt r+ 2\r\n", "match": None, "re": False},
    "truncate_verify": {"cmd": b"storage read d:/test.txt\r\n", "match": "zz", "re": False},
    "rm": {"cmd": b"storage rm d:/test.txt\r\n", "match": None, "re": False},
    # "format": {"cmd": b"storage format d\r\n", "match": "fileName:d:/a.txt already delete", "re": False},
}
