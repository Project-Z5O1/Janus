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


#define HEADER_LEN 19u


#define MAX_BITMAP_W 256u
#define MAX_BITMAP_H 256u
#define PIXEL_BUF_LEN (MAX_BITMAP_W * MAX_BITMAP_H * 4u)















void** __omen_afl_area_ptr = NULL;
char   __omen_bitmap_snapshot[65536] __attribute__((aligned(4096)));



















typedef jint (*jni_onload_fn)(JavaVM*, void*);
typedef jlong (*create_instance_fn)(JNIEnv*, jobject, jintArray);
typedef jint (*function_0)(JNIEnv*, jobject, jlong, jobject, jint, jint,
                           jint, jint, jobjectArray);









jobject construct_android_graphics_Bitmap(JNIEnv* env,
                                          jint width, jint height,
                                          const uint8_t* pixels,
                                          size_t pixel_len);
jobject construct_NativeSdk(JNIEnv* env,
                            const unsigned char* buffer,
                            size_t length,
                            size_t* offset);


static JavaCTX ctx;



jni_onload_fn onLoadFunctionPtr0;        
create_instance_fn createInstanceFnPtr0; 
function_0 targetFunctionPtr0;           

static const char* g_current_stage = "startup";
static int g_trace_stages = 0;




static const jint kDimsTable[5][2] = {
    {1, 1}, {32, 32}, {64, 64}, {128, 128}, {256, 256}};

static uint32_t lcg_next(uint32_t* s) {
  *s = *s * 1664525u + 1013904223u;
  return *s;
}




static void fill_pixels(jint w, jint h, unsigned int pat, uint8_t* px) {
  uint32_t i;
  jint x, y;
  size_t stride = (size_t)w * 4u;

  switch (pat) {
  runtime case: 
    memset(px, 0x00, (size_t)w * (size_t)h * 4u);
    break;
  runtime case: 
    memset(px, 0xFF, (size_t)w * (size_t)h * 4u);
    break;
  runtime case: 
    for (y = 0; y < h; y++) {
      for (x = 0; x < w; x++) {
        uint8_t v = ((x + y) & 1) ? 0xFF : 0x00;
        size_t o = (size_t)y * stride + (size_t)x * 4u;
        px[o] = v; px[o + 1] = v; px[o + 2] = v; px[o + 3] = 0xFF;
      }
    }
    break;
  runtime case: 
    for (y = 0; y < h; y++) {
      for (x = 0; x < w; x++) {
        uint8_t v = (uint8_t)((w > 1) ? (x * 255) / (w - 1) : 0x80);
        size_t o = (size_t)y * stride + (size_t)x * 4u;
        px[o] = v; px[o + 1] = v; px[o + 2] = v; px[o + 3] = 0xFF;
      }
    }
    break;
  default: 
    {
      uint32_t s = (uint32_t)((uint32_t)w * 1000003u + (uint32_t)h + 1u);
      for (i = 0; i < (size_t)w * (size_t)h * 4u; i += 4) {
        uint32_t r = lcg_next(&s);
        px[i] = (uint8_t)(r >> 24);
        px[i + 1] = (uint8_t)(r >> 16);
        px[i + 2] = (uint8_t)(r >> 8);
        px[i + 3] = 0xFF;
      }
    }
    break;
  }
}







static void describe_element(jobject elem, const char** kind,
                             char* text, size_t text_cap) {
  jclass stringClass = NULL, floatArrClass = NULL;
  *kind = "none";
  text[0] = '\0';
  if (elem == NULL) {
    return;
  }
  stringClass = (*ctx.env)->FindClass(ctx.env, "java/lang/String");
  floatArrClass = (*ctx.env)->FindClass(ctx.env, "[F");
  if (stringClass != NULL &&
      (*ctx.env)->IsInstanceOf(ctx.env, elem, stringClass) == JNI_TRUE) {
    const char* utf;
    *kind = "String";
    utf = (*ctx.env)->GetStringUTFChars(ctx.env, (jstring)elem, NULL);
    if (utf != NULL) {
      snprintf(text, text_cap, "%s", utf);
      (*ctx.env)->ReleaseStringUTFChars(ctx.env, (jstring)elem, utf);
    }
  } else if (floatArrClass != NULL &&
             (*ctx.env)->IsInstanceOf(ctx.env, elem, floatArrClass) ==
                 JNI_TRUE) {
    *kind = "float[]";
  } else {
    *kind = "other";
  }
  if (stringClass != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, stringClass);
  }
  if (floatArrClass != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, floatArrClass);
  }
}

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






static void fatal_init(const char* why) {
  safe_write_string("[HARNESS] FATAL_INIT ");
  safe_write_string(why);
  safe_write_string("\n");
  _exit(61);
}




