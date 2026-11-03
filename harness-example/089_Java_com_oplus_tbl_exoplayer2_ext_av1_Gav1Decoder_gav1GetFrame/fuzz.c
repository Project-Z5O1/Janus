#include <dlfcn.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "jenv.h"








#define BUFFER_SIZE 65536





void** __omen_afl_area_ptr = NULL;
char   __omen_bitmap_snapshot[65536] __attribute__((aligned(4096)));





















typedef jlong (*function_prereq_1)(JNIEnv*, jclass, jint);
typedef jint (*function_prereq_2)(JNIEnv*, jclass, jlong, jobject, jint);
typedef jint (*function_0)(JNIEnv*, jclass, jlong, jobject, jboolean);
typedef void (*function_release_close)(JNIEnv*, jclass, jlong);


static JavaCTX ctx;



function_prereq_1 prereqFunctionPtr1; 
function_prereq_2 prereqFunctionPtr2; 
function_0 targetFunctionPtr0;        
function_release_close releaseFunctionPtr0; 

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


jobject construct_VideoDecoderOutputBuffer(JNIEnv* env,
                                           const uint8_t* buffer,
                                           size_t length, size_t* offset);




void fuzz_one_input(const uint8_t* buffer, size_t length) {
  





  jobject bbData = NULL;
  jobject outBuf = NULL;
  jlong decoderCtx = 0;
  size_t offset = 0;
  jint decodeRet = -1;
  jint frameRet = -1;

  set_stage("input_parsing");
  
  









  set_stage("get_obj");
  


  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy((void*)*__omen_afl_area_ptr, __omen_bitmap_snapshot, 65536);
  }

  
  decoderCtx = prereqFunctionPtr1(ctx.env, NULL, 1);
  if (check_and_clear_jni_exception("prereq1_gav1Init")) {
    decoderCtx = 0;
    goto release;
  }
  if (decoderCtx == 0) {
    fprintf(stderr, "[HARNESS] gav1Init returned NULL context (skip target)\n");
    goto release;
  }

  






  bbData = (*ctx.env)->NewDirectByteBuffer(ctx.env, (void*)buffer, (jlong)length);
  if (bbData == NULL || check_and_clear_jni_exception("get_obj_NewDirectByteBuffer")) {
    bbData = NULL;
    goto release;
  }

  








  decodeRet = prereqFunctionPtr2(ctx.env, NULL, decoderCtx, bbData, (jint)length);
  if (check_and_clear_jni_exception("prereq2_gav1Decode")) {
    goto release;
  }

  




  outBuf = construct_VideoDecoderOutputBuffer(ctx.env, buffer, length, &offset);
  if (outBuf == NULL) {
    fprintf(stderr, "[HARNESS] target.arg1 construction failed "
                    "(VideoDecoderOutputBuffer, skip target)\n");
    goto release;
  }

  


  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy(__omen_bitmap_snapshot, (void*)*__omen_afl_area_ptr, 65536);
  }

  set_stage("call_jni");
  
  







  frameRet = targetFunctionPtr0(ctx.env, NULL, decoderCtx, outBuf, (jboolean)0);
  fprintf(stderr, "[HARNESS] decode_ret=%d getframe_ret=%d len=%zu\n",
          (int)decodeRet, (int)frameRet, length);
  fflush(stderr);
  check_and_clear_jni_exception("call_jni_target");

release:
  set_stage("release");
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  if (decoderCtx != 0) {
    releaseFunctionPtr0(ctx.env, NULL, decoderCtx);
    check_and_clear_jni_exception("release_gav1Close");
  }
  if (outBuf != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, outBuf);
  }
  if (bbData != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, bbData);
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
  







  prereqFunctionPtr1 = (function_prereq_1)((uintptr_t)targetLibBase + 0x18384); 
  prereqFunctionPtr2 = (function_prereq_2)((uintptr_t)targetLibBase + 0x188e4); 
  targetFunctionPtr0 = (function_0)((uintptr_t)targetLibBase + 0x1893c);        
  

  releaseFunctionPtr0 = (function_release_close)((uintptr_t)targetLibBase + 0x1887c);

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
