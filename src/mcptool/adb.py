"""
ADB MCP Tools Module

This module wraps Android Debug Bridge (ADB) common operations using FastMCP, providing:
- Device connection and status checking
- Command execution (normal/privileged mode)
- File push and pull
- Device reboot and wait
- Process management (pgrep, pidof, pkill, kill)

Prerequisites:
1. ADB installed and added to PATH
2. Android device rooted (required for some features)
3. Termux installed on device (required for some compilation features)
"""

import subprocess
import time
import logging
import sys
import os
import string
import random
import traceback
from typing import Optional
from fastmcp import FastMCP

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
log = logging.getLogger(__name__)

# Create FastMCP instance
mcp = FastMCP(name="ADB Tools",instructions="""
        Android Debug Bridge (ADB) toolkit for operating Android devices.
    """,
    version="0.0.1",)


# ============== Exception Classes ==============

class DeviceTimeoutException(Exception):
    """Device timeout exception: raised when ADB command execution times out"""
    pass


class DeviceUnresponsiveException(Exception):
    """Device unresponsive exception: raised when device is unresponsive for too long"""
    pass


class ADBSetTimeException(Exception):
    """Set time exception: raised when unable to sync device time"""
    pass


class ADBPushNoFile(Exception):
    """File not found exception: raised when trying to push a non-existent file"""
    pass


# ============== Internal Helper Functions ==============

def subprocess_adb(cmd, device_id=None, wait_for_termination=True):
    """
    Create ADB subprocess
    
    Args:
        cmd: ADB command argument list
        device_id: Target device ID (optional, for multi-device scenarios)
        wait_for_termination: Whether to wait for command completion (unused)
    
    Returns:
        subprocess.Popen: ADB subprocess object
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
    Call ADB command and wait for result
    
    Args:
        cmd: ADB command argument list
        device_id: Target device ID (optional)
        wait_for_termination: Whether to wait for command completion
        timeout: Timeout in seconds
    
    Returns:
        tuple: (stdout, stderr) command output
    
    Raises:
        DeviceTimeoutException: Command execution timed out
    """
    if device_id:
        logging.debug(f"{device_id} calling adb function with command: " + str(cmd))
    else:
        logging.debug(f"calling adb function with command: " + str(cmd))
    p = subprocess_adb(cmd, device_id, wait_for_termination)
    if not wait_for_termination:
        return None, None
    try:
        out, err = p.communicate(timeout=timeout)
        p.wait()
    except subprocess.TimeoutExpired:
        p.terminate()
        raise DeviceTimeoutException(f"Device {device_id} timed out on command after {timeout}s")
    return out, err


def execute_privileged_command(cmd, device_id, wait_for_termination=True, timeout=None):
    """
    Execute privileged command on device
    
    Execute command with root privileges. Chooses different su invocation methods based on device type (emulator/real device).
    
    Args:
        cmd: Command string to execute
        device_id: Target device ID
        wait_for_termination: Whether to wait for command completion
        timeout: Timeout in seconds
    
    Returns:
        tuple: (stdout, stderr) command output
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
    Execute command as nobody user
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
    Execute normal command on device
    """
    out, err = call_adb(["shell", "{}".format(cmd)], device_id=device_id, wait_for_termination=wait_for_termination, timeout=timeout)
    return out, err


def program_exists(program_name, device_id=None):
    """
    Check if program exists on device
    """
    out, _ = execute_command("which {}".format(program_name), device_id)
    if program_name.encode() in out:
        return True
    return False


def path_exists(path, device_id=None):
    """
    Check if path exists on device
    """
    out, err = execute_command("ls {}".format(path), device_id=device_id)
    if b"No such file" in err or b"No such file" in out:
        return False
    return True


def wait_device_booted(device_id):
    """
    Wait for device to finish booting
    """
    boot_status = ""
    nretry = 12
    while True:
        try:
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


# ============== MCP Tool Functions ==============

@mcp.tool
def list_devices() -> str:
    """
    List all connected Android devices
    
    Returns:
        str: Device list with device IDs and their status
    """
    out, err = call_adb(["devices"])
    result = out.decode('utf-8', errors='replace')
    if err:
        result += f"\nError: {err.decode('utf-8', errors='replace')}"
    return result


