#include <dlfcn.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "jenv.h"














#define BUFFER_SIZE 65548






















void** __omen_afl_area_ptr = NULL;
char   __omen_bitmap_snapshot[65536] __attribute__((aligned(4096)));













typedef jint (*function_0)(JNIEnv*, jobject, jlong, jobject, jint, jobject,
                           jint);


static JavaCTX ctx;



function_0 targetFunctionPtr0;  















int construct_FfmpegAudioDecoder_context(JNIEnv* env, jlong* out_ctx);
void destruct_FfmpegAudioDecoder_context(JNIEnv* env, jlong ctx);


static jlong g_target_ctx = (jlong)0;



static uint32_t rd32le(const uint8_t* p) {
  return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) |
         ((uint32_t)p[3] << 24);
}


#define OMEN_ARG2_DOMAIN_CAP 65536u
#define OMEN_ARG4_DOMAIN_CAP 65536u
#define OMEN_OUTCAP_DOMAIN_CAP 65536u

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






static uint32_t count_silence_frames(const uint8_t* img, uint32_t usable) {
  static const uint8_t hdr[4] = {0xFF, 0xFB, 0x90, 0x00};
  const uint32_t frame = 417u;
  uint32_t off = 0;
  uint32_t k = 0;

  while (off + frame <= usable) {
    if (img[off] != hdr[0] || img[off + 1] != hdr[1] ||
        img[off + 2] != hdr[2] || img[off + 3] != hdr[3]) {
      break;
    }
    off += frame;
    k++;
  }
  return k;
}




void fuzz_one_input(const uint8_t* buffer, size_t length) {
  
  
  
  
  
  jobject in_bb = NULL;
  jobject out_bb = NULL;
  uint8_t* staged_in = NULL;
  uint8_t* out_backing = NULL;
  uint32_t raw_in_len;
  uint32_t raw_out_len;
  uint32_t raw_out_cap;
  uint32_t in_len;
  uint32_t out_len;
  uint32_t out_cap;
  uint32_t in_cap;
  jint ret;
  jint expected;
  int oracle_known;

  set_stage("input_parsing");
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  if (buffer == NULL || length < 12) {
    



    fprintf(stderr, "[HARNESS] input_parsing FAIL_CONSTRUCTION length=%zu "
                    "(need 12); skipping target\n", length);
    fflush(stderr);
    goto release_stage;
  }
  raw_in_len = rd32le(buffer + 0);
  raw_out_len = rd32le(buffer + 4);
  raw_out_cap = rd32le(buffer + 8);
  in_len = (raw_in_len > OMEN_ARG2_DOMAIN_CAP) ? OMEN_ARG2_DOMAIN_CAP
                                               : raw_in_len;
  out_len = (raw_out_len > OMEN_ARG4_DOMAIN_CAP) ? OMEN_ARG4_DOMAIN_CAP
                                                 : raw_out_len;
  out_cap = (raw_out_cap > OMEN_OUTCAP_DOMAIN_CAP) ? OMEN_OUTCAP_DOMAIN_CAP
                                                   : raw_out_cap;
  in_cap = (uint32_t)(length - 12);
  if (raw_in_len != in_len || raw_out_len != out_len ||
      raw_out_cap != out_cap) {
    fprintf(stderr, "[HARNESS] input_parsing domain clamp (runtime case guard): "
                    "inLen %u -> %u, outLen %u -> %u, outCap %u -> %u\n",
            raw_in_len, in_len, raw_out_len, out_len, raw_out_cap, out_cap);
  }
  fprintf(stderr, "[HARNESS] input_parsing inLen=%u outLen=%u outCap=%u "
                  "in_cap=%u (length=%zu)\n",
          in_len, out_len, out_cap, in_cap, length);
  fflush(stderr);

  set_stage("get_obj");
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  staged_in = (uint8_t*)malloc(in_cap ? in_cap : 1);
  if (staged_in == NULL) {
    fprintf(stderr, "[HARNESS] get_obj FAIL_CONSTRUCTION staged_in "
                    "malloc(%u) failed; skipping target\n", in_cap);
    fflush(stderr);
    goto release_stage;
  }
  if (in_cap > 0) {
    memcpy(staged_in, buffer + 12, in_cap);
  }
  out_backing = (uint8_t*)malloc(out_cap ? out_cap : 1);
  if (out_backing == NULL) {
    fprintf(stderr, "[HARNESS] get_obj FAIL_CONSTRUCTION out_backing "
                    "malloc(%u) failed; skipping target\n",
            out_cap ? out_cap : 1);
    fflush(stderr);
    goto release_stage;
  }
  in_bb = (*ctx.env)->NewDirectByteBuffer(ctx.env, staged_in, (jlong)in_cap);
  out_bb = (*ctx.env)->NewDirectByteBuffer(ctx.env, out_backing,
                                           (jlong)out_cap);
  if (check_and_clear_jni_exception("get_obj NewDirectByteBuffer") != 0 ||
      in_bb == NULL || out_bb == NULL) {
    



    fprintf(stderr, "[HARNESS] get_obj FAIL_CONSTRUCTION NewDirectByteBuffer "
                    "in=%p out=%p; skipping target\n", (void*)in_bb,
            (void*)out_bb);
    fflush(stderr);
    goto release_stage;
  }
  fprintf(stderr, "[HARNESS] get_obj direct in base=%p cap=%u, direct out "
                  "base=%p cap=%u (exact-size heap backings)\n",
          (void*)staged_in, in_cap, (void*)out_backing, out_cap);
  fflush(stderr);

  set_stage("call_jni");
  
  
  
  
  
  
  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy((void*)*__omen_afl_area_ptr, __omen_bitmap_snapshot, 65536);
  }
  
  
  
  
  
  
  
  
  
  
  
  ret = targetFunctionPtr0(ctx.env, NULL, g_target_ctx, in_bb, (jint)in_len,
                           out_bb, (jint)out_len);
  {
    int target_exc = check_and_clear_jni_exception("call_jni");
    
    expected = (jint)0;
    oracle_known = 0;
    if (in_len < 2) {
      expected = (jint)0;
      oracle_known = 1;
    } else {
      uint32_t usable = (in_len < in_cap) ? in_len : in_cap;
      uint32_t k = count_silence_frames(staged_in, usable);
      if (k > 0) {
        uint32_t want = 4608u * k;
        if (out_len >= want) {
          expected = (jint)want;
        } else {
          expected = (jint)-1; 
        }
        oracle_known = 1;
      }
    }
    if (oracle_known && ret != expected) {
      fprintf(stderr, "[HARNESS] call_jni target ret=%d expected=%d "
                      "MISMATCH (target-behavior observation — ctx state may "
                      "have been corrupted in-loop, runtime case runtime path; "
                      "or payload is not a benign silence-frame seed) "
                      "exc=%d\n", (int)ret, (int)expected, target_exc);
    } else {
      fprintf(stderr, "[HARNESS] call_jni target ret=%d (oracle %s exc=%d\n",
              (int)ret,
              oracle_known ? "known" : "n/a (payload heuristic inactive)",
              target_exc);
    }
    fflush(stderr);
  }

