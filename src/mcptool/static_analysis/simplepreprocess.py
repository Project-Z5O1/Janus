"""
simplepreprocess.py - 支持指定 APK 路径的预处理脚本

该脚本复用 preprocess.py 的分析能力，但不再依赖你手动把 APK 放进固定目录；
你只需要传入单个 APK 文件路径即可。

示例:
    python simplepreprocess.py --apk /path/to/app.apk
    python simplepreprocess.py --apk ./demo.apk -s
    python simplepreprocess.py --apk ./demo.apk -l -d <device_id>

说明:
    - 输出仍会写入 ../target_APK/<appname>/ 目录（与原 preprocess.py 保持兼容）
    - <appname> 默认从 APK 的 packageName 推导；也可用 --target 覆盖
"""

from __future__ import annotations

import argparse
import os
import re
import shutil
import subprocess

try:
    from androguard.core.apk import APK  # type: ignore
except ModuleNotFoundError:  # pragma: no cover
    APK = None

import preprocess


def _abs_path(path: str) -> str:
    return os.path.abspath(os.path.expanduser(path))


def _infer_appname_from_apk(apk_path: str) -> str:
    if APK is not None:
        try:
            apk = APK(apk_path)
            package_name = apk.get_package()
        except Exception as exc:  # pragma: no cover
            raise RuntimeError(f"无法解析 APK（{apk_path}）：{exc}") from exc

        if package_name:
            return package_name

    package_name = _infer_appname_from_aapt(apk_path)
    if package_name:
        return package_name

    raise RuntimeError(
        f"无法从 APK 推导包名（{apk_path}）。请安装 androguard（pip install -r requirements.txt），或使用 --target 手动指定目录名/包名。"
    )


def _infer_appname_from_aapt(apk_path: str) -> str | None:
    try:
        completed = subprocess.run(
            ["aapt", "dump", "badging", apk_path],
            capture_output=True,
            text=True,
            timeout=30,
        )
    except FileNotFoundError:
        return None
    except subprocess.TimeoutExpired:
        return None

    if completed.returncode != 0:
        return None

    m = re.search(r"package: name='([^']+)'", completed.stdout or "")
    return m.group(1) if m else None


def _ensure_dir(path: str) -> None:
    os.makedirs(path, exist_ok=True)


def _cleanup_app_dir(app_dir: str, keep_filenames: set[str] | None = None) -> None:
    if keep_filenames is None:
        keep_filenames = {"base.apk"}

    if not os.path.isdir(app_dir):
        return

    for entry in os.listdir(app_dir):
        if entry in keep_filenames:
            continue
        p = os.path.join(app_dir, entry)
        try:
            if os.path.isdir(p) and not os.path.islink(p):
                shutil.rmtree(p)
            else:
                os.remove(p)
        except FileNotFoundError:
            continue


def _stage_apk_into_target(
    apk_path: str,
    appname: str,
    *,
    overwrite: bool = False,
) -> str:
    target_dir = os.path.join(preprocess.TARGET_APK_PATH, appname)
    _ensure_dir(target_dir)

    dest_apk = os.path.join(target_dir, "base.apk")
    if os.path.exists(dest_apk):
        try:
            if os.path.samefile(apk_path, dest_apk):
                return target_dir
        except OSError:
            pass
        if not overwrite:
            raise FileExistsError(
                f"{dest_apk} 已存在；如需覆盖请加 --overwrite，或使用 --target 指定新目录名。"
            )
        os.remove(dest_apk)

    shutil.copy2(apk_path, dest_apk)
    return target_dir


def _pick_device_id(explicit: str | None) -> str | None:
    if explicit:
        return explicit

    device_ids = preprocess.adb.get_device_ids()
    if not device_ids:
        return None
    return device_ids[0]


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Preprocess a single APK by path (wrapper around preprocess.py)"
    )

    parser.add_argument(
        "--apk",
        required=True,
        help="要分析的 APK 文件路径（单个文件）",
    )
    parser.add_argument(
        "--target",
        required=False,
        help="输出目录名/应用包名；不传则从 APK packageName 自动推导",
    )
    parser.add_argument(
        "--overwrite",
        required=False,
        default=False,
        action="store_true",
        help="若 ../target_APK/<target>/base.apk 已存在则覆盖",
    )

    parser.add_argument(
        "-s",
        "--signatures",
        required=False,
        default=False,
        action="store_true",
        help="提取 native 函数签名（NativeSignatures: jadx + qdox）",
    )
    parser.add_argument(
        "-s_idn",
        "--signatures_idNative",
        required=False,
        default=False,
        action="store_true",
        help="使用 static_analysis/<appname>.json（idNative 输出）作为签名来源",
    )
    parser.add_argument(
        "-l",
        "--libraries",
        required=False,
        default=False,
        action="store_true",
        help="提取 native 函数库名/偏移（JNIOffset，需要设备）",
    )
    parser.add_argument(
        "--libraries_specific",
        required=False,
        type=str,
        help="只在指定库中查找偏移（例如 libfoo.so）",
    )
    parser.add_argument(
        "-f",
        "--simple_flowdroid",
        required=False,
        default=False,
        action="store_true",
        help="运行简化版 FlowDroid 分析",
    )
    parser.add_argument(
        "-c",
        "--cleanup",
        required=False,
        default=False,
        action="store_true",
        help="仅清理 ../target_APK/<target>/ 中的分析产物（保留 base.apk），不做分析",
    )
    parser.add_argument(
        "--init",
        required=False,
        default=False,
        action="store_true",
        help="重新初始化设备上的 JNIOffset 工具",
    )
    parser.add_argument(
        "-d",
        "--device",
        required=False,
        help="指定用于 offset 提取的设备 ID（adb devices 中的序列号）",
    )

    args = parser.parse_args(argv)

    apk_path = _abs_path(args.apk)
    if not os.path.isfile(apk_path):
        raise FileNotFoundError(f"APK 不存在或不是文件：{apk_path}")

    if args.target == "all":
        raise ValueError("simplepreprocess.py 仅支持单个 APK；不支持 --target all。")

    appname = args.target or _infer_appname_from_apk(apk_path)

    if not args.libraries and not args.signatures and not args.simple_flowdroid and not args.cleanup:
        args.libraries = True
        args.signatures = True
        args.simple_flowdroid = True

    if args.cleanup:
        app_dir = os.path.join(preprocess.TARGET_APK_PATH, appname)
        _ensure_dir(app_dir)
        _cleanup_app_dir(app_dir, keep_filenames={"base.apk"})
        return 0

    # 准备输出目录与 base.apk（分析模式必须确保分析的是传入的 APK）
    _stage_apk_into_target(apk_path, appname, overwrite=args.overwrite)

    device_id: str | None = None
    if args.libraries or args.init:
        device_id = _pick_device_id(args.device)
        if device_id is None:
            raise RuntimeError("未检测到可用设备（adb devices 为空），但你选择了需要设备的选项。")
        status = preprocess.adb.check_device(device_id)
        if status != "OK":
            raise RuntimeError(f"设备 {device_id} 不可用：{status}")

    if args.init:
        preprocess.init_JNIOffset(device_id=device_id)

    preprocess.analyze_single_apk(
        appname,
        args.signatures,
        args.libraries,
        simple_FlowDroid=args.simple_flowdroid,
        library_specific=args.libraries_specific,
        use_idNative=args.signatures_idNative,
        device_id=device_id,
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
