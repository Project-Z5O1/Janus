#include <dlfcn.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <android/bitmap.h>
#include "jenv.h"






#include <elf.h>
#include <fcntl.h>




















#define BUFFER_SIZE 16385






#define W 64
#define H 64
#define RGBA_LEN (W * H * 4)      
#define NV21_LEN (W * H * 3 / 2)  










void** __omen_afl_area_ptr = NULL;
char   __omen_bitmap_snapshot[65536] __attribute__((aligned(4096)));



































typedef jlong (*function_1)(JNIEnv*, jclass, jobject bitmap, jint w, jint h);
typedef void (*function_0)(JNIEnv*, jclass, jobject carrier, jlong handle);
typedef void (*function_release)(JNIEnv*, jclass, jlong handle);


static JavaCTX ctx;



function_1 prereqFunctionPtr1;    
function_0 targetFunctionPtr0;    
function_release releaseFunctionPtr0; 







static jobject construct_Bitmap(JNIEnv* env, const char* config_field);

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














typedef int (*NV21ToI420_t)(const uint8_t*, int, const uint8_t*, int, uint8_t*,
                            int, uint8_t*, int, uint8_t*, int, int, int);
typedef int (*J420ToABGR_t)(const uint8_t*, int, const uint8_t*, int,
                            const uint8_t*, int, uint8_t*, int, int, int);

static int g_parity_enabled = -1; 
static void* g_libyuv = NULL;
static NV21ToI420_t g_nv21_to_i420;
static J420ToABGR_t g_j420_to_abgr;

static int parity_oracle_ready(void) {
  if (g_parity_enabled < 0) {
    const char* e = getenv("OMEN_PARITY");
    g_parity_enabled = (e != NULL && e[0] != '\0' && e[0] != '0');
  }
  if (!g_parity_enabled) {
    return 0;
  }
  if (g_libyuv == NULL) {
    g_libyuv = dlopen("libyuv.so", RTLD_NOW);
    if (g_libyuv == NULL) {
      fprintf(stderr, "[HARNESS] parity oracle dlopen libyuv.so: %s\n",
              dlerror());
      g_parity_enabled = 0; 
      return 0;
    }
    g_nv21_to_i420 = (NV21ToI420_t)dlsym(g_libyuv, "NV21ToI420");
    g_j420_to_abgr = (J420ToABGR_t)dlsym(g_libyuv, "J420ToABGR");
    if (g_nv21_to_i420 == NULL || g_j420_to_abgr == NULL) {
      fprintf(stderr, "[HARNESS] parity oracle dlsym NV21ToI420/J420ToABGR "
                      "failed\n");
      g_parity_enabled = 0;
      return 0;
    }
  }
  return 1;
}











static uint8_t* bitmap_readback(JNIEnv* env, jobject bmp, int want_rgba,
                                AndroidBitmapInfo* info_out) {
  AndroidBitmapInfo info;
  void* pixels = NULL;
  size_t row_bytes;
  uint8_t* copy;
  int y;

  if (AndroidBitmap_getInfo(env, bmp, &info) != 0) {
    fprintf(stderr, "[HARNESS] AndroidBitmap_getInfo (readback) failed\n");
    return NULL;
  }
  if ((int)info.format != (want_rgba ? ANDROID_BITMAP_FORMAT_RGBA_8888
                                     : ANDROID_BITMAP_FORMAT_RGB_565)) {
    fprintf(stderr, "[HARNESS] readback format=%d want=%d\n", info.format,
            want_rgba ? 1 : 4);
    return NULL;
  }
  if (info.width != W || info.height != H) {
    fprintf(stderr, "[HARNESS] readback dims %ux%u != %dx%d\n", info.width,
            info.height, W, H);
    return NULL;
  }
  row_bytes = (size_t)W * (want_rgba ? 4 : 2);
  if ((size_t)info.stride < row_bytes) {
    fprintf(stderr, "[HARNESS] readback stride=%u < row=%zu\n", info.stride,
            row_bytes);
    return NULL;
  }
  if (want_rgba && (size_t)info.stride != (size_t)W * 4) {
    fprintf(stderr,
            "[HARNESS] readback stride=%u != W*4=%d (target writes "
            "W*4-strided rows derived from the image struct)\n",
            info.stride, W * 4);
    return NULL;
  }
  if (AndroidBitmap_lockPixels(env, bmp, &pixels) != 0 || pixels == NULL) {
    fprintf(stderr, "[HARNESS] lockPixels (readback) failed\n");
    return NULL;
  }
  copy = (uint8_t*)malloc((size_t)info.stride * H);
  if (copy == NULL) {
    AndroidBitmap_unlockPixels(env, bmp);
    return NULL;
  }
  for (y = 0; y < H; y++) {
    memcpy(copy + (size_t)y * info.stride,
           (uint8_t*)pixels + (size_t)y * info.stride, info.stride);
  }
  AndroidBitmap_unlockPixels(env, bmp);
  if (info_out != NULL) {
    *info_out = info;
  }
  return copy;
}





