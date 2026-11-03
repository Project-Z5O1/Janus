#include <dlfcn.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "jenv.h"












#define BUFFER_SIZE 4096








void** __omen_afl_area_ptr = NULL;
char   __omen_bitmap_snapshot[65536] __attribute__((aligned(4096)));



jobject construct_android_graphics_Bitmap(JNIEnv* env, jint width, jint height,
                                          const uint8_t* pixels,
                                          size_t pixel_len);
typedef jobject (*function_0)(JNIEnv*, jclass, jobject, jfloatArray, jboolean);


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
  













  jclass clazz = NULL;
  jobject bitmap = NULL;
  jfloatArray cornersArray = NULL;
  jobject result = NULL;
  int bmpW = 0;
  int bmpH = 0;
  unsigned int cornersLen = 0;
  int zFlag = 0;
  jboolean z = JNI_FALSE;

  set_stage("input_parsing");
  
  


























  if (length < 35) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION len=%zu (<35B header), skip target\n",
            length);
    fflush(stderr);
    return;
  }
  {
    static const int kSizeTable[6][2] = {
      {1, 1}, {4, 4}, {32, 32}, {64, 64}, {128, 128}, {63, 63}
    };
    static uint8_t s_pixels[128u * 128u * 4u]; 
    uint8_t dimsSelector = buffer[0];
    uint8_t cornersLenRaw = buffer[1];
    uint8_t zByte = buffer[34];
    int dimIndex = (int)(dimsSelector % 6u);
    size_t pixelNeed;
    size_t tailLen;
    size_t copyLen;
    float corners[8];
    int writeCount;

    bmpW = kSizeTable[dimIndex][0];
    bmpH = kSizeTable[dimIndex][1];
    cornersLen = (unsigned int)cornersLenRaw;
    zFlag = (zByte != 0) ? 1 : 0;
    z = zFlag ? JNI_TRUE : JNI_FALSE;

    pixelNeed = (size_t)bmpW * (size_t)bmpH * 4u; 
    tailLen = length - 35;                        
    copyLen = (tailLen < pixelNeed) ? tailLen : pixelNeed;

    memcpy(corners, buffer + 2, sizeof(corners)); 
    memcpy(s_pixels, buffer + 35, copyLen);
    memset(s_pixels + copyLen, 0, pixelNeed - copyLen);

    clazz = (*ctx.env)->FindClass(ctx.env, "com/xiaomi/recognizer/Quadrangle");
    if (clazz == NULL ||
        check_and_clear_jni_exception("input_parsing_FindClass")) {
      clazz = NULL;
      goto release;
    }

    set_stage("construct_bitmap");
    







    bitmap = construct_android_graphics_Bitmap(ctx.env, (jint)bmpW, (jint)bmpH,
                                               s_pixels, pixelNeed);
    if (bitmap == NULL) {
      fprintf(stderr,
              "[HARNESS] FAIL_CONSTRUCTION construct_bitmap w=%d h=%d, skip target\n",
              bmpW, bmpH);
      fflush(stderr);
      goto release;
    }

    

    cornersArray = (*ctx.env)->NewFloatArray(ctx.env, (jsize)cornersLen);
    if (cornersArray == NULL ||
        check_and_clear_jni_exception("input_parsing_NewFloatArray")) {
      cornersArray = NULL;
      goto release;
    }
    writeCount = (cornersLen < 8u) ? (int)cornersLen : 8;
    if (writeCount > 0) {
      (*ctx.env)->SetFloatArrayRegion(ctx.env, cornersArray, 0,
                                      (jsize)writeCount, (const jfloat*)corners);
      if (check_and_clear_jni_exception("input_parsing_SetFloatArrayRegion")) {
        goto release;
      }
    }

    fprintf(stderr,
            "[HARNESS] constructed w=%d h=%d corners_len=%u z=%d pixel_need=%zu tail=%zu\n",
            bmpW, bmpH, cornersLen, zFlag, pixelNeed, tailLen);
    fflush(stderr);
  }

  set_stage("call_target");
  
  














  result = targetFunctionPtr0(ctx.env, clazz, bitmap, cornersArray, z);
  if (check_and_clear_jni_exception("call_jni_target")) {
    goto release;
  }

  



  if (result != NULL) {
    fprintf(stderr,
            "[HARNESS] rectify_ret=%p w=%d h=%d corners_len=%u z=%d (non-NULL Bitmap)\n",
            (void*)result, bmpW, bmpH, cornersLen, zFlag);
  } else {
    fprintf(stderr,
            "[HARNESS] rectify_ret=NULL w=%d h=%d corners_len=%u z=%d (legal target NULL path)\n",
            bmpW, bmpH, cornersLen, zFlag);
  }
  fflush(stderr);

release:
  set_stage("cleanup");
  


  if (result != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, result);
  }
  if (cornersArray != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, cornersArray);
  }
  if (bitmap != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, bitmap);
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
  








  targetFunctionPtr0 = (function_0)((uintptr_t)targetLibBase + 0x4d580);

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
