#!/usr/bin/env python3
"""
脚本用于把 ext_flash 相关配置写入 FlashTool 的 ini 配置文件。

app 目录下 fs_info.json 的 ext_flash_pwrctl 字段形如:

    "ext_flash_pwrctl": { "need": 1, "gpio": 35, "dly": 2 }

会被写成目标 ini 的 [control] 段:

    ext_flash_pwrctl_need = 1
    ext_flash_pwrctl_gpio = 35
    ext_flash_pwrctl_dly = 2

这三个键由 FlashTool 下发给模组上运行的 agentboot，由它在上电初始化
ext flash 之前拉起对应的 GPIO。baseini 模板里这三行默认是注释状态，
本脚本会把注释行原地改写为生效行。

另外可用 flash_control 字段直写 [control] 段里的任意其它键，例如打开
下发运行时控制参数的总开关（不打开发给 agentboot 的结构体里就没有
ExtFlhPwrCtlNeed，pwrctl 配了也不会生效）:

    "flash_control": { "pr_runtime_ctrl_en": 1, "board_ctl_para_en": 1 }

两个字段都没有时直接跳过，不改动 ini。

用法: python3 patch_ext_flash_ini.py <fs_info.json> <ini> [<ini> ...]
"""

import json
import re
import sys

# (ini 键名, fs_info.json 字段名)
KEYS = (
    ("ext_flash_pwrctl_need", "need"),
    ("ext_flash_pwrctl_gpio", "gpio"),
    ("ext_flash_pwrctl_dly", "dly"),
)

SECTION = "control"
DEFAULT_DLY = 2
KEY_RE = re.compile(r"^[A-Za-z0-9_]+$")


def load_fs_info(fs_info_path):
    try:
        with open(fs_info_path, "r", encoding="utf-8") as f:
            return json.load(f)
    except (OSError, ValueError) as e:
        print(f"错误：无法解析 {fs_info_path}: {e}")
        sys.exit(1)


def load_pwrctl(data, fs_info_path):
    """读取 ext_flash_pwrctl 字段，返回 {ini 键名: 值}，没有则返回 {}"""
    cfg = data.get("ext_flash_pwrctl")
    if cfg is None:
        return {}
    if not isinstance(cfg, dict):
        print(f"错误：{fs_info_path} 的 ext_flash_pwrctl 必须是对象")
        sys.exit(1)

    if "need" not in cfg:
        print(f"错误：{fs_info_path} 的 ext_flash_pwrctl 缺少 need 字段")
        sys.exit(1)

    need = cfg["need"]
    if need not in (0, 1):
        print(f"错误：ext_flash_pwrctl.need 只能是 0 或 1，当前为 {need!r}")
        sys.exit(1)

    if need == 0:
        # 显式关闭时只写 need，不把 gpio/dly 的占位值带进 ini
        return {"ext_flash_pwrctl_need": 0}

    gpio = cfg.get("gpio")
    if not isinstance(gpio, int) or isinstance(gpio, bool):
        print(f"错误：ext_flash_pwrctl.need 为 1 时必须给出整数 gpio，当前为 {gpio!r}")
        sys.exit(1)

    dly = cfg.get("dly", DEFAULT_DLY)
    if not isinstance(dly, int) or isinstance(dly, bool):
        print(f"错误：ext_flash_pwrctl.dly 必须是整数，当前为 {dly!r}")
        sys.exit(1)

    return {
        "ext_flash_pwrctl_need": need,
        "ext_flash_pwrctl_gpio": gpio,
        "ext_flash_pwrctl_dly": dly,
    }


def load_flash_control(data, fs_info_path):
    """读取 flash_control 字段，返回 {ini 键名: 值}，没有则返回 {}"""
    cfg = data.get("flash_control")
    if cfg is None:
        return {}
    if not isinstance(cfg, dict):
        print(f"错误：{fs_info_path} 的 flash_control 必须是对象")
        sys.exit(1)

    out = {}
    for key, val in cfg.items():
        if key.startswith("_"):
            # 下划线开头的当作注释，不写进 ini
            continue
        if not KEY_RE.match(key):
            print(f"错误：flash_control 里的键名非法: {key!r}")
            sys.exit(1)
        if isinstance(val, bool) or not isinstance(val, (int, str)):
            print(f"错误：flash_control.{key} 只能是整数或字符串，当前为 {val!r}")
            sys.exit(1)
        if isinstance(val, str) and ("\n" in val or "\r" in val):
            # 值里带换行会往 ini 里注入额外的键值行。
            # '=' 不算非法：厂商参考配置里就有 atreset = at+ecrst=delay,650。
            print(f"错误：flash_control.{key} 的值不能含换行，当前为 {val!r}")
            sys.exit(1)
        out[key] = val
    return out


