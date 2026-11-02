#include <dlfcn.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "jenv.h"

// 4096 bytes consumed: 1 SIMPLE_ARRAY(byte) using full buffer as array content
#define BUFFER_SIZE 4096

// Coverage reset (覆盖率去噪): when the harness contains prerequisite calls,
// AFL bitmap is snapshot/restored so that only the target function's coverage
// delta is observed.  afl.js writes the real __afl_area_ptr here at attach
// time; stays NULL during standalone verification so the memcpy is skipped.
void** __omen_afl_area_ptr = NULL;
char   __omen_bitmap_snapshot[65536] __attribute__((aligned(4096)));

//GENERATOR_FUNCTIONDEFINTION 函数类型定义
typedef void (*function_0)(JNIEnv*, jobject, jbyteArray);

//GENERATOR_JAVACONTEXT JavaCTX 实例
static JavaCTX ctx;

//GENERATOR_GLOBALFUNCTIONS 全局函数指针
function_0 targetFunctionPtr0;

static const char* g_current_stage = "startup";
static int g_trace_stages = 0;

static size_t safe_strlen(const char* value) {
  size_t length = 0;

  if (value == NULL) {
    return 0;
  }

  while (value[length] != '\0') {
    length++;
  }

  return length;
}

static void safe_write_string(const char* value) {
  size_t length = safe_strlen(value);

  if (length > 0) {
    write(STDERR_FILENO, value, length);
  }
}

static const char* signal_name(int signum) {
  switch (signum) {
    case SIGABRT:
      return "SIGABRT";
    case SIGBUS:
      return "SIGBUS";
    case SIGFPE:
      return "SIGFPE";
    case SIGILL:
      return "SIGILL";
    case SIGSEGV:
      return "SIGSEGV";
    case SIGTRAP:
      return "SIGTRAP";
    default:
      return "UNKNOWN";
  }
}

static void crash_signal_handler(int signum) {
  safe_write_string("[HARNESS] fatal_signal=");
  safe_write_string(signal_name(signum));
  safe_write_string(" stage=");
  safe_write_string(g_current_stage);
  safe_write_string("\n");
  // Re-raise with default handler so AFL++ sees WIFSIGNALED()
  signal(signum, SIG_DFL);
  raise(signum);
}

static void install_signal_handlers(void) {
  signal(SIGABRT, crash_signal_handler);
  signal(SIGBUS, crash_signal_handler);
  signal(SIGFPE, crash_signal_handler);
  signal(SIGILL, crash_signal_handler);
  signal(SIGSEGV, crash_signal_handler);
  signal(SIGTRAP, crash_signal_handler);
}

static void set_stage(const char* stage) {
  g_current_stage = stage;
  if (g_trace_stages == 0) {
    return;
  }
  fprintf(stderr, "[HARNESS] stage=%s\n", stage);
  fflush(stderr);
}

static int check_and_clear_jni_exception(const char* stage) {
  if ((*ctx.env)->ExceptionCheck(ctx.env) == JNI_FALSE) {
    return 0;
  }

  fprintf(stderr, "[HARNESS] JNI exception at stage=%s\n", stage);
  (*ctx.env)->ExceptionDescribe(ctx.env);
  (*ctx.env)->ExceptionClear(ctx.env);
  fflush(stderr);
  return 1;
}

/**
 * AFL++ 持久化模式入口函数
 */
void fuzz_one_input(const uint8_t* buffer, size_t length) {
  set_stage("input_parsing");
  // GENERATOR_INPUTPARSING — construct jbyteArray from full fuzz buffer
  jbyteArray jData = (*ctx.env)->NewByteArray(ctx.env, length);
  if (jData == NULL) {
    set_stage("cleanup");
    return;
  }
  (*ctx.env)->SetByteArrayRegion(ctx.env, jData, 0, length, (const jbyte*)(buffer));

  set_stage("call_target");
  // GENERATOR_CALLINGTARGETFUNCTION — no prerequisite calls; directly invoke target
  targetFunctionPtr0(ctx.env, NULL, jData);
  if (check_and_clear_jni_exception("call_target")) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jData);
    set_stage("cleanup");
    return;
  }

  set_stage("cleanup");
  (*ctx.env)->DeleteLocalRef(ctx.env, jData);
}

int main(void) {
  int status;
  uint8_t buffer[BUFFER_SIZE];
  char* options = "-Djava.class.path=./base.apk";
  ssize_t rlength;

  install_signal_handlers();
  g_trace_stages = (getenv("OMEN_TRACE_STAGES") != NULL &&
    strcmp(getenv("OMEN_TRACE_STAGES"), "0") != 0);

  set_stage("read_stdin");
  rlength = fread(buffer, 1, BUFFER_SIZE, stdin);
  if (rlength == -1) {
    return errno;
  }

  set_stage("init_java_env");
  if ((status = init_java_env(&ctx, &options, 1)) != 0) {
    fprintf(stderr, "[HARNESS] init_java_env failed status=%d\n", status);
    return status;
  }

  if (check_and_clear_jni_exception("init_java_env")) {
    return JNI_ERR;
  }

  set_stage("resolve_target_functions");
  // GENERATOR_FUNCTIONOFFSETS 函数地址初始化
  void* targetLibHandle = dlopen("libblogfuzz.so", RTLD_NOW);
  if (targetLibHandle == NULL) {
    fprintf(stderr, "[HARNESS] Failed to dlopen libblogfuzz.so: %s\n", dlerror());
    return -1;
  }
  void* targetLibBase = get_library_base("libblogfuzz.so");
  if (targetLibBase == NULL) {
    fprintf(stderr, "[HARNESS] Failed to get library base for libblogfuzz.so\n");
    return -1;
  }
  (void)targetLibHandle;
  targetFunctionPtr0 = (function_0)((uintptr_t)targetLibBase + 0xC1C);

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
