#include <dlfcn.h>
#include <errno.h>
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





typedef jstring (*function_0)(JNIEnv*, jclass, jobject, jstring, jstring);


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

  set_stage("input_parsing");
  
  
  
  size_t str1_len = 0;
  if (length - offset > 0) {
    str1_len = (length - offset > 252) ? 252 : (length - offset);
  } else {
    str1_len = 1;
  }
  char str1_buf[256];
  memcpy(str1_buf, buffer + offset, str1_len);
  str1_buf[str1_len] = '\0';
  jstring jstr1 = (*ctx.env)->NewStringUTF(ctx.env, str1_buf);
  offset += str1_len;

  
  size_t str2_len = 0;
  if (length - offset > 0) {
    str2_len = (length - offset > 252) ? 252 : (length - offset);
  } else {
    str2_len = 1;
  }
  char str2_buf[256];
  memcpy(str2_buf, buffer + offset, str2_len);
  str2_buf[str2_len] = '\0';
  jstring jstr2 = (*ctx.env)->NewStringUTF(ctx.env, str2_buf);
  offset += str2_len;

  set_stage("call_target");
  
  
  jstring ret = targetFunctionPtr0(ctx.env, NULL, NULL, jstr1, jstr2);
  if (check_and_clear_jni_exception("call_target")) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jstr2);
    (*ctx.env)->DeleteLocalRef(ctx.env, jstr1);
    return;
  }

  set_stage("cleanup");
  if (ret != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, ret);
  }
  (*ctx.env)->DeleteLocalRef(ctx.env, jstr2);
  (*ctx.env)->DeleteLocalRef(ctx.env, jstr1);
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
  
  void* targetLibHandle = dlopen("libdu.so", RTLD_NOW);
  if (targetLibHandle == NULL) {
    fprintf(stderr, "[HARNESS] Failed to dlopen libdu.so: %s\n", dlerror());
    return -1;
  }
  void* targetLibBase = get_library_base("libdu.so");
  if (targetLibBase == NULL) {
    fprintf(stderr, "[HARNESS] Failed to get library base for libdu.so\n");
    return -1;
  }
  (void)targetLibHandle;
  targetFunctionPtr0 = (function_0)((uintptr_t)targetLibBase + 0x36F24);

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