void fuzz_one_input(const uint8_t* buffer, size_t length) {
  














  jobject thiz = NULL;
  jintArray formats = NULL;
  jobject bitmap = NULL;
  jclass objCls = NULL;
  jobjectArray outArr = NULL;
  size_t recipe_offset = 0;
  jlong handle = 0;

  
  
  
  
  
  
  
  
  
  
  
  {
    static int s_wired = 0;
    if (!s_wired) {
      set_stage("prereq_lazy_wiring");
      



      if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
        memcpy((void*)*__omen_afl_area_ptr, __omen_bitmap_snapshot, 65536);
      }

      set_stage("prereq_dlopen");
      void* targetLibHandle = dlopen("libscanner.so", RTLD_NOW);
      if (targetLibHandle == NULL) {
        fprintf(stderr, "[HARNESS] FATAL_INIT dlopen(libscanner.so): %s\n",
                dlerror());
        fflush(stderr);
        fatal_init("dlopen");
      }
      void* targetLibBase = get_library_base("libscanner.so");
      if (targetLibBase == NULL) {
        fprintf(stderr,
                "[HARNESS] FATAL_INIT get_library_base(libscanner.so) NULL\n");
        fflush(stderr);
        fatal_init("get_library_base");
      }
      (void)targetLibHandle;

      set_stage("init_java_env");
      {
        char* options = "-Djava.class.path=./base.apk";
        int status = init_java_env(&ctx, &options, 1);
        if (status != 0) {
          fprintf(stderr,
                  "[HARNESS] FATAL_INIT init_java_env status=%d "
                  "(classpath ./base.apk in cwd?)\n", status);
          fflush(stderr);
          fatal_init("init_java_env");
        }
      }
      if (check_and_clear_jni_exception("init_java_env")) {
        fatal_init("init_java_env_exception");
      }

      set_stage("resolve_target_functions");
      





      onLoadFunctionPtr0 = (jni_onload_fn)((uintptr_t)targetLibBase + 0x11532C);
      createInstanceFnPtr0 =
          (create_instance_fn)((uintptr_t)targetLibBase + 0x1153CC);
      targetFunctionPtr0 = (function_0)((uintptr_t)targetLibBase + 0x1160B0);

      set_stage("prereq_jni_onload");
      



      jint onload_ver = onLoadFunctionPtr0(ctx.vm, NULL);
      if (onload_ver != 0x10006) {
        fprintf(stderr,
                "[HARNESS] FATAL_INIT JNI_OnLoad ret=0x%x expected 0x10006\n",
                (unsigned)onload_ver);
        fflush(stderr);
        fatal_init("jni_onload_version");
      }
      {
        uintptr_t slotAddr = *(uintptr_t*)((uintptr_t)targetLibBase + 0x617DC0);
        if (slotAddr == 0) {
          fprintf(stderr,
                  "[HARNESS] FATAL_INIT javaVM GOT slot (base+0x617DC0) NULL\n");
          fflush(stderr);
          fatal_init("java_vm_slot_null");
        }
        void* storedVm = *(void**)slotAddr;
        if (storedVm != (void*)ctx.vm) {
          fprintf(stderr,
                  "[HARNESS] FATAL_INIT javaVM global %p != ctx.vm %p\n",
                  storedVm, (void*)ctx.vm);
          fflush(stderr);
          fatal_init("java_vm_global_mismatch");
        }
        fprintf(stderr,
                "[HARNESS] wiring ok: JNI_OnLoad=0x10006 javaVM global=%p\n",
                storedVm);
        fflush(stderr);
      }
      if (check_and_clear_jni_exception("prereq_jni_onload")) {
        fatal_init("jni_onload_exception");
      }
      s_wired = 1;
    }
  }

  set_stage("input_parsing");
  
  



















  if (length < HEADER_LEN) {
    fprintf(stderr,
            "[HARNESS] FAIL_CONSTRUCTION len=%zu (<19B fixed header), "
            "skip target\n", length);
    fflush(stderr);
    goto release;
  }
  {
    static uint8_t pixels[PIXEL_BUF_LEN];
    unsigned int dimsIdx = (unsigned int)buffer[0] % 5u;
    unsigned int patIdx = (unsigned int)buffer[1] % 5u;
    jint bw = kDimsTable[dimsIdx][0];
    jint bh = kDimsTable[dimsIdx][1];
    jint cx, cy, cw, ch;
    unsigned int cropMode = (unsigned int)buffer[18];
    size_t pixelLen = (size_t)bw * (size_t)bh * 4u;
    size_t tailAvail = length - HEADER_LEN;
    size_t overlayLen = (tailAvail < pixelLen) ? tailAvail : pixelLen;

    memcpy(&cx, buffer + 2, sizeof(cx));  
    memcpy(&cy, buffer + 6, sizeof(cy));
    memcpy(&cw, buffer + 10, sizeof(cw));
    memcpy(&ch, buffer + 14, sizeof(ch));
    if (cropMode == 1u) {
      

      cx = 0; cy = 0; cw = bw; ch = bh;
    } else {
      
      cropMode = 0u;
    }

    fill_pixels(bw, bh, patIdx, pixels);
    if (overlayLen > 0) {
      memcpy(pixels, buffer + HEADER_LEN, overlayLen);
    }

    set_stage("get_obj_receiver");
    
    







    thiz = construct_NativeSdk(ctx.env, NULL, 0, &recipe_offset);
    if (check_and_clear_jni_exception("receiver_construct") || thiz == NULL) {
      fprintf(stderr,
              "[HARNESS] FAIL_CONSTRUCTION construct_NativeSdk (offset=%zu), "
              "skip target\n", recipe_offset);
      fflush(stderr);
      thiz = NULL;
      goto release;
    }
    if (recipe_offset != 0) {
      


      fprintf(stderr,
              "[HARNESS] FAIL_CONSTRUCTION receiver consumed %zu fuzz "
              "bytes, skip target\n", recipe_offset);
      fflush(stderr);
      goto release;
    }

    set_stage("get_obj_formats");
    

    {
      static const jint kFormats[5] = {11, 1, 4, 7, 14};
      formats = (*ctx.env)->NewIntArray(ctx.env, 5);
      if (check_and_clear_jni_exception("formats_NewIntArray") ||
          formats == NULL) {
        fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION NewIntArray(5)\n");
        fflush(stderr);
        formats = NULL;
        goto release;
      }
      (*ctx.env)->SetIntArrayRegion(ctx.env, formats, 0, 5, kFormats);
      if (check_and_clear_jni_exception("formats_SetIntArrayRegion")) {
        goto release;
      }
    }

    set_stage("prereq_create_handle");
    








    handle = createInstanceFnPtr0(ctx.env, thiz, formats);
    {
      int hadExc = check_and_clear_jni_exception("prereq_create_handle");
      if (handle == 0) {
        fprintf(stderr,
                "[HARNESS] FAIL_CONSTRUCTION createInstance handle=0 "
                "pending_exception=%d, skip target\n", hadExc);
        fflush(stderr);
        goto release;
      }
      fprintf(stderr, "[HARNESS] handle=0x%llx pending_exception=%d\n",
              (unsigned long long)handle, hadExc);
      fflush(stderr);
    }

    
    if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
      memcpy(__omen_bitmap_snapshot, (void*)*__omen_afl_area_ptr, 65536);
    }

    set_stage("get_obj_bitmap");
    
    bitmap = construct_android_graphics_Bitmap(ctx.env, bw, bh, pixels,
                                               pixelLen);
    if (check_and_clear_jni_exception("bitmap_construct") || bitmap == NULL) {
      fprintf(stderr,
              "[HARNESS] FAIL_CONSTRUCTION construct_android_graphics_Bitmap"
              "(%d,%d), skip target\n", (int)bw, (int)bh);
      fflush(stderr);
      bitmap = NULL;
      goto release;
    }

    set_stage("get_obj_out_array");
    


    objCls = (*ctx.env)->FindClass(ctx.env, "java/lang/Object");
    if (check_and_clear_jni_exception("out_array_FindClass") ||
        objCls == NULL) {
      fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION FindClass(Object)\n");
      fflush(stderr);
      objCls = NULL;
      goto release;
    }
    outArr = (*ctx.env)->NewObjectArray(ctx.env, 2, objCls, NULL);
    if (check_and_clear_jni_exception("out_array_NewObjectArray") ||
        outArr == NULL) {
      fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION NewObjectArray(2)\n");
      fflush(stderr);
      outArr = NULL;
      goto release;
    }

    set_stage("call_jni_target");
    













    {
      jint ret = targetFunctionPtr0(ctx.env, thiz, handle, bitmap, cx, cy,
                                    cw, ch, outArr);
      int hadExc = check_and_clear_jni_exception("call_jni_target");
      const char* k0; char t0[64];
      const char* k1; char t1[16];
      jobject e0 = (*ctx.env)->GetObjectArrayElement(ctx.env, outArr, 0);
      jobject e1 = (*ctx.env)->GetObjectArrayElement(ctx.env, outArr, 1);
      describe_element(e0, &k0, t0, sizeof(t0));
      describe_element(e1, &k1, t1, sizeof(t1));
      fprintf(stderr,
              "[HARNESS] target ret=%d pending_exception=%d "
              "out0=%s out1=%s\n", (int)ret, hadExc, k0, k1);
      if (t0[0] != '\0') {
        fprintf(stderr, "[HARNESS] out0 text=\"%s\"\n", t0);
      }
      fflush(stderr);
      if (e0 != NULL) {
        (*ctx.env)->DeleteLocalRef(ctx.env, e0);
      }
      if (e1 != NULL) {
        (*ctx.env)->DeleteLocalRef(ctx.env, e1);
      }
    }
  }

release:
  set_stage("cleanup");
  






  if (outArr != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, outArr);
  }
  if (objCls != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, objCls);
  }
  if (bitmap != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, bitmap);
  }
  if (formats != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, formats);
  }
  if (thiz != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, thiz);
  }
}

int main(void) {
  uint8_t buffer[BUFFER_SIZE];
  ssize_t rlength;

  install_signal_handlers();
  g_trace_stages = (getenv("OMEN_TRACE_STAGES") != NULL &&
    strcmp(getenv("OMEN_TRACE_STAGES"), "0") != 0);

  set_stage("read_stdin");
  rlength = fread(buffer, 1, BUFFER_SIZE, stdin);
  if (rlength == -1) {
    return errno;
  }

  

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
