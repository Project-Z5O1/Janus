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



#include "object_recipes/Bitmap/construct.c"
typedef jboolean (*function_0)(JNIEnv*, jclass, jobject, jstring);


static JavaCTX ctx;



function_0 targetFunctionPtr0;






#define OMEN_SAVE_PATH \
  "/data/local/tmp/omen/Java_com_oplus_wallpaper_sdk_ImageProcess_saveBitmap/out/save_bitmap.bmp"
#define OMEN_SAVE_DIR \
  "/data/local/tmp/omen/Java_com_oplus_wallpaper_sdk_ImageProcess_saveBitmap/out"


#include <sys/stat.h>

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
  






  jobject jBitmap = NULL;
  jstring jPath = NULL;
  size_t offset = 0;

  set_stage("input_parsing");
  
  







  jBitmap = construct_Bitmap(ctx.env, buffer, length, &offset);
  if (jBitmap == NULL) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION: bitmap recipe returned NULL; "
                    "skipping target (never substituting NULL/default object)\n");
    goto release;
  }
  if (check_and_clear_jni_exception("input_parsing_construct_bitmap")) {
    goto release;
  }

  
  jPath = (*ctx.env)->NewStringUTF(ctx.env, OMEN_SAVE_PATH);
  if (jPath == NULL) {
    fprintf(stderr, "[HARNESS] NewStringUTF(CONSTANT path) returned NULL\n");
    goto release;
  }
  if (check_and_clear_jni_exception("input_parsing_newstringutf")) {
    goto release;
  }

  set_stage("call_jni");
  
  



  





  jboolean saved = targetFunctionPtr0(ctx.env, NULL, jBitmap, jPath);
  if (check_and_clear_jni_exception("call_jni_target")) {
    fprintf(stderr, "[HARNESS] saveBitmap threw; exception observed and cleared\n");
  } else {
    fprintf(stderr, "[HARNESS] saveBitmap returned %s (out=%s)\n",
            saved ? "true" : "false", OMEN_SAVE_PATH);
  }

release:
  set_stage("release");
  
  
  
  
  
  
  
  
  
  
  
  if (jPath != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jPath);
  }
  if (jBitmap != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jBitmap);
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
  
  
  void* targetLibHandle = dlopen("liboplus_image_process.so", RTLD_NOW);
  if (targetLibHandle == NULL) {
    fprintf(stderr, "[HARNESS] Failed to dlopen liboplus_image_process.so: %s\n", dlerror());
    return -1;
  }
  void* targetLibBase = get_library_base("liboplus_image_process.so");
  if (targetLibBase == NULL) {
    fprintf(stderr, "[HARNESS] Failed to get library base for liboplus_image_process.so\n");
    return -1;
  }
  (void)targetLibHandle;
  




  targetFunctionPtr0 = (function_0)((uintptr_t)targetLibBase + 0xE7C);

  



  set_stage("ensure_output_dir");
  if (mkdir(OMEN_SAVE_DIR, 0755) != 0 && errno != EEXIST) {
    fprintf(stderr, "[HARNESS] mkdir %s failed: %s (target fopen will early-return)\n",
            OMEN_SAVE_DIR, strerror(errno));
  }

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
