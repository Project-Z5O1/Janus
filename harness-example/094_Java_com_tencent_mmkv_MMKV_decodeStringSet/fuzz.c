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






















typedef void (*function_prereq_1)(JNIEnv*, jclass, jstring, jstring, jint,
                                  jboolean);
typedef jlong (*function_prereq_2)(JNIEnv*, jclass, jstring, jint, jstring,
                                   jstring, jlong);
typedef jobjectArray (*function_0)(JNIEnv*, jobject, jlong, jstring);


static JavaCTX ctx;



function_prereq_1 prereqFunctionPtr1;
function_prereq_2 prereqFunctionPtr2;
function_0 targetFunctionPtr0;





#define OMEN_MMKV_ROOT \
  "/data/local/tmp/omen/Java_com_tencent_mmkv_MMKV_decodeStringSet/mmkv_root"
#define OMEN_MMKV_CACHE \
  "/data/local/tmp/omen/Java_com_tencent_mmkv_MMKV_decodeStringSet/cache"


#define OMEN_META_SIZE 4096
#define OMEN_PATH_MAX 512
#define OMEN_MMAP_ID_MAX 32







#define OMEN_TARGET_KEY "omen_key_gamma"



#define OMEN_KEY_LOG_MAX 16





static size_t g_mmkv_instance_seq = 0;


static int g_trace_keys = -1;




static int check_and_clear_jni_exception(const char* stage);



#include <sys/stat.h>











static uint32_t crc32_buf(const uint8_t* buf, size_t len) {
  static uint32_t table[256];
  static int table_ready = 0;
  uint32_t crc = 0xFFFFFFFFu;
  size_t i;
  if (!table_ready) {
    uint32_t c;
    int n, k;
    for (n = 0; n < 256; n++) {
      c = (uint32_t)n;
      for (k = 0; k < 8; k++) {
        c = (c & 1u) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
      }
      table[n] = c;
    }
    table_ready = 1;
  }
  for (i = 0; i < len; i++) {
    crc = table[(crc ^ buf[i]) & 0xFFu] ^ (crc >> 8);
  }
  return crc ^ 0xFFFFFFFFu;
}




static int write_file_exact(const char* path, const uint8_t* data, size_t len) {
  FILE* f = fopen(path, "wb");
  if (f == NULL) {
    fprintf(stderr, "[HARNESS] staging: fopen(%s) failed: %s\n", path,
            strerror(errno));
    return -1;
  }
  if (len > 0 && fwrite(data, 1, len, f) != len) {
    fprintf(stderr, "[HARNESS] staging: fwrite(%s) failed: %s\n", path,
            strerror(errno));
    fclose(f);
    return -1;
  }
  if (fclose(f) != 0) {
    return -1;
  }
  return 0;
}











static int write_companion_crc(const char* crc_path, const uint8_t* store,
                               size_t store_len) {
  uint8_t meta[OMEN_META_SIZE];
  uint32_t header_actual, actual, crc;
  header_actual = 0;
  if (store_len >= 4) {
    header_actual = (uint32_t)store[0] | ((uint32_t)store[1] << 8) |
                    ((uint32_t)store[2] << 16) | ((uint32_t)store[3] << 24);
    actual = header_actual;
    if ((size_t)actual > store_len - 4) {
      actual = 0; 
    }
    crc = crc32_buf(store + 4, actual);
  } else {
    crc = 0; 
  }
  memset(meta, 0, sizeof(meta));
  meta[0] = (uint8_t)(crc & 0xFF);
  meta[1] = (uint8_t)((crc >> 8) & 0xFF);
  meta[2] = (uint8_t)((crc >> 16) & 0xFF);
  meta[3] = (uint8_t)((crc >> 24) & 0xFF);
  return write_file_exact(crc_path, meta, sizeof(meta));
}







