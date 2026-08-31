case_map = {
    "jsondb_create": {"cmd": b"storage jsondb_create\r\n", "match": None, "re": False},
    "jsondb_add": {"cmd": b"storage jsondb_add\r\n", "match": None, "re": False},
    "jsondb_get_total_count": {"cmd": b"storage jsondb_get_total_count\r\n", "match": "3", "re": False},
    "jsondb_get_all_items": {"cmd": b"storage jsondb_get_all_items\r\n", "match": "0 张三 123\r\n1 李四 456\r\n2 王五 789", "re": False},
    "jsondb_update": {"cmd": b"storage jsondb_update\r\n", "match": None, "re": False},
    "jsondb_get_item_by_id": {"cmd": b"storage jsondb_get_item_by_id 1\r\n", "match": "李23四 654", "re": False},
    "jsondb_delete": {"cmd": b"storage jsondb_delete\r\n", "match": None, "re": False},
    "jsondb_get_json": {"cmd": b"storage jsondb_get_json\r\n", "match": "\"name\":	\"json_db_test\",", "re": False},
    "jsondb_get_type": {"cmd": b"storage jsondb_get_type\r\n", "match": "name: 40\r\nnumber: 40", "re": False},
    "jsondb_match": {"cmd": b"storage jsondb_match 123\r\n", "match": "张三: 123 (0)", "re": False},
}
