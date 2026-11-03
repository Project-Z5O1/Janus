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











typedef jint (*function_prereq_1)(JavaVM*);
typedef jobject (*function_prereq_2)(JNIEnv*, jclass);
typedef jobject (*function_0)(JNIEnv*, jclass, jobject, jobject, jobject);







typedef void (*function_release_close)(JNIEnv*, jobject);


static JavaCTX ctx;



function_prereq_1 prereqFunctionPtr1; 
function_prereq_2 prereqFunctionPtr2; 
function_0 targetFunctionPtr0;        
function_release_close releaseFunctionPtr0; 









static int g_prereq1_done = 0;
static int g_prereq2_done = 0;

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







jobject construct_DummyOpenArchiveCallback(JNIEnv* env,
                                           const uint8_t* buffer, size_t length, size_t* offset);
jobject construct_RandomAccessFileInStream(JNIEnv* env,
                                           const uint8_t* buffer, size_t length, size_t* offset,
                                           const char* staged_archive_path);




#define OMEN_STAGED_ARCHIVE_PATH "omen_input_archive.bin"




void fuzz_one_input(const uint8_t* buffer, size_t length) {
  





  jobject objCallback = NULL;
  jobject objStream = NULL;
  jobject objResult = NULL;
  size_t offset = 0;

  set_stage("input_parsing");
  
  



  objCallback = construct_DummyOpenArchiveCallback(ctx.env, buffer, length, &offset);
  if (objCallback == NULL) {
    fprintf(stderr, "[HARNESS] arg2 construction failed (DummyOpenArchiveCallback)\n");
    goto release;
  }
  objStream = construct_RandomAccessFileInStream(ctx.env, buffer, length, &offset,
                                                 OMEN_STAGED_ARCHIVE_PATH);
  if (objStream == NULL) {
    fprintf(stderr, "[HARNESS] arg1 construction failed (RandomAccessFileInStream)\n");
    goto release;
  }
  

  set_stage("get_obj_prereq");
  
  


  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy((void*)*__omen_afl_area_ptr, __omen_bitmap_snapshot, 65536);
  }

  




  if (g_prereq1_done == 0) {
    jint ver = prereqFunctionPtr1(ctx.vm);
    if (ver == 0) {
      fprintf(stderr, "[HARNESS] JNI_OnLoad returned 0\n");
      goto release;
    }
    if (check_and_clear_jni_exception("prereq1_JNI_OnLoad")) {
      goto release;
    }
    g_prereq1_done = 1;
  }

  





  if (g_prereq2_done == 0) {
    jobject initError = prereqFunctionPtr2(ctx.env, NULL);
    if (initError != NULL) {
      fprintf(stderr, "[HARNESS] nativeInitSevenZipLibrary returned an error string\n");
      check_and_clear_jni_exception("prereq2_nativeInitSevenZipLibrary");
      goto release;
    }
    if (check_and_clear_jni_exception("prereq2_nativeInitSevenZipLibrary")) {
      goto release;
    }
    g_prereq2_done = 1;
  }

  

  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy(__omen_bitmap_snapshot, (void*)*__omen_afl_area_ptr, 65536);
  }

  set_stage("call_jni");
  



  objResult = targetFunctionPtr0(ctx.env, NULL, NULL, objStream, objCallback);
  if (check_and_clear_jni_exception("call_jni_target")) {
    objResult = NULL; 
  } else if (objResult != NULL) {
    fprintf(stderr, "[HARNESS] nativeOpenArchive returned non-NULL IInArchive\n");
  } else {
    fprintf(stderr, "[HARNESS] nativeOpenArchive returned NULL (open error)\n");
  }

release:
  set_stage("release");
  
  
  
  
  
  
  
  if (objResult != NULL) {
    releaseFunctionPtr0(ctx.env, objResult);
    check_and_clear_jni_exception("release_result_nativeClose");
    (*ctx.env)->DeleteLocalRef(ctx.env, objResult);
  }
  if (objStream != NULL) {
    jclass clsStream = (*ctx.env)->GetObjectClass(ctx.env, objStream);
    if (clsStream != NULL) {
      jmethodID midClose = (*ctx.env)->GetMethodID(ctx.env, clsStream, "close", "()V");
      if (midClose != NULL) {
        (*ctx.env)->CallVoidMethod(ctx.env, objStream, midClose);
        check_and_clear_jni_exception("release_stream_close");
      } else {
        check_and_clear_jni_exception("release_stream_GetMethodID_close");
      }
      (*ctx.env)->DeleteLocalRef(ctx.env, clsStream);
    } else {
      check_and_clear_jni_exception("release_stream_GetObjectClass");
    }
    (*ctx.env)->DeleteLocalRef(ctx.env, objStream);
  }
  if (objCallback != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, objCallback);
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
  
  
  void* targetLibHandle = dlopen("lib7-Zip-JBinding.so", RTLD_NOW);
  if (targetLibHandle == NULL) {
    fprintf(stderr, "[HARNESS] Failed to dlopen lib7-Zip-JBinding.so: %s\n", dlerror());
    return -1;
  }
  void* targetLibBase = get_library_base("lib7-Zip-JBinding.so");
  if (targetLibBase == NULL) {
    fprintf(stderr, "[HARNESS] Failed to get library base for lib7-Zip-JBinding.so\n");
    return -1;
  }
  (void)targetLibHandle;
  

  prereqFunctionPtr1 = (function_prereq_1)((uintptr_t)targetLibBase + 0x11511c); 
  prereqFunctionPtr2 = (function_prereq_2)((uintptr_t)targetLibBase + 0x11adf0); 
  targetFunctionPtr0 = (function_0)((uintptr_t)targetLibBase + 0x11aeb4);        
  
  releaseFunctionPtr0 = (function_release_close)((uintptr_t)targetLibBase + 0x11d8c8);

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
