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













typedef jlong (*function_0)(JNIEnv*, jobject, jlong, jbyteArray, jbyteArray);


static JavaCTX ctx;



function_0 targetFunctionPtr0;  


int construct_FfmpegAudioDecoder_context(JNIEnv* env, jlong* out_ctx);
void destruct_FfmpegAudioDecoder_context(JNIEnv* env, jlong ctx);
int construct_FfmpegAudioDecoder_context_truehd(JNIEnv* env, jlong* out_ctx);
void destruct_FfmpegAudioDecoder_context_truehd(JNIEnv* env, jlong ctx);


static jlong g_ctx_mp3 = (jlong)0;   
static jlong g_ctx_thd = (jlong)0;   
static unsigned long g_iteration = 0;



static uint32_t rd32le(const uint8_t* p) {
  return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) |
         ((uint32_t)p[3] << 24);
}





#define OMEN_ARRAY_DOMAIN_CAP 65536u
#define OMEN_LEN_SENTINEL 0xFFFFFFFFu




#define OMEN_CTX_CODEC_ID_OFF    0x18u
#define OMEN_CTX_SAMPLE_RATE_OFF 0x190u
#define OMEN_CTX_CHANNELS_OFF    0x194u
#define OMEN_CTX_CH_LAYOUT_OFF   0x1b0u
#define OMEN_CODEC_ID_TRUEHD     0x1502cu

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


static void log_ctx_state(const char* tag, jlong c) {
  const uint8_t* p = (const uint8_t*)(uintptr_t)c;
  int32_t codec_id = 0, sr = 0, ch = 0;
  int64_t layout = 0;
  if (c == (jlong)0) {
    fprintf(stderr, "[HARNESS] %s ctx=NULL\n", tag);
    fflush(stderr);
    return;
  }
  memcpy(&codec_id, p + OMEN_CTX_CODEC_ID_OFF, 4);
  memcpy(&sr, p + OMEN_CTX_SAMPLE_RATE_OFF, 4);
  memcpy(&ch, p + OMEN_CTX_CHANNELS_OFF, 4);
  memcpy(&layout, p + OMEN_CTX_CH_LAYOUT_OFF, 8);
  fprintf(stderr, "[HARNESS] %s ctx=%llx codec_id=0x%x sample_rate=%d "
          "channels=%d channel_layout=%lld\n", tag,
          (unsigned long long)c, codec_id, sr, ch, (long long)layout);
  fflush(stderr);
}







static int omen_heal_ctx(int arm_truehd) {
  jlong c = (jlong)0;
  int rc;
  if (arm_truehd) {
    rc = construct_FfmpegAudioDecoder_context_truehd(ctx.env, &c);
  } else {
    rc = construct_FfmpegAudioDecoder_context(ctx.env, &c);
  }
  if (rc != 0 || c == (jlong)0) {
    fprintf(stderr, "[HARNESS] self-heal %s recipe FAILED rc=%d c=%llx — "
            "arm will be skipped until a heal succeeds\n",
            arm_truehd ? "truehd" : "mp3", rc, (unsigned long long)c);
    fflush(stderr);
    return -1;
  }
  if (check_and_clear_jni_exception("self-heal recipe") != 0) {
    return -1;
  }
  if (arm_truehd) {
    g_ctx_thd = c;
  } else {
    g_ctx_mp3 = c;
  }
  fprintf(stderr, "[HARNESS] self-heal %s arm OK new ctx=%llx\n",
          arm_truehd ? "truehd" : "mp3", (unsigned long long)c);
  fflush(stderr);
  return 0;
}








static int g_thd_last_sr = -1;
static int g_thd_last_ch = -1;




