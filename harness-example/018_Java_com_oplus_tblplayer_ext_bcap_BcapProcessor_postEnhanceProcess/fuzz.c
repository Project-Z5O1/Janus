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





typedef jint (*function_0)(JNIEnv*, jobject, jlong, jobject, jint, jint,
                           jobject, jint, jint);


typedef jint (*prereq_prepare_t)(JNIEnv*, jobject, jobject, jint, jint, jlong);


typedef void (*prereq_enable_t)(JNIEnv*, jobject, jboolean);



typedef void (*cleanup_destroy_t)(JNIEnv*, jobject);


static JavaCTX ctx;



function_0 targetFunctionPtr0;               
static prereq_prepare_t prereqFunctionPtr1;  
static prereq_enable_t prereqFunctionPtr2;   
static cleanup_destroy_t cleanupDestroyPtr;  


static jobject construct_BcapProcessor(JNIEnv* env,
                                       const uint8_t* buffer,
                                       size_t length,
                                       size_t* offset) {
  (void)buffer;
  (void)length;
  (void)offset; 

  JavaVM* vm = NULL;
  void* lib = NULL;
  void* base = NULL;
  jclass cls = NULL;
  jobject out = NULL;
  jmethodID midCtor = NULL;
  jfieldID fidInstance = NULL;
  jlong handle = 0;
  jlong readback = 0;
  jint (*fnOnLoad)(JavaVM*, void*);
  jlong (*fnNativeInit)(JNIEnv*, jobject, jint, jint, jint, jboolean, jboolean);

  
  lib = dlopen("libbcapJNI.so", RTLD_NOW);
  if (lib == NULL) {
    fprintf(stderr, "[recipe] dlopen libbcapJNI.so failed: %s\n", dlerror());
    goto fail;
  }
  base = get_library_base("libbcapJNI.so");
  if (base == NULL) {
    fprintf(stderr, "[recipe] get_library_base(libbcapJNI.so) failed\n");
    goto fail;
  }
  fnOnLoad = (jint (*)(JavaVM*, void*))((uintptr_t)base + 0x1033ECu);
  fnNativeInit = (jlong (*)(JNIEnv*, jobject, jint, jint, jint, jboolean,
                            jboolean))((uintptr_t)base + 0x103504u);

  
  if ((*env)->GetJavaVM(env, &vm) != JNI_OK) {
    fprintf(stderr, "[recipe] GetJavaVM failed\n");
    goto fail;
  }
  if (fnOnLoad(vm, NULL) < 0) {
    fprintf(stderr, "[recipe] JNI_OnLoad failed\n");
    goto fail;
  }
  if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
    fprintf(stderr, "[recipe] exception after JNI_OnLoad\n");
    goto fail;
  }

  cls = (*env)->FindClass(env, "com/oplus/tblplayer/ext/bcap/BcapProcessor");
  if (cls == NULL) {
    fprintf(stderr, "[recipe] FindClass(BcapProcessor) failed "
                    "(base.apk on classpath?)\n");
    goto fail;
  }

  

  midCtor = (*env)->GetMethodID(env, cls, "<init>", "(I)V");
  if (midCtor == NULL) {
    fprintf(stderr, "[recipe] GetMethodID(<init> (I)V) failed\n");
    goto fail;
  }
  out = (*env)->NewObject(env, cls, midCtor, (jint)3);
  if (out == NULL || (*env)->ExceptionCheck(env) == JNI_TRUE) {
    fprintf(stderr, "[recipe] NewObject(<init>(I)V, 3) failed\n");
    goto fail;
  }

  
  handle = fnNativeInit(env, out, (jint)3, (jint)48000, (jint)2,
                        (jboolean)0, (jboolean)0);
  if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
    fprintf(stderr, "[recipe] exception in nativeInit\n");
    goto fail;
  }
  if (handle == 0) {
    fprintf(stderr, "[recipe] nativeInit returned 0 (bcap_init failed)\n");
    goto fail;
  }

  
  fidInstance = (*env)->GetFieldID(env, cls, "nativeInstance", "J");
  if (fidInstance == NULL) {
    fprintf(stderr, "[recipe] GetFieldID(nativeInstance J) failed\n");
    goto fail;
  }
  (*env)->SetLongField(env, out, fidInstance, handle);
  if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
    fprintf(stderr, "[recipe] SetLongField(nativeInstance) failed\n");
    goto fail;
  }

  
  readback = (*env)->GetLongField(env, out, fidInstance);
  if (readback != handle) {
    fprintf(stderr, "[recipe] nativeInstance readback mismatch\n");
    goto fail;
  }

  (*env)->DeleteLocalRef(env, cls);
  return out;

