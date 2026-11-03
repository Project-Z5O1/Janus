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
typedef jlong (*function_0)(JNIEnv*, jclass, jstring, jint, jint, jstring);


static JavaCTX ctx;



function_prereq_1 prereqFunctionPtr1;
function_0 targetFunctionPtr0;








#define OMEN_MMKV_ROOT \
  "/data/local/tmp/omen/Java_com_tencent_mmkv_MMKV_getMMKVWithIDAndSize" \
  "/mmkv_root"
#define OMEN_MMKV_CACHE \
  "/data/local/tmp/omen/Java_com_tencent_mmkv_MMKV_getMMKVWithIDAndSize/cache"
#define OMEN_SPECIAL_DIR OMEN_MMKV_ROOT "/specialCharacter"
#define OMEN_PATH_MAX 640



#define OMEN_ID_LEN_CAP 512u
#define OMEN_KEY_LEN_CAP 512u
#define OMEN_KEY_NULL_FLAG 0x8000u






static size_t g_unique_seq = 0;






static int g_dic_probe = -1;
static int g_verify = -1;



static int check_and_clear_jni_exception(const char* stage);



#include <sys/stat.h>





typedef struct {
    uint32_t a, b, c, d;
    uint64_t len;
    uint8_t buf[64];
    size_t buflen;
} md5_ctx;

static uint32_t md5_f(uint32_t x, uint32_t y, uint32_t z) { return (x & y) | (~x & z); }
static uint32_t md5_g(uint32_t x, uint32_t y, uint32_t z) { return (x & z) | (y & ~z); }
static uint32_t md5_h(uint32_t x, uint32_t y, uint32_t z) { return x ^ y ^ z; }
static uint32_t md5_i(uint32_t x, uint32_t y, uint32_t z) { return y ^ (x | ~z); }
static uint32_t md5_rot(uint32_t v, int s) { return (v << s) | (v >> (32 - s)); }

static void md5_block(md5_ctx* ctx, const uint8_t* p) {
    static const uint32_t K[64] = {
        0xd76aa478u,0xe8c7b756u,0x242070dbu,0xc1bdceeeu,0xf57c0fafu,0x4787c62au,
        0xa8304613u,0xfd469501u,0x698098d8u,0x8b44f7afu,0xffff5bb1u,0x895cd7beu,
        0x6b901122u,0xfd987193u,0xa679438eu,0x49b40821u,0xf61e2562u,0xc040b340u,
        0x265e5a51u,0xe9b6c7aau,0xd62f105du,0x02441453u,0xd8a1e681u,0xe7d3fbc8u,
        0x21e1cde6u,0xc33707d6u,0xf4d50d87u,0x455a14edu,0xa9e3e905u,0xfcefa3f8u,
        0x676f02d9u,0x8d2a4c8au,0xfffa3942u,0x8771f681u,0x6d9d6122u,0xfde5380cu,
        0xa4beea44u,0x4bdecfa9u,0xf6bb4b60u,0xbebfbc70u,0x289b7ec6u,0xeaa127fau,
        0xd4ef3085u,0x04881d05u,0xd9d4d039u,0xe6db99e5u,0x1fa27cf8u,0xc4ac5665u,
        0xf4292244u,0x432aff97u,0xab9423a7u,0xfc93a039u,0x655b59c3u,0x8f0ccc92u,
        0xffeff47du,0x85845dd1u,0x6fa87e4fu,0xfe2ce6e0u,0xa3014314u,0x4e0811a1u,
        0xf7537e82u,0xbd3af235u,0x2ad7d2bbu,0xeb86d391u};
    static const int S[64] = {7,12,17,22,7,12,17,22,7,12,17,22,7,12,17,22,
                              5,9,14,20,5,9,14,20,5,9,14,20,5,9,14,20,
                              4,11,16,23,4,11,16,23,4,11,16,23,4,11,16,23,
                              6,10,15,21,6,10,15,21,6,10,15,21,6,10,15,21};
    uint32_t m[16], a = ctx->a, b = ctx->b, c = ctx->c, d = ctx->d;
    int i;
    for (i = 0; i < 16; i++) {
        m[i] = (uint32_t)p[i * 4] | ((uint32_t)p[i * 4 + 1] << 8) |
               ((uint32_t)p[i * 4 + 2] << 16) | ((uint32_t)p[i * 4 + 3] << 24);
    }
    for (i = 0; i < 64; i++) {
        uint32_t f;
        int g;
        if (i < 16)      { f = md5_f(b, c, d); g = i; }
        else if (i < 32) { f = md5_g(b, c, d); g = (5 * i + 1) & 15; }
        else if (i < 48) { f = md5_h(b, c, d); g = (3 * i + 5) & 15; }
        else             { f = md5_i(b, c, d); g = (7 * i) & 15; }
        f = f + a + K[i] + m[g];
        a = d;
        d = c;
        c = b;
        b = b + md5_rot(f, S[i]);
    }
    ctx->a += a; ctx->b += b; ctx->c += c; ctx->d += d;
}

