#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "jenv.h"










#define BUFFER_SIZE 4128





void** __omen_afl_area_ptr = NULL;
char   __omen_bitmap_snapshot[65536] __attribute__((aligned(4096)));










typedef jint (*target_init_t)(JNIEnv*, jobject, jstring, jstring, jint,
                              jstring, jstring, jint);











#include "object_recipes/CLoganProtocol/construct.c"


static JavaCTX ctx;


target_init_t targetFunctionPtr0;

static const char* g_current_stage = "startup";
static int g_trace_stages = 0;
static int g_keep_workspace = 0;

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














#define OMEN_NS_PREFIX \
  "/data/local/tmp/omen/Java_com_heytap_log_core_CLoganProtocol_clogan_1init"
#define LAYOUT_HEADER 16
#define MAX_STRING 1024

static int workspace_path_ok(const char* path) {
  size_t length;
  size_t prefix_len = sizeof(OMEN_NS_PREFIX) - 1;

  if (path == NULL || path[0] == '\0') {
    return 0;
  }
  length = strlen(path);
  if (length > 2048) {
    return 0;
  }
  if (strstr(path, "..") != NULL) {
    return 0;
  }
  if (path[0] == '/') {
    if (strncmp(path, OMEN_NS_PREFIX, prefix_len) != 0) {
      return 0;
    }
    if (length <= prefix_len + 1) {
      return 0; 
    }
    return 1;
  }
  return 1; 
}

static int rm_recursive(const char* path, int depth) {
  struct stat st;
  DIR* dir;
  struct dirent* entry;
  int ok = 1;

  if (lstat(path, &st) != 0) {
    return 1; 
  }
  if (!S_ISDIR(st.st_mode)) {
    return unlink(path) == 0;
  }
  if (depth <= 0) {
    return 0;
  }
  dir = opendir(path);
  if (dir == NULL) {
    return 0;
  }
  while ((entry = readdir(dir)) != NULL) {
    char child[4096];
    int n;
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }
    n = snprintf(child, sizeof(child), "%s/%s", path, entry->d_name);
    if (n <= 0 || n >= (int)sizeof(child)) {
      ok = 0;
      continue;
    }
    if (!rm_recursive(child, depth - 1)) {
      ok = 0;
    }
  }
  closedir(dir);
  if (rmdir(path) != 0) {
    ok = 0;
  }
  return ok;
}

static void mkdir_parents(const char* path) {
  char tmp[4096];
  size_t i;
  size_t n = strlen(path);

  if (n == 0 || n >= sizeof(tmp)) {
    return;
  }
  memcpy(tmp, path, n + 1);
  for (i = 1; i < n; i++) {
    if (tmp[i] == '/') {
      tmp[i] = '\0';
      mkdir(tmp, 0755);
      tmp[i] = '/';
    }
  }
  mkdir(tmp, 0755);
}



static void stage_staged_file(const char* cache_dir, const uint8_t* image,
                              size_t image_len) {
  char path[4096];
  int n;
  FILE* file;
  size_t written;

  if (image_len == 0 || !workspace_path_ok(cache_dir)) {
    return;
  }
  n = snprintf(path, sizeof(path), "%s/nlog_cache/nlog.mmap2", cache_dir);
  if (n <= 0 || n >= (int)sizeof(path)) {
    fprintf(stderr, "[HARNESS] staging: path overflow; staged file not "
                    "written (target surface untouched)\n");
    return;
  }
  {
    
    char* last_slash = strrchr(path, '/');
    if (last_slash == NULL) {
      return;
    }
    *last_slash = '\0';
    mkdir_parents(path);
    *last_slash = '/';
  }
  file = fopen(path, "wb");
  if (file == NULL) {
    fprintf(stderr, "[HARNESS] staging: fopen %s failed (%s)\n", path,
            strerror(errno));
    return;
  }
  written = fwrite(image, 1, image_len, file);
  fclose(file);
  if (g_trace_stages) {
    fprintf(stderr, "[HARNESS] staged %zu/%zu bytes -> %s\n", written,
            image_len, path);
  }
}






static void clean_workspaces(const char* cache_dir, const char* log_dir) {
  char path[4096];
  int n;

  if (g_keep_workspace) {
    return;
  }
  if (workspace_path_ok(cache_dir)) {
    n = snprintf(path, sizeof(path), "%s/nlog_cache", cache_dir);
    if (n > 0 && n < (int)sizeof(path)) {
      rm_recursive(path, 16);
    }
  }
  if (workspace_path_ok(log_dir)) {
    rm_recursive(log_dir, 16);
  }
}




