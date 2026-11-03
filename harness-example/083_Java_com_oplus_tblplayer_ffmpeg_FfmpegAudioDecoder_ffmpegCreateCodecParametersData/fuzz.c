#include <dlfcn.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "jenv.h"













#define BUFFER_SIZE 1024










void** __omen_afl_area_ptr = NULL;
char   __omen_bitmap_snapshot[65536] __attribute__((aligned(4096)));











typedef jbyteArray (*function_0)(JNIEnv*, jclass, jint, jint);


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
  
  
  
  
  
  
  jbyteArray ret = NULL;   
  jint arg0;               
  jint arg1;               
  uint32_t raw0;
  uint32_t raw1;

  set_stage("input_parsing");
  
  
  
  
  
  
  
  
  
  if (buffer == NULL || length < 8) {
    


    fprintf(stderr, "[HARNESS] input_parsing FAIL_CONSTRUCTION length=%zu "
                    "(need 8); skipping target\n", length);
    fflush(stderr);
    goto release_stage;
  }
  memcpy(&raw0, buffer + 0, 4);
  memcpy(&raw1, buffer + 4, 4);
  arg0 = (jint)raw0;
  arg1 = (jint)raw1;
  fprintf(stderr, "[HARNESS] input_parsing arg0=%d arg1=%d (length=%zu, "
                  "bytes>=8 deterministically ignored)\n",
          (int)arg0, (int)arg1, length);
  fflush(stderr);

  
  
  
  
  set_stage("get_obj");
  

  set_stage("call_jni");
  
  
  
  
  
  
  
  
  
  
  
  ret = targetFunctionPtr0(ctx.env, NULL, arg0, arg1);
  {
    int target_exc = check_and_clear_jni_exception("call_jni");
    if (ret == NULL) {
      



      fprintf(stderr, "[HARNESS] call_jni target ret=NULL (OOM early-exit "
                      "branch, normal) exc=%d\n", target_exc);
    } else {
      jsize blob_len = (*ctx.env)->GetArrayLength(ctx.env, ret);
      








      if (blob_len <= 0 || blob_len > 4096) {
        fprintf(stderr, "[HARNESS] call_jni target ret=%p len=%d OUTSIDE "
                        "cereal-blob domain (0,4096] — observation, exc=%d\n",
                (void*)ret, (int)blob_len, target_exc);
      } else {
        fprintf(stderr, "[HARNESS] call_jni target ret=%p len=%d "
                        "(cereal-blob domain OK) exc=%d\n",
                (void*)ret, (int)blob_len, target_exc);
      }
    }
    fflush(stderr);
  }

release_stage:
  set_stage("release");
  
  
  
  
  
  
  
  
  if (ret != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, ret);
    ret = NULL;
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
  
  
  
  
  
  
  
  void* targetLibHandle = dlopen("libffmpegJNI.so", RTLD_NOW);
  if (targetLibHandle == NULL) {
    fprintf(stderr, "[HARNESS] Failed to dlopen libffmpegJNI.so: %s\n", dlerror());
    return -1;
  }
  void* targetLibBase = get_library_base("libffmpegJNI.so");
  if (targetLibBase == NULL) {
    fprintf(stderr, "[HARNESS] Failed to get library base for libffmpegJNI.so\n");
    return -1;
  }
  (void)targetLibHandle;
  targetFunctionPtr0 =
      (function_0)((uintptr_t)targetLibBase + 0x6E398u);
  fprintf(stderr,
          "[HARNESS] libffmpegJNI.so base=%p offset 0x6E398 resolved "
          "(JNI_OnLoad skipped per S_m=<m> contract)\n", targetLibBase);
  fflush(stderr);

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
