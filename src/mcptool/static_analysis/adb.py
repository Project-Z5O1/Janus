"""
ADB 工具封装模块

该模块封装了 Android Debug Bridge (ADB) 的常用操作，提供了以下功能：
- 设备连接和状态检查
- 命令执行（普通/特权模式）
- 文件推送和拉取
- 设备重启和等待
- 进程管理（pgrep, pidof, pkill, kill）

使用前提：
1. 系统已安装 adb 并添加到 PATH
2. Android 设备已 root（部分功能需要）
3. 设备已安装 Termux（部分编译功能需要）
"""

import subprocess
import time
import logging
import sys
import os
import string
import random
import traceback

log = logging.getLogger(__name__)


class DeviceTimeoutException(Exception):
    """设备超时异常：当 ADB 命令执行超时时抛出"""
    pass


class DeviceUnresponsiveException(Exception):
    """设备无响应异常：当设备长时间无响应时抛出"""
    pass


class ADBSetTimeException(Exception):
    """设置时间异常：当无法同步设备时间时抛出"""
    pass


class ADBPushNoFile(Exception):
    """文件不存在异常：当尝试推送不存在的文件时抛出"""
    pass


def subprocess_adb(cmd, device_id=None, wait_for_termination=True):
    """
    创建 ADB 子进程
    
    Args:
        cmd: ADB 命令参数列表
        device_id: 目标设备 ID（可选，用于多设备场景）
        wait_for_termination: 是否等待命令完成（未使用）
    
    Returns:
        subprocess.Popen: ADB 子进程对象
    """
    if device_id:
        cmd.insert(0, device_id)
        cmd.insert(0, "-s")
    cmd.insert(0, "adb")
    try:
        p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE, bufsize=0)
    except OSError as e:
            print(traceback.format_exc())
            log.error("Did you add adb to your $PATH?")
            sys.exit(1)
    return p


def call_adb(cmd, device_id=None, wait_for_termination=True, timeout=None):
    """
    调用 ADB 命令并等待结果
    
    Args:
        cmd: ADB 命令参数列表
        device_id: 目标设备 ID（可选）
        wait_for_termination: 是否等待命令完成
        timeout: 超时时间（秒）
    
    Returns:
        tuple: (stdout, stderr) 命令输出
    
    Raises:
        DeviceTimeoutException: 命令执行超时
    """
    if device_id:
        logging.debug(f"{device_id} calling adb function with command: " + str(cmd))
    else:
        logging.debug(f"calling adb function with command: " + str(cmd))
    p = subprocess_adb(cmd, device_id, wait_for_termination)
    if not wait_for_termination:
        return None,None
    try:
        out, err = p.communicate(timeout=timeout)
        p.wait()
    except subprocess.TimeoutExpired:
        p.terminate()
        raise DeviceTimeoutException(f"Device {device_id} timed out on command after {timeout}s")
    return out, err


def set_date(device_id=None):
    """
    同步设备时间
    
    将设备的日期和时区设置为与主机相同的值。
    注意：此函数是针对 Android 上的 'toybox date' 编写的。
    
    Args:
        device_id: 目标设备 ID（可选）
    
    Returns:
        tuple: (stdout, stderr) 命令输出
    
    Raises:
        NotImplementedError: 主机时区配置不支持
        ADBSetTimeException: 无法设置设备时间
    """
    # get timezone from host
    if not os.path.exists("/etc/timezone"):
        raise NotImplementedError("Adapt to your host.")
    with open("/etc/timezone") as f:
        timezone = f.read().strip()
    if not timezone:
        raise NotImplementedError("Adapt to your host.")
    # set time on device
    current_time = time.localtime()
    formatted_time = time.strftime('%m%d%H%M%y.%S', current_time)
    # TODO: get TZ from HOST
    out, err = execute_privileged_command("TZ=CEST date {} && TZ=CEST date +%Y%m%d%H%M".format(formatted_time), device_id, 5)
    # check if we are in sync now
    out_lines = out.split(b"\n")
    if len(out_lines) < 2:
        raise NotImplementedError("This implementation needs to be tuned.")

    if time.strftime('%Y%m%d%H%M', current_time).encode() != out_lines[1].strip():
        raise ADBSetTimeException("Could not set time on device.")

    return out, err