@mcp.tool
def get_device_ids() -> list[str]:
    """
    Get list of all available device IDs
    
    Only returns device IDs with "device" status (accessible).
    
    Returns:
        list[str]: List of device IDs
    """
    ids = []
    out, _ = call_adb(["devices"])
    lines = out.split(b"\n")
    for line in lines:
        tokens = line.split(b"\t")
        if len(tokens) == 2 and tokens[1] == b"device":
            ids.append(tokens[0].decode('utf-8'))
    return ids


@mcp.tool
def shell_command(command: str, device_id: Optional[str] = None, timeout: Optional[int] = 30) -> str:
    """
    Execute shell command on Android device
    
    Args:
        command: Shell command to execute
        device_id: Target device ID (optional, required for multi-device)
        timeout: Command timeout in seconds, default 30
    
    Returns:
        str: Command execution result (stdout and stderr)
    """
    try:
        out, err = execute_command(command, device_id=device_id, timeout=timeout)
        result = ""
        if out:
            result += out.decode('utf-8', errors='replace')
        if err:
            result += f"\n[stderr]: {err.decode('utf-8', errors='replace')}"
        return result.strip() if result else "Command executed successfully, no output"
    except DeviceTimeoutException as e:
        return f"Error: Command execution timeout - {str(e)}"
    except Exception as e:
        return f"Error: {str(e)}"


@mcp.tool
def shell_command_privileged(command: str, device_id: Optional[str] = None, timeout: Optional[int] = 30) -> str:
    """
    Execute shell command with root privileges on Android device
    
    Requires device to be rooted.
    
    Args:
        command: Shell command to execute
        device_id: Target device ID (optional, required for multi-device)
        timeout: Command timeout in seconds, default 30
    
    Returns:
        str: Command execution result (stdout and stderr)
    """
    try:
        out, err = execute_privileged_command(command, device_id=device_id, timeout=timeout)
        result = ""
        if out:
            result += out.decode('utf-8', errors='replace')
        if err:
            result += f"\n[stderr]: {err.decode('utf-8', errors='replace')}"
        return result.strip() if result else "Command executed successfully, no output"
    except DeviceTimeoutException as e:
        return f"Error: Command execution timeout - {str(e)}"
    except Exception as e:
        return f"Error: {str(e)}"


@mcp.tool
def push_file(local_path: str, remote_path: str, device_id: Optional[str] = None) -> str:
    """
    Push local file to Android device
    
    Args:
        local_path: Absolute path of local file
        remote_path: Target path on device
        device_id: Target device ID (optional, required for multi-device)
    
    Returns:
        str: Push result information
    """
    if not os.path.exists(local_path):
        return f"Error: Local file does not exist - {local_path}"
    
    try:
        out, err = call_adb(["push", local_path, remote_path], device_id=device_id)
        result = out.decode('utf-8', errors='replace')
        if err:
            result += f"\n{err.decode('utf-8', errors='replace')}"
        return result.strip()
    except Exception as e:
        return f"Error: {str(e)}"


@mcp.tool
def push_file_privileged(local_path: str, remote_path: str, is_directory: bool = False, device_id: Optional[str] = None) -> str:
    """
    Push file to Android device with root privileges
    
    Uses temporary directory as intermediate, can push files to directories requiring root permission.
    
    Args:
        local_path: Absolute path of local file/directory
        remote_path: Target path on device
        is_directory: Whether it's a directory, default False
        device_id: Target device ID (optional, required for multi-device)
    
    Returns:
        str: Push result information
    """
    if not os.path.exists(local_path):
        return f"Error: Local file does not exist - {local_path}"
    
    try:
        rand_str = list(string.ascii_letters)
        random.shuffle(rand_str)
        workdir = os.path.join("/data/local/tmp", "".join(rand_str[:10]))
        
        what_file = os.path.basename(local_path)
        workdir_file = os.path.join(workdir, what_file)
        
        execute_privileged_command(" ".join(["mkdir", workdir]), device_id=device_id)
        execute_privileged_command(" ".join(["chown", "shell:shell", workdir]), device_id=device_id)
        
        out, err = call_adb(["push", local_path, workdir], device_id=device_id)
        
        if is_directory:
            execute_privileged_command(" ".join(["chown", "-R", "shell:shell", workdir_file]), device_id=device_id)
            execute_privileged_command(" ".join(["cp", "-r", workdir_file, remote_path]), device_id=device_id)
        else:
            execute_privileged_command(" ".join(["chown", "shell:shell", workdir_file]), device_id=device_id)
            execute_privileged_command(" ".join(["cp", workdir_file, remote_path]), device_id=device_id)

        execute_privileged_command(" ".join(["rm", "-rf", workdir]), device_id=device_id)

        result = out.decode('utf-8', errors='replace')
        if err:
            result += f"\n{err.decode('utf-8', errors='replace')}"
        return result.strip()
    except Exception as e:
        return f"Error: {str(e)}"


