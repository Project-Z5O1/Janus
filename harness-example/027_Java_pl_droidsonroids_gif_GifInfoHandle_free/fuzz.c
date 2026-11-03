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

























typedef jint (*function_prereq_1)(JavaVM*);
typedef jlong (*function_prereq_2)(JNIEnv*, jclass, jobject);
typedef void (*function_0)(JNIEnv*, jclass, jlong);


static JavaCTX ctx;



function_prereq_1 prereqFunctionPtr1; 
function_prereq_2 prereqFunctionPtr2; 
function_0 targetFunctionPtr0;        







static int g_prereq1_done = 0;

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







jobject construct_ByteArrayInputStream(JNIEnv* env,
                                        const uint8_t* buffer, size_t length, size_t* offset);




void fuzz_one_input(const uint8_t* buffer, size_t length) {
  








  jobject objStream = NULL;
  jlong gifInfoHandle = 0;
  size_t offset = 0;

  set_stage("input_parsing");
  
  








  objStream = construct_ByteArrayInputStream(ctx.env, buffer, length, &offset);
  if (objStream == NULL) {
    fprintf(stderr, "[HARNESS] arg1 construction failed (ByteArrayInputStream)\n");
    goto release;
  }

  set_stage("get_obj_prereq");
  
  



  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy((void*)*__omen_afl_area_ptr, __omen_bitmap_snapshot, 65536);
  }

  








  if (g_prereq1_done == 0) {
    jint ver = prereqFunctionPtr1(ctx.vm);
    if (ver == 0) {
      fprintf(stderr, "[HARNESS] JNI_OnLoad returned 0\n");
      goto release;
    }
    if (check_and_clear_jni_exception("prereq1_JNI_OnLoad")) {
      goto release;
    }
    g_prereq1_done = 1;
  }

  










  gifInfoHandle = prereqFunctionPtr2(ctx.env, NULL, objStream);
  if (check_and_clear_jni_exception("prereq2_openStream")) {
    




    gifInfoHandle = 0;
    fprintf(stderr, "[HARNESS] step2 openStream threw; exception cleared; "
                    "target receives documented 0 handle\n");
  } else if (gifInfoHandle != 0) {
    fprintf(stderr, "[HARNESS] step2 openStream returned GifInfo handle=0x%llx\n",
            (unsigned long long)gifInfoHandle);
  } else {
    fprintf(stderr, "[HARNESS] step2 openStream returned 0 (open error; "
                    "legal empty-handle no-op value for the target)\n");
  }

  



  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy(__omen_bitmap_snapshot, (void*)*__omen_afl_area_ptr, 65536);
  }

  set_stage("call_jni");
  





  fprintf(stderr, "[HARNESS] target free(handle=0x%llx)\n",
          (unsigned long long)gifInfoHandle);
  targetFunctionPtr0(ctx.env, NULL, gifInfoHandle);
  if (check_and_clear_jni_exception("call_jni_target")) {
    
  }

release:
  set_stage("release");
  
  
  
  
  
  
  
  
  
  if (objStream != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, objStream);
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
  
  
  void* targetLibHandle = dlopen("libpl_droidsonroids_gif.so", RTLD_NOW);
  if (targetLibHandle == NULL) {
    fprintf(stderr, "[HARNESS] Failed to dlopen libpl_droidsonroids_gif.so: %s\n", dlerror());
    return -1;
  }
  void* targetLibBase = get_library_base("libpl_droidsonroids_gif.so");
  if (targetLibBase == NULL) {
    fprintf(stderr, "[HARNESS] Failed to get library base for libpl_droidsonroids_gif.so\n");
    return -1;
  }
  (void)targetLibHandle;
  



  prereqFunctionPtr1 = (function_prereq_1)((uintptr_t)targetLibBase + 0x792c);   
  prereqFunctionPtr2 = (function_prereq_2)((uintptr_t)targetLibBase + 0x5b0c);   
  targetFunctionPtr0 = (function_0)((uintptr_t)targetLibBase + 0x49ac);          

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