static void md5_init(md5_ctx* ctx) {
    ctx->a = 0x67452301u; ctx->b = 0xefcdab89u;
    ctx->c = 0x98badcfeu; ctx->d = 0x10325476u;
    ctx->len = 0; ctx->buflen = 0;
}

static void md5_update(md5_ctx* ctx, const uint8_t* data, size_t len) {
    ctx->len += (uint64_t)len;
    while (len > 0) {
        size_t take = 64 - ctx->buflen;
        if (take > len) {
            take = len;
        }
        memcpy(ctx->buf + ctx->buflen, data, take);
        ctx->buflen += take;
        data += take;
        len -= take;
        if (ctx->buflen == 64) {
            md5_block(ctx, ctx->buf);
            ctx->buflen = 0;
        }
    }
}

static void md5_final(md5_ctx* ctx, uint8_t out[16]) {
    uint64_t bits = ctx->len * 8;
    uint8_t pad = 0x80;
    uint8_t lenb[8];
    int i;
    md5_update(ctx, &pad, 1);
    pad = 0;
    while (ctx->buflen != 56) {
        md5_update(ctx, &pad, 1);
    }
    for (i = 0; i < 8; i++) {
        lenb[i] = (uint8_t)(bits >> (8 * i));
    }
    md5_update(ctx, lenb, 8);
    for (i = 0; i < 4; i++) {
        out[i]      = (uint8_t)(ctx->a >> (8 * i));
        out[4 + i]  = (uint8_t)(ctx->b >> (8 * i));
        out[8 + i]  = (uint8_t)(ctx->c >> (8 * i));
        out[12 + i] = (uint8_t)(ctx->d >> (8 * i));
    }
}

static void md5_hex(const uint8_t* data, size_t len, char out[33]) {
    static const char hexdig[] = "0123456789abcdef";
    md5_ctx ctx;
    uint8_t digest[16];
    int i;
    md5_init(&ctx);
    md5_update(&ctx, data, len);
    md5_final(&ctx, digest);
    for (i = 0; i < 16; i++) {
        out[i * 2] = hexdig[digest[i] >> 4];
        out[i * 2 + 1] = hexdig[digest[i] & 0xF];
    }
    out[32] = '\0';
}






static int id_has_special_char(const uint8_t* id, size_t n) {
    static const char bad[] = "/\\:*?\"<>|";
    size_t i, j;
    for (i = 0; i < n; i++) {
        for (j = 0; j < sizeof(bad) - 1; j++) {
            if (id[i] == (uint8_t)bad[j]) {
                return 1;
            }
        }
    }
    return 0;
}