def subprocess_privileged(cmd, device_id=None):
    """
    创建特权命令子进程
    
    使用 su 执行需要 root 权限的命令。
    
    Args:
        cmd: 要执行的命令字符串
        device_id: 目标设备 ID（可选）
    
    Returns:
        subprocess.Popen: 子进程对象，失败时返回 None
    """
    p = subprocess_adb(["shell", "su -c '{}'".format(cmd)], device_id=device_id)
    time.sleep(2)
    if p.poll():
        p = subprocess_adb(["shell", "su root {}".format(cmd)], device_id=device_id)
    if p.poll():
        log.error("error creating privileged subprocess.")
        p = None
    return p


def execute_privileged_command(cmd, device_id, wait_for_termination=True, timeout=None):
    """
    在设备上执行特权命令
    
    使用 root 权限执行命令。会根据设备类型（模拟器/真机）选择不同的 su 调用方式。
    
    Args:
        cmd: 要执行的命令字符串
        device_id: 目标设备 ID
        wait_for_termination: 是否等待命令完成
        timeout: 超时时间（秒）
    
    Returns:
        tuple: (stdout, stderr) 命令输出
    """
    if (type(device_id) == bytes and b'emulator' in device_id) or (type(device_id) == str and 'emulator' in device_id):
        out, err = call_adb(["shell", "su root sh -c '{}'".format(cmd)], device_id=device_id, wait_for_termination=wait_for_termination, timeout=timeout)
    else:
        out, err = call_adb(["shell", "su -c '{}'".format(cmd)], device_id=device_id, wait_for_termination=wait_for_termination, timeout=timeout)
    if err:
        log.debug("{} error executing privileged cmd. err: {}".format(device_id, err))
    return out, err


def execute_nobody_command(cmd, device_id, wait_for_termination=True, timeout=None):
    """
    以 nobody 用户身份执行命令
    
    使用 nobody 用户权限执行命令，用于降权运行。
    
    Args:
        cmd: 要执行的命令字符串
        device_id: 目标设备 ID
        wait_for_termination: 是否等待命令完成
        timeout: 超时时间（秒）
    
    Returns:
        tuple: (stdout, stderr) 命令输出
    """
    if (type(device_id) == bytes and b'emulator' in device_id) or (type(device_id) == str and 'emulator' in device_id):
        out, err = call_adb(["shell", "su nobody sh -c '{}'".format(cmd)], device_id=device_id, wait_for_termination=wait_for_termination, timeout=timeout)
    else:
        out, err = call_adb(["shell", "su nobody -c '{}'".format(cmd)], device_id=device_id, wait_for_termination=wait_for_termination, timeout=timeout)
    if err:
        log.debug("{} error executing nobody cmd. err: {}".format(device_id, err))
    return out, err


def execute_command(cmd, device_id, wait_for_termination=True, timeout=None):
    """
    在设备上执行普通命令
    
    不需要 root 权限的普通 shell 命令执行。
    
    Args:
        cmd: 要执行的命令字符串
        device_id: 目标设备 ID（可选，用于 adb -s 选项）
        wait_for_termination: 是否等待命令完成
        timeout: 超时时间（秒）
    
    Returns:
        tuple: (stdout, stderr) 命令输出
    """
    out, err = call_adb(["shell", "{}".format(cmd)], device_id=device_id, wait_for_termination=wait_for_termination, timeout=timeout)
    return out, err


def remove_forward(device_id=None):
    """
    移除端口转发
    
    移除之前设置的 ADB 端口转发规则。
    
    Args:
        device_id: 目标设备 ID（可选）
    
    Returns:
        bytes: 命令输出
    """
    out, err = call_adb(["forward", "--remove"], device_id=device_id)
    return out


def forward(lport, dport, device_id=None):
    """
    设置端口转发
    
    将本地端口转发到设备端口。
    
    Args:
        lport: 本地端口号
        dport: 设备端口号
        device_id: 目标设备 ID（可选）
    
    Returns:
        bytes: 命令输出
    """
    out, err = call_adb(["forward", "tcp:{}".format(lport), "tcp:{}".format(dport)], device_id=device_id)
    return out