fail:
  if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
    (*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);
  }
  if (out != NULL) (*env)->DeleteLocalRef(env, out);
  if (cls != NULL) (*env)->DeleteLocalRef(env, cls);
  return NULL;
}

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
  
  
  
  
  
  jobject pcmBB = NULL;      
  jobject bcp1BB = NULL;     
  jobject bcp2BB = NULL;     
  jobject thiz = NULL;       
  unsigned char* pcm_mem = NULL;
  unsigned char* bcp1_mem = NULL;
  unsigned char* bcp2_mem = NULL;
  unsigned char* pcm_before = NULL;
  size_t consumed = 0;
  static int g_pcm_diff = -1;
  int64_t pts = 0;
  uint32_t pos_raw = 0;
  uint32_t blocks_raw = 0;
  unsigned meta_mode = 0;
  uint32_t blocks = 0;
  size_t pcm_len = 0;
  uint32_t pos = 0;
  size_t bcp1_len = 0;
  size_t bcp2_off = 0;
  jint prep_ret = 0;
  jint target_ret = 0;

  set_stage("input_parsing");
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  if (length < 18579u) { 
    fprintf(stderr,
            "[HARNESS] input_parsing FAIL_CONSTRUCTION len=%zu < 18579\n",
            length);
    goto cleanup_stage;
  }
  pts = ((int64_t)buffer[0]) | (((int64_t)buffer[1]) << 8) |
        (((int64_t)buffer[2]) << 16) | (((int64_t)buffer[3]) << 24) |
        (((int64_t)buffer[4]) << 32) | (((int64_t)buffer[5]) << 40) |
        (((int64_t)buffer[6]) << 48) | (((int64_t)buffer[7]) << 56);
  pos_raw = (uint32_t)buffer[8] | ((uint32_t)buffer[9] << 8);
  blocks_raw = (uint32_t)buffer[10] | ((uint32_t)buffer[11] << 8);
  meta_mode = buffer[16396];

  blocks = (blocks_raw > 8u) ? 8u : blocks_raw;      
  pcm_len = (size_t)blocks * 2048u;
  pos = pos_raw;
  if (pcm_len > 0u && pos > (uint32_t)(16384u - pcm_len)) {
    pos = (uint32_t)(16384u - pcm_len);              
  }
  bcp1_len = 1u + (size_t)blocks * 114u;
  bcp2_off = 16397u + bcp1_len;
  if (bcp2_off + 2181u > length) {
    fprintf(stderr,
            "[HARNESS] input_parsing FAIL_CONSTRUCTION len=%zu "
            "blocks=%u inconsistent tail (need %zu)\n",
            length, blocks, bcp2_off + 2181u);
    goto cleanup_stage;
  }

  if (g_pcm_diff < 0) {
    g_pcm_diff = (getenv("OMEN_PCM_DIFF") != NULL);
  }
  pcm_mem = (unsigned char*)malloc(16384u);
  bcp1_mem = (unsigned char*)malloc(bcp1_len);
  bcp2_mem = (unsigned char*)malloc(2181u);
  if (g_pcm_diff) {
    pcm_before = (unsigned char*)malloc(16384u);
  }
  if (pcm_mem == NULL || bcp1_mem == NULL || bcp2_mem == NULL ||
      (g_pcm_diff && pcm_before == NULL)) {
    fprintf(stderr, "[HARNESS] input_parsing oom\n");
    goto cleanup_stage;
  }
  memcpy(pcm_mem, buffer + 12, 16384u);
  memcpy(bcp1_mem, buffer + 16397, bcp1_len);
  memcpy(bcp2_mem, buffer + bcp2_off, 2181u);
  if (pcm_before != NULL) {
    memcpy(pcm_before, pcm_mem, 16384u);
  }

  



  pcmBB = (*ctx.env)->NewDirectByteBuffer(ctx.env, pcm_mem, 16384);
  if (meta_mode != 0) {
    bcp1BB = (*ctx.env)->NewDirectByteBuffer(ctx.env, bcp1_mem,
                                             (jlong)bcp1_len);
  }
  bcp2BB = (*ctx.env)->NewDirectByteBuffer(ctx.env, bcp2_mem, 2181);
  if (pcmBB == NULL || bcp2BB == NULL || (meta_mode != 0 && bcp1BB == NULL) ||
      check_and_clear_jni_exception("input_parsing.NewDirectByteBuffer")) {
    fprintf(stderr, "[HARNESS] input_parsing NewDirectByteBuffer failed\n");
    goto cleanup_stage;
  }

  set_stage("call_target");
  
  
  
  
  
  
  

  


  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy((void*)*__omen_afl_area_ptr, __omen_bitmap_snapshot, 65536);
  }

  set_stage("prereq_construct");
  


  thiz = construct_BcapProcessor(ctx.env, NULL, 0, &consumed);
  if (thiz == NULL ||
      check_and_clear_jni_exception("prereq_construct")) {
    fprintf(stderr,
            "[HARNESS] prereq_construct failed, skipping target\n");
    goto cleanup_stage;
  }

  set_stage("prereq_prepare");
  


  prep_ret = prereqFunctionPtr1(ctx.env, thiz, bcp2BB, (jint)0, (jint)2181,
                                (jlong)0);
  if (check_and_clear_jni_exception("prereq_prepare") || prep_ret != 0) {
    fprintf(stderr,
            "[HARNESS] prereq_prepare ret=%d, baseline state not "
            "established, skipping target\n", prep_ret);
    goto cleanup_stage;
  }

  set_stage("prereq_enable");
  
  prereqFunctionPtr2(ctx.env, thiz, (jboolean)1);
  if (check_and_clear_jni_exception("prereq_enable")) {
    fprintf(stderr,
            "[HARNESS] prereq_enable failed, skipping target\n");
    goto cleanup_stage;
  }

  

  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy(__omen_bitmap_snapshot, (void*)*__omen_afl_area_ptr, 65536);
  }

  set_stage("target");
  


  target_ret = targetFunctionPtr0(ctx.env, thiz, (jlong)pts, pcmBB,
                                  (jint)pos, (jint)pcm_len, bcp1BB, (jint)0,
                                  (jint)bcp1_len);
  {
    int target_exc = check_and_clear_jni_exception("call_target");
    fprintf(stderr,
            "[HARNESS] target pts=%lld K=%u pos=%u pcm_len=%zu meta=%s "
            "bcp1_len=%zu ret=%d exc=%d\n",
            (long long)pts, blocks, pos, pcm_len,
            (meta_mode != 0) ? "bcp1" : "null", bcp1_len, target_ret,
            target_exc);
    if (pcm_before != NULL) {
      size_t diff = 0;
      size_t i;
      for (i = 0; i < 16384u; i++) {
        if (pcm_mem[i] != pcm_before[i]) {
          diff++;
        }
      }
      fprintf(stderr, "[HARNESS] pcm_modified_bytes=%zu\n", diff);
      free(pcm_before);
      pcm_before = NULL;
    }
    fflush(stderr);
  }

