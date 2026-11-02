"""
preprocess.py - APK 静态分析预处理脚本

该脚本用于对 Android APK 进行静态分析预处理，主要功能包括：
1. 提取 APK 中的 native 函数签名（使用 jadx/qdox 或 idNative）
2. 查找 native 函数在共享库中的偏移量（使用 JNIOffset）
3. 运行简单的 FlowDroid 数据流分析

使用方法：
    python preprocess.py --target <app_name> [-s] [-l] [-f]
    
参数说明：
    --target: 目标应用名称（必需）
    -s/--signatures: 提取 native 函数签名
    -l/--libraries: 提取 native 函数的库名和偏移量
    -f/--simple_flowdroid: 运行 FlowDroid 分析
    -c/--cleanup: 清理预处理生成的文件
    --init: 初始化手机上的 JNIOffset 工具
    -d/--device: 指定设备 ID
"""

import sys
import os
import shutil
import argparse
import subprocess
import json
import logging
try:
    from androguard.core.apk import APK  # 使用 androguard 库解析 APK 文件
except ModuleNotFoundError:  # pragma: no cover
    APK = None

# 获取当前脚本所在目录的路径
BASE_PATH = os.path.dirname(__file__)
# 将父目录添加到模块搜索路径，以便导入 adb 模块
#sys.path.append(os.path.join(BASE_PATH, ".."))
# 将父目录添加到模块搜索路径（用于切换到 static_analysis 目录）
#sys.path.append(os.path.join(BASE_PATH, '..'))

import adb  # 导入自定义的 adb 工具模块，用于与 Android 设备交互

# ============================================================================
# 终端输出颜色定义 - 用于在终端中显示彩色日志信息
# ============================================================================
RED='\033[0;31m'         # 红色 - 用于错误信息
YELLOW='\033[0;33m'      # 黄色 - 用于警告信息
GREEN='\033[0;32m'       # 绿色 - 用于成功信息
NC='\033[0m'             # 无颜色 - 重置颜色
BLUE='\033[0;34m'        # 蓝色
PURPLE='\033[0;35m'      # 紫色 - 用于进行中的操作
CYAN='\033[0;36m'        # 青色 - 用于完成信息

# 配置日志记录器，将日志写入 preprocessor.log 文件
logging.basicConfig(filename='preprocessor.log', encoding='utf-8', level=logging.DEBUG, format='%(asctime)s %(message)s')

# 目标 APK 存放路径（相对于脚本父目录的 target_APK 文件夹）
TARGET_APK_PATH = os.path.join(BASE_PATH, "../target_APK")
# JNIOffset 工具在 Android 设备上的工作目录
REMOTE_FOLDER = "/data/local/tmp/offsetFinding/"

# 默认的 LD_PRELOAD 库路径（用于 Termux 环境）
LD_PRELOAD_DEFAULT = "/data/data/com.termux/files/usr/lib/libc++_shared.so"

# ============================================================================
# 特殊应用的库文件路径映射
# 某些应用（如 WhatsApp、Facebook 等）将 native 库存放在非标准位置
# 这个字典记录了这些应用的特殊库路径
# ============================================================================
SPECIAL_LOCATIONS = {
    "com.whatsapp": ["/data/data/com.whatsapp/files/decompressed/libs.spk.zst/"],
    "com.facebook.lite": ["/data/data/com.facebook.lite/lib-compressed/"],
    "com.facebook.orca": ["/data/data/com.facebook.orca/lib-compressed/"],
    "com.instagram.android": ["/data/data/com.instagram.android/lib-compressed/"],
    "com.facebook.pages.app": ["/data/data/com.facebook.pages.app/lib-compressed/"],
    "com.microsoft.office.officehubrow": ["/data/data/com.microsoft.office.officehubrow/files/data/applibs/"],
    "com.whatsapp.w4b": ["/data/data/com.whatsapp.w4b/files/decompressed/libs.spk.zst/"]
}