def list_devices():
    """
    列出已连接的设备
    
    Returns:
        bytes: adb devices 命令输出
    """
    out, err = call_adb(["devices"])
    return out


def get_device_ids():
    """
    获取设备 ID 列表
    
    只返回状态为 "device"（可访问）的设备 ID。
    
    Returns:
        list: 设备 ID 列表（bytes 类型）
    """
    ids = []
    out = list_devices()
    lines = out.split(b"\n")
    for line in lines:
        tokens = line.split(b"\t")
        if len(tokens) == 2 and tokens[1] == b"device":
            ids.append(tokens[0])
    return ids


def get_logcat(device_id):
    """
    获取 logcat 日志
    
    获取设备的完整 logcat 日志（不带过滤器）。
    
    Args:
        device_id: 目标设备 ID
    
    Returns:
        bytes: logcat 日志内容
    """
    out, err = call_adb(["logcat", "-d"], device_id=device_id)
    return out


def push(what, where, device_id=None):
    """
    推送文件到设备
    
    将本地文件推送到设备指定路径。
    
    Args:
        what: 本地文件路径
        where: 设备目标路径
        device_id: 目标设备 ID（可选）
    
    Returns:
        bytes: 命令输出
    """
    out, err = call_adb(["push", what, where], device_id=device_id)
    return out


def push_privileged(what, where, is_directory=False, device_id=None):
    """
    以特权模式推送文件到设备
    
    通过临时目录中转，将文件推送到需要 root 权限的目录。
    解决普通 adb push 无法写入受保护目录的问题。
    
    Args:
        what: 本地文件/目录路径
        where: 设备目标路径
        is_directory: 是否为目录
        device_id: 目标设备 ID（可选）
    
    Returns:
        tuple: (stdout, stderr) 命令输出
    
    Raises:
        ADBPushNoFile: 源文件不存在
    """
    if not os.path.exists(what):
        logging.error(f"trying to push non-existent file: {what}")
        raise(ADBPushNoFile(f'trying to push non-existent file: {what}'))
        return None, "no_file"
    """ Push to the device """
    rand_str = list(string.ascii_letters)
    random.shuffle(rand_str)
    workdir = os.path.join("/data/local/tmp", "".join(rand_str[:10]))
    
    what_file = os.path.basename(what)
    workdir_file = os.path.join(workdir, what_file)
    
    execute_privileged_command(" ".join(["mkdir", workdir]), device_id=device_id)
    execute_privileged_command(" ".join(["chown", "shell:shell", workdir]), device_id=device_id)
    
    out, err = call_adb(["push", what, workdir], device_id=device_id)
    
    if is_directory:
        execute_privileged_command(" ".join(["chown", "-R", "shell:shell", workdir_file]), device_id=device_id)
        execute_privileged_command(" ".join(["cp", "-r", workdir_file, where]), device_id=device_id)
    else:
        execute_privileged_command(" ".join(["chown", "shell:shell", workdir_file]), device_id=device_id)
        execute_privileged_command(" ".join(["cp", workdir_file, where]), device_id=device_id)

    execute_privileged_command(" ".join(["rm", "-rf", workdir]), device_id=device_id)

    return out, err


def pull(what, where, device_id=None):
    """
    从设备拉取文件
    
    将设备上的文件拉取到本地。
    
    Args:
        what: 设备文件路径
        where: 本地目标路径
        device_id: 目标设备 ID（可选）
    
    Returns:
        bytes: 命令输出
    """
    out, err = call_adb(["pull", what, where], device_id=device_id)
    return out


def pull_privileged(what, where, is_directory=False, device_id=None):
    """
    以特权模式从设备拉取文件
    
    通过临时目录中转，从需要 root 权限才能访问的目录拉取文件。
    
    Args:
        what: 设备文件/目录路径
        where: 本地目标路径
        is_directory: 是否为目录
        device_id: 目标设备 ID（可选）
    
    Returns:
        bytes: 命令输出
    """
    rand_str = list(string.ascii_letters)
    random.shuffle(rand_str)
    workdir = os.path.join("/data/local/tmp", "".join(rand_str[:10]))
    what_file = os.path.basename(what)
    workdir_file = os.path.join(workdir, what_file)

    execute_privileged_command(" ".join(["mkdir", workdir]), device_id=device_id)
    if is_directory:
        execute_privileged_command(" ".join(["cp", "-r", what, workdir]), device_id=device_id)
        execute_privileged_command(" ".join(["chown", "-R", "shell:shell", workdir_file]), device_id=device_id)
    else:
        execute_privileged_command(" ".join(["cp", what, workdir]), device_id=device_id)
        execute_privileged_command(" ".join(["chown", "shell:shell", workdir_file]), device_id=device_id)
    
    out, err = call_adb(["pull", workdir_file, where], device_id=device_id)
    execute_privileged_command(" ".join(["rm", "-rf", workdir]), device_id=device_id)

    return out


