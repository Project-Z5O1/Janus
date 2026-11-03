#include <dlfcn.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "jenv.h"




#include <elf.h>
#include <fcntl.h>















#define BUFFER_SIZE 12







#define YUV_W 64
#define YUV_H 64
#define BMP_W 64
#define BMP_H 64









void** __omen_afl_area_ptr = NULL;
char   __omen_bitmap_snapshot[65536] __attribute__((aligned(4096)));























jobject construct_YuvData_input(JNIEnv* env, jlong real_ptr, jint w, jint h);
typedef jint (*function_0)(JNIEnv*, jclass, jobject yuvData, jobject bitmap,
                           jint waterMarkWidth, jint waterMarkHeight,
                           jint degree, jboolean wideGamut);


static JavaCTX ctx;



function_0 targetFunctionPtr0; 








static jmethodID g_getPtr, g_getW, g_getH, g_getRel;
static int g_oracle_ready = 0;

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






















static uintptr_t rx_load_vaddr_page(const char* path, size_t page) {
  unsigned char hdr[sizeof(Elf64_Ehdr)];
  unsigned char ph_buf[32 * sizeof(Elf64_Phdr)];
  ssize_t k;
  int fd;
  uint16_t phentsize, phnum;
  uint64_t phoff;
  int i;
  uintptr_t best = 0;

  fd = open(path, O_RDONLY);
  if (fd < 0) {
    return 0;
  }
  k = pread(fd, hdr, sizeof(hdr), 0);
  if (k < (ssize_t)sizeof(Elf64_Ehdr) ||
      memcmp(hdr, ELFMAG, SELFMAG) != 0 ||
      hdr[EI_CLASS] != ELFCLASS64) {
    close(fd);
    return 0;
  }
  phoff = ((Elf64_Ehdr*)hdr)->e_phoff;
  phentsize = ((Elf64_Ehdr*)hdr)->e_phentsize;
  phnum = ((Elf64_Ehdr*)hdr)->e_phnum;
  if (phentsize != sizeof(Elf64_Phdr) || phnum == 0 || phnum > 32) {
    close(fd);
    return 0;
  }
  k = pread(fd, ph_buf, (size_t)phnum * phentsize, (off_t)phoff);
  close(fd);
  if (k < (ssize_t)((size_t)phnum * phentsize)) {
    return 0;
  }
  for (i = 0; i < phnum; i++) {
    Elf64_Phdr* p = (Elf64_Phdr*)(ph_buf + (size_t)i * phentsize);
    if (p->p_type == PT_LOAD && (p->p_flags & PF_X) != 0 &&
        (p->p_flags & PF_R) != 0) {
      best = (uintptr_t)(p->p_vaddr & ~(uintptr_t)(page - 1));
      break; 
    }
  }
  return best;
}







static void* true_library_base(const char* lib_name, void* handle) {
  uintptr_t reported;
  long page_l;
  size_t page;
  uintptr_t rx_page = 0;
  const char* candidates[2];
  const char* override_path = getenv("OMEN_TARGET_LIB_PATH");
  int i;
  int n = 0;

  (void)handle;
  page_l = sysconf(_SC_PAGESIZE);
  page = page_l > 0 ? (size_t)page_l : 4096;
  reported = (uintptr_t)get_library_base((char*)lib_name);
  if (reported == 0) {
    return NULL;
  }
  if (override_path != NULL) {
    candidates[n++] = override_path;
  } else {
    candidates[n++] = lib_name;          
    candidates[n++] = "lib/librefocus.so"; 
  }
  for (i = 0; i < n && rx_page == 0; i++) {
    rx_page = rx_load_vaddr_page(candidates[i], page);
    if (rx_page != 0) {
      fprintf(stderr,
              "[BASE] %s r-xp_reported=%p rx_vaddr_page=0x%zx file=%s -> "
              "bias=%p\n",
              lib_name, (void*)reported, (size_t)rx_page, candidates[i],
              (void*)(reported - rx_page));
    }
  }
  if (rx_page == 0) {
    return NULL;
  }
  return (void*)(reported - rx_page);
}