# FlowDroid 分析的超时时间（10分钟）
FLOWDROID_TIMEOUT = 10*60


def getlibs_sig_offsets(appname):
    """
    从 signatures_libraries_offsets.txt 文件中读取并提取所有库名称
    
    参数:
        appname: 应用名称（包名）
    
    返回:
        set: 包含所有库名称的集合
    """
    filename = os.path.join(TARGET_APK_PATH, appname, "signatures_libraries_offsets.txt")
    with open(filename, "r") as f:
        lines = f.read().splitlines()
    libs = set()
    for l in lines:
        # 每行格式为: "函数名 签名 库名 偏移量"，提取第三个字段（库名）
        library = l.split(" ")[2]
        libs.add(library)
    return libs


def get_mangled_signature(sig):
    """
    将 Java 方法签名转换为 JNI 规范的混淆签名（mangled signature）
    
    JNI 规范参考: https://docs.oracle.com/en/java/javase/11/docs/specs/jni/design.html
    
    这个函数用于处理 JNI 中的重载函数，需要在函数名后附加参数类型签名
    
    参数:
        sig: 原始签名字符串，格式如 "返回类型:参数1,参数2,..."
    
    返回:
        str: JNI 混淆后的参数签名字符串
    """
    # 提取参数部分（冒号后面的内容）
    args = sig.split(":")[1]
    args = args.split(",")
    
    # JNI 类型映射表：Java 类型 -> JNI 类型描述符
    type_mapping = {
        "boolean": "Z",   # 布尔类型
        "byte": "B",      # 字节类型
        "char": "C",      # 字符类型
        "short": "S",     # 短整型
        "int": "I",       # 整型
        "long": "J",      # 长整型
        "float": "F",     # 单精度浮点
        "double": "D",    # 双精度浮点
        "String": "Ljava_lang_String_2",   # 字符串类型
        "rest": "Ljava_lang_Object_2"      # 其他对象类型（默认）
    }
    
    mangled_arg_sig = ""
    for arg in args:
        if arg == '':
            continue
        if arg in type_mapping:
            # 基本类型或已知类型，直接映射
            mangled_arg_sig += type_mapping[arg]
        elif arg[-2:] == "[]":
            # 数组类型，使用 _3 前缀表示
            if arg[:-2] in type_mapping:
                mangled_arg_sig += "_3" + type_mapping[arg[:-2]]
            else:
                mangled_arg_sig += "_3" + type_mapping["rest"]
        else:
            # 未知对象类型，使用默认的 Object 映射
            mangled_arg_sig += type_mapping["rest"]
    return mangled_arg_sig


def idnative_2_signatures_patterns(idnative):
    """
    将 idNative 工具生成的 JSON 格式转换为 signatures_pattern.txt 格式
    
    参数:
        idnative: idNative 工具输出的 JSON 数据（列表形式）
    
    返回:
        str: 转换后的签名模式字符串，每行格式为 "Java_类名_函数名 返回类型:参数列表"
    """
    output = ""
    functions = set()  # 用于去重
    for f in idnative:
        ins = f["instruction"]
        # 提取类名：从指令字符串中解析，格式如 "<com.example.Class: void method()>"
        classname = ins[ins.find("<")+1:ins.rfind(":")]
        # 将类名转换为 JNI 格式：
        # 1. 先将下划线替换为临时标记 #
        # 2. 将点号替换为下划线
        # 3. 将临时标记 # 替换为 _1（JNI 中下划线的转义）
        classname = classname.replace("_", "#")
        classname = classname.replace(".", "_")
        classname = classname.replace("#", "_1")
        
        # 获取 native 函数名并转换
        fname = f["nativeName"]
        fname = fname.replace("_", "_1")  # 转义函数名中的下划线
        
        # 跳过已处理的函数（去重）
        if fname in functions:
            continue
        functions.add(fname)
        
        # 提取返回类型
        ret_type = ins[ins.find("<")+1:ins.rfind(">")]
        ret_type = ret_type.split(":")[1]
        print(ret_type)
        ret_type = ret_type.split(" ")[1]
        ret_type = ret_type.split(".")[-1]  # 只保留简单类名
        print(classname, ret_type)
        
        # 构建参数列表字符串
        args = ""
        for arg in f["params"]:
            args += arg.split(".")[-1] + ","
        
        # 生成输出行：JNI 函数名 返回类型:参数列表
        output += f"Java_{classname}_{fname} {ret_type}:{args}\n"
    print(output)
    return output


