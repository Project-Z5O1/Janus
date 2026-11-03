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










typedef jint (*prereq_init_t)(JNIEnv*, jobject, jstring, jstring, jint,
                              jstring, jstring, jint);









typedef jint (*target_open_t)(JNIEnv*, jobject, jstring);









#include "object_recipes/CLoganProtocol/construct.c"


static JavaCTX ctx;


prereq_init_t prereqFunctionPtr0;
target_open_t targetFunctionPtr0;

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
  char* arg0buf = NULL;
  jobject protocol = NULL;
  jstring jarg0 = NULL;
  jstring jcacheDir = NULL;
  jstring jlogDir = NULL;
  jstring jkey = NULL;
  jstring jiv = NULL;
  jint init_rc = 0;
  jint open_rc = 0;

  set_stage("input_parsing");
  
  
  
  
  
  
  
  
  
  
  arg0buf = (char*)malloc((size_t)length + 1);
  if (arg0buf == NULL) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION: OOM arg0 (%zu bytes)\n",
            length);
    goto release_refs;
  }
  memcpy(arg0buf, buffer, (size_t)length);
  arg0buf[length] = '\0';
  jarg0 = (*ctx.env)->NewStringUTF(ctx.env, arg0buf);
  if (check_and_clear_jni_exception("input_parsing:NewStringUTF(arg0)")) {
    fprintf(stderr,
            "[HARNESS] FAIL_CONSTRUCTION: JVM rejected input as modified "
            "UTF-8 (len=%zu); JVM-side input rejection, not a target "
            "defect\n", length);
    goto release_refs;
  }
  if (jarg0 == NULL) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION: NewStringUTF(arg0) NULL\n");
    goto release_refs;
  }

  set_stage("get_obj");
  
  
  
  
  
  
  
  
  
  
  protocol = construct_CLoganProtocol(ctx.env, buffer, length, NULL);
  if (check_and_clear_jni_exception("get_obj:construct_CLoganProtocol")) {
    goto release_refs;
  }
  if (protocol == NULL) {
    fprintf(stderr,
            "[HARNESS] FAIL_CONSTRUCTION: construct_CLoganProtocol returned "
            "NULL (isCloganSuccess gate refused an instance that could not "
            "reach libheytaplog.so; check -Djava.library.path)\n");
    goto release_refs;
  }
  jcacheDir = (*ctx.env)->NewStringUTF(ctx.env,
      "/data/local/tmp/omen/Java_com_heytap_log_core_CLoganProtocol_"
      "clogan_1open/cache");
  if (check_and_clear_jni_exception("get_obj:NewStringUTF(cacheDir)")) {
    goto release_refs;
  }
  jlogDir = (*ctx.env)->NewStringUTF(ctx.env,
      "/data/local/tmp/omen/Java_com_heytap_log_core_CLoganProtocol_"
      "clogan_1open/log");
  if (check_and_clear_jni_exception("get_obj:NewStringUTF(logDir)")) {
    goto release_refs;
  }
  jkey = (*ctx.env)->NewStringUTF(ctx.env, "omen-key-nonempty");
  if (check_and_clear_jni_exception("get_obj:NewStringUTF(key)")) {
    goto release_refs;
  }
  jiv = (*ctx.env)->NewStringUTF(ctx.env, "omen-iv-nonempty");
  if (check_and_clear_jni_exception("get_obj:NewStringUTF(iv)")) {
    goto release_refs;
  }

  set_stage("call_prereq");
  
  
  
  
  
  
  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy((void*)*__omen_afl_area_ptr, __omen_bitmap_snapshot, 65536);
  }
  init_rc = prereqFunctionPtr0(ctx.env, protocol, jcacheDir, jlogDir, 100,
                               jkey, jiv, 768000);
  if (check_and_clear_jni_exception("call_prereq:clogan_init")) {
    fprintf(stderr,
            "[HARNESS] prereq gate: clogan_init left a pending exception; "
            "target SKIPPED (open would abort on any further JNI call)\n");
    goto release_refs;
  }
  if (g_trace_stages) {
    fprintf(stderr,
            "[HARNESS] prereq clogan_init rc=%d (0x%08x) [-1010/-1012 "
            "armed; field set even on arming failure -> -2050 path]\n",
            (int)init_rc, (unsigned int)init_rc);
  }
  
  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy(__omen_bitmap_snapshot, (void*)*__omen_afl_area_ptr, 65536);
  }

  set_stage("call_target");
  
  
  
  
  
  
  
  
  
  open_rc = targetFunctionPtr0(ctx.env, protocol, jarg0);
  if (check_and_clear_jni_exception("call_target:clogan_open")) {
    goto release_refs;
  }
  if (g_trace_stages) {
    fprintf(stderr,
            "[HARNESS] target clogan_open rc=%d (0x%08x) [-2010 open ok; "
            "-2020 fopen fail; -2050 is_init_ok==0; -9999 field==0]\n",
            (int)open_rc, (unsigned int)open_rc);
  }

release_refs:
  set_stage("cleanup");
  
  
  
  
  
  
  
  
  
  if (jiv != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jiv);
  }
  if (jkey != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jkey);
  }
  if (jlogDir != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jlogDir);
  }
  if (jcacheDir != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jcacheDir);
  }
  if (protocol != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, protocol);
  }
  if (jarg0 != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jarg0);
  }
  free(arg0buf);
}

int main(void) {
  int status;
  uint8_t buffer[BUFFER_SIZE];
  
  
  
  
  
  
  char* options[2] = {
    "-Djava.class.path=./base.apk",
    "-Djava.library.path=./lib"
  };
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
  if ((status = init_java_env(&ctx, options, 2)) != 0) {
    fprintf(stderr, "[HARNESS] init_java_env failed status=%d\n", status);
    return status;
  }

  if (check_and_clear_jni_exception("init_java_env")) {
    return JNI_ERR;
  }

  set_stage("resolve_target_functions");
  
  
  
  
  
  
  
  
  void* targetLibHandle = dlopen("libheytaplog.so", RTLD_NOW);
  if (targetLibHandle == NULL) {
    fprintf(stderr, "[HARNESS] Failed to dlopen libheytaplog.so: %s\n", dlerror());
    return -1;
  }
  void* targetLibBase = get_library_base("libheytaplog.so");
  if (targetLibBase == NULL) {
    fprintf(stderr, "[HARNESS] Failed to get library base for libheytaplog.so\n");
    return -1;
  }
  (void)targetLibHandle;
  
  
  prereqFunctionPtr0 = (prereq_init_t)((uintptr_t)targetLibBase + 0x3f14);
  
  
  targetFunctionPtr0 = (target_open_t)((uintptr_t)targetLibBase + 0x412c);

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