@mcp.tool
def pull_file(remote_path: str, local_path: str, device_id: Optional[str] = None) -> str:
    """
    Pull file from Android device to local
    
    Args:
        remote_path: File path on device
        local_path: Local target path
        device_id: Target device ID (optional, required for multi-device)
    
    Returns:
        str: Pull result information
    """
    try:
        out, err = call_adb(["pull", remote_path, local_path], device_id=device_id)
        result = out.decode('utf-8', errors='replace')
        if err:
            result += f"\n{err.decode('utf-8', errors='replace')}"
        return result.strip()
    except Exception as e:
        return f"Error: {str(e)}"


@mcp.tool
def pull_file_privileged(remote_path: str, local_path: str, is_directory: bool = False, device_id: Optional[str] = None) -> str:
    """
    Pull file from Android device with root privileges
    
    Uses temporary directory as intermediate, can pull files from directories requiring root permission.
    
    Args:
        remote_path: File/directory path on device
        local_path: Local target path
        is_directory: Whether it's a directory, default False
        device_id: Target device ID (optional, required for multi-device)
    
    Returns:
        str: Pull result information
    """
    try:
        rand_str = list(string.ascii_letters)
        random.shuffle(rand_str)
        workdir = os.path.join("/data/local/tmp", "".join(rand_str[:10]))
        what_file = os.path.basename(remote_path)
        workdir_file = os.path.join(workdir, what_file)

        execute_privileged_command(" ".join(["mkdir", workdir]), device_id=device_id)
        if is_directory:
            execute_privileged_command(" ".join(["cp", "-r", remote_path, workdir]), device_id=device_id)
            execute_privileged_command(" ".join(["chown", "-R", "shell:shell", workdir_file]), device_id=device_id)
        else:
            execute_privileged_command(" ".join(["cp", remote_path, workdir]), device_id=device_id)
            execute_privileged_command(" ".join(["chown", "shell:shell", workdir_file]), device_id=device_id)
        
        out, err = call_adb(["pull", workdir_file, local_path], device_id=device_id)
        execute_privileged_command(" ".join(["rm", "-rf", workdir]), device_id=device_id)

        result = out.decode('utf-8', errors='replace')
        if err:
            result += f"\n{err.decode('utf-8', errors='replace')}"
        return result.strip()
    except Exception as e:
        return f"Error: {str(e)}"


@mcp.tool
def get_logcat(device_id: Optional[str] = None, lines: int = 100) -> str:
    """
    Get logcat logs from Android device
    
    Args:
        device_id: Target device ID (optional, required for multi-device)
        lines: Number of log lines to return, default 100
    
    Returns:
        str: Logcat log content
    """
    try:
        out, err = call_adb(["logcat", "-d", "-t", str(lines)], device_id=device_id)
        result = out.decode('utf-8', errors='replace')
        if err:
            result += f"\n[stderr]: {err.decode('utf-8', errors='replace')}"
        return result
    except Exception as e:
        return f"Error: {str(e)}"


