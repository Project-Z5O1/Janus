#include <dlfcn.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "jenv.h"




#define BUFFER_SIZE 256





void** __omen_afl_area_ptr = NULL;
char   __omen_bitmap_snapshot[65536] __attribute__((aligned(4096)));





typedef jint (*function_0)(JNIEnv*, jobject, jstring, jstring, jstring);


static JavaCTX ctx;



function_0 targetFunctionPtr0;






static void* g_target_lib_handle = NULL;


static size_t g_recipe_offset_sentinel = 0x5A5A5A5A;




#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define OMEN_STAGE_DIR  "/data/local/tmp/omen/Java_miuix_util_patcher_Patcher_applyPatch"
#define OMEN_OLD_PATH   OMEN_STAGE_DIR "/old.bin"
#define OMEN_NEW_PATH   OMEN_STAGE_DIR "/new.out"
#define OMEN_PATCH_PATH OMEN_STAGE_DIR "/patch.bin"
#define OMEN_OLD_SIZE   4096
#define OMEN_NEW_OUT_MAX (64ULL * 1024 * 1024) 


static int omen_stage_files(void) {
  uint8_t old_content[OMEN_OLD_SIZE];
  struct stat st;
  size_t i;
  int fd;

  mkdir(OMEN_STAGE_DIR, 0777); 
  if (stat(OMEN_STAGE_DIR, &st) != 0 || !S_ISDIR(st.st_mode)) {
    fprintf(stderr, "[HARNESS] stage dir %s unavailable\n", OMEN_STAGE_DIR);
    return -1;
  }
  for (i = 0; i < OMEN_OLD_SIZE; i++) {
    old_content[i] = (uint8_t)((i * 31 + 7) & 0xff);
  }
  fd = open(OMEN_OLD_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (fd < 0) {
    fprintf(stderr, "[HARNESS] open old.bin failed: %s\n", strerror(errno));
    return -1;
  }
  if (write(fd, old_content, OMEN_OLD_SIZE) != OMEN_OLD_SIZE) {
    fprintf(stderr, "[HARNESS] write old.bin failed\n");
    close(fd);
    return -1;
  }
  close(fd);
  fd = open(OMEN_NEW_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (fd < 0) {
    fprintf(stderr, "[HARNESS] open new.out failed: %s\n", strerror(errno));
    return -1;
  }
  close(fd);
  return 0;
}


static int omen_write_patch_bin(const uint8_t* data, size_t len) {
  int fd = open(OMEN_PATCH_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (fd < 0) {
    fprintf(stderr, "[HARNESS] open patch.bin failed: %s\n", strerror(errno));
    return -1;
  }
  if (write(fd, data, len) != (ssize_t)len) {
    fprintf(stderr, "[HARNESS] write patch.bin failed\n");
    close(fd);
    return -1;
  }
  close(fd);
  return 0;
}





static void omen_trim_new_out(void) {
  struct stat st;
  if (stat(OMEN_NEW_PATH, &st) == 0 &&
      (unsigned long long)st.st_size > OMEN_NEW_OUT_MAX) {
    fprintf(stderr, "[HARNESS] staging: unlink oversized new.out (%lld bytes)\n",
            (long long)st.st_size);
    unlink(OMEN_NEW_PATH);
  }
}


#include "object_recipes/Patcher/construct.c"




#include "smoke/transformer.inc"

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
  jobject receiver = NULL;
  jstring jold = NULL;
  jstring jnew = NULL;
  jstring jdiff = NULL;
  uint8_t patch_bin[OMEN_PATCH_OUT_CAP];
  struct omen_patch_detail detail;
  size_t patch_len = 0;
  jint target_rc = -2;

  set_stage("input_parsing");
  
  



  if (omen_transform_patch_input(buffer, length, g_target_lib_handle,
                                 patch_bin, sizeof(patch_bin), &patch_len,
                                 &detail) != 0) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION transform length=%zu\n", length);
    return;
  }
  if (g_trace_stages) {
    fprintf(stderr, "[HARNESS] transform patch_len=%zu newsize=%lld "
            "ctrl0=(%lld,%lld,%lld) ctrl1=(%lld,%lld,%lld)\n",
            patch_len, (long long)detail.newsize,
            (long long)detail.ctrl[0][0], (long long)detail.ctrl[0][1],
            (long long)detail.ctrl[0][2], (long long)detail.ctrl[1][0],
            (long long)detail.ctrl[1][1], (long long)detail.ctrl[1][2]);
  }
  if (omen_write_patch_bin(patch_bin, patch_len) != 0) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION patch.bin write\n");
    return;
  }
  omen_trim_new_out();

  


  receiver = construct_Patcher(ctx.env, NULL, 0, &g_recipe_offset_sentinel);
  if (receiver == NULL || check_and_clear_jni_exception("construct_receiver")) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION receiver\n");
    if (receiver != NULL) {
      (*ctx.env)->DeleteLocalRef(ctx.env, receiver);
    }
    return;
  }

  

  jold = (*ctx.env)->NewStringUTF(ctx.env, OMEN_OLD_PATH);
  jnew = (*ctx.env)->NewStringUTF(ctx.env, OMEN_NEW_PATH);
  jdiff = (*ctx.env)->NewStringUTF(ctx.env, OMEN_PATCH_PATH);
  if (jold == NULL || jnew == NULL || jdiff == NULL ||
      check_and_clear_jni_exception("input_parsing")) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION jstrings\n");
    if (jdiff != NULL) {
      (*ctx.env)->DeleteLocalRef(ctx.env, jdiff);
    }
    if (jnew != NULL) {
      (*ctx.env)->DeleteLocalRef(ctx.env, jnew);
    }
    if (jold != NULL) {
      (*ctx.env)->DeleteLocalRef(ctx.env, jold);
    }
    (*ctx.env)->DeleteLocalRef(ctx.env, receiver);
    return;
  }

  set_stage("call_target");
  
  


  target_rc = targetFunctionPtr0(ctx.env, receiver, jold, jnew, jdiff);
  if (g_trace_stages) {
    fprintf(stderr, "[HARNESS] target_rc=%d\n", target_rc);
    fflush(stderr);
  }
  

  if (check_and_clear_jni_exception("call_target")) {
    
  }

  set_stage("cleanup");
  




  (*ctx.env)->DeleteLocalRef(ctx.env, jdiff);
  (*ctx.env)->DeleteLocalRef(ctx.env, jnew);
  (*ctx.env)->DeleteLocalRef(ctx.env, jold);
  (*ctx.env)->DeleteLocalRef(ctx.env, receiver);
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
  
  
  
  
  
  
  void* targetLibHandle = dlopen("libmiuidiffpatcher.so", RTLD_NOW);
  if (targetLibHandle == NULL) {
    fprintf(stderr, "[HARNESS] Failed to dlopen libmiuidiffpatcher.so: %s\n", dlerror());
    return -1;
  }
  void* targetLibBase = get_library_base("libmiuidiffpatcher.so");
  if (targetLibBase == NULL) {
    fprintf(stderr, "[HARNESS] Failed to get library base for libmiuidiffpatcher.so\n");
    return -1;
  }
  g_target_lib_handle = targetLibHandle;
  
  
  targetFunctionPtr0 = (function_0)((uintptr_t)targetLibBase + 0x27E0);

  
  
  if (omen_stage_files() != 0) {
    return -1;
  }

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