static void oracle_alpha_check(const char* tag, const uint8_t* frame) {
  size_t i, bad = 0;
  for (i = 0; i < (size_t)W * H; i++) {
    if (frame[i * 4 + 3] != 0xFF) {
      bad++;
    }
  }
  if (bad != 0) {
    fprintf(stderr,
            "[HARNESS] ORACLE_VIOLATION %s alpha!=0xFF pixels=%zu/%d "
            "(native write-proof broken)\n",
            tag, bad, W * H);
  } else {
    fprintf(stderr, "[HARNESS] [ORACLE] %s alpha proof OK (%d/%d == 0xFF)\n",
            tag, W * H, W * H);
  }
}









static void oracle_parity_check(JNIEnv* env, jlong handle,
                                const uint8_t* frame) {
  const uint8_t* p = (const uint8_t*)(uintptr_t)handle;
  int32_t w, h;
  const uint8_t* nv21;
  uint8_t *y, *u, *v, *abgr;
  int uv_stride, rc;
  size_t i, mism = 0;

  memcpy(&w, p + 4, sizeof(w));
  memcpy(&h, p + 8, sizeof(h));
  memcpy(&nv21, p + 0x10, sizeof(nv21));
  if (w != W || h != H || nv21 == NULL) {
    fprintf(stderr, "[HARNESS] [PARITY_RESULT] FAIL (struct w=%d h=%d "
                    "nv21=%p not the expected 64x64 state)\n", w, h,
            (const void*)nv21);
    return;
  }
  y = (uint8_t*)malloc((size_t)w * h);
  u = (uint8_t*)malloc((size_t)w * h / 4);
  v = (uint8_t*)malloc((size_t)w * h / 4);
  abgr = (uint8_t*)malloc((size_t)w * h * 4);
  if (y == NULL || u == NULL || v == NULL || abgr == NULL) {
    fprintf(stderr, "[HARNESS] [PARITY_RESULT] FAIL (host malloc)\n");
    free(y); free(u); free(v); free(abgr);
    return;
  }
  uv_stride = (w + 1) & ~1;
  rc = g_nv21_to_i420(nv21, w, nv21 + (size_t)w * h, uv_stride,
                      y, w, u, w / 2, v, w / 2, w, h);
  if (rc != 0) {
    fprintf(stderr, "[HARNESS] [PARITY_RESULT] FAIL (NV21ToI420 rc=%d)\n", rc);
    free(y); free(u); free(v); free(abgr);
    return;
  }
  rc = g_j420_to_abgr(y, w, u, w / 2, v, w / 2, abgr, w * 4, w, h);
  if (rc != 0) {
    fprintf(stderr, "[HARNESS] [PARITY_RESULT] FAIL (J420ToABGR rc=%d)\n", rc);
    free(y); free(u); free(v); free(abgr);
    return;
  }
  for (i = 0; i < (size_t)w * h * 4; i++) {
    if (abgr[i] != frame[i]) {
      if (mism < 4) {
        fprintf(stderr,
                "[HARNESS] [PARITY] byte[%zu] ref=%02x dst=%02x\n",
                i, abgr[i], frame[i]);
      }
      mism++;
    }
  }
  if (mism == 0) {
    fprintf(stderr,
            "[HARNESS] [PARITY_RESULT] OK (dst == libyuv(NV21->I420->ABGR) "
            "of the factory's own NV21, %u bytes, 0 mismatch)\n",
            (unsigned)(w * h * 4));
  } else {
    fprintf(stderr,
            "[HARNESS] [PARITY_RESULT] FAIL (%zu mismatch bytes over %u)\n",
            mism, (unsigned)(w * h * 4));
  }
  free(y); free(u); free(v); free(abgr);
}