@mcp.tool
def reboot_device(device_id: Optional[str] = None, mode: str = "normal") -> str:
    """
    Reboot Android device
    
    Args:
        device_id: Target device ID (optional, required for multi-device)
        mode: Reboot mode, available values:
            - "normal": Normal reboot (default)
            - "recovery": Reboot to Recovery mode
            - "bootloader": Reboot to Bootloader/Fastboot mode
    
    Returns:
        str: Reboot operation result
    """
    try:
        if mode == "normal":
            out, err = call_adb(["reboot"], device_id=device_id, timeout=20)
        elif mode == "recovery":
            out, err = call_adb(["reboot", "recovery"], device_id=device_id, timeout=20)
        elif mode == "bootloader":
            out, err = call_adb(["reboot", "bootloader"], device_id=device_id, timeout=20)
        else:
            return f"Error: Unsupported reboot mode - {mode}"
        
        result = f"Device is rebooting to {mode} mode"
        if out:
            result += f"\n{out.decode('utf-8', errors='replace')}"
        if err:
            result += f"\n{err.decode('utf-8', errors='replace')}"
        return result
    except DeviceTimeoutException:
        return f"Device is rebooting to {mode} mode (command sent)"
    except Exception as e:
        return f"Error: {str(e)}"


@mcp.tool
def wait_for_device(device_id: Optional[str] = None, timeout: int = 120) -> str:
    """
    Wait for Android device to finish booting
    
    Args:
        device_id: Target device ID (optional, required for multi-device)
        timeout: Wait timeout in seconds, default 120
    
    Returns:
        str: Wait result
    """
    try:
        start_time = time.time()
        while time.time() - start_time < timeout:
            try:
                out, _ = execute_command("pwd", device_id=device_id, timeout=5)
                if out and b"/" in out:
                    return "Device has finished booting and is ready"
            except (DeviceTimeoutException, DeviceUnresponsiveException):
                pass
            time.sleep(5)
        return f"Error: Wait for device timeout ({timeout}s)"
    except Exception as e:
        return f"Error: {str(e)}"


@mcp.tool
def check_device_status(device_id: Optional[str] = None) -> str:
    """
    Check Android device status
    
    Verify if device meets running requirements, including:
    - Root permission check
    - File push capability
    
    Args:
        device_id: Target device ID (optional, required for multi-device)
    
    Returns:
        str: Device status report
    """
    result = []
    
    # Check device connection
    try:
        out, _ = execute_command("pwd", device_id=device_id, timeout=5)
        if out and b"/" in out:
            result.append("✓ Device connected")
        else:
            return "✗ Device not properly connected"
    except Exception as e:
        return f"✗ Device connection failed: {str(e)}"
    
    # Check root permission
    try:
        out, _ = execute_privileged_command("id", device_id=device_id, timeout=5)
        if b"root" in out:
            result.append("✓ Root access granted")
        else:
            result.append("✗ Root access not available")
    except Exception as e:
        result.append(f"✗ Root check failed: {str(e)}")
    
    # Get device info
    try:
        out, _ = execute_command("getprop ro.product.model", device_id=device_id, timeout=5)
        model = out.decode('utf-8', errors='replace').strip()
        if model:
            result.append(f"Device Model: {model}")
    except:
        pass
    
    try:
        out, _ = execute_command("getprop ro.build.version.release", device_id=device_id, timeout=5)
        version = out.decode('utf-8', errors='replace').strip()
        if version:
            result.append(f"Android Version: {version}")
    except:
        pass
    
    return "\n".join(result)


@mcp.tool
def install_apk(apk_path: str, device_id: Optional[str] = None, reinstall: bool = False) -> str:
    """
    Install APK to Android device
    
    Args:
        apk_path: Local path of APK file
        device_id: Target device ID (optional, required for multi-device)
        reinstall: Whether to reinstall (keep data), default False
    
    Returns:
        str: Installation result information
    """
    if not os.path.exists(apk_path):
        return f"Error: APK file does not exist - {apk_path}"
    
    try:
        cmd = ["install"]
        if reinstall:
            cmd.append("-r")
        cmd.append(apk_path)
        
        out, err = call_adb(cmd, device_id=device_id, timeout=120)
        result = out.decode('utf-8', errors='replace')
        if err:
            result += f"\n{err.decode('utf-8', errors='replace')}"
        return result.strip()
    except DeviceTimeoutException:
        return "Error: APK installation timeout"
    except Exception as e:
        return f"Error: {str(e)}"