def extract_libs(appname):
    """
    从 APK 文件中提取 arm64-v8a 架构的 native 库文件
    
    该函数会解析 APK 文件，找出所有位于 lib/arm64-v8a 或 lib/arm64 目录下的
    共享库文件（.so 文件），并将其提取到本地目录
    
    参数:
        appname: 应用名称（包名）
    
    注意:
        目前该函数只读取文件列表，实际的提取逻辑（data = apk.get_file(filepath)）
        后续代码未完成
    """
    if APK is None:
        raise ModuleNotFoundError(
            "androguard 未安装，无法执行 extract_libs；请先安装依赖（例如 pip install -r requirements.txt）。"
        )

    apk = APK(os.path.join(TARGET_APK_PATH, appname, "base.apk"))
    files = apk.get_files()
    
    # 创建输出目录结构
    if not os.path.exists(os.path.join(TARGET_APK_PATH, "lib")):
        os.mkdir(os.path.join(TARGET_APK_PATH, "lib"))
    if not os.path.exists(os.path.join(TARGET_APK_PATH, "lib", "arm64-v8a")):
        os.mkdir(os.path.join(TARGET_APK_PATH, "lib", "arm64-v8a"))
    
    print("Files:", files)
    for filepath in files:
        file_path = os.path.normpath(filepath).split("/")
        if len(file_path) > 2:
            # 只处理 arm64-v8a 或 arm64 架构的库文件
            if file_path[0] == "lib" and (file_path[1] == "arm64-v8a" or file_path[1] == "arm64"):
                lib = file_path[2]
                data = apk.get_file(filepath)  # 获取文件内容（后续可写入本地）


