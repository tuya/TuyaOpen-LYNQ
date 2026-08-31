case_map = {
    # "create": {"cmd": b"gpio create 28\r\n", "match": None, "re": False},
    # "create_all": {"cmd": b"gpio create_all\r\n", "match": None, "re": False},
    # "delete": {"cmd": b"gpio delete 28\r\n", "match": "3", "re": False},
    # "delete_all": {"cmd": b"gpio delete_all\r\n", "match": "0 张三 123\r\n1 李四 456\r\n2 王五 789", "re": False},
    "write_1": {"cmd": b"gpio write 28 1\r\n", "match": None, "re": False},
    "write_0": {"cmd": b"gpio write 28 0\r\n", "match": None, "re": False},
    # "read": {"cmd": b"gpio read 28\r\n", "match": "李23四 654", "re": False},
    # "write_all": {"cmd": b"gpio write_all 0\r\n", "match": None, "re": False},
    # "interrupt": {"cmd": b"gpio interrupt 28\r\n", "match": "\"name\":	\"json_db_test\",", "re": False},
}
