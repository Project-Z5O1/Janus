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


typedef void (*function_0)(JNIEnv*, jobject, jstring, jstring, jint);
typedef jlong (*function_1)(JNIEnv*, jobject, jint, jstring);
typedef jboolean (*function_2)(JNIEnv*, jobject, jlong, jstring, jboolean);


static JavaCTX ctx;


function_0 prereqFunctionPtr0;
function_1 prereqFunctionPtr1;
function_2 targetFunctionPtr0;

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
  set_stage("input_parsing");
  
  size_t offset = 0;

  
  uint16_t keyLen = 0;
  jstring jKey = NULL;
  if (length - offset >= 2) {
    keyLen = *(uint16_t*)(buffer + offset);
    offset += 2;
  }
  if (keyLen > 0 && (length - offset) >= keyLen) {
    char* keyBuf = (char*)malloc(keyLen + 1);
    memcpy(keyBuf, buffer + offset, keyLen);
    keyBuf[keyLen] = '\0';
    jKey = (*ctx.env)->NewStringUTF(ctx.env, keyBuf);
    free(keyBuf);
    offset += keyLen;
  } else {
    jKey = (*ctx.env)->NewStringUTF(ctx.env, "");
  }
  check_and_clear_jni_exception("create_key_string");

  
  jboolean jValue = JNI_FALSE;
  if (length - offset >= 1) {
    jValue = (buffer[offset] & 1) ? JNI_TRUE : JNI_FALSE;
    offset += 1;
  }

  set_stage("call_target");
  
  
  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy((void*)*__omen_afl_area_ptr, __omen_bitmap_snapshot, 65536);
  }

  
  jstring jRootPath = (*ctx.env)->NewStringUTF(ctx.env, "/data/mmkv");
  jstring jCachePath = (*ctx.env)->NewStringUTF(ctx.env, "/data/cache");
  prereqFunctionPtr0(ctx.env, NULL, jRootPath, jCachePath, 1);
  check_and_clear_jni_exception("prereq_jniInitialize");
  (*ctx.env)->DeleteLocalRef(ctx.env, jRootPath);
  (*ctx.env)->DeleteLocalRef(ctx.env, jCachePath);

  
  jlong nativeHandle = prereqFunctionPtr1(ctx.env, NULL, 1, NULL);
  check_and_clear_jni_exception("prereq_getDefaultMMKV");

  
  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy(__omen_bitmap_snapshot, (void*)*__omen_afl_area_ptr, 65536);
  }

  
  targetFunctionPtr0(ctx.env, NULL, nativeHandle, jKey, jValue);
  check_and_clear_jni_exception("call_target");

  set_stage("cleanup");
  
  if (jKey) (*ctx.env)->DeleteLocalRef(ctx.env, jKey);
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
  
  void* mmkvHandle = dlopen("libmmkv.so", RTLD_NOW);
  if (mmkvHandle == NULL) {
    fprintf(stderr, "[HARNESS] Failed to dlopen libmmkv.so: %s\n", dlerror());
    return -1;
  }
  void* mmkvBase = get_library_base("libmmkv.so");
  if (mmkvBase == NULL) {
    fprintf(stderr, "[HARNESS] Failed to get library base for libmmkv.so\n");
    return -1;
  }
  (void)mmkvHandle;

  
  prereqFunctionPtr0 = (function_0)((uintptr_t)mmkvBase + 0x1795C);
  
  prereqFunctionPtr1 = (function_1)((uintptr_t)mmkvBase + 0x18048);
  
  targetFunctionPtr0 = (function_2)((uintptr_t)mmkvBase + 0x18380);

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