@mcp.tool
def uninstall_app(package_name: str, device_id: Optional[str] = None, keep_data: bool = False) -> str:
    """
    Uninstall Android application
    
    Args:
        package_name: Application package name (e.g., com.example.app)
        device_id: Target device ID (optional, required for multi-device)
        keep_data: Whether to keep app data, default False
    
    Returns:
        str: Uninstall result information
    """
    try:
        cmd = ["uninstall"]
        if keep_data:
            cmd.append("-k")
        cmd.append(package_name)
        
        out, err = call_adb(cmd, device_id=device_id, timeout=30)
        result = out.decode('utf-8', errors='replace')
        if err:
            result += f"\n{err.decode('utf-8', errors='replace')}"
        return result.strip()
    except Exception as e:
        return f"Error: {str(e)}"


@mcp.tool
def list_packages(device_id: Optional[str] = None, filter_type: str = "all") -> str:
    """
    List installed application packages on device
    
    Args:
        device_id: Target device ID (optional, required for multi-device)
        filter_type: Filter type, available values:
            - "all": All applications (default)
            - "system": System applications only
            - "third_party": Third-party applications only
            - "disabled": Disabled applications only
            - "enabled": Enabled applications only
    
    Returns:
        str: Application package list
    """
    try:
        cmd = "pm list packages"
        if filter_type == "system":
            cmd += " -s"
        elif filter_type == "third_party":
            cmd += " -3"
        elif filter_type == "disabled":
            cmd += " -d"
        elif filter_type == "enabled":
            cmd += " -e"
        
        out, err = execute_command(cmd, device_id=device_id, timeout=30)
        result = out.decode('utf-8', errors='replace')
        if err:
            result += f"\n[stderr]: {err.decode('utf-8', errors='replace')}"
        return result.strip()
    except Exception as e:
        return f"Error: {str(e)}"


@mcp.tool
def forward_port(local_port: int, device_port: int, device_id: Optional[str] = None) -> str:
    """
    Set up ADB port forwarding
    
    Forward local port to device port.
    
    Args:
        local_port: Local port number
        device_port: Device port number
        device_id: Target device ID (optional, required for multi-device)
    
    Returns:
        str: Port forwarding result
    """
    try:
        out, err = call_adb(["forward", f"tcp:{local_port}", f"tcp:{device_port}"], device_id=device_id)
        if err and err.strip():
            return f"Error: {err.decode('utf-8', errors='replace')}"
        return f"Port forwarding set up successfully: local {local_port} -> device {device_port}"
    except Exception as e:
        return f"Error: {str(e)}"


@mcp.tool
def remove_port_forward(device_id: Optional[str] = None) -> str:
    """
    Remove all ADB port forwarding
    
    Args:
        device_id: Target device ID (optional, required for multi-device)
    
    Returns:
        str: Operation result
    """
    try:
        out, err = call_adb(["forward", "--remove-all"], device_id=device_id)
        return "All port forwarding removed"
    except Exception as e:
        return f"Error: {str(e)}"


@mcp.tool
def get_process_list(device_id: Optional[str] = None, filter_name: Optional[str] = None) -> str:
    """
    Get list of running processes on device
    
    Args:
        device_id: Target device ID (optional, required for multi-device)
        filter_name: Process name filter (optional)
    
    Returns:
        str: Process list
    """
    try:
        if filter_name:
            out, err = execute_command(f"ps -A | grep {filter_name}", device_id=device_id, timeout=10)
        else:
            out, err = execute_command("ps -A", device_id=device_id, timeout=10)
        
        result = out.decode('utf-8', errors='replace')
        if not result.strip():
            return "No matching processes found"
        return result
    except Exception as e:
        return f"Error: {str(e)}"


@mcp.tool
def kill_process(process_name: str, device_id: Optional[str] = None) -> str:
    """
    Kill specified process (requires root permission)
    
    Args:
        process_name: Process name
        device_id: Target device ID (optional, required for multi-device)
    
    Returns:
        str: Operation result
    """
    try:
        if program_exists("pkill", device_id):
            out, err = execute_privileged_command(f"pkill {process_name}", device_id=device_id, timeout=10)
            return f"Attempted to kill process: {process_name}"
        else:
            # Use pidof + kill method
            out, _ = execute_privileged_command(f"pidof {process_name}", device_id=device_id, timeout=5)
            pids = out.decode('utf-8', errors='replace').strip()
            if pids:
                execute_privileged_command(f"kill -9 {pids}", device_id=device_id, timeout=5)
                return f"Process killed: {process_name} (PID: {pids})"
            else:
                return f"Process not found: {process_name}"
    except Exception as e:
        return f"Error: {str(e)}"