release_stage:
  set_stage("release");
  
  
  
  
  
  
  
  
  
  
  if (out_bb != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, out_bb);
    out_bb = NULL;
  }
  if (in_bb != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, in_bb);
    in_bb = NULL;
  }
  free(out_backing);
  free(staged_in);
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
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  void* targetLibHandle = dlopen("libffmpegJNI.so", RTLD_NOW);
  if (targetLibHandle == NULL) {
    fprintf(stderr, "[HARNESS] Failed to dlopen libffmpegJNI.so: %s\n", dlerror());
    return -1;
  }
  void* targetLibBase = get_library_base("libffmpegJNI.so");
  if (targetLibBase == NULL) {
    fprintf(stderr, "[HARNESS] Failed to get library base for libffmpegJNI.so\n");
    return -1;
  }
  (void)targetLibHandle;
  targetFunctionPtr0 =
      (function_0)((uintptr_t)targetLibBase + 0x6DDFCu);
  fprintf(stderr,
          "[HARNESS] libffmpegJNI.so base=%p offset 0x6DDFC resolved "
          "(JNI_OnLoad skipped per verified initialization variant 3 contract)\n",
          targetLibBase);
  fflush(stderr);

  
  
  
  
  
  
  
  
  
  
  
  
  
  
  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy((void*)*__omen_afl_area_ptr, __omen_bitmap_snapshot, 65536);
  }
  if (construct_FfmpegAudioDecoder_context(ctx.env, &g_target_ctx) != 0 ||
      g_target_ctx == (jlong)0) {
    fprintf(stderr, "[HARNESS] bring-up construct_FfmpegAudioDecoder_context "
                    "failed (factory 0x6DD8C via sister 0x6E398); refusing "
                    "to run the target with a fabricated/zero handle\n");
    return -1;
  }
  if (check_and_clear_jni_exception("get_obj factory ffmpegInitialize")) {
    return JNI_ERR;
  }
  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy(__omen_bitmap_snapshot, (void*)*__omen_afl_area_ptr, 65536);
  }

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  
  
  
  
  
  
  set_stage("teardown_context");
  destruct_FfmpegAudioDecoder_context(ctx.env, g_target_ctx);
  g_target_ctx = (jlong)0;
  if (check_and_clear_jni_exception("teardown ffmpegRelease")) {
    return JNI_ERR;
  }

  set_stage("done");
  return 0;
}
