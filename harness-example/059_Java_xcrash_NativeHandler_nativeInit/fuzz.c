#include <dlfcn.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "jenv.h"




#include <sys/stat.h>












#define BUFFER_SIZE 4352





void** __omen_afl_area_ptr = NULL;
char   __omen_bitmap_snapshot[65536] __attribute__((aligned(4096)));





































typedef jint (*function_prereq_1)(JavaVM*, void*);
typedef jint (*function_0)(JNIEnv*, jclass,
                           jint,          
                           jstring,       
                           jstring,       
                           jstring,       
                           jstring,       
                           jstring,       
                           jstring,       
                           jstring,       
                           jstring,       
                           jstring,       
                           jstring,       
                           jboolean,      
                           jboolean,      
                           jint,          
                           jint,          
                           jint,          
                           jboolean,      
                           jboolean,      
                           jboolean,      
                           jboolean,      
                           jboolean,      
                           jint,          
                           jobjectArray,  
                           jboolean,      
                           jboolean,      
                           jint,          
                           jint,          
                           jint,          
                           jboolean,      
                           jboolean);     


static JavaCTX ctx;



function_prereq_1 prereqFunctionPtr1;   
function_0 targetFunctionPtr0;          















#define OMEN_TASK_DIR \
  "/data/local/tmp/omen/Java_xcrash_NativeHandler_nativeInit"
#define OMEN_LOG_DIR_PREFIX OMEN_TASK_DIR "/"
#define OMEN_LIB_DIR        OMEN_TASK_DIR "/lib"
#define OMEN_PREFIX_LEN     (sizeof(OMEN_LOG_DIR_PREFIX) - 1)
#define OMEN_LOG_DIR_SUFFIX_MAX 4200
#define OMEN_LOG_DIR_PATH_MAX (OMEN_PREFIX_LEN + OMEN_LOG_DIR_SUFFIX_MAX + 1)
#define OMEN_JNI_ONLOAD_OK 0x10006



#define OMEN_RET_INVALID     0x3ea
#define OMEN_RET_INTERNAL    0x3eb
#define OMEN_RET_REPEAT      0x3f7


#define CONST_SDK_INT          31
#define CONST_OS_VERSION       "12"
#define CONST_ABI_LIST         "arm64-v8a"
#define CONST_MANUFACTURER     "unknown"
#define CONST_BRAND            "Android"
#define CONST_MODEL            "Android SDK built for arm64"
#define CONST_FINGERPRINT \
  "Android/sdk_phone64_arm64/emulator64_arm64:12/SE1A.220621.001/" \
  "8752307:userdebug/test-keys"
#define CONST_APP_VERSION      "omen-argplan"
#define CONST_APP_VERSION_CODE "1"





static int g_jni_onload_done = 0;

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














































typedef struct {
    uint8_t* suffix;     
    size_t suffix_len;   
    uint8_t z10, z11;              
    int32_t i10, i11, i12;         
    uint8_t z12, z13, z14, z15, z16; 
    int32_t i13;                   
    uint8_t z18;                   
    int32_t i14, i15, i16;         
    uint8_t z20, z21;              
} xs_args_t;

#define OMEN_FIXED_TAIL 38

static void xs_free(xs_args_t* a) {
    if (a->suffix != NULL) {
        free(a->suffix);
        a->suffix = NULL;
    }
    a->suffix_len = 0;
}

static uint8_t xs_bool(const uint8_t b) {
    return b != 0 ? 1 : 0;
}