static int build_store_path(char* out, size_t outsz, const char* id,
                            size_t idlen) {
    if (id_has_special_char((const uint8_t*)id, idlen)) {
        char hex[33];
        md5_hex((const uint8_t*)id, idlen, hex);
        if ((size_t)snprintf(out, outsz, "%s/%s", OMEN_SPECIAL_DIR, hex) >=
            outsz) {
            return -1;
        }
        return 0;
    }
    
    if (idlen >= outsz - strlen(OMEN_MMKV_ROOT) - 2) {
        return -1;
    }
    memcpy(out, OMEN_MMKV_ROOT, strlen(OMEN_MMKV_ROOT));
    out[strlen(OMEN_MMKV_ROOT)] = '/';
    memcpy(out + strlen(OMEN_MMKV_ROOT) + 1, id, idlen);
    out[strlen(OMEN_MMKV_ROOT) + 1 + idlen] = '\0';
    return 0;
}





static int mkdirs(const char* path) {
    char tmp[OMEN_PATH_MAX];
    size_t i, len;
    if ((size_t)snprintf(tmp, sizeof(tmp), "%s", path) >= sizeof(tmp)) {
        return -1;
    }
    len = strlen(tmp);
    if (len == 0) return -1;
    if (tmp[len - 1] == '/') tmp[len - 1] = '\0';
    for (i = 1; tmp[i]; i++) {
        if (tmp[i] == '/') {
            tmp[i] = '\0';
            if (mkdir(tmp, 0755) != 0 && errno != EEXIST) return -1;
            tmp[i] = '/';
        }
    }
    return mkdir(tmp, 0755) != 0 && errno != EEXIST ? -1 : 0;
}




