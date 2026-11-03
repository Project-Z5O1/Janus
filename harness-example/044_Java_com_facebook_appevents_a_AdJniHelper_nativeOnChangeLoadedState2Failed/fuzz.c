#include <dlfcn.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "jenv.h"



#define BUFFER_SIZE 512



void** __omen_afl_area_ptr = NULL;
char   __omen_bitmap_snapshot[65536] __attribute__((aligned(4096)));



static sigjmp_buf g_crash_jmpbuf;
static volatile int g_crash_jmpbuf_valid = 0;


typedef void (*function_0)(JNIEnv*, jclass, jstring, jstring);


static JavaCTX ctx;


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

  
  
  
  if (g_crash_jmpbuf_valid) {
    g_crash_jmpbuf_valid = 0;
    siglongjmp(g_crash_jmpbuf, 1);
  }

  
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




void fuzz_one_input(const uint8_t* buffer, size_t length) {
  size_t offset = 0;
  jstring jStr0 = NULL;
  jstring jStr1 = NULL;

  
  if (sigsetjmp(g_crash_jmpbuf, 1) != 0) {
    set_stage("recovered");
    g_crash_jmpbuf_valid = 0;
    if (jStr1) (*ctx.env)->DeleteLocalRef(ctx.env, jStr1);
    if (jStr0) (*ctx.env)->DeleteLocalRef(ctx.env, jStr0);
    return;
  }
  g_crash_jmpbuf_valid = 1;

  set_stage("parse_string_0");
  uint32_t strLen0 = 0;
  if (length - offset >= 4) {
    strLen0 = *(const uint32_t*)(buffer + offset);
  }
  offset += 4;
  if (strLen0 > (length - offset)) {
    strLen0 = (length - offset);
  }
  if (strLen0 > 252) {
    strLen0 = 252;
  }
  char strBuf0[256];
  memset(strBuf0, 0, sizeof(strBuf0));
  if (strLen0 > 0) {
    memcpy(strBuf0, buffer + offset, strLen0);
  }
  jStr0 = (*ctx.env)->NewStringUTF(ctx.env, strBuf0);
  check_and_clear_jni_exception("parse_string_0");
  offset += strLen0;

  set_stage("parse_string_1");
  uint32_t strLen1 = 0;
  if (length - offset >= 4) {
    strLen1 = *(const uint32_t*)(buffer + offset);
  }
  offset += 4;
  if (strLen1 > (length - offset)) {
    strLen1 = (length - offset);
  }
  if (strLen1 > 252) {
    strLen1 = 252;
  }
  char strBuf1[256];
  memset(strBuf1, 0, sizeof(strBuf1));
  if (strLen1 > 0) {
    memcpy(strBuf1, buffer + offset, strLen1);
  }
  jStr1 = (*ctx.env)->NewStringUTF(ctx.env, strBuf1);
  check_and_clear_jni_exception("parse_string_1");
  offset += strLen1;

  set_stage("call_target");
  targetFunctionPtr0(ctx.env, NULL, jStr0, jStr1);
  check_and_clear_jni_exception("call_target");

  set_stage("cleanup");
  g_crash_jmpbuf_valid = 0;
  if (jStr1) (*ctx.env)->DeleteLocalRef(ctx.env, jStr1);
  if (jStr0) (*ctx.env)->DeleteLocalRef(ctx.env, jStr0);
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
  
  void* targetLibHandle = dlopen("libBilliards.so", RTLD_NOW);
  if (targetLibHandle == NULL) {
    fprintf(stderr, "[HARNESS] Failed to dlopen libBilliards.so: %s\n", dlerror());
    return -1;
  }
  void* targetLibBase = get_library_base("libBilliards.so");
  if (targetLibBase == NULL) {
    fprintf(stderr, "[HARNESS] Failed to get library base for libBilliards.so\n");
    return -1;
  }
  (void)targetLibHandle;
  targetFunctionPtr0 = (function_0)((uintptr_t)targetLibBase + 0x7f3df0);

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