static int32_t xs_int32(const uint8_t* p) {
    

    uint32_t v = (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
                 ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
    return (int32_t)(v & 0x7fffffffu);
}



static int xs_parse(const uint8_t* buf, size_t len, xs_args_t* out) {
    size_t off;
    size_t suffix_len;
    size_t comp_start;

    memset(out, 0, sizeof(*out));
    if (len < 2) {
        return -1; 
    }
    suffix_len = (size_t)buf[0] | ((size_t)buf[1] << 8);
    if (suffix_len > OMEN_LOG_DIR_SUFFIX_MAX) {
        return -1; 
    }
    if (2 + suffix_len + OMEN_FIXED_TAIL > len) {
        return -1; 
    }
    
    if (suffix_len > 0 && buf[2] == '/') {
        return -1; 
    }
    comp_start = 0;
    for (off = 0; off < suffix_len; off++) {
        if (buf[2 + off] == '/') {
            



            if (off - comp_start == 2 &&
                buf[2 + comp_start] == '.' &&
                buf[2 + comp_start + 1] == '.') {
                return -1; 
            }
            comp_start = off + 1;
        }
    }
    if (suffix_len - comp_start == 2 &&
        buf[2 + comp_start] == '.' &&
        buf[2 + comp_start + 1] == '.') {
        return -1; 
    }
    out->suffix = (uint8_t*)malloc(suffix_len + 1); 
    if (out->suffix == NULL) {
        return -1;
    }
    memcpy(out->suffix, buf + 2, suffix_len);
    out->suffix[suffix_len] = '\0';
    out->suffix_len = suffix_len;
    off = 2 + suffix_len;
    out->z10 = xs_bool(buf[off + 0]);
    out->z11 = xs_bool(buf[off + 1]);
    out->i10 = xs_int32(buf + off + 2);
    out->i11 = xs_int32(buf + off + 6);
    out->i12 = xs_int32(buf + off + 10);
    out->z12 = xs_bool(buf[off + 14]);
    out->z13 = xs_bool(buf[off + 15]);
    out->z14 = xs_bool(buf[off + 16]);
    out->z15 = xs_bool(buf[off + 17]);
    out->z16 = xs_bool(buf[off + 18]);
    out->i13 = xs_int32(buf + off + 19);
    out->z18 = xs_bool(buf[off + 23]);
    out->i14 = xs_int32(buf + off + 24);
    out->i15 = xs_int32(buf + off + 28);
    out->i16 = xs_int32(buf + off + 32);
    out->z20 = xs_bool(buf[off + 36]);
    out->z21 = xs_bool(buf[off + 37]);
    return 0;
}






void fuzz_one_input(const uint8_t* buffer, size_t length) {
  















  xs_args_t args;
  jstring jOsVersion = NULL, jAbiList = NULL, jManufacturer = NULL;
  jstring jBrand = NULL, jModel = NULL, jFingerprint = NULL;
  jstring jAppVersion = NULL, jAppVersionCode = NULL;
  jstring jNativeLibraryDir = NULL, jLogDir = NULL;
  char* log_dir = NULL;
  jint target_ret = 0;
  
  uint8_t z10, z11, z12, z13, z14, z15, z16, z18, z20, z21;
  int32_t i10, i11, i12, i13, i14, i15, i16;

  set_stage("jni_onload_init");
  











  if (g_jni_onload_done == 0) {
    int onload_ret = prereqFunctionPtr1(ctx.vm, NULL);
    fprintf(stderr, "[HARNESS] JNI_OnLoad -> 0x%x (expect 0x%x)\n",
            (unsigned)onload_ret, (unsigned)OMEN_JNI_ONLOAD_OK);
    if (onload_ret != OMEN_JNI_ONLOAD_OK) {
      fprintf(stderr, "[HARNESS] FATAL_INIT: JNI_OnLoad returned 0x%x != "
                      "0x%x — jclass global DAT_00125270 unwritten (D2 "
                      "unresolved); hard-fail, no target call.\n",
              (unsigned)onload_ret, (unsigned)OMEN_JNI_ONLOAD_OK);
      fflush(stderr);
      _exit(-7);
    }
    if (check_and_clear_jni_exception("jni_onload_init")) {
      fflush(stderr);
      _exit(JNI_ERR);
    }
    g_jni_onload_done = 1;
  }

  set_stage("input_parsing");
  
  if (xs_parse(buffer, length, &args) != 0) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION: truncated/malformed/"
                    "namespace-violating input (len=%zu); no target call\n",
            length);
    goto release;
  }
  z10 = args.z10;   z11 = args.z11;
  i10 = args.i10;   i11 = args.i11;   i12 = args.i12;
  z12 = args.z12;   z13 = args.z13;   z14 = args.z14;
  z15 = args.z15;   z16 = args.z16;
  i13 = args.i13;
  z18 = args.z18;
  i14 = args.i14;   i15 = args.i15;   i16 = args.i16;
  z20 = args.z20;   z21 = args.z21;

  


  log_dir = (char*)malloc(OMEN_LOG_DIR_PATH_MAX);
  if (log_dir == NULL ||
      (size_t)snprintf(log_dir, OMEN_LOG_DIR_PATH_MAX, "%s%s",
                       OMEN_LOG_DIR_PREFIX,
                       (const char*)args.suffix) >= OMEN_LOG_DIR_PATH_MAX) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION: logDir build failed\n");
    if (log_dir != NULL) {
      free(log_dir);
      log_dir = NULL;
    }
    xs_free(&args);
    goto release;
  }
  xs_free(&args); 
  fprintf(stderr, "[HARNESS] logdir=%s (len=%d) z10=%u z17=CONST(0) "
                  "strArr=CONST(NULL)\n", log_dir,
          (int)(OMEN_PREFIX_LEN + strlen(log_dir + OMEN_PREFIX_LEN)),
          z10);

  






  jOsVersion = (*ctx.env)->NewStringUTF(ctx.env, CONST_OS_VERSION);
  jAbiList = (jOsVersion == NULL) ? NULL
      : (*ctx.env)->NewStringUTF(ctx.env, CONST_ABI_LIST);
  jManufacturer = (jAbiList == NULL) ? NULL
      : (*ctx.env)->NewStringUTF(ctx.env, CONST_MANUFACTURER);
  jBrand = (jManufacturer == NULL) ? NULL
      : (*ctx.env)->NewStringUTF(ctx.env, CONST_BRAND);
  jModel = (jBrand == NULL) ? NULL
      : (*ctx.env)->NewStringUTF(ctx.env, CONST_MODEL);
  jFingerprint = (jModel == NULL) ? NULL
      : (*ctx.env)->NewStringUTF(ctx.env, CONST_FINGERPRINT);
  jAppVersion = (jFingerprint == NULL) ? NULL
      : (*ctx.env)->NewStringUTF(ctx.env, CONST_APP_VERSION);
  jAppVersionCode = (jAppVersion == NULL) ? NULL
      : (*ctx.env)->NewStringUTF(ctx.env, CONST_APP_VERSION_CODE);
  jNativeLibraryDir = (jAppVersionCode == NULL) ? NULL
      : (*ctx.env)->NewStringUTF(ctx.env, OMEN_LIB_DIR);
  jLogDir = (jNativeLibraryDir == NULL) ? NULL
      : (*ctx.env)->NewStringUTF(ctx.env, log_dir);
  if (jLogDir == NULL ||
      check_and_clear_jni_exception("input_parsing_strings")) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION: argument string "
                    "construction failed; no target call\n");
    goto release;
  }

  set_stage("call_target");
  
  







  




  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy((void*)*__omen_afl_area_ptr, __omen_bitmap_snapshot, 65536);
  }
  



  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy(__omen_bitmap_snapshot, (void*)*__omen_afl_area_ptr, 65536);
  }

  






  target_ret = targetFunctionPtr0(
      ctx.env, NULL,
      CONST_SDK_INT,                       
      jOsVersion,                          
      jAbiList,                            
      jManufacturer,                       
      jBrand,                              
      jModel,                              
      jFingerprint,                        
      jAppVersion,                         
      jAppVersionCode,                     
      jNativeLibraryDir,                   
      jLogDir,                             
      (jboolean)z10,                       
      (jboolean)z11,                       
      i10, i11, i12,                       
      (jboolean)z12, (jboolean)z13, (jboolean)z14,
      (jboolean)z15, (jboolean)z16,        
      i13,                                 
      NULL,                                
      (jboolean)0,                         
      (jboolean)z18,                       
      i14, i15, i16,                       
      (jboolean)z20, (jboolean)z21);       
  fprintf(stderr, "[HARNESS] target_ret=0x%x (%s)\n", (unsigned)target_ret,
          target_ret == 0 ? "ok" :
          target_ret == OMEN_RET_INVALID ? "invalid-args" :
          target_ret == OMEN_RET_INTERNAL ? "internal-error" :
          target_ret == OMEN_RET_REPEAT ? "repeat-or-step-failed" :
          "UNDOCUMENTED");
  if (check_and_clear_jni_exception("call_target")) {
    goto release;
  }