void fuzz_one_input(const uint8_t* buffer, size_t length) {
  uint16_t len0 = 0, len1 = 0, len2 = 0, len3 = 0;
  int32_t capacity = 0, mmap_length = 0;
  size_t declared = 0, slack = 0;
  char* cache_copy = NULL;
  char* log_copy = NULL;
  char* key_copy = NULL;
  char* iv_copy = NULL;
  jobject protocol = NULL;
  jstring jcache = NULL, jlog = NULL, jkey = NULL, jiv = NULL;
  jint target_rc = 0;

  set_stage("input_parsing");
  
  
  
  
  
  
  
  
  if (length < LAYOUT_HEADER) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION: shorter than 16B header "
                    "(%zu)\n", length);
    goto release_refs;
  }
  len0 = (uint16_t)(buffer[0] | ((uint16_t)buffer[1] << 8));
  len1 = (uint16_t)(buffer[2] | ((uint16_t)buffer[3] << 8));
  len2 = (uint16_t)(buffer[4] | ((uint16_t)buffer[5] << 8));
  len3 = (uint16_t)(buffer[6] | ((uint16_t)buffer[7] << 8));
  capacity = (int32_t)((uint32_t)buffer[8] | ((uint32_t)buffer[9] << 8) |
                       ((uint32_t)buffer[10] << 16) |
                       ((uint32_t)buffer[11] << 24));
  mmap_length = (int32_t)((uint32_t)buffer[12] | ((uint32_t)buffer[13] << 8) |
                          ((uint32_t)buffer[14] << 16) |
                          ((uint32_t)buffer[15] << 24));
  if (len0 > MAX_STRING || len1 > MAX_STRING || len2 > MAX_STRING ||
      len3 > MAX_STRING) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION: string length exceeds %dB "
                    "cap (%u/%u/%u/%u)\n", MAX_STRING, len0, len1, len2,
            len3);
    goto release_refs;
  }
  declared = (size_t)len0 + len1 + len2 + len3;
  if (LAYOUT_HEADER + declared > length) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION: declared lengths (%u+%u+"
                    "%u+%u) exceed buffer %zu\n", len0, len1, len2, len3,
            length);
    goto release_refs;
  }
  slack = length - LAYOUT_HEADER - declared;

  cache_copy = (char*)malloc((size_t)len0 + 1);
  log_copy = (char*)malloc((size_t)len1 + 1);
  key_copy = (char*)malloc((size_t)len2 + 1);
  iv_copy = (char*)malloc((size_t)len3 + 1);
  if (cache_copy == NULL || log_copy == NULL || key_copy == NULL ||
      iv_copy == NULL) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION: OOM working copies\n");
    goto release_refs;
  }
  memcpy(cache_copy, buffer + LAYOUT_HEADER, len0);
  cache_copy[len0] = '\0';
  memcpy(log_copy, buffer + LAYOUT_HEADER + len0, len1);
  log_copy[len1] = '\0';
  memcpy(key_copy, buffer + LAYOUT_HEADER + len0 + len1, len2);
  key_copy[len2] = '\0';
  memcpy(iv_copy, buffer + LAYOUT_HEADER + len0 + len1 + len2, len3);
  iv_copy[len3] = '\0';
  if (g_trace_stages) {
    fprintf(stderr, "[HARNESS] layout len=%zu len0=%u len1=%u len2=%u "
                    "len3=%u capacity=%d mmapLength=%d slack=%zu\n", length,
            len0, len1, len2, len3, capacity, mmap_length, slack);
  }

  set_stage("get_obj");
  
  
  
  
  
  
  stage_staged_file(cache_copy, buffer + LAYOUT_HEADER + declared, slack);

  
  
  
  
  protocol = construct_CLoganProtocol(ctx.env, buffer, length, NULL);
  if (check_and_clear_jni_exception("get_obj:construct_CLoganProtocol")) {
    goto release_refs;
  }
  if (protocol == NULL) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION: construct_CLoganProtocol "
                    "returned NULL (isCloganSuccess gate refused an instance "
                    "that could not reach libheytaplog.so)\n");
    goto release_refs;
  }

  
  
  
  
  jcache = (*ctx.env)->NewStringUTF(ctx.env, cache_copy);
  if (check_and_clear_jni_exception("get_obj:NewStringUTF(cacheDir)") ||
      jcache == NULL) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION: NewStringUTF(cacheDir)\n");
    goto release_refs;
  }
  jlog = (*ctx.env)->NewStringUTF(ctx.env, log_copy);
  if (check_and_clear_jni_exception("get_obj:NewStringUTF(logDir)") ||
      jlog == NULL) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION: NewStringUTF(logDir)\n");
    goto release_refs;
  }
  jkey = (*ctx.env)->NewStringUTF(ctx.env, key_copy);
  if (check_and_clear_jni_exception("get_obj:NewStringUTF(key)") ||
      jkey == NULL) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION: NewStringUTF(key)\n");
    goto release_refs;
  }
  jiv = (*ctx.env)->NewStringUTF(ctx.env, iv_copy);
  if (check_and_clear_jni_exception("get_obj:NewStringUTF(iv)") ||
      jiv == NULL) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION: NewStringUTF(iv)\n");
    goto release_refs;
  }

  set_stage("call_target");
  
  
  
  
  
  
  
  
  
  
  
  target_rc = targetFunctionPtr0(ctx.env, protocol, jcache, jlog,
                                 (jint)capacity, jkey, jiv,
                                 (jint)mmap_length);
  if (check_and_clear_jni_exception("call_target:clogan_1init")) {
    goto release_refs;
  }
  if (g_trace_stages) {
    
    
    jclass obs_cls = (*ctx.env)->GetObjectClass(ctx.env, protocol);
    jfieldID obs_fid = (obs_cls != NULL)
        ? (*ctx.env)->GetFieldID(ctx.env, obs_cls, "nativeConfigPointer", "J")
        : NULL;
    jlong obs_ptr = -1;
    if (obs_fid != NULL &&
        (*ctx.env)->ExceptionCheck(ctx.env) == JNI_FALSE) {
      obs_ptr = (*ctx.env)->GetLongField(ctx.env, protocol, obs_fid);
    }
    if ((*ctx.env)->ExceptionCheck(ctx.env)) {
      (*ctx.env)->ExceptionDescribe(ctx.env);
      (*ctx.env)->ExceptionClear(ctx.env);
      obs_ptr = -1;
    }
    {
      char obs_path[4096];
      int n = snprintf(obs_path, sizeof(obs_path),
                       "%s/nlog_cache/nlog.mmap2", cache_copy);
      long long mmap2_size = -1;
      int log_entries = -1;
      if (n > 0 && n < (int)sizeof(obs_path)) {
        struct stat st;
        if (stat(obs_path, &st) == 0) {
          mmap2_size = (long long)st.st_size;
        }
      }
      if (workspace_path_ok(log_copy)) {
        DIR* dir = opendir(log_copy);
        if (dir != NULL) {
          struct dirent* entry;
          log_entries = 0;
          while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 ||
                strcmp(entry->d_name, "..") == 0) {
              continue;
            }
            log_entries++;
          }
          closedir(dir);
        }
      }
      fprintf(stderr, "[HARNESS] obs target_rc=%d (0x%08x) "
                      "nativeConfigPointer=0x%llx staged_bytes=%zu "
                      "mmap2_size=%lld log_dir_entries=%d\n",
              (int)target_rc, (unsigned int)target_rc,
              (unsigned long long)obs_ptr, slack, mmap2_size, log_entries);
    }
    if (obs_cls != NULL) {
      (*ctx.env)->DeleteLocalRef(ctx.env, obs_cls);
    }
  }

