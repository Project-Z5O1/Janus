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












#define BUFFER_SIZE 65551









void** __omen_afl_area_ptr = NULL;
char   __omen_bitmap_snapshot[65536] __attribute__((aligned(4096)));




















jobject construct_YuvData(JNIEnv* env, const uint8_t* buffer, size_t length,
                          size_t* offset);
typedef jlong (*function_0)(JNIEnv*, jclass, jint fd, jobject yuvData,
                            jint offset, jint length, jint degree);


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







static const char* const kStagedPath = "staged_fuzz.jpg";
static jint stage_and_open(const uint8_t* payload, size_t n) {
  int wfd;
  jint rfd;
  ssize_t off = 0;

  wfd = open(kStagedPath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (wfd < 0) {
    fprintf(stderr, "[HARNESS] stage_open_write_failed path=%s errno=%d\n",
            kStagedPath, errno);
    return -1;
  }
  while ((size_t)off < n) {
    ssize_t k = write(wfd, payload + off, n - (size_t)off);
    if (k <= 0) {
      fprintf(stderr, "[HARNESS] stage_write_failed errno=%d\n", errno);
      close(wfd);
      return -1;
    }
    off += k;
  }
  if (close(wfd) != 0) {
    fprintf(stderr, "[HARNESS] stage_close_write_failed errno=%d\n", errno);
    return -1;
  }
  rfd = open(kStagedPath, O_RDONLY);
  if (rfd < 0) {
    fprintf(stderr, "[HARNESS] stage_reopen_failed path=%s errno=%d\n",
            kStagedPath, errno);
    return -1;
  }
  return rfd;
}




void fuzz_one_input(const uint8_t* buffer, size_t length) {
  









  jobject yuv = NULL;
  jint fd = -1;
  jlong ret = -2;
  jint argOffset = 0;
  jint argLength = 0;
  jint argDegree = 0;
  unsigned int jpegLen = 0;

  set_stage("input_parsing");
  
  












  if (length < 18u) {
    fprintf(stderr,
            "[HARNESS] FAIL_CONSTRUCTION len=%zu (<18B header), skip target\n",
            length);
    fflush(stderr);
    goto release;
  }
  {
    uint32_t off_raw, len_raw, deg_raw;
    uint16_t jlen_raw;
    size_t avail = length - 14u;

    memcpy(&off_raw, buffer + 0, 4);
    memcpy(&len_raw, buffer + 4, 4);
    memcpy(&deg_raw, buffer + 8, 4);
    memcpy(&jlen_raw, buffer + 12, 2);
    memcpy(&argOffset, &off_raw, 4); 
    memcpy(&argLength, &len_raw, 4); 
    memcpy(&argDegree, &deg_raw, 4); 
    jpegLen = (unsigned int)jlen_raw;
    if ((size_t)jpegLen > avail) {
      fprintf(stderr,
              "[HARNESS] FAIL_CONSTRUCTION jpeg_len=%u crosses N=%zu "
              "(avail=%zu), skip target\n",
              jpegLen, length, avail);
      fflush(stderr);
      goto release;
    }

    
    fd = stage_and_open(buffer + 14, jpegLen);
    if (fd < 0) {
      goto release;
    }

    




    if (!g_oracle_ready && init_oracle_accessors() != 0) {
      


      fprintf(stderr,
              "[HARNESS] FAIL_CONSTRUCTION oracle accessor init, skip "
              "target\n");
      fflush(stderr);
      goto release;
    }
    yuv = construct_YuvData(ctx.env, NULL, 0, NULL);
    if (yuv == NULL) {
      fprintf(stderr,
              "[HARNESS] FAIL_CONSTRUCTION construct_YuvData NULL, skip "
              "target\n");
      fflush(stderr);
      goto release;
    }
  }

  set_stage("call_target");
  
  













  ret = targetFunctionPtr0(ctx.env, NULL ,
                           fd, yuv, argOffset, argLength, argDegree);
  



  fd = -1;
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
    if (check_and_clear_jni_exception("oracle_readback")) {
      goto release;
    }
    fprintf(stderr,
            "[HARNESS] target_ret=%lld yuvDataPointer=0x%llx w=%d h=%d "
            "needRelease=%d (offset=%d length=%d degree=%d jpeg_len=%u)\n",
            (long long)ret, (unsigned long long)ptr, w, h, rel,
            argOffset, argLength, argDegree, jpegLen);
    if (ret == 0 && (ptr == 0 || w <= 0 || h <= 0)) {
      fprintf(stderr,
              "[HARNESS] ORACLE_VIOLATION ret=0 but ptr/w/h=%lld/%d/%d\n",
              (long long)ptr, w, h);
    }
  }
  fflush(stderr);

release:
  set_stage("cleanup");
  





  if (fd >= 0) {
    close(fd);
    fprintf(stderr, "[HARNESS] unconsumed staging fd closed (target skipped)\n");
  }
  if (yuv != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, yuv);
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
        (function_0)((uintptr_t)targetLibBase + 0x8734);
  }
  (void)targetLibHandle;

  











  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  set_stage("done");
  return 0;
}
