#include <dlfcn.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "jenv.h"

// Template default — the harness refiner overrides this value from the
// argument plan. See feedback-driven-harness-refiner's template contract.
// This value must equal BUF_LEN in afl.js and the -G flag passed to afl-fuzz.
#define BUFFER_SIZE 256

// Coverage reset (覆盖率去噪): when the harness contains prerequisite calls,
// AFL bitmap is snapshot/restored so that only the target function's coverage
// delta is observed.  afl.js writes the real __afl_area_ptr here at attach
// time; stays NULL during standalone verification so the memcpy is skipped.
void** __omen_afl_area_ptr = NULL;
char   __omen_bitmap_snapshot[65536] __attribute__((aligned(4096)));

//GENERATOR_FUNCTIONDEFINTION 函数类型定义, 例如 fuzzMeWrapper
typedef jobject (*function_0)(JNIEnv*, jclass, jbyteArray);

//GENERATOR_JAVACONTEXT JavaCTX 实例
static JavaCTX ctx;

//GENERATOR_GLOBALFUNCTIONS 全局函数指针
// 例如 function_0 targetFunctionPtr0;
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
  // GENERATOR_INPUTPARSING, Parse the input byte stream
  jbyteArray jBuffer = (*ctx.env)->NewByteArray(ctx.env, length);
  (*ctx.env)->SetByteArrayRegion(ctx.env, jBuffer, 0, length, (const jbyte*)buffer);
  jclass wrapperClass = (*ctx.env)->FindClass(ctx.env, "qb/blogfuzz/Wrapper");
  jmethodID wrapperCtor = (*ctx.env)->GetMethodID(ctx.env, wrapperClass, "<init>", "([B)V");
  jobject objWrapper = (*ctx.env)->NewObject(ctx.env, wrapperClass, wrapperCtor, jBuffer);
  if (check_and_clear_jni_exception("input_parsing")) {
    return;
  }

  set_stage("call_target");
  // GENERATOR_CALLINGTARGETFUNCTION, call target function
  targetFunctionPtr0(ctx.env, NULL, objWrapper);
  if (check_and_clear_jni_exception("call_target")) {
    return;
  }

  set_stage("cleanup");
  (*ctx.env)->DeleteLocalRef(ctx.env, objWrapper);
  (*ctx.env)->DeleteLocalRef(ctx.env, jBuffer);
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
  // Explicitly load the target library before resolving its base address.
  void* targetLibHandle = dlopen("YOUR_TARGET_LIBRARY.so", RTLD_NOW);
  if (targetLibHandle == NULL) {
    fprintf(stderr, "[HARNESS] Failed to dlopen YOUR_TARGET_LIBRARY.so: %s\n", dlerror());
    return -1;
  }
  void* targetLibBase = get_library_base("YOUR_TARGET_LIBRARY.so");
  if (targetLibBase == NULL) {
    fprintf(stderr, "[HARNESS] Failed to get library base for YOUR_TARGET_LIBRARY.so\n");
    return -1;
  }
  (void)targetLibHandle;
  targetFunctionPtr0 = (function_0)((uintptr_t)targetLibBase + 0x5678);

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
