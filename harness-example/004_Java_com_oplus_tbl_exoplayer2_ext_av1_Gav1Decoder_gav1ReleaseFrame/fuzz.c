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
typedef jint (*function_prereq_3)(JNIEnv*, jclass, jlong, jobject, jboolean);
typedef void (*function_0)(JNIEnv*, jclass, jlong, jobject);
typedef void (*function_release_close)(JNIEnv*, jclass, jlong);


static JavaCTX ctx;



function_prereq_1 prereqFunctionPtr1; 
function_prereq_2 prereqFunctionPtr2; 
function_prereq_3 prereqFunctionPtr3; 
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















static int omen_read_int32(uintptr_t base, size_t offset) {
  int32_t value = 0;
  memcpy(&value, (const void*)(base + offset), sizeof(value));
  return value;
}

static int omen_read_table_count(jlong decoder_ctx) {
  return omen_read_int32((uintptr_t)decoder_ctx, 0x128);
}



static int omen_read_refcount(jlong decoder_ctx, int idx) {
  uintptr_t ctx_base = (uintptr_t)decoder_ctx;
  int count = omen_read_table_count(decoder_ctx);
  uintptr_t buffer_ptr = 0;

  if (idx < 0 || idx >= count) {
    return -1;
  }
  memcpy(&buffer_ptr, (const void*)(ctx_base + 0x28 + (size_t)idx * 8),
         sizeof(buffer_ptr));
  if (buffer_ptr == 0) {
    return -1;
  }
  return omen_read_int32(buffer_ptr, 0x44);
}




void fuzz_one_input(const uint8_t* buffer, size_t length) {
  









  jobject bbData = NULL;
  jobject outBuf = NULL;
  jclass outCls = NULL;
  jfieldID modeField = NULL;
  jfieldID decoderPrivateField = NULL;
  jlong decoderCtx = 0;
  size_t offset = 0;
  jint decodeRet = -1;
  jint frameRet = -1;
  int privBeforeConstruct = -1;
  int tableCount = -1;
  int refcountBeforeTarget = -1;
  int refcountAfterTarget = -1;
  int errorSlot = 0;
  jint privBeforeTarget = -1;
  jint privAfterTarget = -1;

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

  



  outCls = (*ctx.env)->FindClass(ctx.env,
                                 "com/oplus/tbl/exoplayer2/video/VideoDecoderOutputBuffer");
  if (outCls == NULL || check_and_clear_jni_exception("get_obj_FindClass")) {
    outCls = NULL;
    fprintf(stderr, "[HARNESS] VideoDecoderOutputBuffer class lookup failed "
                    "(skip target)\n");
    goto release;
  }
  modeField = (*ctx.env)->GetFieldID(ctx.env, outCls, "mode", "I");
  decoderPrivateField = (*ctx.env)->GetFieldID(ctx.env, outCls,
                                               "decoderPrivate", "I");
  if (modeField == NULL || decoderPrivateField == NULL ||
      check_and_clear_jni_exception("get_obj_GetFieldID")) {
    modeField = NULL;
    decoderPrivateField = NULL;
    fprintf(stderr, "[HARNESS] mode/decoderPrivate field lookup failed "
                    "(skip target)\n");
    goto release;
  }

  




  (*ctx.env)->SetIntField(ctx.env, outBuf, modeField, 1);
  if (check_and_clear_jni_exception("get_obj_SetIntField_mode_delta")) {
    goto release;
  }

  



  privBeforeConstruct = (*ctx.env)->GetIntField(ctx.env, outBuf,
                                                decoderPrivateField);
  tableCount = omen_read_table_count(decoderCtx);

  








  frameRet = prereqFunctionPtr3(ctx.env, NULL, decoderCtx, outBuf,
                                (jboolean)0);
  if (check_and_clear_jni_exception("prereq3_gav1GetFrame")) {
    goto release;
  }

  


  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy(__omen_bitmap_snapshot, (void*)*__omen_afl_area_ptr, 65536);
  }

  




  privBeforeTarget = (*ctx.env)->GetIntField(ctx.env, outBuf,
                                             decoderPrivateField);
  refcountBeforeTarget = omen_read_refcount(decoderCtx,
                                            (int)privBeforeTarget);
  fprintf(stderr, "[HARNESS] decode_ret=%d getframe_ret=%d len=%zu "
                  "pre: priv=%d table_count=%d refcount=%d\n",
          (int)decodeRet, (int)frameRet, length, (int)privBeforeTarget,
          tableCount, refcountBeforeTarget);
  fflush(stderr);

  set_stage("call_jni");
  
  











  targetFunctionPtr0(ctx.env, NULL, decoderCtx, outBuf);
  check_and_clear_jni_exception("call_jni_target");

  




  privAfterTarget = (*ctx.env)->GetIntField(ctx.env, outBuf,
                                            decoderPrivateField);
  errorSlot = omen_read_int32((uintptr_t)decoderCtx, 0x2c4);
  refcountAfterTarget = omen_read_refcount(decoderCtx,
                                           (int)privBeforeTarget);
  fprintf(stderr, "[HARNESS] release_obs post: priv=%d error_slot=%d "
                  "refcount=%d\n", (int)privAfterTarget, errorSlot,
          refcountAfterTarget);
  fflush(stderr);

release:
  set_stage("release");
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  if (decoderCtx != 0) {
    releaseFunctionPtr0(ctx.env, NULL, decoderCtx);
    check_and_clear_jni_exception("release_gav1Close");
  }
  if (outCls != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, outCls);
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
  prereqFunctionPtr3 = (function_prereq_3)((uintptr_t)targetLibBase + 0x1893c); 
  targetFunctionPtr0 = (function_0)((uintptr_t)targetLibBase + 0x1913c);        
  

  releaseFunctionPtr0 = (function_release_close)((uintptr_t)targetLibBase + 0x1887c);

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
