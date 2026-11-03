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


typedef void     (*prereqFunctionPtr0_t)(JNIEnv*, jobject, jstring, jstring, jint);
typedef jlong    (*prereqFunctionPtr1_t)(JNIEnv*, jobject, jint, jstring);
typedef jstring  (*targetFunctionPtr0_t)(JNIEnv*, jobject, jlong, jstring, jstring);


static JavaCTX ctx;


prereqFunctionPtr0_t prereqFunctionPtr0;
prereqFunctionPtr1_t prereqFunctionPtr1;
targetFunctionPtr0_t targetFunctionPtr0;

static const char* g_current_stage = "startup";
static int g_trace_stages = 0;

static size_t safe_strlen(const char* value) {
  size_t length = 0;
  if (value == NULL) return 0;
  while (value[length] != '\0') length++;
  return length;
}

static void safe_write_string(const char* value) {
  size_t length = safe_strlen(value);
  if (length > 0) write(STDERR_FILENO, value, length);
}

static const char* signal_name(int signum) {
  switch (signum) {
    case SIGABRT: return "SIGABRT";
    case SIGBUS:  return "SIGBUS";
    case SIGFPE:  return "SIGFPE";
    case SIGILL:  return "SIGILL";
    case SIGSEGV: return "SIGSEGV";
    case SIGTRAP: return "SIGTRAP";
    default:      return "UNKNOWN";
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
  if (g_trace_stages == 0) return;
  fprintf(stderr, "[HARNESS] stage=%s\n", stage);
  fflush(stderr);
}

static int check_and_clear_jni_exception(const char* stage) {
  if ((*ctx.env)->ExceptionCheck(ctx.env) == JNI_FALSE) return 0;
  fprintf(stderr, "[HARNESS] JNI exception at stage=%s\n", stage);
  (*ctx.env)->ExceptionDescribe(ctx.env);
  (*ctx.env)->ExceptionClear(ctx.env);
  fflush(stderr);
  return 1;
}

static jstring read_jstring_from_buffer(JNIEnv* env,
    const uint8_t* buffer, size_t length, size_t* offset, size_t max_content) {
  uint32_t strLen = 0;
  if (*offset + 4 > length) return NULL;
  strLen = *(uint32_t*)(buffer + *offset);
  *offset += 4;
  if (strLen > max_content) strLen = (uint32_t)max_content;
  if (strLen > (uint32_t)(length - *offset)) strLen = (uint32_t)(length - *offset);
  if (strLen == 0) return (*env)->NewStringUTF(env, "");
  char strBuf[257];
  memcpy(strBuf, buffer + *offset, strLen);
  strBuf[strLen] = '\0';
  *offset += strLen;
  return (*env)->NewStringUTF(env, strBuf);
}

void fuzz_one_input(const uint8_t* buffer, size_t length) {
  size_t offset = 0;
  jstring result = NULL;

  set_stage("input_parsing");
  
  jstring jKey = read_jstring_from_buffer(ctx.env, buffer, length, &offset, 252);
  jstring jDefault = read_jstring_from_buffer(ctx.env, buffer, length, &offset, 252);

  set_stage("call_target");
  

  
  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy((void*)*__omen_afl_area_ptr, __omen_bitmap_snapshot, 65536);
  }

  
  set_stage("prereq_jniInitialize");
  jstring jRootPath = (*ctx.env)->NewStringUTF(ctx.env, "/data/mmkv");
  jstring jCachePath = (*ctx.env)->NewStringUTF(ctx.env, "/data/cache");
  prereqFunctionPtr0(ctx.env, NULL, jRootPath, jCachePath, 1);
  check_and_clear_jni_exception("prereq_jniInitialize");
  (*ctx.env)->DeleteLocalRef(ctx.env, jRootPath);
  (*ctx.env)->DeleteLocalRef(ctx.env, jCachePath);

  
  set_stage("prereq_getDefaultMMKV");
  jlong nativeHandle = prereqFunctionPtr1(ctx.env, NULL, 1, NULL);
  check_and_clear_jni_exception("prereq_getDefaultMMKV");

  
  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy(__omen_bitmap_snapshot, (void*)*__omen_afl_area_ptr, 65536);
  }

  
  set_stage("call_target_func");
  result = targetFunctionPtr0(ctx.env, NULL, nativeHandle, jKey, jDefault);
  check_and_clear_jni_exception("call_target");

  (void)result;

cleanup:
  set_stage("cleanup");
  if (result) (*ctx.env)->DeleteLocalRef(ctx.env, result);
  if (jDefault) (*ctx.env)->DeleteLocalRef(ctx.env, jDefault);
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
  if (rlength == -1) return errno;

  set_stage("init_java_env");
  if ((status = init_java_env(&ctx, &options, 1)) != 0) {
    fprintf(stderr, "[HARNESS] init_java_env failed status=%d\n", status);
    return status;
  }
  if (check_and_clear_jni_exception("init_java_env")) return JNI_ERR;

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

  
  prereqFunctionPtr0 = (prereqFunctionPtr0_t)((uintptr_t)mmkvBase + 0x1795C);
  
  prereqFunctionPtr1 = (prereqFunctionPtr1_t)((uintptr_t)mmkvBase + 0x18048);
  
  targetFunctionPtr0 = (targetFunctionPtr0_t)((uintptr_t)mmkvBase + 0x18FB4);

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