def extract_signatures(appname, use_idNative=False, fixup_overloaded=True):
    """
    提取 APK 中的 native 函数签名
    
    该函数支持两种提取方式：
    1. 使用 jadx + qdox 工具提取（默认方式）
    2. 使用 idNative 工具生成的 JSON 文件提取
    
    参数:
        appname: 应用名称（包名）
        use_idNative: 是否使用 idNative 的 JSON 输出作为签名来源
        fixup_overloaded: 是否修复重载函数的签名（添加参数类型后缀）
    
    输出:
        在 target_APK/{appname}/ 目录下生成 signatures_pattern.txt 文件
    """
    if not use_idNative:
        # 方式1：使用 jadx/qdox 提取签名
        print(f"{PURPLE}[SIGS]{NC} Extracting signatures using jadx/qdox")
        logging.info(f"Starting to extract native function signatures for with jadx/qdox: {appname}")
        os.system(f"{BASE_PATH}/NativeSignatures/analyze_native_signatures.sh {appname}")
        logging.info(f"Finished extracting native function signatures for: {appname}")
        print(f"{GREEN}[SIGS]{NC} Finished signature extraction using jadx/qdox")
        # 记录已分析的应用
        os.system(f"echo {appname} >> {os.path.join(TARGET_APK_PATH, 'analyzed_signatures.txt')}")
    else:
        # 方式2：从 idNative JSON 文件加载签名
        idnative_path = os.path.join(TARGET_APK_PATH, appname, "static_analysis", f"{appname}.json")
        if not os.path.exists(idnative_path):
            # JSON 文件不存在，回退到 jadx/qdox 方式
            print(f"{PURPLE}[SIGS]{NC} No JSON! Extracting signatures using jadx/qdox")
            logging.info(f"Starting to extract native function signatures for with jadx/qdox: {appname}")
            os.system(f"{BASE_PATH}/NativeSignatures/analyze_native_signatures.sh {appname}")
            logging.info(f"Finished extracting native function signatures for: {appname}")
            print(f"{GREEN}[SIGS]{NC} Finished signature extraction using jadx/qdox")
            os.system(f"echo {appname} >> {os.path.join(TARGET_APK_PATH, 'analyzed_signatures.txt')}")
        else:
            # 解析 JSON 文件并转换为签名模式格式
            print(f"{PURPLE}[SIGS]{NC} loading signatures from json")
            with open(idnative_path, "r") as f:
                idnative_json = json.loads(f.read())
            signature_data = idnative_2_signatures_patterns(idnative_json)
            with open(os.path.join(TARGET_APK_PATH, appname, "signatures_pattern.txt"), "w") as f:
                f.write(signature_data)
            print(f"{GREEN}[SIGS]{NC} finished loading signatures from json")
    
    # 修复重载函数的签名
    # JNI 规范要求重载函数在函数名后附加参数类型签名以区分不同版本
    if fixup_overloaded:
        print(f"{PURPLE}[SIGS]{NC} fixing up overloaded signatures")
        output_path = os.path.join(TARGET_APK_PATH, appname, "signatures_pattern.txt")
        with open (output_path, "r") as f:
            found = f.read().splitlines()
        
        # 查找所有重载函数（同名但参数不同的函数）
        overloaded = set()
        for i,f in enumerate(found):
            fname = f.split(" ")[0]
            for k,f2 in enumerate(found):
                if k == i:
                    continue
                if fname == f2.split(" ")[0]:
                    # 找到同名函数，标记为重载
                    overloaded.add((i,f))
                    overloaded.add((k,f2))
        
        # 为重载函数添加混淆签名后缀
        for i,f in overloaded:
            print(f"[SIGS] fixing up overloaded function {f}")
            fname = f.split(" ")[0]
            sig = f.split(" ")[1]
            # 在函数名后添加 __ 和参数类型签名
            fname = fname + "__" + get_mangled_signature(sig)
            found[i] = fname + " " + sig
        
        # 写回修复后的签名
        output = "\n".join(found)
        with open(output_path, "w") as f:
            f.write(output)
        print(f"{GREEN}[SIGS]{NC} done fixing up overloaded signatures")
    
    print(f"{CYAN}[SIGS]{NC} finished signature extraction")


def init_JNIOffset(device_id=None):
    """
    在 Android 设备上初始化 JNIOffset 工具环境
    
    该函数会将 JNIOffset 工具的必要文件上传到设备，并在设备上编译 jniOffsetFinder
    
    JNIOffset 工具用于在 native 库中查找 JNI 函数的偏移量
    
    参数:
        device_id: 目标设备 ID（可选，用于多设备情况）
    
    执行步骤:
        1. 在设备上创建工作目录
        2. 上传 harness 文件（模拟 AFL 环境）
        3. 上传并编译 jniOffsetFinder 工具
        4. 上传查找脚本
    """
    print(f"{PURPLE}[SETUP]{NC} Setting up JNIOffset on phone..")
    
    # 创建远程工作目录
    adb.execute_privileged_command(f"mkdir -p {REMOTE_FOLDER}", device_id=device_id)
    
    # 上传必要的文件
    # harness: 一个模拟的 AFL harness，导出必要的 AFL_AREA_PTR 符号
    adb.push_privileged(os.path.join(BASE_PATH, "JNIOffset", "harness"), REMOTE_FOLDER, device_id=device_id)
    adb.push_privileged(os.path.join(BASE_PATH, "JNIOffset", "example_harness"), REMOTE_FOLDER, device_id=device_id)
    
    # 上传 jniOffsetFinder 源码
    adb.push_privileged(os.path.join(BASE_PATH, "JNIOffset", "cpp", "jniOffsetFinder.cpp"), REMOTE_FOLDER, device_id=device_id)
    adb.push_privileged(os.path.join(BASE_PATH, "JNIOffset", "cpp", "jniOffsetFinder.h"), REMOTE_FOLDER, device_id=device_id)
    
    # 在设备上使用 clang++ 编译 jniOffsetFinder
    # 编译选项说明:
    # -std=c++17: 使用 C++17 标准
    # -Wl,--export-dynamic: 导出所有动态符号
    # -g: 包含调试信息
    # -O0: 禁用优化
    adb.execute_privileged_command(f"/data/data/com.termux/files/usr/bin/clang++ -std=c++17 -v -Wl,--export-dynamic -g -O0 {REMOTE_FOLDER}jniOffsetFinder.cpp -o {REMOTE_FOLDER}jniOffsetFinder", device_id=device_id)
    
    # 上传并设置查找脚本
    adb.push_privileged(os.path.join(BASE_PATH, "JNIOffset", "findJNIOffsets.sh"), REMOTE_FOLDER, device_id=device_id)
    adb.execute_privileged_command(f"chmod +x {REMOTE_FOLDER}/findJNIOffsets.sh", device_id=device_id)
    
    print(f"{GREEN}[SETUP]{NC} Done setting up JNIOffset on phone")


