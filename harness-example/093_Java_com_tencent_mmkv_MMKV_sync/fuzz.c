#include <dlfcn.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "jenv.h"

#define BUFFER_SIZE 256

void** __omen_afl_area_ptr = NULL;
char   __omen_bitmap_snapshot[65536] __attribute__((aligned(4096)));



typedef void (*function_0)(JNIEnv*, jobject, jboolean);

typedef void (*function_1)(JNIEnv*, jclass, jstring, jstring, jint);

typedef jlong (*function_2)(JNIEnv*, jclass, jint, jstring);


static JavaCTX ctx;


function_0 targetFunctionPtr0;
function_1 prereqFunctionPtr0;  
function_2 prereqFunctionPtr1;  

static jobject g_mmkv_obj = NULL;

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
    case SIGBUS: return "SIGBUS";
    case SIGFPE: return "SIGFPE";
    case SIGILL: return "SIGILL";
    case SIGSEGV: return "SIGSEGV";
    case SIGTRAP: return "SIGTRAP";
    default: return "UNKNOWN";
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
  if (g_trace_stages) {
    fprintf(stderr, "[HARNESS] stage=%s\n", stage);
    fflush(stderr);
  }
}

static int check_and_clear_jni_exception(const char* stage) {
  if ((*ctx.env)->ExceptionCheck(ctx.env) == JNI_FALSE) return 0;
  fprintf(stderr, "[HARNESS] JNI exception at stage=%s\n", stage);
  (*ctx.env)->ExceptionDescribe(ctx.env);
  (*ctx.env)->ExceptionClear(ctx.env);
  fflush(stderr);
  return 1;
}

void fuzz_one_input(const uint8_t* buffer, size_t length) {
  set_stage("input_parsing");
  
  jboolean syncFlag = (length > 0) ? ((buffer[0] & 1) ? JNI_TRUE : JNI_FALSE) : JNI_FALSE;

  set_stage("call_target");
  
  targetFunctionPtr0(ctx.env, g_mmkv_obj, syncFlag);
  if (check_and_clear_jni_exception("call_target")) {
    return;
  }

  set_stage("cleanup");
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
    fflush(stderr);
    return status;
  }
  if (check_and_clear_jni_exception("init_java_env")) return JNI_ERR;

  set_stage("resolve_target_functions");
  
  void* targetLibHandle = dlopen("libmmkv.so", RTLD_NOW);
  if (targetLibHandle == NULL) {
    fprintf(stderr, "[HARNESS] Failed to dlopen libmmkv.so: %s\n", dlerror());
    fflush(stderr);
    return -1;
  }
  fprintf(stderr, "[HARNESS] libmmkv.so loaded\n");
  fflush(stderr);

  
  
  typedef jint (*JniOnLoadFn)(JavaVM*, void*);
  JniOnLoadFn jniOnLoad = (JniOnLoadFn)dlsym(targetLibHandle, "JNI_OnLoad");
  if (jniOnLoad != NULL) {
    jint onLoadResult = jniOnLoad(ctx.vm, NULL);
    fprintf(stderr, "[HARNESS] JNI_OnLoad returned %d\n", onLoadResult);
    fflush(stderr);
  } else {
    fprintf(stderr, "[HARNESS] No JNI_OnLoad in libmmkv.so\n");
    fflush(stderr);
  }

  void* targetLibBase = get_library_base("libmmkv.so");
  if (targetLibBase == NULL) {
    fprintf(stderr, "[HARNESS] Failed to get library base for libmmkv.so\n");
    fflush(stderr);
    return -1;
  }
  targetFunctionPtr0 = (function_0)((uintptr_t)targetLibBase + 0x17700);
  prereqFunctionPtr0 = (function_1)((uintptr_t)targetLibBase + 0x1795C);  
  prereqFunctionPtr1 = (function_2)((uintptr_t)targetLibBase + 0x18048);  

  
  set_stage("mmkv_setup");
  {
    jclass mmkvClass = (*ctx.env)->FindClass(ctx.env, "com/tencent/mmkv/MMKV");
    if (mmkvClass == NULL || (*ctx.env)->ExceptionCheck(ctx.env)) {
      check_and_clear_jni_exception("mmkv_findclass");
      fprintf(stderr, "[HARNESS] MMKV class not found\n");
      fflush(stderr);
    } else {
      
      
      jstring rootDir = (*ctx.env)->NewStringUTF(ctx.env,
        "/data/local/tmp/omen/Java_com_tencent_mmkv_MMKV_sync");
      jstring cacheDir = (*ctx.env)->NewStringUTF(ctx.env,
        "/data/local/tmp/omen/Java_com_tencent_mmkv_MMKV_sync/cache");
      prereqFunctionPtr0(ctx.env, mmkvClass, rootDir, cacheDir, (jint)1);
      check_and_clear_jni_exception("jniInitialize");

      (*ctx.env)->DeleteLocalRef(ctx.env, rootDir);
      (*ctx.env)->DeleteLocalRef(ctx.env, cacheDir);

      
      jlong nativeHandle = prereqFunctionPtr1(ctx.env, mmkvClass, (jint)1, NULL);
      check_and_clear_jni_exception("getDefaultMMKV");
      fprintf(stderr, "[HARNESS] MMKV nativeHandle=0x%lx\n", nativeHandle);
      fflush(stderr);

      
      jmethodID mmkvCtor = (*ctx.env)->GetMethodID(ctx.env, mmkvClass, "<init>", "(J)V");
      if (mmkvCtor != NULL && !(*ctx.env)->ExceptionCheck(ctx.env)) {
        jobject localMMKV = (*ctx.env)->NewObject(ctx.env, mmkvClass, mmkvCtor, nativeHandle);
        if (localMMKV != NULL && !(*ctx.env)->ExceptionCheck(ctx.env)) {
          g_mmkv_obj = (*ctx.env)->NewGlobalRef(ctx.env, localMMKV);
          (*ctx.env)->DeleteLocalRef(ctx.env, localMMKV);
          fprintf(stderr, "[HARNESS] MMKV created with real nativeHandle\n");
          fflush(stderr);
        } else {
          fprintf(stderr, "[HARNESS] MMKV NewObject failed, falling back to NULL this\n");
          fflush(stderr);
        }
      }
      (*ctx.env)->DeleteLocalRef(ctx.env, mmkvClass);
    }
    check_and_clear_jni_exception("mmkv_setup_end");
  }

  if (g_mmkv_obj == NULL) {
    fprintf(stderr, "[HARNESS] MMKV not available, sync with NULL this\n");
    fflush(stderr);
  }

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