void fuzz_one_input(const uint8_t* buffer, size_t length) {
  












  jobject bmp = NULL;
  jobject probe_bmp = NULL;
  jlong handle = 0;
  int release_handle = 0;
  int probe_mode = 0;
  uint8_t mode_raw = 0;
  uint8_t* frame = NULL;      
  uint8_t* carrier = NULL;    

  set_stage("input_parsing");
  
  











  if (length >= 1) {
    mode_raw = buffer[0];
  }
  probe_mode = (mode_raw == 0x01);

  frame = (uint8_t*)malloc(RGBA_LEN);
  if (frame == NULL) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION frame malloc\n");
    fflush(stderr);
    goto release;
  }
  {
    size_t avail = (length >= 1) ? length - 1 : 0;
    if (avail > (size_t)RGBA_LEN) {
      avail = (size_t)RGBA_LEN;
    }
    if (avail > 0) {
      memcpy(frame, buffer + 1, avail);
    }
    if (avail < (size_t)RGBA_LEN) {
      memset(frame + avail, 0, (size_t)RGBA_LEN - avail);
    }
    
    {
      size_t i;
      for (i = 0; i < (size_t)W * H; i++) {
        frame[i * 4 + 3] = 0xFF;
      }
    }
  }

  set_stage("get_obj");
  




  bmp = construct_Bitmap(ctx.env, "ARGB_8888");
  if (bmp == NULL) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION construct_Bitmap ARGB_8888, "
                    "skip target\n");
    fflush(stderr);
    goto release;
  }
  {
    



    AndroidBitmapInfo info;
    void* pixels = NULL;
    int y;
    memset(&info, 0, sizeof(info));
    if (AndroidBitmap_getInfo(ctx.env, bmp, &info) != 0 ||
        info.width != W || info.height != H ||
        (int)info.format != ANDROID_BITMAP_FORMAT_RGBA_8888 ||
        (size_t)info.stride != (size_t)W * 4) {
      fprintf(stderr,
              "[HARNESS] FAIL_CONSTRUCTION carrier info "
              "(fmt=%d dims=%ux%u stride=%u want %d/%dx%d/%d)\n",
              (int)info.format, info.width, info.height, info.stride,
              ANDROID_BITMAP_FORMAT_RGBA_8888, W, H, W * 4);
      goto release;
    }
    if (AndroidBitmap_lockPixels(ctx.env, bmp, &pixels) != 0 ||
        pixels == NULL) {
      fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION lockPixels (fill)\n");
      goto release;
    }
    for (y = 0; y < H; y++) {
      memcpy((uint8_t*)pixels + (size_t)y * info.stride,
             frame + (size_t)y * W * 4, (size_t)W * 4);
    }
    AndroidBitmap_unlockPixels(ctx.env, bmp);
  }

  



  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy((void*)*__omen_afl_area_ptr, __omen_bitmap_snapshot, 65536);
  }

  set_stage("call_target");
  
  





  handle = prereqFunctionPtr1(ctx.env, NULL ,
                              bmp, (jint)W, (jint)H);
  fprintf(stderr, "[HARNESS] [STEP1] generateNativeImage handle=0x%llx\n",
          (unsigned long long)handle);
  if (check_and_clear_jni_exception("step1_generateNativeImage")) {
    
    handle = 0;
    goto release;
  }
  if (handle == 0) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION handle == 0, skip target\n");
    fflush(stderr);
    goto release;
  }
  release_handle = 1; 
  {
    




    const uint8_t* p = (const uint8_t*)(uintptr_t)handle;
    int32_t sw = 0, sh = 0;
    const uint8_t* nv21 = NULL;
    memcpy(&sw, p + 4, sizeof(sw));
    memcpy(&sh, p + 8, sizeof(sh));
    memcpy(&nv21, p + 0x10, sizeof(nv21));
    fprintf(stderr,
            "[HARNESS] [STRUCT] w=%d h=%d nv21=%p (handle +4/+8/+0x10)\n",
            sw, sh, (const void*)nv21);
    if (sw != W || sh != H || nv21 == NULL) {
      fprintf(stderr,
              "[HARNESS] FAIL_CONSTRUCTION struct gate "
              "(w=%d h=%d nv21=%p want %d/%d/non-NULL), skip target\n",
              sw, sh, (const void*)nv21, W, H);
      fflush(stderr);
      goto release;
    }
  }

  
  if (probe_mode) {
    probe_bmp = construct_Bitmap(ctx.env, "RGB_565");
    if (probe_bmp == NULL) {
      fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION construct_Bitmap RGB_565, "
                      "skip target\n");
      fflush(stderr);
      goto release;
    }
    fprintf(stderr,
            "[HARNESS] [PROBE] v2 carrier format=RGB_565(4) dims 64x64 "
            "(dims contract kept; format gate = licensed vector-2 axis)\n");
  }

  
  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy(__omen_bitmap_snapshot, (void*)*__omen_afl_area_ptr, 65536);
  }

  







  targetFunctionPtr0(ctx.env, NULL ,
                     probe_mode ? probe_bmp : bmp, handle);
  if (check_and_clear_jni_exception("call_jni_target")) {
    goto release;
  }

  set_stage("oracle");
  








  if (probe_mode) {
    fprintf(stderr,
            "[HARNESS] [PROBE] target survived; post-call paired lock/unlock "
            "on the never-locked RGB_565 carrier (Scudo abort expected, "
            "archived as V2 crash face)\n");
    fflush(stderr);
  }
  carrier = bitmap_readback(ctx.env, probe_mode ? probe_bmp : bmp,
                            !probe_mode, NULL);
  if (carrier != NULL) {
    if (!probe_mode) {
      oracle_alpha_check("legal", carrier);
      if (parity_oracle_ready()) {
        oracle_parity_check(ctx.env, handle, carrier);
      }
    } else {
      
      fprintf(stderr,
              "[HARNESS] [RECORD] v2 probe read-back survived (no abort this "
              "run; unlock-on-never-locked face did not fire) — recorded, "
              "not suppressed\n");
    }
    free(carrier);
    carrier = NULL;
  } else if (!probe_mode) {
    fprintf(stderr,
            "[HARNESS] ORACLE_VIOLATION legal readback unavailable "
            "(getInfo/lock/stride gate)\n");
  }
  fflush(stderr);

