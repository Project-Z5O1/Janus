#include <dlfcn.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "jenv.h"





#define BUFFER_SIZE 2417







void** __omen_afl_area_ptr = NULL;
char   __omen_bitmap_snapshot[65536] __attribute__((aligned(4096)));








typedef jbyteArray (*function_0)(JNIEnv*, jclass, jbyteArray, jbyteArray,
                                 jbyteArray, jbyteArray, jint, jbyteArray,
                                 jbyteArray, jint);


static JavaCTX ctx;


function_0 targetFunctionPtr0;







#define MAX_HASH 64u
#define MAX_PKG 128u
#define MAX_IV 32u
#define MAX_DATA 1024u
#define MAX_WBID 1024u
#define MAX_KEYID 128u





static const uint8_t WB_ID_CONST_1[] = "438501652102689867";
static const uint8_t WB_ID_CONST_2[] = "473291779446203616";

typedef struct {
  const uint8_t* p;
  size_t len;
  size_t off;
} omen_cursor;

static int omen_take(omen_cursor* c, size_t n, const uint8_t** out) {
  if (n > c->len - c->off) return -1; 
  *out = c->p + c->off;
  c->off += n;
  return 0;
}

static int omen_take_u8(omen_cursor* c, uint8_t* v) {
  const uint8_t* b;
  if (omen_take(c, 1, &b)) return -1;
  *v = b[0];
  return 0;
}

static int omen_take_u16(omen_cursor* c, uint16_t* v) {
  const uint8_t* b;
  if (omen_take(c, 2, &b)) return -1;
  *v = (uint16_t)(b[0] | ((uint16_t)b[1] << 8));
  return 0;
}

static int omen_take_u32(omen_cursor* c, uint32_t* v) {
  const uint8_t* b;
  if (omen_take(c, 4, &b)) return -1;
  *v = (uint32_t)b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16) |
       ((uint32_t)b[3] << 24);
  return 0;
}


static jbyteArray omen_new_array(JNIEnv* env, const uint8_t* bytes, size_t n) {
  jbyteArray arr = (*env)->NewByteArray(env, (jsize)n);
  if (arr == NULL) return NULL;
  if (n > 0) {
    (*env)->SetByteArrayRegion(env, arr, 0, (jsize)n, (const jbyte*)bytes);
  }
  return arr;
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
  set_stage("input_parsing");
  
  jbyteArray jHash = NULL, jPkg = NULL, jIv = NULL, jData = NULL;
  jbyteArray jWbId = NULL, jKeyId = NULL, jResult = NULL;
  const uint8_t* hash_b;
  const uint8_t* pkg_b;
  const uint8_t* iv_b;
  const uint8_t* data_b;
  const uint8_t* wbid_b;
  const uint8_t* keyid_b;
  uint16_t hash_len, pkg_len, iv_len, wbid_len, keyid_len;
  uint32_t data_len;
  uint8_t padding_raw, wb_id_mode, version_raw;
  jint padding;
  omen_cursor cur;

  cur.p = buffer;
  cur.len = length;
  cur.off = 0;
  if (omen_take_u16(&cur, &hash_len) != 0 || hash_len > MAX_HASH ||
      omen_take(&cur, hash_len, &hash_b) != 0 ||
      omen_take_u16(&cur, &pkg_len) != 0 || pkg_len > MAX_PKG ||
      omen_take(&cur, pkg_len, &pkg_b) != 0 ||
      omen_take_u16(&cur, &iv_len) != 0 || iv_len > MAX_IV ||
      omen_take(&cur, iv_len, &iv_b) != 0 ||
      omen_take_u32(&cur, &data_len) != 0 || data_len > MAX_DATA ||
      omen_take(&cur, data_len, &data_b) != 0 ||
      omen_take_u8(&cur, &padding_raw) != 0 ||
      omen_take_u8(&cur, &wb_id_mode) != 0 ||
      omen_take_u16(&cur, &wbid_len) != 0 || wbid_len > MAX_WBID ||
      omen_take(&cur, wbid_len, &wbid_b) != 0 ||
      omen_take_u16(&cur, &keyid_len) != 0 || keyid_len > MAX_KEYID ||
      omen_take(&cur, keyid_len, &keyid_b) != 0 ||
      omen_take_u8(&cur, &version_raw) != 0) {
    



    if (g_trace_stages) {
      fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION layout_exhausted consumed=%zu total=%zu\n",
              cur.off, length);
    }
    return;
  }
  

  padding = (jint)padding_raw; 

  jHash = omen_new_array(ctx.env, hash_b, hash_len);
  jPkg = omen_new_array(ctx.env, pkg_b, pkg_len);
  jIv = omen_new_array(ctx.env, iv_b, iv_len);
  jData = omen_new_array(ctx.env, data_b, data_len);
  

  if (wb_id_mode == 0) {
    jWbId = omen_new_array(ctx.env, WB_ID_CONST_1, sizeof(WB_ID_CONST_1));
  } else if (wb_id_mode == 1) {
    jWbId = omen_new_array(ctx.env, WB_ID_CONST_2, sizeof(WB_ID_CONST_2));
  } else {
    jWbId = omen_new_array(ctx.env, wbid_b, wbid_len);
  }
  jKeyId = omen_new_array(ctx.env, keyid_b, keyid_len);
  if (jHash == NULL || jPkg == NULL || jIv == NULL || jData == NULL ||
      jWbId == NULL || jKeyId == NULL) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION NewByteArray_failed\n");
    goto release;
  }
  if (check_and_clear_jni_exception("input_parsing")) {
    goto release;
  }

  set_stage("call_target");
  
  



  jResult = targetFunctionPtr0(ctx.env, NULL, jHash, jPkg, jIv, jData,
                               padding, jWbId, jKeyId, (jint)1);
  if (check_and_clear_jni_exception("call_target")) {
    goto release;
  }
  if (g_trace_stages) {
    if (jResult == NULL) {
      fprintf(stderr, "[HARNESS] target_result=NULL\n");
    } else {
      jsize out_len = (*ctx.env)->GetArrayLength(ctx.env, jResult);
      fprintf(stderr, "[HARNESS] target_result=byte[] len=%d\n", out_len);
    }
  }

release:
  set_stage("cleanup");
  
  
  
  if (jResult != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jResult);
  }
  if (jKeyId != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jKeyId);
  }
  if (jWbId != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jWbId);
  }
  if (jData != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jData);
  }
  if (jIv != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jIv);
  }
  if (jPkg != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jPkg);
  }
  if (jHash != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jHash);
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
  
  
  void* targetLibHandle = dlopen("libwbkit-seckit3.so", RTLD_NOW);
  if (targetLibHandle == NULL) {
    fprintf(stderr, "[HARNESS] Failed to dlopen libwbkit-seckit3.so: %s\n", dlerror());
    return -1;
  }
  void* targetLibBase = get_library_base("libwbkit-seckit3.so");
  if (targetLibBase == NULL) {
    fprintf(stderr, "[HARNESS] Failed to get library base for libwbkit-seckit3.so\n");
    return -1;
  }
  (void)targetLibHandle;
  


  targetFunctionPtr0 = (function_0)((uintptr_t)targetLibBase + 0x7BE4);

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