def reboot(device_id):
    """
    重启设备
    
    注意：在较新的设备上，adb shell "reboot" 返回可能需要更长时间。
    
    Args:
        device_id: 目标设备 ID
    
    Returns:
        bytes: 命令输出
    """
    out, err = call_adb(["reboot"], device_id=device_id, timeout=20)
    return out


def reboot_recovery(device_id=None):
    """
    重启设备进入 Recovery 模式
    
    Args:
        device_id: 目标设备 ID（可选）
    
    Returns:
        bytes: 命令输出
    """
    out, err = call_adb(["reboot", "recovery"], device_id=device_id)
    return out


def wait_device_booted(device_id):
    """
    等待设备启动完成
    
    检查 init.svc.bootanim 属性，直到状态为 "stopped"，表示系统完全启动。
    
    Args:
        device_id: 目标设备 ID
    
    Returns:
        bool: 启动成功返回 True
    
    Raises:
        DeviceUnresponsiveException: 设备长时间无响应
    """
    boot_status = ""
    nretry = 12
    while True:
        try:
            #out, _ = execute_command("getprop init.svc.bootanim", device_id, timeout=5)
            out, _ = execute_command("pwd", device_id=device_id, timeout=5)
            boot_status = out
        except DeviceUnresponsiveException:
            nretry -= 1
            if not nretry:
                raise

        if b"/" in boot_status:
            break

        if b"stopped" in boot_status:
            break

        if b"running" in boot_status:
            log.info("boot anim running...")
        else:
            nretry -= 1
            log.info("no response, retrying...")
            if nretry <= 0:
                raise DeviceUnresponsiveException("Device {} does not respond.".format(device_id))
        time.sleep(10)
    return True


def is_device_ready(device_id):
    """
    检查设备是否就绪
    
    等待设备启动完成，并验证 pwd 命令可以正常执行。
    
    Args:
        device_id: 目标设备 ID
    
    Returns:
        bool: 设备就绪返回 True，否则返回 False
    """
    device_ready = False
    if wait_device_booted(device_id):
        out, _ = execute_command('pwd', device_id=device_id, timeout=5)
        if out.strip() == b"/":
            device_ready = True
    return device_ready


def is_booted(device_id):
    """
    检查设备是否已启动
    
    循环检测设备是否正确启动，通过执行 pwd 命令验证。
    最多重试 20 次，每次间隔 10 秒。
    
    Args:
        device_id: 目标设备 ID
    
    Returns:
        bool: 设备已启动返回 True
    
    Raises:
        DeviceUnresponsiveException: 设备长时间无响应
    """
    device_ready = False
    nretry = 20
    while True:
        try:
            out, _ = execute_command("pwd", device_id=device_id, timeout=5)
            if out.strip() == b"/":
                device_ready = True
                break
        except DeviceUnresponsiveException:
            nretry -= 1
            if not nretry:
                raise
        nretry -= 1
        log.info("no response, retrying...")
        if nretry <= 0:
            raise DeviceUnresponsiveException("Device {} does not respond.".format(device_id))
        time.sleep(10)
    return device_ready


def program_exists(program_name, device_id=None):
    """
    检查程序是否存在
    
    使用 which 命令检查指定程序是否在设备的 PATH 中。
    
    Args:
        program_name: 程序名称
        device_id: 目标设备 ID（可选）
    
    Returns:
        bool: 程序存在返回 True
    """
    out, _ = execute_command("which {}".format(program_name), device_id)
    if program_name.encode() in out:
        return True
    return False


