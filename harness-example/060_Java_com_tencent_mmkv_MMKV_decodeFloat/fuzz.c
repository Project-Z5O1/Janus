#include <dlfcn.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "jenv.h"


#define BUFFER_SIZE 260





void** __omen_afl_area_ptr = NULL;
char   __omen_bitmap_snapshot[65536] __attribute__((aligned(4096)));



typedef void (*prereq_function_0)(JNIEnv*, jobject, jstring, jstring, jint);

typedef jlong (*prereq_function_1)(JNIEnv*, jobject, jint, jstring);

typedef jfloat (*function_0)(JNIEnv*, jobject, jlong, jstring, jfloat);


static JavaCTX ctx;


prereq_function_0 prereqFunctionPtr0;
prereq_function_1 prereqFunctionPtr1;
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
  
  uint32_t keyLen = 0;
  if (length - offset >= 4) {
    keyLen = *(uint32_t*)(buffer + offset);
    offset += 4;
  }
  size_t maxKeyLen = (length > offset + 4) ? (length - offset - 4) : 0;
  if (maxKeyLen > 0 && keyLen > maxKeyLen) {
    keyLen = (uint32_t)maxKeyLen;
  }
  if (keyLen > 252) {
    keyLen = 252;
  }

  char keyBuf[256];
  jstring jKey = NULL;
  if (keyLen > 0 && offset + keyLen <= length) {
    memcpy(keyBuf, buffer + offset, keyLen);
    keyBuf[keyLen] = '\0';
    jKey = (*ctx.env)->NewStringUTF(ctx.env, keyBuf);
    offset += keyLen;
  } else {
    jKey = (*ctx.env)->NewStringUTF(ctx.env, "");
  }
  if (check_and_clear_jni_exception("input_parsing_key")) { return; }

  jfloat defaultValue = 0.0f;
  if (length - offset >= 4) {
    defaultValue = *(jfloat*)(buffer + offset);
    offset += 4;
  }

  jstring jRootDir = NULL;
  jstring jCacheDir = NULL;

  
  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy((void*)*__omen_afl_area_ptr, __omen_bitmap_snapshot, 65536);
  }

  set_stage("prereq_jniInitialize");
  jRootDir = (*ctx.env)->NewStringUTF(ctx.env, "/data/local/tmp/mmkv");
  jCacheDir = (*ctx.env)->NewStringUTF(ctx.env, "/data/local/tmp");
  prereqFunctionPtr0(ctx.env, NULL, jRootDir, jCacheDir, 1);
  if (check_and_clear_jni_exception("prereq_jniInitialize")) {
    goto cleanup;
  }

  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy((void*)*__omen_afl_area_ptr, __omen_bitmap_snapshot, 65536);
  }

  set_stage("prereq_getDefaultMMKV");
  jlong nativeHandle = prereqFunctionPtr1(ctx.env, NULL, 1, NULL);
  if (check_and_clear_jni_exception("prereq_getDefaultMMKV")) {
    goto cleanup;
  }

  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy(__omen_bitmap_snapshot, (void*)*__omen_afl_area_ptr, 65536);
  }

  set_stage("call_target");
  targetFunctionPtr0(ctx.env, NULL, nativeHandle, jKey, defaultValue);
  if (check_and_clear_jni_exception("call_target")) {
    goto cleanup;
  }

cleanup:
  set_stage("cleanup");
  if (jKey) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jKey);
  }
  if (jCacheDir) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jCacheDir);
  }
  if (jRootDir) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jRootDir);
  }
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
  
  
  void* targetLibHandle = dlopen("libmmkv.so", RTLD_NOW);
  if (targetLibHandle == NULL) {
    fprintf(stderr, "[HARNESS] Failed to dlopen libmmkv.so: %s\n", dlerror());
    return -1;
  }
  void* targetLibBase = get_library_base("libmmkv.so");
  if (targetLibBase == NULL) {
    fprintf(stderr, "[HARNESS] Failed to get library base for libmmkv.so\n");
    return -1;
  }
  (void)targetLibHandle;

  
  prereqFunctionPtr0 = (prereq_function_0)((uintptr_t)targetLibBase + 0x1795C);
  
  prereqFunctionPtr1 = (prereq_function_1)((uintptr_t)targetLibBase + 0x18048);
  
  targetFunctionPtr0 = (function_0)((uintptr_t)targetLibBase + 0x18AC8);

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
