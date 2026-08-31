case_map = {
    "play_pcm": {"cmd": b"audio play d:/16k.pcm\r\n", "match": None, "re": False, "wait": 6},
    "play_wav": {"cmd": b"audio play d:/44k.wav\r\n", "match": None, "re": False, "wait": 6},
    "play_mp3": {"cmd": b"audio play d:/44k192k.mp3\r\n", "match": None, "re": False, "wait": 3},
    "stop_play": {"cmd": b"audio stop_play\r\n", "match": None, "re": False},
    "record_5s": {"cmd": b"audio record d:/record.amr 5\r\n", "match": None, "re": False, "wait": 6},
    "play_amr_5s": {"cmd": b"audio play d:/record.amr\r\n", "match": None, "re": False, "wait": 6},
    "record": {"cmd": b"audio record d:/record.amr\r\n", "match": None, "re": False, "wait": 5},
    "stop_record": {"cmd": b"audio stop_record\r\n", "match": None, "re": False},
    "play_amr": {"cmd": b"audio play d:/record.amr\r\n", "match": None, "re": False, "wait": 6},
    # "pause_record": {"cmd": b"audio pause_record\r\n", "match": None, "re": False},
    # "resume_record": {"cmd": b"audio resume_record\r\n", "match": None, "re": False},
    "get_record_time": {"cmd": b"audio get_record_time\r\n", "match": "5", "re": False},
}
