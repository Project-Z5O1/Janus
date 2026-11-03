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





typedef jlong (*prereq_function_0)(JNIEnv*, jclass, jint, jstring);




typedef jboolean (*prereq_function_1)(JNIEnv*, jobject, jlong, jstring, jobjectArray);




typedef jobjectArray (*target_function_0)(JNIEnv*, jobject, jlong, jstring);


static JavaCTX ctx;


prereq_function_0 prereqFunctionPtr0;
prereq_function_1 prereqFunctionPtr1;
target_function_0 targetFunctionPtr0;
static jclass mmkvClass = NULL;
static jclass stringClass = NULL;

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
  

  
  size_t strContentBytes = (length - offset > 252) ? 252 : (length - offset);
  char keyStr[253];
  if (strContentBytes > 0) {
    memcpy(keyStr, buffer + offset, strContentBytes);
  }
  keyStr[strContentBytes] = '\0';
  offset += strContentBytes;
  jstring jKey = (*ctx.env)->NewStringUTF(ctx.env, keyStr);

  
  
  
  uint8_t numRaw = (offset < length) ? buffer[offset] : 0;
  offset += 1;
  uint32_t numValues = (numRaw > 4) ? 4 : numRaw;

  jobjectArray jValues = NULL;
  if (numValues > 0 && stringClass != NULL) {
    jValues = (*ctx.env)->NewObjectArray(ctx.env, (jsize)numValues,
      stringClass, NULL);
    for (uint32_t i = 0; i < numValues && offset < length; i++) {
      size_t valLen = (length - offset > 63) ? 63 : (length - offset);
      char valStr[64];
      if (valLen > 0) {
        memcpy(valStr, buffer + offset, valLen);
      }
      valStr[valLen] = '\0';
      offset += valLen;
      jstring jVal = (*ctx.env)->NewStringUTF(ctx.env, valStr);
      (*ctx.env)->SetObjectArrayElement(ctx.env, jValues, (jsize)i, jVal);
      (*ctx.env)->DeleteLocalRef(ctx.env, jVal);
    }
  }

  set_stage("call_target");
  
  
  

  
  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy((void*)*__omen_afl_area_ptr, __omen_bitmap_snapshot, 65536);
  }

  
  set_stage("prereq_getDefaultMMKV");
  jlong nativeHandle = prereqFunctionPtr0(ctx.env, mmkvClass, 1, NULL);
  check_and_clear_jni_exception("prereq_getDefaultMMKV");

  
  set_stage("prereq_encodeSet");
  if (jValues != NULL) {
    prereqFunctionPtr1(ctx.env, NULL, nativeHandle, jKey, jValues);
    check_and_clear_jni_exception("prereq_encodeSet");
  }

  
  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy(__omen_bitmap_snapshot, (void*)*__omen_afl_area_ptr, 65536);
  }

  
  set_stage("call_target_func");
  jobjectArray result = targetFunctionPtr0(ctx.env, NULL, nativeHandle, jKey);
  check_and_clear_jni_exception("call_target");

  (void)result;

cleanup:
  set_stage("cleanup");
  
  if (result) {
    (*ctx.env)->DeleteLocalRef(ctx.env, result);
  }
  if (jValues) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jValues);
  }
  if (jKey) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jKey);
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

  
  prereqFunctionPtr0 = (prereq_function_0)((uintptr_t)mmkvBase + 0x18048);
  
  prereqFunctionPtr1 = (prereq_function_1)((uintptr_t)mmkvBase + 0x1913C);
  
  targetFunctionPtr0 = (target_function_0)((uintptr_t)mmkvBase + 0x19334);

  
  typedef jint (*jni_onload_t)(JavaVM*, void*);
  jni_onload_t JNI_OnLoad_fn = (jni_onload_t)dlsym(mmkvHandle, "JNI_OnLoad");
  if (JNI_OnLoad_fn != NULL) {
    jint jniVersion = JNI_OnLoad_fn(ctx.vm, NULL);
    fprintf(stderr, "[HARNESS] JNI_OnLoad returned version=%d\n", jniVersion);
  } else {
    fprintf(stderr, "[HARNESS] JNI_OnLoad not found, continuing\n");
  }

  typedef void (*mmkv_init_t)(const char*, int);
  mmkv_init_t mmkvInit = (mmkv_init_t)dlsym(mmkvHandle, "mmkvInitialize");
  if (mmkvInit != NULL) {
    mmkvInit(
      "/data/local/tmp/omen/Java_com_tencent_mmkv_MMKV_decodeStringSet", 0);
    fprintf(stderr, "[HARNESS] mmkvInitialize called\n");
  }

  
  mmkvClass = (*ctx.env)->FindClass(ctx.env, "com/tencent/mmkv/MMKV");
  if (mmkvClass == NULL || (*ctx.env)->ExceptionCheck(ctx.env)) {
    fprintf(stderr, "[HARNESS] Failed to find MMKV class\n");
    check_and_clear_jni_exception("find_mmkv_class");
    return -1;
  }

  
  stringClass = (*ctx.env)->FindClass(ctx.env, "java/lang/String");
  if (stringClass == NULL || (*ctx.env)->ExceptionCheck(ctx.env)) {
    fprintf(stderr, "[HARNESS] Failed to find String class\n");
    check_and_clear_jni_exception("find_string_class");
    return -1;
  }

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
