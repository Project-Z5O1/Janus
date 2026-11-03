#include <dlfcn.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "jenv.h"














#define BUFFER_SIZE 8448





void** __omen_afl_area_ptr = NULL;
char   __omen_bitmap_snapshot[65536] __attribute__((aligned(4096)));



typedef jstring (*function_0)(JNIEnv*, jclass, jstring, jstring);


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










#define OMEN_TARGET_CLASS "com/ted/android/contacts/common/ComManager"
#define OMEN_MAGIC "644e1bd336440e348b87ead6601dbcb6"
#define OMEN_MAGIC_LEN 32


#define OMEN_ARG0_MAX 128
#define OMEN_ARG1_MAX 4200



typedef struct {
    uint8_t* arg0;      
    size_t arg0_len;    
    uint8_t* arg1;      
    size_t arg1_len;    
} gv_args_t;

static void gv_free(gv_args_t* a) {
    if (a->arg0 != NULL) {
        free(a->arg0);
        a->arg0 = NULL;
    }
    if (a->arg1 != NULL) {
        free(a->arg1);
        a->arg1 = NULL;
    }
    a->arg0_len = 0;
    a->arg1_len = 0;
}


static int gv_parse(const uint8_t* buf, size_t len, gv_args_t* out) {
    size_t off;
    size_t len0;
    size_t len1;

    memset(out, 0, sizeof(*out));
    if (len < 2) {
        return -1; 
    }
    len0 = (size_t)buf[0] | ((size_t)buf[1] << 8);
    if (len0 > OMEN_ARG0_MAX) {
        return -1; 
    }
    if (2 + len0 + 2 > len) {
        return -1; 
    }
    len1 = (size_t)buf[2 + len0] | ((size_t)buf[2 + len0 + 1] << 8);
    if (len1 > OMEN_ARG1_MAX) {
        return -1; 
    }
    if (2 + len0 + 2 + len1 > len) {
        return -1; 
    }
    if (len0 > 0) {
        out->arg0 = (uint8_t*)malloc(len0 + 1); 
        if (out->arg0 == NULL) {
            return -1;
        }
        memcpy(out->arg0, buf + 2, len0);
        out->arg0[len0] = '\0';
        out->arg0_len = len0;
    }
    off = 2 + len0 + 2;
    if (len1 > 0) {
        out->arg1 = (uint8_t*)malloc(len1 + 1);
        if (out->arg1 == NULL) {
            gv_free(out);
            return -1;
        }
        memcpy(out->arg1, buf + off, len1);
        out->arg1[len1] = '\0';
        out->arg1_len = len1;
    }
    return 0;
}




void fuzz_one_input(const uint8_t* buffer, size_t length) {
  
  gv_args_t args;
  jclass targetClass = NULL;
  jstring jSign = NULL;
  jstring jAppKey = NULL;
  jstring jResult = NULL;
  const char* result_chars = NULL;

  set_stage("input_parsing");
  
  if (gv_parse(buffer, length, &args) != 0) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION: truncated/malformed/"
                    "over-declared-max input (len=%zu); no target call\n",
            length);
    goto release;
  }
  fprintf(stderr, "[HARNESS] args arg0_len=%zu arg1_len=%zu "
                  "arg1_magic_prefix=%d\n", args.arg0_len, args.arg1_len,
          (args.arg1_len >= OMEN_MAGIC_LEN &&
           memcmp(args.arg1, OMEN_MAGIC, OMEN_MAGIC_LEN) == 0) ? 1 : 0);

  



  set_stage("input_parsing_strings");
  jSign = (args.arg0 == NULL)
      ? (*ctx.env)->NewStringUTF(ctx.env, "")
      : (*ctx.env)->NewStringUTF(ctx.env, (const char*)args.arg0);
  jAppKey = (jSign == NULL) ? NULL
      : ((args.arg1 == NULL)
         ? (*ctx.env)->NewStringUTF(ctx.env, "")
         : (*ctx.env)->NewStringUTF(ctx.env, (const char*)args.arg1));
  if (jAppKey == NULL ||
      check_and_clear_jni_exception("input_parsing_strings")) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION_JNI: argument string "
                    "construction failed; no target call\n");
    goto release;
  }

  set_stage("get_obj_prereq");
  
  



  targetClass = (*ctx.env)->FindClass(ctx.env, OMEN_TARGET_CLASS);
  if (targetClass == NULL ||
      check_and_clear_jni_exception("get_obj_findclass")) {
    fprintf(stderr, "[HARNESS] FATAL_INIT: FindClass(%s) failed — valid "
                    "jclass policy violated; no target call\n",
            OMEN_TARGET_CLASS);
    fflush(stderr);
    goto release;
  }

  






  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy((void*)*__omen_afl_area_ptr, __omen_bitmap_snapshot, 65536);
  }

  
  set_stage("call_target");
  jResult = targetFunctionPtr0(ctx.env, targetClass, jSign, jAppKey);
  if (jResult == NULL ||
      check_and_clear_jni_exception("call_target")) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION_JNI: target returned "
                    "NULL/exception\n");
    goto release;
  }

  



  result_chars = (*ctx.env)->GetStringUTFChars(ctx.env, jResult, NULL);
  if (result_chars == NULL) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION_JNI: result "
                    "GetStringUTFChars NULL\n");
    goto release;
  }
  {
    size_t rlen = safe_strlen(result_chars);
    int is_hex = (rlen == OMEN_MAGIC_LEN);
    size_t i;
    for (i = 0; is_hex && i < rlen; i++) {
      char c = result_chars[i];
      if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) {
        is_hex = 0;
      }
    }
    fprintf(stderr, "[HARNESS] result=\"%s\" result_len=%zu "
                    "result_hex32=%d result_const_match=%d\n",
            result_chars, rlen, is_hex,
            (rlen == OMEN_MAGIC_LEN &&
             memcmp(result_chars, OMEN_MAGIC, OMEN_MAGIC_LEN) == 0) ? 1 : 0);
  }

release:
  set_stage("release");
  









  if (result_chars != NULL) {
    (*ctx.env)->ReleaseStringUTFChars(ctx.env, jResult, result_chars);
    result_chars = NULL;
  }
  if (jResult != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jResult);
    jResult = NULL;
  }
  if (jAppKey != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jAppKey);
    jAppKey = NULL;
  }
  if (jSign != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jSign);
    jSign = NULL;
  }
  if (targetClass != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, targetClass);
    targetClass = NULL;
  }
  gv_free(&args);
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
  
  
  
  
  
  
  void* targetLibHandle = dlopen("libteddycom-jni.so", RTLD_NOW);
  if (targetLibHandle == NULL) {
    fprintf(stderr, "[HARNESS] Failed to dlopen libteddycom-jni.so: %s\n",
            dlerror());
    return -1;
  }
  void* targetLibBase = get_library_base("libteddycom-jni.so");
  if (targetLibBase == NULL) {
    fprintf(stderr, "[HARNESS] Failed to get library base for "
                    "libteddycom-jni.so\n");
    return -1;
  }
  (void)targetLibHandle;
  






  targetFunctionPtr0 = (function_0)((uintptr_t)targetLibBase + 0x14E4);

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