release_refs:
  set_stage("cleanup");
  
  
  
  
  
  
  
  if (jiv != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jiv);
  }
  if (jkey != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jkey);
  }
  if (jlog != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jlog);
  }
  if (jcache != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jcache);
  }
  if (protocol != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, protocol);
  }
  construct_CLoganProtocol_shutdown(ctx.env);
  clean_workspaces(cache_copy, log_copy);
  free(iv_copy);
  free(key_copy);
  free(log_copy);
  free(cache_copy);
}

int main(void) {
  int status;
  uint8_t buffer[BUFFER_SIZE];
  
  
  
  
  
  
  char* options[2] = {
    "-Djava.class.path=./base.apk",
    "-Djava.library.path=./lib"
  };
  ssize_t rlength;

  install_signal_handlers();
  g_trace_stages = (getenv("OMEN_TRACE_STAGES") != NULL &&
    strcmp(getenv("OMEN_TRACE_STAGES"), "0") != 0);
  g_keep_workspace = (getenv("OMEN_KEEP_WORKSPACE") != NULL);

  set_stage("read_stdin");
  rlength = fread(buffer, 1, BUFFER_SIZE, stdin);
  if (rlength == -1) {
    return errno;
  }

  set_stage("init_java_env");
  if ((status = init_java_env(&ctx, options, 2)) != 0) {
    fprintf(stderr, "[HARNESS] init_java_env failed status=%d\n", status);
    return status;
  }

  if (check_and_clear_jni_exception("init_java_env")) {
    return JNI_ERR;
  }

  set_stage("resolve_target_functions");
  
  
  
  
  
  
  void* targetLibHandle = dlopen("libheytaplog.so", RTLD_NOW);
  if (targetLibHandle == NULL) {
    fprintf(stderr, "[HARNESS] Failed to dlopen libheytaplog.so: %s\n", dlerror());
    return -1;
  }
  void* targetLibBase = get_library_base("libheytaplog.so");
  if (targetLibBase == NULL) {
    fprintf(stderr, "[HARNESS] Failed to get library base for libheytaplog.so\n");
    return -1;
  }
  (void)targetLibHandle;
  
  
  targetFunctionPtr0 = (target_init_t)((uintptr_t)targetLibBase + 0x3f14);

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
