#include <dlfcn.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "jenv.h"
#include "construct.h"








#define BUFFER_SIZE 4096








void** __omen_afl_area_ptr = NULL;
char   __omen_bitmap_snapshot[65536] __attribute__((aligned(4096)));

















typedef jobject (*function_0)(JNIEnv*, jclass, jobject);


static JavaCTX ctx;



function_0 targetFunctionPtr0; 













typedef enum {
  PAT_BLACK,          
  PAT_WHITE,          
  PAT_CHECKER_1PX,    
  PAT_CHECKER_2PX,    
  PAT_GRAD_H,         
  PAT_GRAD_LONG_SIDE, 
  PAT_UNIFORM_GRAY,   
  PAT_LCG_NOISE,      
  PAT_SPACE_0X20      
} pat_kind;

typedef struct {
  const char* name;   
  const char* kind;   
  jint width;
  jint height;
  pat_kind pattern;
} pattern_entry;

static const pattern_entry TABLE[] = {
  
  { "benign_1x1_black",      "gate",    1,   1,    PAT_BLACK        },
  { "benign_4x4_checker2",   "gate",    4,   4,    PAT_CHECKER_2PX  },
  { "benign_8x8_checker1",   "gate",    8,   8,    PAT_CHECKER_1PX  },
  { "benign_32x32_gradient", "gate",   32,  32,    PAT_GRAD_H       },
  { "benign_64x64_uniform",  "gate",   64,  64,    PAT_UNIFORM_GRAY },
  { "benign_128x128_noise",  "gate",  128, 128,    PAT_LCG_NOISE    },
  { "benign_63x63_noise",    "gate",   63,  63,    PAT_LCG_NOISE    },
  
  { "probe_min_1x1_white",   "probe",   1,   1,    PAT_WHITE        },
  { "probe_min_2x2_checker", "probe",   2,   2,    PAT_CHECKER_1PX  },
  { "probe_aspect_1x4096",   "probe",   1,4096,    PAT_GRAD_LONG_SIDE },
  { "probe_aspect_4096x1",   "probe",4096,   1,    PAT_GRAD_LONG_SIDE },
  { "probe_content_sister9", "probe",  32,  32,    PAT_SPACE_0X20   },
};
#define TABLE_N ((int)(sizeof(TABLE) / sizeof(TABLE[0])))



static uint8_t g_payload[65536];
static size_t  g_payload_len;

static uint32_t lcg_next(uint32_t* s)
{
  *s = *s * 1664525u + 1013904223u;
  return *s;
}

