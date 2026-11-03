#include <dlfcn.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "jenv.h"






#define BUFFER_SIZE 1560








void** __omen_afl_area_ptr = NULL;
char   __omen_bitmap_snapshot[65536] __attribute__((aligned(4096)));








typedef jstring (*function_0)(JNIEnv*, jclass, jstring, jstring, jstring,
                              jstring, jstring, jstring);


static JavaCTX ctx;


function_0 targetFunctionPtr0;

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
















#define OMEN_ARG_MAX 256

static int omen_need(size_t length, size_t off, size_t n) {
  return off <= length && n <= length - off;
}

static uint16_t omen_rd_u16(const uint8_t* p) {
  uint16_t v;
  memcpy(&v, p, sizeof(v));
  return v;
}





static jstring omen_make_jstring(const uint8_t* data, uint16_t n, int* failed) {
  char* tmp = (char*)malloc((size_t)n + 1);
  jstring s;

  if (tmp == NULL) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION reason=oom_string\n");
    *failed = 1;
    return NULL;
  }
  memcpy(tmp, data, n);
  tmp[n] = '\0';
  s = (*ctx.env)->NewStringUTF(ctx.env, tmp);
  free(tmp);
  if (s == NULL || check_and_clear_jni_exception("input_parsing") != 0) {
    *failed = 1;
    return NULL;
  }
  return s;
}




void fuzz_one_input(const uint8_t* buffer, size_t length) {
  size_t off = 0;
  jclass clazz = NULL;
  jstring args[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
  jstring ret = NULL;
  int i;
  int failed = 0;

  set_stage("input_parsing");
  
  
  
  
  
  clazz = (*ctx.env)->FindClass(ctx.env, "java/lang/String");
  if (clazz == NULL || check_and_clear_jni_exception("input_parsing") != 0) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION reason=find_class\n");
    clazz = NULL;
    goto cleanup;
  }
  for (i = 0; i < 6 && failed == 0; i++) {
    uint16_t n;
    if (!omen_need(length, off, 2)) {
      fprintf(stderr,
              "[HARNESS] FAIL_CONSTRUCTION reason=str_len_prefix idx=%d "
              "len=%zu\n", i, length);
      failed = 1;
      break;
    }
    n = omen_rd_u16(buffer + off);
    off += 2;
    if (n > OMEN_ARG_MAX) {
      fprintf(stderr,
              "[HARNESS] FAIL_CONSTRUCTION reason=str_len_overflow idx=%d "
              "n=%u max=%d\n", i, (unsigned)n, OMEN_ARG_MAX);
      failed = 1;
      break;
    }
    if (!omen_need(length, off, (size_t)n)) {
      fprintf(stderr,
              "[HARNESS] FAIL_CONSTRUCTION reason=str_payload idx=%d n=%u "
              "len=%zu\n", i, (unsigned)n, length);
      failed = 1;
      break;
    }
    args[i] = omen_make_jstring(buffer + off, n, &failed);
    off += n;
  }
  if (failed != 0) {
    goto cleanup; 
  }

  set_stage("call_target");
  
  
  
  
  ret = targetFunctionPtr0(ctx.env, clazz,
                           args[0], args[1], args[2],
                           args[3], args[4], args[5]);
  

  if (check_and_clear_jni_exception("call_target") == 0 && ret != NULL) {
    const char* sig = (*ctx.env)->GetStringUTFChars(ctx.env, ret, NULL);
    if (sig != NULL) {
      fprintf(stderr, "[HARNESS] target_ret=%.64s\n", sig);
      (*ctx.env)->ReleaseStringUTFChars(ctx.env, ret, sig);
    } else {
      fprintf(stderr, "[HARNESS] target_ret=<GetStringUTFChars NULL>\n");
      check_and_clear_jni_exception("call_target");
    }
  } else {
    fprintf(stderr, "[HARNESS] target_ret=<NULL or pending-exception>\n");
  }

  set_stage("cleanup");
cleanup:
  
  
  if (ret != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, ret);
  }
  for (i = 5; i >= 0; i--) {
    if (args[i] != NULL) {
      (*ctx.env)->DeleteLocalRef(ctx.env, args[i]);
    }
  }
  if (clazz != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, clazz);
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
  
  
  
  
  
  
  
  
  void* targetLibHandle = dlopen("libMZSignature.so", RTLD_NOW);
  if (targetLibHandle == NULL) {
    fprintf(stderr, "[HARNESS] Failed to dlopen libMZSignature.so: %s\n", dlerror());
    return -1;
  }
  void* targetLibBase = get_library_base("libMZSignature.so");
  if (targetLibBase == NULL) {
    fprintf(stderr, "[HARNESS] Failed to get library base for libMZSignature.so\n");
    return -1;
  }
  (void)targetLibHandle;
  
  
  targetFunctionPtr0 = (function_0)((uintptr_t)targetLibBase + 0xF50);

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