def extract_library_offset(appname, cleanup=True, library_specific=None, device_id=None):
    """
    在 Android 设备上查找 native 函数在共享库中的偏移量
    
    该函数会遍历所有 native 库和函数签名，使用 jniOffsetFinder 工具
    查找每个 native 函数在对应库中的偏移地址
    
    参数:
        appname: 应用名称（包名）
        cleanup: 是否在完成后清理设备上的临时文件
        library_specific: 如果指定，则只在该库中查找
        device_id: 目标设备 ID
    
    输出:
        在 target_APK/{appname}/ 目录下生成 signatures_libraries_offsets.txt 文件
        每行格式: "函数名 签名 库名 偏移量"
    """
    print(f"{PURPLE}[OFFSET]{NC} Starting to look for native functions")
    logging.info(f"[{appname}] Starting to look for native functions")
    
    # 读取应用路径和库文件列表
    app_path = os.path.join(TARGET_APK_PATH, appname)
    libraries = []
    
    # 处理特殊应用的库路径
    # 某些应用（如 WhatsApp、Facebook）将库文件存放在非标准位置
    if appname in SPECIAL_LOCATIONS:
        for custom_lib_location in SPECIAL_LOCATIONS[appname]:
            logging.debug(f"[{appname}] handling speial library location")
            out, err = adb.execute_privileged_command(f"ls {custom_lib_location}", device_id=device_id)
            out = out.decode()
            entries = out.split("\n")
            for l in entries:
                if ".so" in l:
                    # 如果本地还没有这个库文件，从设备上下载
                    if not os.path.exists(os.path.join(TARGET_APK_PATH, appname, 'lib', 'arm64-v8a', l)):
                        adb.pull_privileged(os.path.join(custom_lib_location,l), os.path.join(TARGET_APK_PATH, appname, 'lib', 'arm64-v8a'), device_id=device_id)
    
    # 检查是否存在 arm64-v8a 架构的库目录
    # 注意：这里忽略了只有特殊位置库的情况
    if not os.path.isdir(os.path.join(app_path, "lib", "arm64-v8a")):
        print(f"{YELLOW}[OFFSET]{NC} no native libraries, nothing to be done")
        logging.info(f"[{appname}] No native libraries!")
        return
    
    # 获取所有库文件名
    for lib in os.listdir(os.path.join(app_path, "lib", "arm64-v8a")):
        libraries.append(lib)
    
    # 读取函数签名文件
    if not os.path.exists(os.path.join(app_path, "signatures_pattern.txt")):
        print(f"{RED}[OFFSET]{NC} No signatures_pattern.txt, make sure to first get the native signatures")
        logging.info(f"[{appname}] No signatures_pattern.txt")
        return
    
    with open(os.path.join(app_path, "signatures_pattern.txt")) as f:
        signature_data = f.read()
    
    # 解析签名数据
    signatures = []
    signature_data = signature_data.split("\n")
    for s in signature_data:
        if s == '':
            continue
        signatures.append({"fname": s.split(" ")[0], "signature": s.split(" ")[1]})
    
    if len(signatures) == 0:
        print(f"{RED}[OFFSET]{NC} no native function signatures, nothing to be done")
        logging.info(f"[{appname}] No native function signatures")
        return
    
    # 初始化设备上的 JNIOffset 环境
    out, err = adb.execute_privileged_command(f"ls {REMOTE_FOLDER}", device_id=device_id)
    init_JNIOffset(device_id=device_id)
    
    # 上传应用文件到设备
    print(f"{PURPLE}[OFFSET]{NC} uploading necessary files from {app_path} to phone")
    logging.info(f"[{appname}] uploading {app_path} to phone")
    adb.execute_privileged_command(f"mkdir {REMOTE_FOLDER}{appname}", device_id=device_id)
    adb.push_privileged(os.path.join(app_path, "base.apk"), os.path.join(REMOTE_FOLDER,appname), device_id=device_id)
    adb.push_privileged(os.path.join(app_path, "signatures_pattern.txt"), os.path.join(REMOTE_FOLDER,appname), device_id=device_id)
    adb.push_privileged(os.path.join(app_path, "lib"), os.path.join(REMOTE_FOLDER,appname), is_directory=True, device_id=device_id)
    
    # 运行 jniOffsetFinder 查找函数偏移
    # 命令格式: LD_LIBRARY_PATH=... ./jniOffsetfinder <app_name> <target_library_name> <target_function_name> <target_function_signature>
    print(f"{GREEN}[OFFSET]{NC} finished uploading to phone")
    logging.info(f"[{appname}] finished uploading {app_path} to phone")
    
    out = b''
    err = b''
    print(f"{PURPLE}[OFFSET]{NC} Looking for native function offset...")
    
    # 如果指定了特定库，则只处理该库
    if library_specific is not None:
        libraries = [library_specific]
    
    # 遍历所有库，查找 native 函数
    for lib in libraries:
        # 如果应用自带 libc++_shared.so，需要设置 LD_PRELOAD
        ld_preload = ""
        if "libc++_shared.so" in libraries:
            ld_preload = os.path.join(REMOTE_FOLDER, appname, "lib", "arm64-v8a", "libc++_shared.so")
        try:
            # 运行查找脚本，超时时间 60 秒
            out, err = adb.execute_privileged_command(f"{REMOTE_FOLDER}findJNIOffsets.sh {appname} {lib} {REMOTE_FOLDER} {ld_preload}", timeout=60, device_id=device_id)
            logging.debug(f"[{appname}] JNIOFfsets output: {out}, {err}")
        except adb.DeviceTimeoutException:
            # 处理超时情况（可能是库加载崩溃）
            logging.error(f"[{appname}] crash while checking library {lib}, continuing")
        
        # 处理返回结果
        if err == b'' and b"RETURN CODE 0" in out:
            # 成功找到函数
            logging.info(f"[{appname}] Finished running jniOffsetfinder for library {lib}")
        elif err != b'' and b"RETURN CODE 1" in out:
            # 返回码 1 表示未找到，继续下一个库
            logging.info(f"[{appname}] jniOffsetfinder returned code of 1 for library {lib}")
            pass
        else:
            # 发生异常，可能是工具崩溃
            logging.error(f"[{appname}] jniOffsetfinder Error when checking library {lib}. Error: {err}")
    
    # 从设备下载结果文件
    adb.pull_privileged(f"{REMOTE_FOLDER}/{appname}/signatures_libraries_offsets.txt", app_path, device_id=device_id)
    
    # 清理设备上的临时文件
    if cleanup:
        adb.execute_privileged_command(f"rm -r {REMOTE_FOLDER}{appname}", device_id=device_id)
    
    # 统计找到的函数数量
    if not os.path.exists(os.path.join(app_path, "signatures_libraries_offsets.txt")):
        nr_found_functions = 0
        # 创建空文件
        with open(os.path.join(app_path, "signatures_libraries_offsets.txt"), 'w') as fp:
            pass
    else:
        with open(os.path.join(app_path, "signatures_libraries_offsets.txt")) as fp:
            nr_found_functions = len(fp.readlines())
    
    print(f"{CYAN}[OFFSET]{NC} Finished looking for native function offsets! Found: {nr_found_functions} offsets")
    logging.info(f"[{appname}] Finished looking for native function offsets! Found: {nr_found_functions} offsets")

    # 记录统计信息
    with open(os.path.join(app_path, "signatures_pattern.txt")) as fp:
        nr_signatures = len(fp.readlines())
    os.system(f"echo {appname} >> {os.path.join(TARGET_APK_PATH, 'analyzed_libraries_offsets.txt')}")
    logging.info(f"{appname} nr native functions found: {nr_found_functions}, nr of native functions defined: {nr_signatures}")


