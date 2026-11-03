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







typedef jint (*postEnhancePrepare_t)(JNIEnv*, jobject, jobject, jint, jint,
                                     jlong);



typedef void (*nativeDestroy_t)(JNIEnv*, jobject);









#include "construct.c"


static JavaCTX ctx;


postEnhancePrepare_t targetFunctionPtr0;
nativeDestroy_t nativeDestroyPtr;






static uint8_t g_input_backing[BUFFER_SIZE] __attribute__((aligned(4096)));

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
  
  
  
  
  
  
  
  jobject bb = NULL;
  jobject thiz = NULL;

  if (length > BUFFER_SIZE) {
    length = BUFFER_SIZE; 
  }
  if (length > 0) {
    memcpy(g_input_backing, buffer, length);
  }

  bb = (*ctx.env)->NewDirectByteBuffer(ctx.env, g_input_backing, (jlong)length);
  if (bb == NULL || check_and_clear_jni_exception("input_parsing")) {
    fprintf(stderr, "[HARNESS] NewDirectByteBuffer failed length=%zu\n",
            length);
    return;
  }
  if ((*ctx.env)->GetDirectBufferAddress(ctx.env, bb) == NULL) {
    fprintf(stderr, "[HARNESS] direct buffer address NULL, skipping\n");
    goto cleanup;
  }

  set_stage("call_target");
  
  
  
  
  
  
  

  

  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy((void*)*__omen_afl_area_ptr, __omen_bitmap_snapshot, 65536);
  }

  thiz = construct_BcapProcessor(ctx.env, NULL, 0, NULL);
  if (thiz == NULL || check_and_clear_jni_exception("prereq_construct")) {
    

    fprintf(stderr, "[HARNESS] receiver construction failed, skipping input\n");
    goto cleanup;
  }

  






  {
    jint arg1 = (jint)0;
    jint arg2 = (jint)(length - (size_t)arg1);
    jlong arg3 = (jlong)0;

    


    if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
      memcpy(__omen_bitmap_snapshot, (void*)*__omen_afl_area_ptr, 65536);
    }

    set_stage("call_target_invoke");
    jint rc = targetFunctionPtr0(ctx.env, thiz, bb, arg1, arg2, arg3);
    check_and_clear_jni_exception("call_target_invoke");

    



    if (g_trace_stages) {
      jclass clsD = (*ctx.env)->FindClass(
          ctx.env, "com/oplus/tblplayer/ext/bcap/BcapProcessor");
      jfieldID fidD = clsD ? (*ctx.env)->GetFieldID(ctx.env, clsD,
                                                    "nativeInstance", "J")
                           : NULL;
      if (fidD != NULL &&
          !check_and_clear_jni_exception("evidence_fieldid")) {
        jlong handle = (*ctx.env)->GetLongField(ctx.env, thiz, fidD);
        const uint8_t* obj = (const uint8_t*)(uintptr_t)handle;
        if (handle != 0 &&
            !check_and_clear_jni_exception("evidence_getfield")) {
          uint64_t baseline = 0, p_lev = 0;
          uint32_t version = 0, mode = 0, levl = 0;
          memcpy(&baseline, obj + 0x00, 8);
          memcpy(&version, obj + 0x08, 4);
          memcpy(&mode, obj + 0x1c, 4);
          memcpy(&p_lev, obj + 0x98, 8);
          memcpy(&levl, obj + 0xa0, 4);
          fprintf(stderr,
                  "[HARNESS] target_rc=%d version=%u mode=%u baseline=%llu "
                  "lev_ptr=0x%llx levl=%u\n",
                  (int)rc, version, mode, (unsigned long long)baseline,
                  (unsigned long long)p_lev, levl);
        }
      }
      if (clsD != NULL) {
        (*ctx.env)->DeleteLocalRef(ctx.env, clsD);
      }
    }
  }

  set_stage("cleanup");
  
  
  nativeDestroyPtr(ctx.env, thiz);
  check_and_clear_jni_exception("cleanup_nativeDestroy");

cleanup:
  if (thiz != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, thiz);
  }
  if (bb != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, bb);
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
  targetFunctionPtr0 = (postEnhancePrepare_t)((uintptr_t)targetLibBase + 0x103920);
  nativeDestroyPtr = (nativeDestroy_t)((uintptr_t)targetLibBase + 0x1035D4);

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