static int write_file_exact(const char* path, const uint8_t* data, size_t len) {
    FILE* f = fopen(path, "wb");
    if (f == NULL) {
        fprintf(stderr, "[HARNESS] probe: fopen(%s) failed: %s\n", path,
                strerror(errno));
        return -1;
    }
    if (len > 0 && fwrite(data, 1, len, f) != len) {
        fprintf(stderr, "[HARNESS] probe: fwrite(%s) failed: %s\n", path,
                strerror(errno));
        fclose(f);
        return -1;
    }
    if (fclose(f) != 0) {
        return -1;
    }
    return 0;
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
  jstring jCryptKey = NULL;
  char unique_id[OMEN_ID_LEN_CAP + 16] = {0};
  char key_z[OMEN_KEY_LEN_CAP + 1] = {0};
  char store_path[OMEN_PATH_MAX] = {0};
  size_t id_len = 0;
  size_t key_len = 0;
  size_t off = 0;
  const uint8_t* id_bytes = NULL;
  const uint8_t* key_bytes = NULL;
  uint16_t raw_keylen = 0;
  int key_is_null = 0;
  int32_t size = 0;
  int32_t mode = 0;
  jlong handle = 0;

  if (g_dic_probe < 0) {
    const char* e = getenv("OMEN_DIC_PROBE");
    g_dic_probe = (e != NULL && strcmp(e, "0") != 0);
  }
  if (g_verify < 0) {
    const char* e = getenv("OMEN_VERIFY");
    g_verify = (e != NULL && strcmp(e, "0") != 0);
  }

  set_stage("input_parsing");
  
  











  if (length < 2) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION: %zu bytes < 2-byte idLen "
                    "header; skipping iteration (no call)\n", length);
    goto release;
  }
  id_len = (size_t)((uint16_t)(buffer[0] | ((uint16_t)buffer[1] << 8)));
  off = 2;
  if (id_len > OMEN_ID_LEN_CAP || off + id_len > length) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION: idLen=%zu exceeds cap=%u or "
                    "buffer length=%zu; skipping iteration (no call)\n",
            id_len, (unsigned)OMEN_ID_LEN_CAP, length);
    goto release;
  }
  id_bytes = buffer + off;
  off += id_len;
  if (off + 8 > length) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION: size/mode fields claim "
                    "bytes beyond buffer length=%zu; skipping iteration "
                    "(no call)\n", length);
    goto release;
  }
  {
    uint32_t u;
    u = (uint32_t)buffer[off] | ((uint32_t)buffer[off + 1] << 8) |
        ((uint32_t)buffer[off + 2] << 16) | ((uint32_t)buffer[off + 3] << 24);
    size = (int32_t)u;
    off += 4;
    u = (uint32_t)buffer[off] | ((uint32_t)buffer[off + 1] << 8) |
        ((uint32_t)buffer[off + 2] << 16) | ((uint32_t)buffer[off + 3] << 24);
    mode = (int32_t)u;
    off += 4;
  }
  if (off + 2 > length) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION: keyLen header claims bytes "
                    "beyond buffer length=%zu; skipping iteration (no call)\n",
            length);
    goto release;
  }
  raw_keylen = (uint16_t)(buffer[off] | ((uint16_t)buffer[off + 1] << 8));
  off += 2;
  key_is_null = (raw_keylen & OMEN_KEY_NULL_FLAG) != 0;
  key_len = (size_t)(raw_keylen & (OMEN_KEY_NULL_FLAG - 1u));
  if (key_len > OMEN_KEY_LEN_CAP || off + key_len > length) {
    fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION: effective keyLen=%zu exceeds "
                    "cap=%u or buffer length=%zu; skipping iteration "
                    "(no call)\n", key_len, (unsigned)OMEN_KEY_LEN_CAP, length);
    goto release;
  }
  key_bytes = buffer + off;

  








  if (id_len == 0) {
    unique_id[0] = '\0';
    fprintf(stderr, "[HARNESS] layout: EMPTY id (idLen=0) — documented "
                    "core-0 rejection regime (libmmkv.c:16732-16738)\n");
  } else {
    size_t prefix = strnlen((const char*)id_bytes, id_len);
    int n = snprintf(unique_id, sizeof(unique_id), "%.*s#%zu",
                     (int)prefix, (const char*)id_bytes, g_unique_seq);
    g_unique_seq++;
    if (n < 0 || (size_t)n >= sizeof(unique_id)) {
      fprintf(stderr, "[HARNESS] FAIL_CONSTRUCTION: unique-id build failed; "
                      "skipping iteration (no call)\n");
      goto release;
    }
    


    if (build_store_path(store_path, sizeof(store_path), unique_id,
                         (size_t)n) != 0) {
      fprintf(stderr, "[HARNESS] cleanup: path build skipped for id of %zu "
                      "bytes; factory still called\n", (size_t)n);
      store_path[0] = '\0';
    }
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
  jMmapID = (*ctx.env)->NewStringUTF(ctx.env, unique_id);
  if (jMmapID == NULL ||
      check_and_clear_jni_exception("input_parsing_mmapid")) {
    goto release;
  }
  if (key_is_null) {
    

    jCryptKey = NULL;
  } else {
    size_t keyz = strnlen((const char*)key_bytes, key_len);
    memcpy(key_z, key_bytes, keyz);
    key_z[keyz] = '\0';
    

    jCryptKey = (*ctx.env)->NewStringUTF(ctx.env, key_z);
    if (jCryptKey == NULL ||
        check_and_clear_jni_exception("input_parsing_cryptkey")) {
      goto release;
    }
  }
  fprintf(stderr, "[HARNESS] layout: idLen=%zu id=%s size=%d mode=%d "
                  "key=%s (len=%zu)\n", id_len,
          id_len == 0 ? "(empty)" : unique_id, size, mode,
          key_is_null ? "NULL" : (key_len == 0 ? "\"\" empty non-null"
                                               : (const char*)key_z),
          key_len);

  set_stage("call_prereq_init");
  
  



  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy((void*)*__omen_afl_area_ptr, __omen_bitmap_snapshot, 65536);
  }

  







  prereqFunctionPtr1(ctx.env, NULL, jRootDir, jCachePath, 1, JNI_FALSE);
  if (check_and_clear_jni_exception("prereq_jniInitialize")) {
    goto release;
  }

  


  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy(__omen_bitmap_snapshot, (void*)*__omen_afl_area_ptr, 65536);
  }

  set_stage("call_target");
  







  handle = targetFunctionPtr0(ctx.env, NULL, jMmapID, (jint)size, (jint)mode,
                              jCryptKey);
  if (check_and_clear_jni_exception("call_target")) {
    goto release;
  }
  fprintf(stderr, "[HARNESS] factory %s -> %#llx%s\n",
          id_len == 0 ? "(empty)" : unique_id, (unsigned long long)handle,
          handle == 0 ? " (documented rejection regime)" : "");

  





  if (g_verify) {
    if (store_path[0] != '\0' && ((mode >> 3) & 1) == 0) {
      struct stat st, stc;
      char crc_path[OMEN_PATH_MAX];
      long sz = -1, crclen = -1;
      if (stat(store_path, &st) == 0) {
        sz = (long)st.st_size;
      }
      if ((size_t)snprintf(crc_path, sizeof(crc_path), "%s.crc", store_path) <
          sizeof(crc_path)) {
        if (stat(crc_path, &stc) == 0) {
          crclen = (long)stc.st_size;
        }
      }
      fprintf(stderr, "[HARNESS] verify: file-mode store %s size=%ld .crc=%ld "
                      "(factory-created; size=%d does NOT control file length, "
                      "libmmkv.c:32656)\n", store_path, sz, crclen, size);
    }
    if (store_path[0] != '\0' && ((mode >> 3) & 1) == 1) {
      FILE* mf = fopen("/proc/self/maps", "r");
      char line[512];
      char wantname[600];
      long mapped = -1;
      if ((size_t)snprintf(wantname, sizeof(wantname), "/dev/ashmem/%s",
                           unique_id) < sizeof(wantname) && mf != NULL) {
        size_t wlen = strlen(wantname);
        while (fgets(line, sizeof(line), mf) != NULL) {
          char* sp = strstr(line, wantname);
          unsigned long ms, me;
          if (sp == NULL) continue;
          
          if (sp[wlen] != ' ' && sp[wlen] != '\n') continue;
          if (sscanf(line, "%lx-%lx", &ms, &me) == 2) {
            mapped = (long)(me - ms);
          }
          break;
        }
        fclose(mf);
      }
      fprintf(stderr, "[HARNESS] verify: ashmem-mode maps /dev/ashmem/%s "
                      "length=%ld (page-round-up(size=%d) min 1 page, "
                      "32106-32113); NO file expected at %s\n", unique_id,
              mapped, size, store_path);
    }
  }

  





  if (g_dic_probe) {
    jlong handle2 = targetFunctionPtr0(ctx.env, NULL, jMmapID, (jint)size,
                                       (jint)mode, jCryptKey);
    if (check_and_clear_jni_exception("dic_probe_second_call")) {
      goto release;
    }
    fprintf(stderr, "[HARNESS] dic_probe second call -> %#llx same=%d\n",
            (unsigned long long)handle2, handle2 == handle);
  }

release:
  set_stage("release");
  








  if (store_path[0] != '\0') {
    char crc_path[OMEN_PATH_MAX];
    unlink(store_path);
    if ((size_t)snprintf(crc_path, sizeof(crc_path), "%s.crc", store_path) <
        sizeof(crc_path)) {
      unlink(crc_path);
    }
  }
  


  if (jCryptKey != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, jCryptKey);
    jCryptKey = NULL;
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
  targetFunctionPtr0 = (function_0)((uintptr_t)targetLibBase + 0x38368);

  











  set_stage("ensure_step1_dirs");
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
      fprintf(stderr, "[HARNESS] step-1 dir %s unusable; aborting (no "
                      "valid rootDir for jniInitialize)\n", OMEN_MMKV_ROOT);
      return -1;
    }
    unlink(probe_path);
  }

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