void fuzz_one_input(const uint8_t* buffer, size_t length) {
  
  
  
  
  
  jbyteArray a1 = NULL;
  jbyteArray a2 = NULL;
  uint32_t raw_len1;
  uint32_t raw_len2;
  uint32_t len1;      
  uint32_t len2;
  int pass_null1;
  int pass_null2;
  uint64_t mirror_off2;
  jlong r1;
  jlong r2;

  g_iteration++;
  set_stage("input_parsing");
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  if (buffer == NULL || length < 8) {
    fprintf(stderr, "[HARNESS] input_parsing FAIL_CONSTRUCTION length=%zu "
                    "(need 8); skipping target\n", length);
    fflush(stderr);
    goto release_stage;
  }
  raw_len1 = rd32le(buffer + 0);
  raw_len2 = rd32le(buffer + 4);
  pass_null1 = (raw_len1 == OMEN_LEN_SENTINEL);
  pass_null2 = (raw_len2 == OMEN_LEN_SENTINEL);
  len1 = pass_null1 ? 0u : raw_len1;
  len2 = pass_null2 ? 0u : raw_len2;
  if ((!pass_null1 && raw_len1 > OMEN_ARRAY_DOMAIN_CAP) ||
      (!pass_null2 && raw_len2 > OMEN_ARRAY_DOMAIN_CAP)) {
    fprintf(stderr, "[HARNESS] input_parsing FAIL_CONSTRUCTION len1=%u "
                    "len2=%u exceeds domain cap %u (sentinel=0x%x); skipping "
                    "target (no clamp per plan)\n", raw_len1, raw_len2,
            OMEN_ARRAY_DOMAIN_CAP, OMEN_LEN_SENTINEL);
    fflush(stderr);
    goto release_stage;
  }
  mirror_off2 = 8ull + (uint64_t)len1;
  if (8ull + (uint64_t)len1 > (uint64_t)length ||
      mirror_off2 + (uint64_t)len2 > (uint64_t)length) {
    fprintf(stderr, "[HARNESS] input_parsing FAIL_CONSTRUCTION mirror "
                    "insufficient: len1=%u len2=%u need %llu have %zu; "
                    "skipping target\n", raw_len1, raw_len2,
            (unsigned long long)(mirror_off2 + (uint64_t)len2), length);
    fflush(stderr);
    goto release_stage;
  }
  fprintf(stderr, "[HARNESS] input_parsing iter=%lu len1=%s%u len2=%s%u "
                  "(length=%zu)\n", g_iteration,
          pass_null1 ? "NULL:" : "", raw_len1,
          pass_null2 ? "NULL:" : "", raw_len2, length);
  fflush(stderr);

  set_stage("get_obj");
  
  
  
  
  
  
  
  if (!pass_null1) {
    a1 = (*ctx.env)->NewByteArray(ctx.env, (jsize)len1);
    if (a1 == NULL ||
        check_and_clear_jni_exception("get_obj NewByteArray a1") != 0) {
      fprintf(stderr, "[HARNESS] get_obj FAIL_CONSTRUCTION NewByteArray(%u) "
                      "a1; skipping target\n", len1);
      fflush(stderr);
      goto release_stage;
    }
    if (len1 > 0) {
      (*ctx.env)->SetByteArrayRegion(ctx.env, a1, 0, (jsize)len1,
                                     (const jbyte*)(buffer + 8));
    }
  }
  if (!pass_null2) {
    a2 = (*ctx.env)->NewByteArray(ctx.env, (jsize)len2);
    if (a2 == NULL ||
        check_and_clear_jni_exception("get_obj NewByteArray a2") != 0) {
      fprintf(stderr, "[HARNESS] get_obj FAIL_CONSTRUCTION NewByteArray(%u) "
                      "a2; skipping target\n", len2);
      fflush(stderr);
      goto release_stage;
    }
    if (len2 > 0) {
      (*ctx.env)->SetByteArrayRegion(ctx.env, a2, 0, (jsize)len2,
                                     (const jbyte*)(buffer + 8 + len1));
    }
  }

  set_stage("call_jni");
  
  
  
  
  
  
  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy((void*)*__omen_afl_area_ptr, __omen_bitmap_snapshot, 65536);
  }

  
  
  
  
  
  
  
  
  
  set_stage("call_jni_arm_mp3");
  if (g_ctx_mp3 == (jlong)0) {
    fprintf(stderr, "[HARNESS] arm_mp3 SKIPPED (ctx==0, heal pending — "
                    "ctx==0 is never fed to the target, runtime case)\n");
    fflush(stderr);
  } else {
    r1 = targetFunctionPtr0(ctx.env, NULL, g_ctx_mp3, a1, a2);
    if (check_and_clear_jni_exception("call_jni arm_mp3") != 0) {
      goto release_stage;
    }
    if (r1 == g_ctx_mp3) {
      fprintf(stderr, "[HARNESS] arm_mp3 ret==ctx (%llx) flush oracle OK\n",
              (unsigned long long)r1);
    } else if (r1 == (jlong)0) {
      
      
      
      
      
      fprintf(stderr, "[HARNESS] arm_mp3 ret==0 (UNEXPECTED on flush arm) — "
                      "self-heal per Harness Notes 2\n");
      fflush(stderr);
      if (omen_heal_ctx(0) != 0) {
        goto release_stage;
      }
    } else {
      
      
      fprintf(stderr, "[HARNESS] arm_mp3 ret=%llx != ctx=%llx MISMATCH "
                      "(flush arm must return the original pointer :2261 — "
                      "target-behavior observation)\n",
              (unsigned long long)r1, (unsigned long long)g_ctx_mp3);
      g_ctx_mp3 = r1; 
    }
    if (g_iteration == 1) {
      log_ctx_state("arm_mp3 post-call contrast (flush cannot rewrite "
                    "fields)", g_ctx_mp3);
    }
  }

  
  
  
  
  
  
  
  
  
  
  set_stage("call_jni_arm_truehd");
  if (g_ctx_thd == (jlong)0) {
    fprintf(stderr, "[HARNESS] arm_truehd SKIPPED (ctx==0, heal pending — "
                    "ctx==0 is never fed to the target, runtime case)\n");
    fflush(stderr);
  } else {
    r2 = targetFunctionPtr0(ctx.env, NULL, g_ctx_thd, a1, a2);
    if (check_and_clear_jni_exception("call_jni arm_truehd") != 0) {
      goto release_stage;
    }
    if (r2 == (jlong)0) {
      
      
      
      
      
      
      fprintf(stderr, "[HARNESS] arm_truehd ret==0 (recreate failed; old ctx "
                      "released at :2248) — self-heal per Harness Notes 2\n");
      fflush(stderr);
      if (omen_heal_ctx(1) != 0) {
        goto release_stage;
      }
    } else {
      const uint8_t* p = (const uint8_t*)(uintptr_t)r2;
      int32_t codec_id = 0;
      int32_t sr = 0, ch = 0;
      memcpy(&codec_id, p + OMEN_CTX_CODEC_ID_OFF, 4);
      memcpy(&sr, p + OMEN_CTX_SAMPLE_RATE_OFF, 4);
      memcpy(&ch, p + OMEN_CTX_CHANNELS_OFF, 4);
      fprintf(stderr, "[HARNESS] arm_truehd ret=%llx%s codec_id=0x%x "
                      "sample_rate=%d channels=%d\n",
              (unsigned long long)r2,
              (r2 == g_ctx_thd) ? " (==old ctx: scudo heap reuse, not arm "
                                  "evidence)" : "",
              codec_id, sr, ch);
      fflush(stderr);
      if ((uint32_t)codec_id != OMEN_CODEC_ID_TRUEHD) {
        fprintf(stderr, "[HARNESS] arm_truehd ADOPTED ctx NOT in recreate "
                        "domain (codec_id 0x%x != 0x1502c) — target-behavior "
                        "observation\n", codec_id);
        fflush(stderr);
      }
      g_ctx_thd = r2; 
      if (sr != g_thd_last_sr || ch != g_thd_last_ch) {
        fprintf(stderr, "[HARNESS] arm_truehd X-FLIP EVIDENCE: domain "
                        "%dch/%dHz -> %dch/%dHz (recreate consumed reset-time "
                        "arg2; flush cannot rewrite these fields)\n",
                g_thd_last_ch, g_thd_last_sr, ch, sr);
        fflush(stderr);
        g_thd_last_sr = sr;
        g_thd_last_ch = ch;
      }
    }
  }