static void fill_pixels(const pattern_entry* e, uint8_t* px)
{
  size_t i;
  jint x, y;
  size_t stride = (size_t)e->width * 4u;

  switch (e->pattern) {
  case PAT_BLACK:
    for (i = 0; i < g_payload_len; i += 4) {
      px[i] = 0x00; px[i+1] = 0x00; px[i+2] = 0x00; px[i+3] = 0xFF;
    }
    break;
  case PAT_WHITE:
    for (i = 0; i < g_payload_len; i += 4) {
      px[i] = 0xFF; px[i+1] = 0xFF; px[i+2] = 0xFF; px[i+3] = 0xFF;
    }
    break;
  case PAT_CHECKER_1PX:
  case PAT_CHECKER_2PX: {
    int cell = (e->pattern == PAT_CHECKER_1PX) ? 1 : 2;
    for (y = 0; y < e->height; y++) {
      for (x = 0; x < e->width; x++) {
        uint8_t v = (((x / cell) + (y / cell)) & 1) ? 0xFF : 0x00;
        size_t o = (size_t)y * stride + (size_t)x * 4u;
        px[o] = v; px[o+1] = v; px[o+2] = v; px[o+3] = 0xFF;
      }
    }
    break;
  }
  case PAT_GRAD_H:
    for (y = 0; y < e->height; y++) {
      for (x = 0; x < e->width; x++) {
        uint8_t v = (uint8_t)((e->width > 1)
            ? (x * 255) / (e->width - 1) : 0x80);
        size_t o = (size_t)y * stride + (size_t)x * 4u;
        px[o] = v; px[o+1] = v; px[o+2] = v; px[o+3] = 0xFF;
      }
    }
    break;
  case PAT_GRAD_LONG_SIDE: {
    jint long_side = (e->width >= e->height) ? e->width : e->height;
    jint horiz = (e->width >= e->height);
    for (y = 0; y < e->height; y++) {
      for (x = 0; x < e->width; x++) {
        jint t = horiz ? x : y;
        uint8_t v = (uint8_t)((long_side > 1)
            ? (t * 255) / (long_side - 1) : 0x80);
        size_t o = (size_t)y * stride + (size_t)x * 4u;
        px[o] = v; px[o+1] = v; px[o+2] = v; px[o+3] = 0xFF;
      }
    }
    break;
  }
  case PAT_UNIFORM_GRAY:
    for (i = 0; i < g_payload_len; i += 4) {
      px[i] = 0x80; px[i+1] = 0x80; px[i+2] = 0x80; px[i+3] = 0xFF;
    }
    break;
  case PAT_LCG_NOISE: {
    uint32_t s = (uint32_t)(e->width * 1000003u + e->height + 1u);
    for (i = 0; i < g_payload_len; i += 4) {
      uint32_t r = lcg_next(&s);
      px[i]   = (uint8_t)(r >> 24);
      px[i+1] = (uint8_t)(r >> 16);
      px[i+2] = (uint8_t)(r >> 8);
      px[i+3] = 0xFF;
    }
    break;
  }
  case PAT_SPACE_0X20:
    

    memset(px, 0x20, g_payload_len);
    break;
  }
}

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
  jobject result = NULL;
  jclass resultClass = NULL;
  jfloatArray cornersArray = NULL;
  

  unsigned patternId = 0;
  int patW = 0;
  int patH = 0;

  set_stage("input_parsing");
  
  








  if (length < 1) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION len=0 (layout requires the "
                    "1B pattern selector), skip target\n");
    fflush(stderr);
    return;
  }
  {
    unsigned patternIndex = (unsigned)buffer[0] % (unsigned)TABLE_N;
    const pattern_entry* entry = &TABLE[patternIndex];
    size_t need = (size_t)entry->width * (size_t)entry->height * 4u;

    


    if (need > sizeof(g_payload)) {
      fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION pattern_id=%u payload "
                      "%zu > cap %zu, skip target\n",
              patternIndex, need, sizeof(g_payload));
      fflush(stderr);
      return;
    }
    g_payload_len = need;
    fill_pixels(entry, g_payload);
    patternId = patternIndex;
    patW = (int)entry->width;
    patH = (int)entry->height;
    fprintf(stderr, "[HARNESS] pattern_id=%u entry=%s kind=%s w=%d h=%d "
                    "pixel_bytes=%zu\n",
            patternIndex, entry->name, entry->kind, patW, patH, need);
    fflush(stderr);

    clazz = (*ctx.env)->FindClass(ctx.env, "com/xiaomi/recognizer/Quadrangle");
    if (clazz == NULL ||
        check_and_clear_jni_exception("input_parsing_FindClass")) {
      clazz = NULL;
      goto release;
    }

    
    bitmap = construct_android_graphics_Bitmap(ctx.env, entry->width,
                                               entry->height, g_payload,
                                               g_payload_len);
    if (bitmap == NULL) {
      fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION pattern_id=%u w=%d h=%d "
                      "(recipe returned NULL), skip target\n",
              patternIndex, (int)entry->width, (int)entry->height);
      fflush(stderr);
      goto release;
    }
    if (check_and_clear_jni_exception("input_parsing_construct_bitmap")) {
      goto release;
    }
  }

  set_stage("call_target");
  
  












  result = targetFunctionPtr0(ctx.env, clazz, bitmap);
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
                "[HARNESS] detect_ret=%p w=%d h=%d pattern_id=%u "
                "resultValid=%d corners_len=%d\n",
                (void*)result, patW, patH, patternId,
                (int)resultValid, cornersLen);
      } else {
        fprintf(stderr,
                "[HARNESS] detect_ret=%p w=%d h=%d pattern_id=%u "
                "resultValid=<field lookup failed>\n",
                (void*)result, patW, patH, patternId);
      }
      fflush(stderr);
    }
  } else {
    fprintf(stderr, "[HARNESS] detect_ret=NULL w=%d h=%d pattern_id=%u "
                    "(gate seeds must NOT hit this: argument_plan.md Harness "
                    "Notes item 5)\n",
                    patW, patH, patternId);
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
  









  targetFunctionPtr0 = (function_0)((uintptr_t)targetLibBase + 0x4e92c);

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
