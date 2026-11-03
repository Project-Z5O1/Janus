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


#define ARG0_MAX_ELEMS 1022u












void** __omen_afl_area_ptr = NULL;
char   __omen_bitmap_snapshot[65536] __attribute__((aligned(4096)));













typedef jint (*jni_onload_fn)(JavaVM*, void*);
typedef jlong (*function_0)(JNIEnv*, jobject, jintArray);





jobject construct_NativeSdk(JNIEnv* env,
                            const unsigned char* buffer,
                            size_t length,
                            size_t* offset);


static JavaCTX ctx;



jni_onload_fn onLoadFunctionPtr0;  
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






static void fatal_init(const char* why) {
  safe_write_string("[HARNESS] FATAL_INIT ");
  safe_write_string(why);
  safe_write_string("\n");
  _exit(61);
}




void fuzz_one_input(const uint8_t* buffer, size_t length) {
  









  jobject thiz = NULL;
  jintArray arg0 = NULL;
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
      targetFunctionPtr0 = (function_0)((uintptr_t)targetLibBase + 0x1153CC);

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

      


      if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
        memcpy(__omen_bitmap_snapshot, (void*)*__omen_afl_area_ptr, 65536);
      }
    }
  }

  set_stage("input_parsing");
  
  












  if (length < 1) {
    fprintf(stderr,
            "[HARNESS] FAIL_CONSTRUCTION len=%zu (<1B mode), skip target\n",
            length);
    fflush(stderr);
    goto release;
  }
  {
    uint8_t mode = buffer[0];
    if (mode == 1u) {
      unsigned int countRaw = 0;
      unsigned int countEff = 0;
      size_t need;
      if (length < 5) {
        fprintf(stderr,
                "[HARNESS] FAIL_CONSTRUCTION len=%zu mode=1 (<5B header), "
                "skip target\n", length);
        fflush(stderr);
        goto release;
      }
      memcpy(&countRaw, buffer + 1, sizeof(countRaw)); 
      countEff = (countRaw < ARG0_MAX_ELEMS) ? countRaw : ARG0_MAX_ELEMS;
      need = 5u + (size_t)countEff * 4u;
      if (length < need) {
        fprintf(stderr,
                "[HARNESS] FAIL_CONSTRUCTION len=%zu < payload %zu "
                "(count=%u count_eff=%u), skip target\n",
                length, need, countRaw, countEff);
        fflush(stderr);
        goto release;
      }
      set_stage("get_obj_construct_arg0");
      arg0 = (*ctx.env)->NewIntArray(ctx.env, (jsize)countEff);
      if (check_and_clear_jni_exception("arg0_NewIntArray") || arg0 == NULL) {
        fprintf(stderr,
                "[HARNESS] FAIL_CONSTRUCTION NewIntArray(n=%u), skip target\n",
                countEff);
        fflush(stderr);
        arg0 = NULL;
        goto release;
      }
      if (countEff > 0) {
        (*ctx.env)->SetIntArrayRegion(ctx.env, arg0, 0, (jsize)countEff,
                                      (const jint*)(buffer + 5));
        if (check_and_clear_jni_exception("arg0_SetIntArrayRegion")) {
          goto release;
        }
      }
      fprintf(stderr,
              "[HARNESS] arg0 jintArray n=%u (count_raw=%u)\n",
              countEff, countRaw);
      fflush(stderr);
    } else {
      


      fprintf(stderr, "[HARNESS] arg0 NULL array path (mode=%u)\n", mode);
      fflush(stderr);
    }
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
            "[HARNESS] FAIL_CONSTRUCTION receiver consumed %zu fuzz bytes, "
            "skip target\n", recipe_offset);
    fflush(stderr);
    goto release;
  }

  set_stage("call_jni_target");
  










  handle = targetFunctionPtr0(ctx.env, thiz, arg0);
  {
    int hadExc = check_and_clear_jni_exception("call_jni_target");
    fprintf(stderr,
            "[HARNESS] target handle=0x%llx pending_exception=%d\n",
            (unsigned long long)handle, hadExc);
    fflush(stderr);
  }

release:
  set_stage("cleanup");
  




  if (arg0 != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, arg0);
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