def split_eol(line):
    """把一行拆成 (内容, 行尾)，保留原文件的换行风格"""
    body = line.rstrip("\r\n")
    return body, line[len(body):]


def find_section(lines, name):
    """返回 [name] 段的 (起始行号, 结束行号)，结束行号为下一段或文件末尾"""
    start = None
    for i, line in enumerate(lines):
        stripped = line.strip()
        if not (stripped.startswith("[") and stripped.endswith("]")):
            continue
        if start is None:
            if stripped[1:-1].strip() == name:
                start = i
        else:
            return start, i
    if start is None:
        return None, None
    return start, len(lines)


def patch_ini(ini_path, wanted):
    """把 wanted 里的键值写入 ini_path 的 [control] 段（原地修改）"""
    try:
        # newline="" 关闭换行归一化，保证 CRLF 原样读写（这些 ini 给 Windows 版 FlashTool 用）
        with open(ini_path, "r", encoding="utf-8", newline="") as f:
            lines = f.readlines()
    except OSError as e:
        print(f"错误：无法读取 {ini_path}: {e}")
        sys.exit(1)

    start, end = find_section(lines, SECTION)
    if start is None:
        print(f"错误：{ini_path} 中找不到 [{SECTION}] 段")
        sys.exit(1)

    body = lines[start + 1:end]
    eol = "\n"
    for line in lines:
        _, line_eol = split_eol(line)
        if line_eol:
            eol = line_eol
            break

    changed = []
    missing = dict(wanted)

    for i, line in enumerate(body):
        content, line_eol = split_eol(line)
        # 兼容 "key = v" / ";key = v" / "; key = v" 三种写法
        m = re.match(r"^\s*;?\s*([A-Za-z0-9_]+)\s*=", content)
        if not m:
            continue
        key = m.group(1)
        if key not in missing:
            continue
        new_line = f"{key} = {missing.pop(key)}{line_eol or eol}"
        if new_line != line:
            body[i] = new_line
            changed.append(key)

    # 段里没有的键，插到该段最后一个非空行之后
    if missing:
        insert_at = 0
        for i, line in enumerate(body):
            if line.strip():
                insert_at = i + 1
        # 按 wanted 的顺序插入，保证多次运行结果稳定
        for key in wanted:
            if key not in missing:
                continue
            body.insert(insert_at, f"{key} = {missing[key]}{eol}")
            insert_at += 1
            changed.append(key)

    if not changed:
        print(f"{ini_path}: 配置已是目标值，无改动")
        return

    lines[start + 1:end] = body
    try:
        with open(ini_path, "w", encoding="utf-8", newline="") as f:
            f.writelines(lines)
    except OSError as e:
        print(f"错误：无法写入 {ini_path}: {e}")
        sys.exit(1)

    print(f"{ini_path}: 已写入 {', '.join(changed)}")


def main():
    if len(sys.argv) < 3:
        print("用法: python3 patch_ext_flash_ini.py <fs_info.json> <ini> [<ini> ...]")
        sys.exit(1)

    fs_info_path = sys.argv[1]
    data = load_fs_info(fs_info_path)

    # 顺序固定：先 pwrctl 三键，再 flash_control（按键名排序）
    wanted = load_pwrctl(data, fs_info_path)
    extra = load_flash_control(data, fs_info_path)
    for key in sorted(extra):
        wanted[key] = extra[key]

    if not wanted:
        print(f"{fs_info_path}: 未配置 ext_flash_pwrctl / flash_control，跳过")
        return

    for key, val in wanted.items():
        print(f"  {key} = {val}")

    for ini_path in sys.argv[2:]:
        patch_ini(ini_path, wanted)


if __name__ == "__main__":
    main()