release:
  set_stage("release");
  







  if (jLogDir != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jLogDir);
    jLogDir = NULL;
  }
  if (jNativeLibraryDir != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jNativeLibraryDir);
    jNativeLibraryDir = NULL;
  }
  if (jAppVersionCode != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jAppVersionCode);
    jAppVersionCode = NULL;
  }
  if (jAppVersion != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jAppVersion);
    jAppVersion = NULL;
  }
  if (jFingerprint != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jFingerprint);
    jFingerprint = NULL;
  }
  if (jModel != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jModel);
    jModel = NULL;
  }
  if (jBrand != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jBrand);
    jBrand = NULL;
  }
  if (jManufacturer != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jManufacturer);
    jManufacturer = NULL;
  }
  if (jAbiList != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jAbiList);
    jAbiList = NULL;
  }
  if (jOsVersion != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jOsVersion);
    jOsVersion = NULL;
  }
  if (log_dir != NULL) {
    free(log_dir);
    log_dir = NULL;
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
  
  
  
  void* targetLibHandle = dlopen("libxcrash.so", RTLD_NOW);
  if (targetLibHandle == NULL) {
    fprintf(stderr, "[HARNESS] Failed to dlopen libxcrash.so: %s\n",
            dlerror());
    return -1;
  }
  void* targetLibBase = get_library_base("libxcrash.so");
  if (targetLibBase == NULL) {
    fprintf(stderr, "[HARNESS] Failed to get library base for libxcrash.so\n");
    return -1;
  }
  (void)targetLibHandle;
  




  prereqFunctionPtr1 = (function_prereq_1)((uintptr_t)targetLibBase + 0x328C);
  targetFunctionPtr0 = (function_0)((uintptr_t)targetLibBase + 0x336C);

  







  








  set_stage("ensure_staging_dirs");
  if (mkdir(OMEN_TASK_DIR, 0755) != 0 && errno != EEXIST) {
    fprintf(stderr, "[HARNESS] mkdir %s: %s (non-fatal; usability checked "
                    "by probe)\n", OMEN_TASK_DIR, strerror(errno));
  }
  if (mkdir(OMEN_LIB_DIR, 0755) != 0 && errno != EEXIST) {
    fprintf(stderr, "[HARNESS] mkdir %s: %s (non-fatal)\n", OMEN_LIB_DIR,
            strerror(errno));
  }
  {
    char probe_path[256];
    FILE* pf;
    if ((size_t)snprintf(probe_path, sizeof(probe_path), "%s/.omen_probe",
                         OMEN_TASK_DIR) >= sizeof(probe_path)) {
      return -1;
    }
    pf = fopen(probe_path, "wb");
    if (pf == NULL || fclose(pf) != 0) {
      fprintf(stderr, "[HARNESS] staging dir %s unusable; aborting\n",
              OMEN_TASK_DIR);
      return -1;
    }
    unlink(probe_path);
  }

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
