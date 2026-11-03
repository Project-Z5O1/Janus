#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h> 
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

















typedef jlong (*factory_0)(JNIEnv*, jobject, jint, jint, jint);
typedef jint (*beginwrite_0)(JNIEnv*, jobject, jlong);
typedef jint (*function_0)(JNIEnv*, jobject, jlong, jbyteArray, jbyteArray, jint);


static JavaCTX ctx;


factory_0 factoryFunctionPtr0;     
beginwrite_0 beginWriteFunctionPtr0; 
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

































































#define OMEN_NAME_MAX 4096
#define OMEN_DATA_MAX 8192
#define STAGE_PATH \
  "/data/local/tmp/omen/Java_com_oplus_gallery_foundation_fileaccess_" \
  "extension_FileExtendedContainer_nativeAddExtensionData/stage.bin"

static int omen_need(size_t length, size_t off, size_t n) {
  return off <= length && n <= length - off;
}

static uint16_t omen_rd_u16(const uint8_t* p) {
  uint16_t v;
  memcpy(&v, p, sizeof(v));
  return v;
}




static jbyteArray omen_make_bytes(const uint8_t* data, uint16_t n,
                                  int* failed, const char* what) {
  jbyteArray a = (*ctx.env)->NewByteArray(ctx.env, (jsize)n);
  if (a == NULL) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION reason=new_byte_array_%s\n",
            what);
    *failed = 1;
    return NULL;
  }
  if (n > 0) {
    (*ctx.env)->SetByteArrayRegion(ctx.env, a, 0, (jsize)n,
                                   (const jbyte*)data);
  }
  if (check_and_clear_jni_exception("input_parsing") != 0) {
    (*ctx.env)->DeleteLocalRef(ctx.env, a); 
    *failed = 1;
    return NULL;
  }
  return a;
}




