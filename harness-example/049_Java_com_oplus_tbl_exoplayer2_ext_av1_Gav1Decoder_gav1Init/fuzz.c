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

























typedef jlong (*function_0)(JNIEnv*, jclass, jint);
typedef jint (*function_check)(JNIEnv*, jclass, jlong);
typedef void (*function_release_close)(JNIEnv*, jclass, jlong);


static JavaCTX ctx;



function_0 targetFunctionPtr0;             
function_check assertFunctionPtr0;         
function_release_close releaseFunctionPtr0; 



static uint8_t g_input_digest = 0;

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
  





  jlong decoderCtx = 0;
  jint initOk = -1;

  set_stage("input_parsing");
  
  










  {
    uint8_t digest = 0;
    size_t i;
    for (i = 0; i < length; i++) {
      digest = (uint8_t)(digest ^ buffer[i]);
    }
    g_input_digest = digest;
  }

  set_stage("get_obj");
  





  set_stage("call_jni");
  
  
  decoderCtx = targetFunctionPtr0(ctx.env, NULL, 1);
  if (check_and_clear_jni_exception("call_jni_gav1Init")) {
    decoderCtx = 0;
    goto release;
  }

  







  if (decoderCtx != 0) {
    initOk = assertFunctionPtr0(ctx.env, NULL, decoderCtx);
    if (check_and_clear_jni_exception("call_jni_gav1CheckError")) {
      initOk = -1;
    }
  }
  fprintf(stderr, "[HARNESS] gav1Init ctx=0x%llx check_error=%d (1=init_ok) "
                  "len=%zu digest=0x%02x\n", (unsigned long long)decoderCtx,
          (int)initOk, length, (unsigned)g_input_digest);
  fflush(stderr);

release:
  set_stage("release");
  
  
  
  
  
  
  
  
  
  
  
  
  if (decoderCtx != 0) {
    releaseFunctionPtr0(ctx.env, NULL, decoderCtx);
    check_and_clear_jni_exception("release_gav1Close");
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
  
  
  void* targetLibHandle = dlopen("libgav1JNI.so", RTLD_NOW);
  if (targetLibHandle == NULL) {
    fprintf(stderr, "[HARNESS] Failed to dlopen libgav1JNI.so: %s\n", dlerror());
    return -1;
  }
  void* targetLibBase = get_library_base("libgav1JNI.so");
  if (targetLibBase == NULL) {
    fprintf(stderr, "[HARNESS] Failed to get library base for libgav1JNI.so\n");
    return -1;
  }
  (void)targetLibHandle;
  







  targetFunctionPtr0 = (function_0)((uintptr_t)targetLibBase + 0x18384);       
  assertFunctionPtr0 = (function_check)((uintptr_t)targetLibBase + 0x19320);   
  

  releaseFunctionPtr0 = (function_release_close)((uintptr_t)targetLibBase + 0x1887c);

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