cleanup_stage:
  set_stage("cleanup");
  
  
  
  
  if (thiz != NULL && cleanupDestroyPtr != NULL) {
    cleanupDestroyPtr(ctx.env, thiz); 
    check_and_clear_jni_exception("cleanup.nativeDestroy");
  }
  free(pcm_before);
  free(bcp2_mem);
  free(bcp1_mem);
  free(pcm_mem);
  if (thiz != NULL) (*ctx.env)->DeleteLocalRef(ctx.env, thiz);
  if (bcp2BB != NULL) (*ctx.env)->DeleteLocalRef(ctx.env, bcp2BB);
  if (bcp1BB != NULL) (*ctx.env)->DeleteLocalRef(ctx.env, bcp1BB);
  if (pcmBB != NULL) (*ctx.env)->DeleteLocalRef(ctx.env, pcmBB);
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
  
  
  
  
  
  void* targetLibHandle = dlopen("libbcapJNI.so", RTLD_NOW);
  if (targetLibHandle == NULL) {
    fprintf(stderr, "[HARNESS] Failed to dlopen libbcapJNI.so: %s\n", dlerror());
    return -1;
  }
  void* targetLibBase = get_library_base("libbcapJNI.so");
  if (targetLibBase == NULL) {
    fprintf(stderr, "[HARNESS] Failed to get library base for libbcapJNI.so\n");
    return -1;
  }
  (void)targetLibHandle;
  targetFunctionPtr0 =
      (function_0)((uintptr_t)targetLibBase + 0x1039C4u);
  prereqFunctionPtr1 =
      (prereq_prepare_t)((uintptr_t)targetLibBase + 0x103920u);
  prereqFunctionPtr2 =
      (prereq_enable_t)((uintptr_t)targetLibBase + 0x103750u);
  cleanupDestroyPtr =
      (cleanup_destroy_t)((uintptr_t)targetLibBase + 0x1035D4u);
  fprintf(stderr,
          "[HARNESS] libbcapJNI.so base=%p offsets 0x1039C4/0x103920/"
          "0x103750/0x1035D4 resolved\n", targetLibBase);

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