void fuzz_one_input(const uint8_t* buffer, size_t length) {
  size_t off = 0;
  size_t payload_len;
  uint16_t name_len = 0;
  uint16_t data_len = 0;
  int32_t length_channel = 0;
  jbyteArray jName = NULL;
  jbyteArray jData = NULL;
  jbyteArray jEmpty = NULL; 
  jint stage_fd = -1;
  jlong handle = 0;
  jint delete_rc = 0;
  jint add_rc = 0;
  FILE* stage_file;
  int failed = 0;

  set_stage("input_parsing");
  
  if (!omen_need(length, off, 2)) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION reason=name_len_prefix "
                    "len=%zu\n", length);
    failed = 1;
  } else {
    name_len = omen_rd_u16(buffer + off);
    off += 2;
    if (name_len > OMEN_NAME_MAX) {
      fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION reason=name_len_over_max "
                      "name_len=%u\n", (unsigned)name_len);
      failed = 1;
    } else if (!omen_need(length, off, (size_t)name_len)) {
      fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION reason=name_payload_short "
                      "name_len=%u len=%zu\n", (unsigned)name_len, length);
      failed = 1;
    }
  }
  if (failed == 0) {
    jName = omen_make_bytes(buffer + off, name_len, &failed, "name");
    off += (size_t)name_len;
  }
  if (failed == 0) {
    if (!omen_need(length, off, 2)) {
      fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION reason=data_len_prefix "
                      "len=%zu\n", length);
      failed = 1;
    } else {
      data_len = omen_rd_u16(buffer + off);
      off += 2;
      if (data_len > OMEN_DATA_MAX) {
        fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION reason=data_len_over_max "
                        "data_len=%u\n", (unsigned)data_len);
        failed = 1;
      } else if (!omen_need(length, off, (size_t)data_len)) {
        fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION reason=data_payload_short "
                        "data_len=%u len=%zu\n", (unsigned)data_len, length);
        failed = 1;
      }
    }
  }
  if (failed == 0) {
    jData = omen_make_bytes(buffer + off, data_len, &failed, "data");
    off += (size_t)data_len;
  }
  if (failed == 0) {
    if (!omen_need(length, off, 4)) {
      fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION reason=length_channel_short "
                      "len=%zu\n", length);
      failed = 1;
    } else {
      

      memcpy(&length_channel, buffer + off, sizeof(length_channel));
      off += 4;
    }
  }
  if (failed == 0) {
    

    jEmpty = omen_make_bytes(buffer, 0, &failed, "empty");
  }
  if (failed != 0) {
    goto cleanup; 
  }

  set_stage("prereq_open_stage_file");
  
  
  
  
  
  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy((void*)*__omen_afl_area_ptr, __omen_bitmap_snapshot, 65536);
  }
  stage_file = fopen(STAGE_PATH, "wb");
  if (stage_file == NULL) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION reason=stage_write_open\n");
    failed = 1;
  } else {
    payload_len = length - off;
    if (payload_len > 0 &&
        fwrite(buffer + off, 1, payload_len, stage_file) != payload_len) {
      fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION reason=stage_write "
                      "want=%zu\n", payload_len);
      failed = 1;
    }
    if (fclose(stage_file) != 0 && failed == 0) {
      fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION reason=stage_close\n");
      failed = 1;
    }
  }
  if (failed == 0) {
    stage_fd = open(STAGE_PATH, O_RDWR);
    if (stage_fd < 0) {
      fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION reason=stage_open\n");
      failed = 1;
    }
  }

  if (failed == 0) {
    set_stage("prereq_factory");
    
    
    
    
    
    
    
    
    handle = factoryFunctionPtr0(ctx.env, NULL, stage_fd,
                                 1 ,
                                 1 );
    if (check_and_clear_jni_exception("prereq_factory") != 0) {
      failed = 1;
    } else if (handle == 0) {
      fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION reason=factory_null_handle\n");
      failed = 1;
    } else {
      fprintf(stderr, "[HARNESS] factory handle=0x%llx payload=%zu\n",
              (unsigned long long)handle, (size_t)(length - off));
    }
  }

  if (failed == 0) {
    set_stage("prereq_beginwrite");
    
    
    
    
    
    
    
    
    jint bw = beginWriteFunctionPtr0(ctx.env, NULL, handle);
    if (check_and_clear_jni_exception("prereq_beginwrite") != 0) {
      failed = 1;
    } else if (bw != 0) {
      fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION reason=beginwrite_rc_%d\n",
              (int)bw);
      failed = 1;
    } else {
      fprintf(stderr, "[HARNESS] beginwrite rc=0 (Writer* at +0x18)\n");
    }
    
    
    
  }

  



  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy(__omen_bitmap_snapshot, (void*)*__omen_afl_area_ptr, 65536);
  }

  if (failed == 0) {
    set_stage("call_target_delete");
    
    
    
    
    
    
    delete_rc = targetFunctionPtr0(ctx.env, NULL, handle, jName, jEmpty, 0);
    if (check_and_clear_jni_exception("call_target_delete") != 0) {
      
    }
  }

  if (failed == 0) {
    set_stage("call_target_add");
    
    
    
    
    
    
    add_rc = targetFunctionPtr0(ctx.env, NULL, handle, jName, jData,
                                length_channel);
    if (check_and_clear_jni_exception("call_target_add") != 0) {
      
    }
    fprintf(stderr, "[HARNESS] add_result delete_rc=%d add_rc=%d name_len=%u "
                    "data_len=%u length=%d payload=%zu\n", (int)delete_rc,
            (int)add_rc, (unsigned)name_len, (unsigned)data_len,
            (int)length_channel, (size_t)(length - off));
  }

  set_stage("cleanup");
cleanup:
  
  
  
  
  
  
  
  
  
  
  if (stage_fd >= 0) {
    close(stage_fd);
  }
  
  if (jEmpty != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jEmpty);
  }
  if (jData != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jData);
  }
  if (jName != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jName);
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
  
  
  
  
  
  
  
  void* targetLibHandle = dlopen("libgallery_fileextender.so", RTLD_NOW);
  if (targetLibHandle == NULL) {
    fprintf(stderr, "[HARNESS] Failed to dlopen libgallery_fileextender.so: %s\n",
            dlerror());
    return -1;
  }
  void* targetLibBase = get_library_base("libgallery_fileextender.so");
  if (targetLibBase == NULL) {
    fprintf(stderr,
            "[HARNESS] Failed to get library base for libgallery_fileextender.so\n");
    return -1;
  }
  (void)targetLibHandle;
  




  factoryFunctionPtr0 = (factory_0)((uintptr_t)targetLibBase + 0x66ECC);
  beginWriteFunctionPtr0 = (beginwrite_0)((uintptr_t)targetLibBase + 0x675DC);
  targetFunctionPtr0 = (function_0)((uintptr_t)targetLibBase + 0x6774C);

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