static int init_oracle_accessors(void) {
  jclass cls = (*ctx.env)->FindClass(ctx.env,
      "com/miui/extraphoto/refocus/model/YuvData");
  if (cls == NULL) {
    check_and_clear_jni_exception("init_oracle_FindClass");
    return -1;
  }
  g_getPtr = (*ctx.env)->GetMethodID(ctx.env, cls, "getYuvDataPointer", "()J");
  g_getW = (*ctx.env)->GetMethodID(ctx.env, cls, "getWidth", "()I");
  g_getH = (*ctx.env)->GetMethodID(ctx.env, cls, "getHeight", "()I");
  g_getRel = (*ctx.env)->GetMethodID(ctx.env, cls, "getNeedReleaseYuvPointer",
                                     "()I");
  (*ctx.env)->DeleteLocalRef(ctx.env, cls);
  if (g_getPtr == NULL || g_getW == NULL || g_getH == NULL ||
      g_getRel == NULL) {
    check_and_clear_jni_exception("init_oracle_GetMethodID");
    return -1;
  }
  g_oracle_ready = 1;
  return 0;
}








static uint8_t* make_i420(int w, int h) {
  size_t ysz = (size_t)w * (size_t)h;
  size_t usz = (size_t)(w / 2) * (size_t)(h / 2);
  uint8_t* buf = (uint8_t*)malloc(ysz + 2 * usz);
  size_t i;

  if (buf == NULL) {
    return NULL;
  }
  for (i = 0; i < ysz; i++) {
    buf[i] = (uint8_t)(0x10 + (i % 0xEF));
  }
  memset(buf + ysz, 0x80, 2 * usz);
  return buf;
}













static jobject make_bitmap(JNIEnv* env, int w, int h) {
  jclass bmp_cls, cfg_cls;
  jmethodID factory;
  jfieldID cfg_fid;
  jobject cfg, bmp;

  bmp_cls = (*env)->FindClass(env, "android/graphics/Bitmap");
  if (bmp_cls == NULL) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION FindClass Bitmap\n");
    if ((*env)->ExceptionCheck(env) != JNI_FALSE) {
      (*env)->ExceptionDescribe(env);
      (*env)->ExceptionClear(env);
    }
    return NULL;
  }
  cfg_cls = (*env)->FindClass(env, "android/graphics/Bitmap$Config");
  if (cfg_cls == NULL) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION FindClass Bitmap$Config\n");
    if ((*env)->ExceptionCheck(env) != JNI_FALSE) {
      (*env)->ExceptionDescribe(env);
      (*env)->ExceptionClear(env);
    }
    (*env)->DeleteLocalRef(env, bmp_cls);
    return NULL;
  }
  cfg_fid = (*env)->GetStaticFieldID(env, cfg_cls, "ARGB_8888",
                                     "Landroid/graphics/Bitmap$Config;");
  if (cfg_fid == NULL) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION GetStaticFieldID ARGB_8888\n");
    if ((*env)->ExceptionCheck(env) != JNI_FALSE) {
      (*env)->ExceptionDescribe(env);
      (*env)->ExceptionClear(env);
    }
    (*env)->DeleteLocalRef(env, cfg_cls);
    (*env)->DeleteLocalRef(env, bmp_cls);
    return NULL;
  }
  cfg = (*env)->GetStaticObjectField(env, cfg_cls, cfg_fid);
  if (cfg == NULL) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION GetStaticObjectField cfg\n");
    (*env)->DeleteLocalRef(env, cfg_cls);
    (*env)->DeleteLocalRef(env, bmp_cls);
    return NULL;
  }
  factory = (*env)->GetStaticMethodID(
      env, bmp_cls, "createBitmap",
      "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
  if (factory == NULL) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION GetStaticMethodID "
                    "createBitmap\n");
    if ((*env)->ExceptionCheck(env) != JNI_FALSE) {
      (*env)->ExceptionDescribe(env);
      (*env)->ExceptionClear(env);
    }
    (*env)->DeleteLocalRef(env, cfg);
    (*env)->DeleteLocalRef(env, cfg_cls);
    (*env)->DeleteLocalRef(env, bmp_cls);
    return NULL;
  }
  bmp = (*env)->CallStaticObjectMethod(env, bmp_cls, factory, (jint)w, (jint)h,
                                       cfg);
  if ((*env)->ExceptionCheck(env) != JNI_FALSE) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION createBitmap(%d,%d) "
                    "exception\n", w, h);
    (*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);
    (*env)->DeleteLocalRef(env, cfg);
    (*env)->DeleteLocalRef(env, cfg_cls);
    (*env)->DeleteLocalRef(env, bmp_cls);
    return NULL;
  }
  if (bmp == NULL) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION createBitmap NULL\n");
    (*env)->DeleteLocalRef(env, cfg);
    (*env)->DeleteLocalRef(env, cfg_cls);
    (*env)->DeleteLocalRef(env, bmp_cls);
    return NULL;
  }
  (*env)->DeleteLocalRef(env, cfg);
  (*env)->DeleteLocalRef(env, cfg_cls);
  (*env)->DeleteLocalRef(env, bmp_cls);
  return bmp; 
}


