release:
  set_stage("cleanup");
  








  if (handle != 0 && release_handle) {
    releaseFunctionPtr0(ctx.env, NULL , handle);
    if (check_and_clear_jni_exception("release_native_image")) {
      
    } else {
      fprintf(stderr, "[HARNESS] releaseNativeImage(0x%llx) paired\n",
              (unsigned long long)handle);
    }
    handle = 0;
  }
  if (probe_bmp != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, probe_bmp);
    probe_bmp = NULL;
  }
  if (bmp != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, bmp);
    bmp = NULL;
  }
  free(carrier);
  free(frame);
}












static jobject construct_Bitmap(JNIEnv* env, const char* config_field) {
  jclass bmp_cls;
  jclass cfg_cls;
  jfieldID cfg_fid;
  jobject config;
  jmethodID create;
  jobject bmp;

  bmp_cls = (*env)->FindClass(env, "android/graphics/Bitmap");
  if (bmp_cls == NULL) {
    fprintf(stderr, "[recipe:Bitmap] FindClass android/graphics/Bitmap\n");
    check_and_clear_jni_exception("recipe_Bitmap_FindClass");
    return NULL;
  }
  cfg_cls = (*env)->FindClass(env, "android/graphics/Bitmap$Config");
  if (cfg_cls == NULL) {
    fprintf(stderr, "[recipe:Bitmap] FindClass Bitmap$Config\n");
    check_and_clear_jni_exception("recipe_Bitmap_FindClass_Config");
    (*env)->DeleteLocalRef(env, bmp_cls);
    return NULL;
  }
  cfg_fid = (*env)->GetStaticFieldID(env, cfg_cls, config_field,
                                     "Landroid/graphics/Bitmap$Config;");
  if (cfg_fid == NULL) {
    fprintf(stderr, "[recipe:Bitmap] GetStaticFieldID %s\n", config_field);
    check_and_clear_jni_exception("recipe_Bitmap_GetStaticFieldID");
    (*env)->DeleteLocalRef(env, cfg_cls);
    (*env)->DeleteLocalRef(env, bmp_cls);
    return NULL;
  }
  config = (*env)->GetStaticObjectField(env, cfg_cls, cfg_fid);
  if (check_and_clear_jni_exception("recipe_Bitmap_GetStaticObjectField")) {
    (*env)->DeleteLocalRef(env, cfg_cls);
    (*env)->DeleteLocalRef(env, bmp_cls);
    return NULL;
  }
  if (config == NULL) {
    fprintf(stderr, "[recipe:Bitmap] Config.%s is NULL\n", config_field);
    (*env)->DeleteLocalRef(env, cfg_cls);
    (*env)->DeleteLocalRef(env, bmp_cls);
    return NULL;
  }
  create = (*env)->GetStaticMethodID(
      env, bmp_cls, "createBitmap",
      "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
  if (create == NULL) {
    fprintf(stderr, "[recipe:Bitmap] GetStaticMethodID createBitmap\n");
    check_and_clear_jni_exception("recipe_Bitmap_GetStaticMethodID");
    (*env)->DeleteLocalRef(env, config);
    (*env)->DeleteLocalRef(env, cfg_cls);
    (*env)->DeleteLocalRef(env, bmp_cls);
    return NULL;
  }
  bmp = (*env)->CallStaticObjectMethod(env, bmp_cls, create, (jint)W, (jint)H,
                                       config);
  if (check_and_clear_jni_exception("recipe_Bitmap_CallStaticObjectMethod")) {
    (*env)->DeleteLocalRef(env, config);
    (*env)->DeleteLocalRef(env, cfg_cls);
    (*env)->DeleteLocalRef(env, bmp_cls);
    return NULL;
  }
  
  (*env)->DeleteLocalRef(env, config);
  (*env)->DeleteLocalRef(env, cfg_cls);
  (*env)->DeleteLocalRef(env, bmp_cls);
  if (bmp == NULL) {
    fprintf(stderr, "[recipe:Bitmap] createBitmap returned NULL\n");
    return NULL;
  }
  return bmp;
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
    





    prereqFunctionPtr1 =
        (function_1)((uintptr_t)targetLibBase + 0x81C8);
    targetFunctionPtr0 =
        (function_0)((uintptr_t)targetLibBase + 0x8048);
    releaseFunctionPtr0 =
        (function_release)((uintptr_t)targetLibBase + 0x8260);
  }
  (void)targetLibHandle;

  








  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