release_stage:
  set_stage("release");
  
  
  
  
  
  
  if (a2 != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, a2);
    a2 = NULL;
  }
  if (a1 != NULL) {
    (*ctx.env)->DeleteLocalRef(ctx.env, a1);
    a1 = NULL;
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
      (function_0)((uintptr_t)targetLibBase + 0x6E2B8u);
  fprintf(stderr,
          "[HARNESS] libffmpegJNI.so base=%p offset 0x6E2B8 resolved "
          "(JNI_OnLoad skipped per verified initialization contract)\n",
          targetLibBase);
  fflush(stderr);

  
  
  
  
  
  
  
  
  
  
  set_stage("get_obj_bringup");
  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy((void*)*__omen_afl_area_ptr, __omen_bitmap_snapshot, 65536);
  }
  if (construct_FfmpegAudioDecoder_context(ctx.env, &g_ctx_mp3) != 0 ||
      g_ctx_mp3 == (jlong)0) {
    fprintf(stderr, "[HARNESS] bring-up construct_FfmpegAudioDecoder_context "
                    "(mp3/flush recipe 36beea7e) failed; refusing to run the "
                    "target with a fabricated/zero handle\n");
    return -1;
  }
  if (check_and_clear_jni_exception("get_obj factory ffmpegInitialize mp3")) {
    return JNI_ERR;
  }
  if (construct_FfmpegAudioDecoder_context_truehd(ctx.env, &g_ctx_thd) != 0 ||
      g_ctx_thd == (jlong)0) {
    fprintf(stderr, "[HARNESS] bring-up "
                    "construct_FfmpegAudioDecoder_context_truehd (recreate "
                    "recipe 13ad3ca3) failed; refusing to run the target "
                    "with a fabricated/zero handle\n");
    return -1;
  }
  if (check_and_clear_jni_exception("get_obj factory ffmpegInitialize truehd")) {
    return JNI_ERR;
  }
  g_thd_last_sr = 48000; 
  g_thd_last_ch = 6;
  fprintf(stderr, "[HARNESS] dual-arm bring-up OK: mp3 ctx=%llx (flush arm) "
                  "truehd ctx=%llx (recreate arm, 6ch/48000)\n",
          (unsigned long long)g_ctx_mp3, (unsigned long long)g_ctx_thd);
  fflush(stderr);
  if (__omen_afl_area_ptr && *__omen_afl_area_ptr) {
    memcpy(__omen_bitmap_snapshot, (void*)*__omen_afl_area_ptr, 65536);
  }

  set_stage("execute_fuzz_case");
  fuzz_one_input(buffer, (size_t)rlength);

  
  
  
  
  
  
  set_stage("teardown_context");
  destruct_FfmpegAudioDecoder_context_truehd(ctx.env, g_ctx_thd);
  g_ctx_thd = (jlong)0;
  destruct_FfmpegAudioDecoder_context(ctx.env, g_ctx_mp3);
  g_ctx_mp3 = (jlong)0;
  if (check_and_clear_jni_exception("teardown ffmpegRelease")) {
    return JNI_ERR;
  }

  set_stage("done");
  return 0;
}