def analyze_single_apk(appname, signatures=False, libraries=False, simple_FlowDroid=False, library_specific=None, use_idNative=False, device_id=None):
    """
    对单个 APK 进行完整的静态分析
    
    该函数是主要的分析入口，可以执行以下分析任务：
    1. FlowDroid 数据流分析
    2. 提取 native 函数签名
    3. 查找 native 函数在库中的偏移量
    
    参数:
        appname: 应用名称（包名）
        signatures: 是否提取 native 函数签名
        libraries: 是否查找 native 函数偏移量
        simple_FlowDroid: 是否运行 FlowDroid 分析
        library_specific: 如果指定，只在该库中查找偏移量
        use_idNative: 是否使用 idNative JSON 文件作为签名来源
        device_id: 目标设备 ID
    
    前置条件:
        - target_APK/{appname}/ 目录必须存在
        - target_APK/{appname}/base.apk 文件必须存在
    """
    print(f"{PURPLE}[*]{NC} Analyzing {appname}")
    
    # 检查应用目录是否存在
    if not os.path.exists(os.path.join(TARGET_APK_PATH, appname)):
        print(f"{RED}[-]{NC} {os.path.join(TARGET_APK_PATH, appname)} does not exist!")
        logging.error(f"{os.path.join(TARGET_APK_PATH, appname)} does not exist!")
        exit(-1)
    
    # 检查 APK 文件是否存在
    if not os.path.exists(os.path.join(TARGET_APK_PATH, appname, "base.apk")):
        print(f"{RED}[-]{NC} {os.path.join(TARGET_APK_PATH, appname, 'base.apk')} does not exist!")
        logging.error(f"{os.path.join(TARGET_APK_PATH, appname, 'base.apk')} does not exist!")
        exit(-1)
    
    # 步骤 1: 运行 FlowDroid 分析（可选）
    if simple_FlowDroid:
        print(f"{PURPLE}[*]{NC} Starting simple flowdroid analysis for {appname}")
        app_path = os.path.join(TARGET_APK_PATH, appname)
        # 启动 FlowDroid 分析进程
        p = subprocess.Popen(["java", "-jar", f"{BASE_PATH}/FlowdroidSimple/FlowDroidAnalysis.jar", app_path, "abcd"])
        try:
            # 等待分析完成，设置超时时间
            p.wait(FLOWDROID_TIMEOUT)
        except subprocess.TimeoutExpired:
            # 超时则终止进程
            print(f"{RED}[-]{NC} timeout for app: {appname} :/")
            p.kill()
        print(f"{GREEN}[+]{NC} Finished simple flowdroid analysis for {appname}")
    
    # 步骤 2: 提取 native 函数签名（可选）
    if signatures:
        extract_signatures(appname, use_idNative)
    
    # 步骤 3: 查找 native 函数偏移量（可选）
    if libraries:
        extract_library_offset(appname, library_specific=library_specific, device_id=device_id)
    
    print(f"{CYAN}[+]{NC} Finished Analyzing {appname}")