@mcp.tool
def take_screenshot(local_path: str, device_id: Optional[str] = None) -> str:
    """
    Take device screenshot
    
    Args:
        local_path: Local path to save screenshot (should end with .png)
        device_id: Target device ID (optional, required for multi-device)
    
    Returns:
        str: Operation result
    """
    try:
        # Take screenshot on device
        remote_path = "/sdcard/screenshot_temp.png"
        out, err = execute_command(f"screencap -p {remote_path}", device_id=device_id, timeout=10)
        
        # Pull to local
        out, err = call_adb(["pull", remote_path, local_path], device_id=device_id)
        
        # Delete temp file on device
        execute_command(f"rm {remote_path}", device_id=device_id, timeout=5)
        
        if os.path.exists(local_path):
            return f"Screenshot saved to: {local_path}"
        else:
            return "Error: Failed to save screenshot"
    except Exception as e:
        return f"Error: {str(e)}"


@mcp.tool
def record_screen(local_path: str, duration: int = 10, device_id: Optional[str] = None) -> str:
    """
    Record device screen (max 180 seconds)
    
    Args:
        local_path: Local path to save recording (should end with .mp4)
        duration: Recording duration in seconds, default 10, max 180
        device_id: Target device ID (optional, required for multi-device)
    
    Returns:
        str: Operation result
    """
    try:
        if duration > 180:
            duration = 180
        
        remote_path = "/sdcard/screenrecord_temp.mp4"
        
        # Record screen
        out, err = execute_command(f"screenrecord --time-limit {duration} {remote_path}", device_id=device_id, timeout=duration + 30)
        
        # Pull to local
        out, err = call_adb(["pull", remote_path, local_path], device_id=device_id)
        
        # Delete temp file on device
        execute_command(f"rm {remote_path}", device_id=device_id, timeout=5)
        
        if os.path.exists(local_path):
            return f"Recording saved to: {local_path}"
        else:
            return "Error: Failed to save recording"
    except Exception as e:
        return f"Error: {str(e)}"


@mcp.tool
def get_device_properties(device_id: Optional[str] = None, property_name: Optional[str] = None) -> str:
    """
    Get device property information
    
    Args:
        device_id: Target device ID (optional, required for multi-device)
        property_name: Property name (optional). Returns common properties summary if not specified.
            Common property examples:
            - ro.product.model: Device model
            - ro.build.version.release: Android version
            - ro.product.manufacturer: Manufacturer
            - ro.serialno: Serial number
    
    Returns:
        str: Device property information
    """
    try:
        if property_name:
            out, _ = execute_command(f"getprop {property_name}", device_id=device_id, timeout=5)
            return out.decode('utf-8', errors='replace').strip()
        else:
            # Return common properties summary
            properties = {
                "Model": "ro.product.model",
                "Manufacturer": "ro.product.manufacturer", 
                "Android Version": "ro.build.version.release",
                "SDK Version": "ro.build.version.sdk",
                "Serial Number": "ro.serialno",
                "Build ID": "ro.build.display.id",
            }
            result = []
            for label, prop in properties.items():
                out, _ = execute_command(f"getprop {prop}", device_id=device_id, timeout=5)
                value = out.decode('utf-8', errors='replace').strip()
                if value:
                    result.append(f"{label}: {value}")
            return "\n".join(result)
    except Exception as e:
        return f"Error: {str(e)}"


@mcp.tool
def input_text(text: str, device_id: Optional[str] = None) -> str:
    """
    Input text on device
    
    Args:
        text: Text to input (note: spaces need to be escaped)
        device_id: Target device ID (optional, required for multi-device)
    
    Returns:
        str: Operation result
    """
    try:
        # Escape spaces
        escaped_text = text.replace(" ", "%s")
        out, err = execute_command(f"input text '{escaped_text}'", device_id=device_id, timeout=10)
        return f"Text entered: {text}"
    except Exception as e:
        return f"Error: {str(e)}"