static int32_t arm_sdiv32(int32_t a, int32_t b) {
  if (b == 0) {
    return 0; 
  }
  if (a == INT32_MIN && b == -1) {
    return INT32_MIN; 
  }
  return a / b; 
}

static int32_t expected_height(jint wmW, jint wmH, jint degree,
                               int* geo_valid) {
  uint32_t q = (uint32_t)degree * 0xA4FA4FA5u + 0x2D82D80u;
  uint32_t r = (q >> 2) | (q << 30); 
  int32_t sw, sh, ratio, finalH;

  *geo_valid = 0;
  if (r < 0x16C16C1u) {
    
    sw = (int32_t)wmW;
    sh = (int32_t)wmH;
  } else if (degree == 90 || degree == 180 || degree == 270) {
    
    sw = (int32_t)wmH;
    sh = (int32_t)wmW;
  } else {
    


    return 0;
  }
  *geo_valid = 1;
  ratio = arm_sdiv32(YUV_W, sw);
  finalH = (int32_t)((uint32_t)ratio * (uint32_t)sh);
  return (int32_t)((uint32_t)YUV_H + (uint32_t)finalH);
}




void fuzz_one_input(const uint8_t* buffer, size_t length) {
  











  jobject yuv = NULL;
  jobject bmp = NULL;
  uint8_t* host_buf = NULL;
  jint ret = 0;
  jint argWmW = 0;
  jint argWmH = 0;
  jint argDegree = 0;

  set_stage("input_parsing");
  
  












  if (length < 12u) {
    fprintf(stderr,
            "[HARNESS] FAIL_CONSTRUCTION len=%zu (<12B header), skip target\n",
            length);
    fflush(stderr);
    goto release;
  }
  {
    uint32_t w_raw, h_raw, d_raw;

    memcpy(&w_raw, buffer + 0, 4);
    memcpy(&h_raw, buffer + 4, 4);
    memcpy(&d_raw, buffer + 8, 4);
    memcpy(&argWmW, &w_raw, 4);    
    memcpy(&argWmH, &h_raw, 4);    
    memcpy(&argDegree, &d_raw, 4); 

    




    host_buf = make_i420(YUV_W, YUV_H);
    if (host_buf == NULL) {
      fprintf(stderr,
              "[HARNESS] FAIL_CONSTRUCTION host I420 malloc(%d,%d), skip "
              "target\n",
              YUV_W, YUV_H);
      fflush(stderr);
      goto release;
    }

    


    if (!g_oracle_ready && init_oracle_accessors() != 0) {
      fprintf(stderr,
              "[HARNESS] FAIL_CONSTRUCTION oracle accessor init, skip "
              "target\n");
      fflush(stderr);
      goto release;
    }

    





    yuv = construct_YuvData_input(ctx.env, (jlong)(uintptr_t)host_buf,
                                  (jint)YUV_W, (jint)YUV_H);
    if (yuv == NULL) {
      fprintf(stderr,
              "[HARNESS] FAIL_CONSTRUCTION construct_YuvData_input NULL, "
              "skip target\n");
      fflush(stderr);
      goto release;
    }

    



    {
      static int ctor_logged_once = 0;
      jlong c_ptr = (*ctx.env)->CallLongMethod(ctx.env, yuv, g_getPtr);
      jint c_w = (*ctx.env)->CallIntMethod(ctx.env, yuv, g_getW);
      jint c_h = (*ctx.env)->CallIntMethod(ctx.env, yuv, g_getH);
      jint c_rel = (*ctx.env)->CallIntMethod(ctx.env, yuv, g_getRel);
      if (check_and_clear_jni_exception("ctor_readback")) {
        goto release;
      }
      if (!ctor_logged_once || c_ptr != (jlong)(uintptr_t)host_buf ||
          c_w != YUV_W || c_h != YUV_H || c_rel != 0) {
        fprintf(stderr,
                "[HARNESS] ctor_state%s ptr=0x%llx w=%d h=%d rel=%d "
                "(want 0x%llx/%d/%d/0)%s\n",
                ctor_logged_once ? "_VIOLATION" : "",
                (unsigned long long)c_ptr, c_w, c_h, c_rel,
                (unsigned long long)(uintptr_t)host_buf, YUV_W, YUV_H,
                (c_ptr != (jlong)(uintptr_t)host_buf || c_w != YUV_W ||
                 c_h != YUV_H || c_rel != 0)
                    ? ", skip target"
                    : "");
        ctor_logged_once = 1;
        if (c_ptr != (jlong)(uintptr_t)host_buf || c_w != YUV_W ||
            c_h != YUV_H || c_rel != 0) {
          fflush(stderr);
          goto release;
        }
      }
    }

    


    bmp = make_bitmap(ctx.env, BMP_W, BMP_H);
    if (bmp == NULL) {
      fprintf(stderr,
              "[HARNESS] FAIL_CONSTRUCTION make_bitmap NULL, skip target\n");
      fflush(stderr);
      goto release;
    }
  }

  set_stage("call_target");
  
  


















  ret = targetFunctionPtr0(ctx.env, NULL , yuv,
                           bmp, argWmW, argWmH, argDegree,
                           (jboolean)0 );
  {
    int exc = check_and_clear_jni_exception("call_jni_target");
    if (exc) {
      goto release;
    }
  }

  







  if (g_oracle_ready) {
    jlong ptr = (*ctx.env)->CallLongMethod(ctx.env, yuv, g_getPtr);
    jint w = (*ctx.env)->CallIntMethod(ctx.env, yuv, g_getW);
    jint h = (*ctx.env)->CallIntMethod(ctx.env, yuv, g_getH);
    jint rel = (*ctx.env)->CallIntMethod(ctx.env, yuv, g_getRel);
    int32_t exp_h = 0;
    int geo_valid = 0;
    int h_ok = 1;
    int carrier_unchanged = (ptr == (jlong)(uintptr_t)host_buf);
    int merged_freed = 0;

    if (check_and_clear_jni_exception("oracle_readback")) {
      goto release;
    }
    if (ret == 0) {
      exp_h = expected_height(argWmW, argWmH, argDegree, &geo_valid);
      if (!geo_valid || ptr == 0 || carrier_unchanged || w != YUV_W ||
          rel != 0 || (int32_t)h != exp_h) {
        h_ok = 0;
        fprintf(stderr,
                "[HARNESS] ORACLE_VIOLATION ret=0 geo_valid=%d "
                "ptr=0x%llx w=%d h=%d exp_h=%d rel=%d\n",
                geo_valid, (unsigned long long)ptr, w, h, (int)exp_h, rel);
      }
      








      if (ptr != 0 && !carrier_unchanged) {
        free((void*)(uintptr_t)ptr);
        merged_freed = 1;
      }
    } else if (!carrier_unchanged) {
      
      fprintf(stderr,
              "[HARNESS] ORACLE_VIOLATION ret=%d but carrier pointer was "
              "replaced (ptr=0x%llx)\n",
              ret, (unsigned long long)ptr);
      h_ok = 0;
    }
    fprintf(stderr,
            "[HARNESS] target_ret=%d yuvDataPointer=0x%llx w=%d h=%d "
            "needRelease=%d old_buf=0x%llx carrier_unchanged=%d h_ok=%d "
            "merged_freed=%d (wmW=%d wmH=%d degree=%d exp_h=%d)\n",
            ret, (unsigned long long)ptr, w, h, rel,
            (unsigned long long)(uintptr_t)host_buf, carrier_unchanged, h_ok,
            merged_freed, argWmW, argWmH, argDegree, (int)exp_h);
  }
  fflush(stderr);

release:
  set_stage("cleanup");
  






  if (bmp != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, bmp);
  }
  if (yuv != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, yuv);
  }
  if (host_buf != NULL) {
    
    free(host_buf);
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
  
  
  
  
  
  
  
  
  
  
  
  
  void* targetLibHandle = dlopen("librefocus.so", RTLD_NOW);
  if (targetLibHandle == NULL) {
    fprintf(stderr, "[HARNESS] Failed to dlopen librefocus.so: %s\n", dlerror());
    return -1;
  }
  {
    








    void* targetLibBase = true_library_base("librefocus.so", targetLibHandle);
    if (targetLibBase == NULL) {
      fprintf(stderr,
              "[HARNESS] Failed to resolve load-bias-corrected base for "
              "librefocus.so\n");
      return -1;
    }
    



    targetFunctionPtr0 =
        (function_0)((uintptr_t)targetLibBase + 0x8E38);
  }
  (void)targetLibHandle;

  

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