def path_exists(path, device_id=None):
    """
    检查路径是否存在
    
    使用 ls 命令检查设备上的路径是否存在。
    
    Args:
        path: 要检查的路径
        device_id: 目标设备 ID（可选）
    
    Returns:
        bool: 路径存在返回 True
    """
    out, err = execute_command("ls {}".format(path), device_id=device_id)
    if b"No such file" in err or b"No such file" in out:
        return False
    return True


def pgrep(process_name, device_id=None):
    """
    通过进程名搜索进程
    
    使用 pgrep 命令搜索匹配的进程。
    
    Args:
        process_name: 进程名称
        device_id: 目标设备 ID（可选）
    
    Returns:
        bytes: pgrep 输出（进程 ID 列表）
    
    Raises:
        NotImplementedError: 设备上没有 pgrep 命令
    """
    out = b""
    if program_exists("pgrep", device_id):
        tmp_out, _ = execute_privileged_command("pgrep {}".format(process_name), device_id=device_id)
        out += tmp_out
    else:
        raise NotImplementedError("Extend cmds to pgrep processes here.")
    return out


def pidof(process_name, device_id=None):
    """
    获取进程 ID
    
    使用 pidof 命令获取指定进程名的 PID。
    
    Args:
        process_name: 进程名称
        device_id: 目标设备 ID（可选）
    
    Returns:
        bytes: 进程 ID
    
    Raises:
        NotImplementedError: 设备上没有 pidof 命令
    """
    out = b""
    if program_exists("pidof", device_id):
        tmp_out, _ = execute_privileged_command(f"pidof {process_name}",
                                                device_id=device_id)
        out += tmp_out
    else:
        raise NotImplementedError("Extend cmds to pidof processes here.")
    return out


def pkill(process_name, device_id=None):
    """
    通过进程名杀死进程
    
    使用 pkill 命令杀死所有匹配的进程。
    
    Args:
        process_name: 进程名称
        device_id: 目标设备 ID（可选）
    
    Raises:
        NotImplementedError: 设备上没有 pkill 命令
    """
    if program_exists("pkill", device_id):
        execute_privileged_command(f"pkill {process_name}", device_id=device_id)
    else:
        raise NotImplementedError("Extend cmds to pkill processes here.")


def kill(pid, device_id=None):
    """
    通过 PID 杀死进程
    
    使用 kill -9 强制杀死指定 PID 的进程。
    
    Args:
        pid: 进程 ID
        device_id: 目标设备 ID（可选）
    
    Raises:
        NotImplementedError: 设备上没有 kill 命令
    """
    if program_exists("kill", device_id):
        execute_privileged_command(f"kill -9 {pid}", device_id=device_id)
    else:
        raise NotImplementedError("Extend cmds to kill processes here.")


def check_device(device_id, check_space=False):
    """
    检查设备状态
    
    验证设备是否满足运行要求：
    1. 具有 root 权限
    2. 可以推送文件
    3. 安装了 g++ 编译器（Termux）
    4. （可选）检查存储空间
    
    Args:
        device_id: 目标设备 ID
        check_space: 是否检查存储空间
    
    Returns:
        str: "OK" 表示正常，其他值表示错误类型：
            - "NOROOT": 没有 root 权限
            - "NOPUSH": 无法推送文件
            - "NOG++": 没有安装 g++ 编译器
            - "NOSPACE_DATA": /data 分区空间不足
    """
    out, err = execute_privileged_command("id", device_id=device_id)
    if b"root" not in out:
        return "NOROOT"
    # Use requirements.txt from the project directory for testing push functionality
    base_path = os.path.dirname(os.path.abspath(__file__))
    test_file = os.path.join(base_path, "requirements.txt")
    out, err = push_privileged(test_file, "/data/local/tmp", device_id=device_id)
    if b"failed to copy" in out or b"failed to copy" in err:
        return "NOPUSH"
    out, err = execute_privileged_command("/data/data/com.termux/files/usr/bin/g++", device_id=device_id)
    if b"error: no input files" not in err:
        return "NOG++"
    if check_space:
        out, _ = execute_privileged_command('df -h /data | cut -d " "  -f 6', device_id=device_id)
        if float(out.strip(b'\n')[:-1]) < 3:
            return "NOSPACE_DATA"
    return "OK"
