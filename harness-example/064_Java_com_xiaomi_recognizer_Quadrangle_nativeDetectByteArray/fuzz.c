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














typedef jobject (*function_0)(JNIEnv*, jclass, jbyteArray, jint, jint);


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




void fuzz_one_input(const uint8_t* buffer, size_t length) {
  










  jbyteArray dataArray = NULL;
  jclass clazz = NULL;
  jobject result = NULL;
  jclass resultClass = NULL;
  jfloatArray cornersArray = NULL;
  jint width = 0;
  jint height = 0;

  set_stage("input_parsing");
  
  











  if (length < 8) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION len=%zu (<8B header), skip target\n",
            length);
    fflush(stderr);
    return;
  }
  {
    uint32_t widthRaw = 0;
    uint32_t heightRaw = 0;
    size_t dataLen = length - 8;
    memcpy(&widthRaw, buffer + 0, 4);
    memcpy(&heightRaw, buffer + 4, 4);
    width = (jint)widthRaw;   
    height = (jint)heightRaw; 

    dataArray = (*ctx.env)->NewByteArray(ctx.env, (jsize)dataLen);
    if (dataArray == NULL ||
        check_and_clear_jni_exception("input_parsing_NewByteArray")) {
      dataArray = NULL;
      goto release;
    }
    if (dataLen > 0) {
      (*ctx.env)->SetByteArrayRegion(ctx.env, dataArray, 0, (jsize)dataLen,
                                     (const jbyte*)(buffer + 8));
      if (check_and_clear_jni_exception("input_parsing_SetByteArrayRegion")) {
        goto release;
      }
    }

    clazz = (*ctx.env)->FindClass(ctx.env, "com/xiaomi/recognizer/Quadrangle");
    if (clazz == NULL ||
        check_and_clear_jni_exception("input_parsing_FindClass")) {
      clazz = NULL;
      goto release;
    }
  }

  set_stage("call_target");
  
  









  result = targetFunctionPtr0(ctx.env, clazz, dataArray, width, height);
  if (check_and_clear_jni_exception("call_jni_target")) {
    goto release;
  }

  



  if (result != NULL) {
    resultClass = (*ctx.env)->GetObjectClass(ctx.env, result);
    if (resultClass != NULL &&
        !check_and_clear_jni_exception("result_GetObjectClass")) {
      jfieldID validField = (*ctx.env)->GetFieldID(ctx.env, resultClass,
                                                   "resultValid", "Z");
      jfieldID cornersField = NULL;
      if (check_and_clear_jni_exception("result_GetFieldID_resultValid")) {
        validField = NULL;
      } else {
        cornersField = (*ctx.env)->GetFieldID(ctx.env, resultClass,
                                              "quadrangleCorners", "[F");
        if (check_and_clear_jni_exception("result_GetFieldID_quadrangleCorners")) {
          cornersField = NULL;
        }
      }
      if (validField != NULL) {
        jboolean resultValid = (*ctx.env)->GetBooleanField(ctx.env, result,
                                                           validField);
        int cornersLen = -1;
        if (cornersField != NULL) {
          cornersArray = (jfloatArray)(*ctx.env)->GetObjectField(ctx.env, result,
                                                                 cornersField);
          if (check_and_clear_jni_exception("result_GetObjectField_corners")) {
            cornersArray = NULL;
          } else if (cornersArray != NULL) {
            cornersLen = (int)(*ctx.env)->GetArrayLength(ctx.env, cornersArray);
          }
        }
        fprintf(stderr,
                "[HARNESS] detect_ret=%p w=%d h=%d data_len=%zu "
                "resultValid=%d corners_len=%d\n",
                (void*)result, (int)width, (int)height,
                length - 8, (int)resultValid, cornersLen);
      } else {
        fprintf(stderr,
                "[HARNESS] detect_ret=%p w=%d h=%d data_len=%zu "
                "resultValid=<field lookup failed>\n",
                (void*)result, (int)width, (int)height, length - 8);
      }
      fflush(stderr);
    }
  } else {
    fprintf(stderr, "[HARNESS] detect_ret=NULL w=%d h=%d data_len=%zu "
                    "(legal target NULL path)\n",
            (int)width, (int)height, length - 8);
    fflush(stderr);
  }

release:
  set_stage("cleanup");
  



  if (cornersArray != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, cornersArray);
  }
  if (resultClass != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, resultClass);
  }
  if (result != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, result);
  }
  if (clazz != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, clazz);
  }
  if (dataArray != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, dataArray);
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
  
  
  void* targetLibHandle = dlopen("libQuadrangleRecognizer.so", RTLD_NOW);
  if (targetLibHandle == NULL) {
    fprintf(stderr, "[HARNESS] Failed to dlopen libQuadrangleRecognizer.so: %s\n",
            dlerror());
    return -1;
  }
  void* targetLibBase = get_library_base("libQuadrangleRecognizer.so");
  if (targetLibBase == NULL) {
    fprintf(stderr, "[HARNESS] Failed to get library base for libQuadrangleRecognizer.so\n");
    return -1;
  }
  (void)targetLibHandle;
  








  targetFunctionPtr0 = (function_0)((uintptr_t)targetLibBase + 0x4e220);

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
