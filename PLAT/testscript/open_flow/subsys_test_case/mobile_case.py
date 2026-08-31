case_map = {
    "dial": {"cmd": b"mobile dial 10010\r\n", "match": None, "re": False, "wait": 10},
    # "vts": {"cmd": b"mobile vts 1\r\n", "match": None, "re": False},
    "hang_up": {"cmd": b"mobile hang_up\r\n", "match": None, "re": False},
    # "answer": {"cmd": b"mobile answer\r\n", "match": "0 张三 123\r\n1 李四 456\r\n2 王五 789", "re": False},
    # "sms": {"cmd": b"mobile sms 10010 10010\r\n", "match": None, "re": False},
}