# ============================================================================
# 主程序入口
# ============================================================================
if __name__ == "__main__":
    
    # 创建命令行参数解析器
    parser = argparse.ArgumentParser(description='extract the native function signatures and libries/offsets for all apks or a given apk')
    
    # 定义命令行参数
    parser.add_argument("--target", type=str, required=True, 
                        help="the app to analyze, by default all apps inside the target_APK folder")
    # 注释掉的参数：提取 native 库到 libs 文件夹
    #parser.add_argument("-e", "--extract", required=False, default=False, action="store_true", 
    #                    help="extract the native libraries from the apk into the libs folder (if libraries already exist backup copies are made)")
    parser.add_argument("-s", "--signatures", required=False, default=False, action="store_true", 
                        help="extract the native funciton signatures (with NativeSignatures i.e. jadx + qdox)")
    parser.add_argument("-s_idn", "--signatures_idNative", required=False, default=False, action="store_true", 
                        help="if this is set, the prepocssing script will look for static_analysis/{appname}.json for the native function signatures")
    parser.add_argument("-l", "--libraries", required=False, default=False, action="store_true", 
                        help="extract the native function library names/offset (with JNIOffset)")
    parser.add_argument("--libraries_specific", required=False, type=str, 
                        help="only extract function offsets from specific library")
    parser.add_argument("-f", "--simple_flowdroid", required=False, default=False, action="store_true", 
                        help="run the simple flowdroid analysis on the apps")
    parser.add_argument("-c", "--cleanup", required=False, default=False, action="store_true", 
                        help="if set no analysis is done and instead all files created by the prepocessing are deleted")
    parser.add_argument("--init", required=False, default=False, action="store_true", 
                        help="if set the jniOffsetfinder and script are newly uploaded")
    parser.add_argument("-d", "--device", required=False, 
                        help="specify device to be used for the offset extraction task")
    # 注释掉的参数：自定义 target_APK 路径
    #parser.add_argument("--target_APK_path", required=False, help="another path to target_APK_path")
    

    args = parser.parse_args()

    # 处理清理选项：删除预处理生成的文件，只保留 base.apk
    if args.cleanup:
        if args.target == "all":
            # 安全起见，不允许清理所有应用
            print("we're not just deleting everything sorry...")
            exit(0)
        # 备份 base.apk，删除其他文件，然后恢复 base.apk
        os.system(f"cp {TARGET_APK_PATH}/{args.target}/base.apk /tmp/{args.target}.apk")
        os.system(f"rm -rf {TARGET_APK_PATH}/{args.target}/*")
        os.system(f"cp /tmp/{args.target}.apk {TARGET_APK_PATH}/{args.target}/base.apk")

    # 确定要使用的设备 ID
    device_id = None
    if args.device is not None:
        device_id = args.device
    if len(adb.get_device_ids()) >= 1 and device_id is None:
        # 如果没有指定设备但有多个设备连接，选择第一个设备
        device_id = adb.get_device_ids()[0]

    # 检查设备状态
    status = adb.check_device(device_id)
    if status != "OK":
        print(f"{RED}[ERR]{NC} device {device_id} is not functional: {status} !!ABORTING!!")
        exit(-1)

    # 如果指定了 --init，重新初始化设备上的 JNIOffset 工具
    if args.init:
        init_JNIOffset(device_id=device_id)

    # 如果没有指定任何分析选项，则执行所有分析
    if not args.libraries and not args.signatures and not args.simple_flowdroid:
        args.libraries = True
        args.signatures = True
        args.simple_flowdroid = True

    # 注释掉的功能：提取库文件
    #if args.extract:
    #    extract_libs(args.target)

    # 执行单个 APK 的分析
    analyze_single_apk(args.target, args.signatures, args.libraries, simple_FlowDroid=args.simple_flowdroid, 
        library_specific=args.libraries_specific, use_idNative=args.signatures_idNative, device_id=device_id)