@mcp.tool
def input_keyevent(keycode: str, device_id: Optional[str] = None) -> str:
    """
    Send key event
    
    Args:
        keycode: Key code, can be number or name. Common keys:
            - KEYCODE_HOME (3): Home key
            - KEYCODE_BACK (4): Back key
            - KEYCODE_POWER (26): Power key
            - KEYCODE_ENTER (66): Enter key
            - KEYCODE_VOLUME_UP (24): Volume up
            - KEYCODE_VOLUME_DOWN (25): Volume down
        device_id: Target device ID (optional, required for multi-device)
    
    Returns:
        str: Operation result
    """
    try:
        out, err = execute_command(f"input keyevent {keycode}", device_id=device_id, timeout=5)
        return f"Key event sent: {keycode}"
    except Exception as e:
        return f"Error: {str(e)}"


@mcp.tool
def input_tap(x: int, y: int, device_id: Optional[str] = None) -> str:
    """
    Simulate screen tap
    
    Args:
        x: X coordinate of tap position
        y: Y coordinate of tap position
        device_id: Target device ID (optional, required for multi-device)
    
    Returns:
        str: Operation result
    """
    try:
        out, err = execute_command(f"input tap {x} {y}", device_id=device_id, timeout=5)
        return f"Tapped at position: ({x}, {y})"
    except Exception as e:
        return f"Error: {str(e)}"


@mcp.tool
def input_swipe(x1: int, y1: int, x2: int, y2: int, duration_ms: int = 300, device_id: Optional[str] = None) -> str:
    """
    Simulate screen swipe
    
    Args:
        x1: Start X coordinate
        y1: Start Y coordinate
        x2: End X coordinate
        y2: End Y coordinate
        duration_ms: Swipe duration in milliseconds, default 300ms
        device_id: Target device ID (optional, required for multi-device)
    
    Returns:
        str: Operation result
    """
    try:
        out, err = execute_command(f"input swipe {x1} {y1} {x2} {y2} {duration_ms}", device_id=device_id, timeout=10)
        return f"Swiped: ({x1}, {y1}) -> ({x2}, {y2})"
    except Exception as e:
        return f"Error: {str(e)}"


@mcp.tool
def start_activity(component: str, device_id: Optional[str] = None, extras: Optional[str] = None) -> str:
    """
    Start Android Activity
    
    Args:
        component: Activity component name (format: package_name/activity_name)
            Example: com.android.settings/.Settings
        device_id: Target device ID (optional, required for multi-device)
        extras: Extra parameters (optional), e.g., "-e key value"
    
    Returns:
        str: Operation result
    """
    try:
        cmd = f"am start -n {component}"
        if extras:
            cmd += f" {extras}"
        out, err = execute_command(cmd, device_id=device_id, timeout=10)
        result = out.decode('utf-8', errors='replace')
        if err:
            result += f"\n{err.decode('utf-8', errors='replace')}"
        return result.strip()
    except Exception as e:
        return f"Error: {str(e)}"


@mcp.tool
def force_stop_app(package_name: str, device_id: Optional[str] = None) -> str:
    """
    Force stop application
    
    Args:
        package_name: Application package name (e.g., com.example.app)
        device_id: Target device ID (optional, required for multi-device)
    
    Returns:
        str: Operation result
    """
    try:
        out, err = execute_command(f"am force-stop {package_name}", device_id=device_id, timeout=10)
        return f"Force stopped application: {package_name}"
    except Exception as e:
        return f"Error: {str(e)}"


@mcp.tool
def clear_app_data(package_name: str, device_id: Optional[str] = None) -> str:
    """
    Clear application data
    
    Args:
        package_name: Application package name (e.g., com.example.app)
        device_id: Target device ID (optional, required for multi-device)
    
    Returns:
        str: Operation result
    """
    try:
        out, err = execute_command(f"pm clear {package_name}", device_id=device_id, timeout=30)
        result = out.decode('utf-8', errors='replace').strip()
        if "Success" in result:
            return f"Application data cleared: {package_name}"
        else:
            return f"Clear failed: {result}"
    except Exception as e:
        return f"Error: {str(e)}"


# ============== Main Entry ==============

if __name__ == "__main__":
    mcp.run()