static void log_decodestringset_result(JNIEnv* env, jobjectArray strings,
                                       jlong handle) {
  jsize count;
  jsize i;
  if (strings == NULL) {
    fprintf(stderr, "[HARNESS] decodeStringSet(handle=%#llx, key=%s) -> NULL "
                    "(documented null-return contract: miss/decode-fail/"
                    "gate)\n",
            (unsigned long long)handle, OMEN_TARGET_KEY);
    return;
  }
  count = (*env)->GetArrayLength(env, strings);
  fprintf(stderr, "[HARNESS] decodeStringSet(handle=%#llx, key=%s) -> "
                  "String[%d]\n",
          (unsigned long long)handle, OMEN_TARGET_KEY, count);
  if (g_trace_keys == 0) {
    return;
  }
  for (i = 0; i < count && i < OMEN_KEY_LOG_MAX; i++) {
    jstring element = (*env)->GetObjectArrayElement(env, strings, i);
    if (check_and_clear_jni_exception("target_result_element")) {
      break;
    }
    if (element != NULL) {
      const char* utf = (*env)->GetStringUTFChars(env, element, NULL);
      if (utf != NULL) {
        fprintf(stderr, "[HARNESS]   set[%d]=%s\n", i, utf);
        (*env)->ReleaseStringUTFChars(env, element, utf);
      }
      (*env)->DeleteLocalRef(env, element);
    }
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
  

















  jstring jRootDir = NULL;
  jstring jCachePath = NULL;
  jstring jMmapID = NULL;
  jstring jKey = NULL;
  jobjectArray set = NULL;
  char mmap_id[OMEN_MMAP_ID_MAX] = {0};
  char store_path[OMEN_PATH_MAX] = {0};
  char crc_path[OMEN_PATH_MAX] = {0};
  jlong handle = 0;

  if (g_trace_keys < 0) {
    g_trace_keys = (getenv("OMEN_TRACE_STAGES") != NULL &&
      strcmp(getenv("OMEN_TRACE_STAGES"), "0") != 0);
  }

  set_stage("input_parsing");
  
  




  












  if ((size_t)snprintf(mmap_id, sizeof(mmap_id), "omen_%010zu",
                       g_mmkv_instance_seq) >= sizeof(mmap_id) ||
      (size_t)snprintf(store_path, sizeof(store_path), "%s/%s", OMEN_MMKV_ROOT,
                       mmap_id) >= sizeof(store_path) ||
      (size_t)snprintf(crc_path, sizeof(crc_path), "%s.crc", store_path) >=
          sizeof(crc_path)) {
    fprintf(stderr, "[HARNESS] FAIL_STAGING: mmap-id/path construction "
                    "failed; skipping iteration\n");
    goto release;
  }
  g_mmkv_instance_seq++;
  if (write_file_exact(store_path, buffer, length) != 0 ||
      write_companion_crc(crc_path, buffer, length) != 0) {
    



    fprintf(stderr, "[HARNESS] FAIL_STAGING: %s not staged; skipping "
                    "iteration\n", store_path);
    goto release;
  }

  



  jRootDir = (*ctx.env)->NewStringUTF(ctx.env, OMEN_MMKV_ROOT);
  if (jRootDir == NULL ||
      check_and_clear_jni_exception("input_parsing_rootdir")) {
    goto release;
  }
  jCachePath = (*ctx.env)->NewStringUTF(ctx.env, OMEN_MMKV_CACHE);
  if (jCachePath == NULL ||
      check_and_clear_jni_exception("input_parsing_cachepath")) {
    goto release;
  }
  jMmapID = (*ctx.env)->NewStringUTF(ctx.env, mmap_id);
  if (jMmapID == NULL ||
      check_and_clear_jni_exception("input_parsing_mmapid")) {
    goto release;
  }
  jKey = (*ctx.env)->NewStringUTF(ctx.env, OMEN_TARGET_KEY);
  if (jKey == NULL ||
      check_and_clear_jni_exception("input_parsing_key")) {
    goto release;
  }

  set_stage("call_prereq_init");
  
  

  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy((void*)*__omen_afl_area_ptr, __omen_bitmap_snapshot, 65536);
  }

  





  prereqFunctionPtr1(ctx.env, NULL, jRootDir, jCachePath, 1, JNI_FALSE);
  if (check_and_clear_jni_exception("prereq_jniInitialize")) {
    goto release;
  }

  set_stage("call_prereq_factory");
  






  handle = prereqFunctionPtr2(ctx.env, NULL, jMmapID, 2, NULL, NULL, 0L);
  if (check_and_clear_jni_exception("prereq_getMMKVWithID")) {
    goto release;
  }
  fprintf(stderr, "[HARNESS] factory %s -> %#llx\n", mmap_id,
          (unsigned long long)handle);

  

  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy(__omen_bitmap_snapshot, (void*)*__omen_afl_area_ptr, 65536);
  }

  set_stage("call_target");
  







  set = targetFunctionPtr0(ctx.env, NULL, handle, jKey);
  if (check_and_clear_jni_exception("call_target")) {
    goto release;
  }
  log_decodestringset_result(ctx.env, set, handle);

release:
  set_stage("release");
  







  if (store_path[0] != '\0') {
    unlink(store_path);
  }
  if (crc_path[0] != '\0') {
    unlink(crc_path);
  }
  



  if (set != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, set);
    set = NULL;
  }
  if (jKey != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jKey);
    jKey = NULL;
  }
  if (jMmapID != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jMmapID);
    jMmapID = NULL;
  }
  if (jCachePath != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jCachePath);
    jCachePath = NULL;
  }
  if (jRootDir != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jRootDir);
    jRootDir = NULL;
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
  
  
  void* targetLibHandle = dlopen("libmmkv.so", RTLD_NOW);
  if (targetLibHandle == NULL) {
    fprintf(stderr, "[HARNESS] Failed to dlopen libmmkv.so: %s\n", dlerror());
    return -1;
  }
  void* targetLibBase = get_library_base("libmmkv.so");
  if (targetLibBase == NULL) {
    fprintf(stderr, "[HARNESS] Failed to get library base for libmmkv.so\n");
    return -1;
  }
  (void)targetLibHandle;
  






  prereqFunctionPtr1 =
      (function_prereq_1)((uintptr_t)targetLibBase + 0x37f2c);
  prereqFunctionPtr2 =
      (function_prereq_2)((uintptr_t)targetLibBase + 0x380f8);
  targetFunctionPtr0 = (function_0)((uintptr_t)targetLibBase + 0x39c6c);

  











  set_stage("ensure_staging_dirs");
  if (mkdir(OMEN_MMKV_ROOT, 0755) != 0 && errno != EEXIST) {
    fprintf(stderr, "[HARNESS] mkdir %s: %s (non-fatal; usability checked "
                    "by probe)\n", OMEN_MMKV_ROOT, strerror(errno));
  }
  if (mkdir(OMEN_MMKV_CACHE, 0755) != 0 && errno != EEXIST) {
    fprintf(stderr, "[HARNESS] mkdir %s: %s (non-fatal; usability checked "
                    "by probe)\n", OMEN_MMKV_CACHE, strerror(errno));
  }
  {
    char probe_path[OMEN_PATH_MAX];
    if ((size_t)snprintf(probe_path, sizeof(probe_path), "%s/.omen_probe",
                         OMEN_MMKV_ROOT) >= sizeof(probe_path) ||
        write_file_exact(probe_path, NULL, 0) != 0) {
      fprintf(stderr, "[HARNESS] staging dir %s unusable; aborting (no "
                      "fuzz surface possible)\n", OMEN_MMKV_ROOT);
      return -1;
    }
    unlink(probe_path);
  }

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
