"""
AAPT MCP Tools Module

This module wraps Android Asset Packaging Tool (aapt) common operations using FastMCP, providing:
- Basic aapt command execution
- APK inspection helpers (dump badging/permissions/xmltree, list contents, etc.)

Prerequisites:
1. aapt installed and added to PATH (or set env var AAPT/AAPT_PATH)
"""

from __future__ import annotations

import logging
import os
import shlex
import subprocess
from typing import Optional, Sequence

from fastmcp import FastMCP

# Configure logging
logging.basicConfig(level=logging.INFO, format="%(asctime)s - %(name)s - %(levelname)s - %(message)s")
log = logging.getLogger(__name__)


DEFAULT_AAPT_EXECUTABLE = os.environ.get("AAPT") or os.environ.get("AAPT_PATH") or "aapt"

# Create FastMCP instance
mcp = FastMCP(
    name="AAPT Tools",
    instructions="""
        Android Asset Packaging Tool (aapt) toolkit for inspecting APK files.
        The aapt executable is expected to be available in PATH.
    """,
    version="0.0.1",
)


# ============== Internal Helper Functions ==============

def _resolve_path(path: str) -> str:
    return os.path.abspath(os.path.expanduser(os.path.expandvars(path)))


def _format_output(stdout: bytes | None, stderr: bytes | None) -> str:
    out_text = (stdout or b"").decode("utf-8", errors="replace")
    err_text = (stderr or b"").decode("utf-8", errors="replace")
    out_text = out_text.strip("\n")
    err_text = err_text.strip("\n")

    if out_text and err_text:
        return f"{out_text}\n[stderr]: {err_text}".strip()
    if out_text:
        return out_text.strip()
    if err_text:
        return f"[stderr]: {err_text}".strip()
    return ""


def _run_aapt(args: Sequence[str], timeout: Optional[int] = 30, executable: str = DEFAULT_AAPT_EXECUTABLE) -> str:
    cmd = [executable, *args]
    try:
        completed = subprocess.run(cmd, capture_output=True, timeout=timeout)
    except FileNotFoundError as e:
        return f"Error: Did you add aapt to your $PATH? (executable: {executable})"
    except subprocess.TimeoutExpired as e:
        details = _format_output(getattr(e, "stdout", b""), getattr(e, "stderr", b""))
        return f"Error: aapt command timed out after {timeout}s\nCommand: {' '.join(cmd)}\n{details}".strip()
    except Exception as e:
        return f"Error: {str(e)}"

    details = _format_output(completed.stdout, completed.stderr)
    if completed.returncode != 0:
        if details:
            return f"Error: aapt exited with code {completed.returncode}\n{details}".strip()
        return f"Error: aapt exited with code {completed.returncode}"

    return details.strip() if details else "Command executed successfully, no output"


def _validate_apk_path(apk_path: str) -> tuple[bool, str]:
    resolved = _resolve_path(apk_path)
    if not os.path.exists(resolved):
        return False, f"Error: APK file does not exist - {resolved}"
    if not os.path.isfile(resolved):
        return False, f"Error: Path is not a file - {resolved}"
    return True, resolved


# ============== MCP Tool Functions ==============

@mcp.tool
def aapt_version() -> str:
    """Get aapt version information."""
    return _run_aapt(["version"])


@mcp.tool
def aapt_help() -> str:
    """Show aapt help/usage information."""
    return _run_aapt(["help"], timeout=10)


@mcp.tool
def aapt_run(args: list[str], timeout: Optional[int] = 30) -> str:
    """
    Run aapt with raw argument list (no shell).

    Example:
        args=["dump","badging","/path/to/app.apk"]
    """
    if not args:
        return "Error: args is empty"
    if args and args[0] == "aapt":
        args = args[1:]
        if not args:
            return "Error: args only contains 'aapt'"
    return _run_aapt(args, timeout=timeout)


@mcp.tool
def aapt_command(command: str, timeout: Optional[int] = 30) -> str:
    """
    Run aapt with a command string (parsed by shlex; no shell).

    The `command` should be everything after `aapt`, but if it starts with `aapt` it will be stripped.
    Example:
        command="dump badging /path/to/app.apk"
    """
    command = (command or "").strip()
    if not command:
        return "Error: command is empty"
    args = shlex.split(command)
    if args and args[0] == "aapt":
        args = args[1:]
    if not args:
        return "Error: command only contains 'aapt'"
    return _run_aapt(args, timeout=timeout)


@mcp.tool
def aapt_list(apk_path: str, verbose: bool = False, include_asset_details: bool = False) -> str:
    """
    List contents of an APK (zip).

    Args:
        apk_path: APK file path
        verbose: Use `-v` for verbose listing
        include_asset_details: Use `-a` to include asset details (when supported)
    """
    ok, resolved = _validate_apk_path(apk_path)
    if not ok:
        return resolved

    args: list[str] = ["list"]
    if verbose:
        args.append("-v")
    if include_asset_details:
        args.append("-a")
    args.append(resolved)
    return _run_aapt(args, timeout=30)


@mcp.tool
def aapt_dump_badging(apk_path: str, timeout: Optional[int] = 30) -> str:
    """Dump APK badging information (package name, version, SDK, labels, etc.)."""
    ok, resolved = _validate_apk_path(apk_path)
    if not ok:
        return resolved
    return _run_aapt(["dump", "badging", resolved], timeout=timeout)


@mcp.tool
def aapt_dump_permissions(apk_path: str, timeout: Optional[int] = 30) -> str:
    """Dump APK permissions."""
    ok, resolved = _validate_apk_path(apk_path)
    if not ok:
        return resolved
    return _run_aapt(["dump", "permissions", resolved], timeout=timeout)


@mcp.tool
def aapt_dump_configurations(apk_path: str, timeout: Optional[int] = 30) -> str:
    """Dump APK configurations."""
    ok, resolved = _validate_apk_path(apk_path)
    if not ok:
        return resolved
    return _run_aapt(["dump", "configurations", resolved], timeout=timeout)


@mcp.tool
def aapt_dump_strings(apk_path: str, timeout: Optional[int] = 30) -> str:
    """Dump APK strings."""
    ok, resolved = _validate_apk_path(apk_path)
    if not ok:
        return resolved
    return _run_aapt(["dump", "strings", resolved], timeout=timeout)


@mcp.tool
def aapt_dump_resources(apk_path: str, timeout: Optional[int] = 60) -> str:
    """Dump APK resources table (can be large)."""
    ok, resolved = _validate_apk_path(apk_path)
    if not ok:
        return resolved
    return _run_aapt(["dump", "resources", resolved], timeout=timeout)


@mcp.tool
def aapt_dump_xmltree(apk_path: str, xml_file: str = "AndroidManifest.xml", timeout: Optional[int] = 30) -> str:
    """
    Dump parsed XML tree from an APK.

    Common xml_file values:
    - AndroidManifest.xml
    - res/xml/*.xml (if present in the APK)
    """
    ok, resolved = _validate_apk_path(apk_path)
    if not ok:
        return resolved
    xml_file = (xml_file or "").strip() or "AndroidManifest.xml"
    return _run_aapt(["dump", "xmltree", resolved, xml_file], timeout=timeout)


# ============== Main Entry ==============

if __name__ == "__main__":
    mcp.run()
