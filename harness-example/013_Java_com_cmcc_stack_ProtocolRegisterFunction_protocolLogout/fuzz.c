#include <dlfcn.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "jenv.h"






























#define BUFFER_SIZE 1880





void** __omen_afl_area_ptr = NULL;
char   __omen_bitmap_snapshot[65536] __attribute__((aligned(4096)));



typedef jint (*function_prereq_1)(JNIEnv*, jclass);
typedef jint (*function_0)(JNIEnv*, jclass, jobject);


static JavaCTX ctx;



function_prereq_1 prereqFunctionPtr1;
function_0 targetFunctionPtr0;


static int g_protocol_init_done = 0;


jobject construct_RegisterBean(JNIEnv* env, const uint8_t* buffer,
                                size_t length, size_t* offset);
jobject construct_Authorization(JNIEnv* env, const uint8_t* buffer,
                                size_t length, size_t* offset);

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
  
  jobject bean = NULL;
  jint rc = -99;
  size_t offset = 0;

  set_stage("prereq_protocol_init");
  if (g_protocol_init_done == 0) {
    int init_rc;
    
    init_rc = prereqFunctionPtr1(ctx.env, NULL);
    fprintf(stderr, "[HARNESS] protocolInit -> %d (once per forkserver "
                    "child)\n", init_rc);
    if (init_rc != 0) {
      fprintf(stderr, "[HARNESS] FATAL_INIT: protocolInit returned %d != 0 — "
                      "field-ID tables unverified, the transform would hit "
                      "the runtime case fatal at the first own-scalar read. "
                      "Hard-fail, no target call.\n", init_rc);
      fflush(stderr);
      _exit(-7);
    }
    if (check_and_clear_jni_exception("prereq_protocol_init")) {
      fprintf(stderr, "[HARNESS] FATAL_INIT: pending exception after "
                      "protocolInit (runtime case: classpath cannot resolve "
                      "com.cmdc.rcsprotocol.* — check -Djava.class.path="
                      "./base.apk). Hard-fail, no target call.\n");
      fflush(stderr);
      _exit(JNI_ERR);
    }
    g_protocol_init_done = 1;
  }

  set_stage("input_parsing");
  
  










  if (length < 1) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION: RegisterBean recipe needs "
                    ">= 1 byte (mFeatureTag count prefix), got %zu; no "
                    "target call\n", length);
    goto release;
  }
  bean = construct_RegisterBean(ctx.env, buffer, length, &offset);
  if (bean == NULL) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION: RegisterBean recipe "
                    "returned NULL (len=%zu); no target call\n", length);
    goto release;
  }
  if (check_and_clear_jni_exception("construct_RegisterBean")) {
    goto release;
  }
  fprintf(stderr, "[HARNESS] construct consumed=%zu/%zu\n", offset, length);

  














  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy((void*)*__omen_afl_area_ptr, __omen_bitmap_snapshot, 65536);
  }

  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy(__omen_bitmap_snapshot, (void*)*__omen_afl_area_ptr, 65536);
  }

  set_stage("call_target");
  
  
  rc = targetFunctionPtr0(ctx.env, NULL, bean);
  fprintf(stderr, "[HARNESS] target rc=%d (0=enqueue/graceful, -1=null "
                  "bean)\n", rc);
  if (check_and_clear_jni_exception("call_target")) {
    goto release;
  }

release:
  set_stage("release");
  



  if (bean != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, bean);
    bean = NULL;
  }
}

int main(void) {
  int status;
  uint8_t buffer[BUFFER_SIZE];
  










  char* options[2] = { "-Djava.class.path=./base.apk",
                       "-Djava.library.path=./lib" };
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
  
  
  void* targetLibHandle = dlopen("libRcsStack.so", RTLD_NOW);
  if (targetLibHandle == NULL) {
    fprintf(stderr, "[HARNESS] Failed to dlopen libRcsStack.so: %s\n", dlerror());
    return -1;
  }
  void* targetLibBase = get_library_base("libRcsStack.so");
  if (targetLibBase == NULL) {
    fprintf(stderr, "[HARNESS] Failed to get library base for libRcsStack.so\n");
    return -1;
  }
  (void)targetLibHandle;
  








  prereqFunctionPtr1 =
      (function_prereq_1)((uintptr_t)targetLibBase + 0x253158);
  targetFunctionPtr0 =
      (function_0)((uintptr_t)targetLibBase + 0x2536AC);

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  
  fflush(stdout);
  fflush(stderr);
  _exit(0);
}
