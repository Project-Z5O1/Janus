

#include <jni.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#include "jenv.h" 

#define OMEN_ADEC_LIB "libffmpegJNI.so"
#define OMEN_ADEC_FACTORY_OFFSET 0x6DD8Cu  
#define OMEN_ADEC_SISTER_OFFSET  0x6E398u  
#define OMEN_ADEC_RELEASE_OFFSET 0x6E388u  

#define OMEN_ADEC_CODEC_NAME "mp3"     
#define OMEN_ADEC_CHANNELS 2           
#define OMEN_ADEC_SAMPLE_RATE 44100    



#define OMEN_ADEC_CTX_SAMPLE_RATE_OFF 0x190u  
#define OMEN_ADEC_CTX_CHANNELS_OFF    0x194u  
#define OMEN_ADEC_CTX_SAMPLE_FMT_OFF  0x198u  
#define OMEN_ADEC_CTX_CH_LAYOUT_OFF   0x1b0u  

typedef jbyteArray (*omen_adec_create_params_fn)(JNIEnv*, jclass, jint, jint);
typedef jlong (*omen_adec_init_fn)(JNIEnv*, jobject, jstring, jbyteArray,
                                   jbyteArray);
typedef void (*omen_adec_release_fn)(JNIEnv*, jobject, jlong);

int construct_FfmpegAudioDecoder_context(JNIEnv* env, jlong* out_ctx) {
  jbyteArray blob = NULL;
  jstring codec_name = NULL;
  jsize blob_len = 0;
  jlong ctx = (jlong)0;
  void* lib = NULL;
  void* base = NULL;
  omen_adec_create_params_fn sister;
  omen_adec_init_fn factory;
  int rc = -1;

  if (out_ctx == NULL) {
    fprintf(stderr, "[recipe] out_ctx is NULL\n");
    return -1;
  }
  *out_ctx = (jlong)0;

  
  lib = dlopen(OMEN_ADEC_LIB, RTLD_NOW);
  if (lib == NULL) {
    fprintf(stderr, "[recipe] dlopen(%s) failed: %s\n", OMEN_ADEC_LIB,
            dlerror());
    return -1;
  }
  base = get_library_base(OMEN_ADEC_LIB);
  if (base == NULL) {
    fprintf(stderr, "[recipe] get_library_base(%s) failed\n", OMEN_ADEC_LIB);
    return -1;
  }
  fprintf(stderr, "[recipe] libffmpegJNI.so base %p: sister 0x%lx factory 0x%lx\n",
          base, (unsigned long)OMEN_ADEC_SISTER_OFFSET,
          (unsigned long)OMEN_ADEC_FACTORY_OFFSET);

  




  sister = (omen_adec_create_params_fn)((uintptr_t)base + OMEN_ADEC_SISTER_OFFSET);
  blob = sister(env, NULL, (jint)OMEN_ADEC_CHANNELS,
                (jint)OMEN_ADEC_SAMPLE_RATE);
  if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
    fprintf(stderr, "[recipe] exception from sister factory\n");
    (*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);
    goto out;
  }
  if (blob == NULL) {
    fprintf(stderr,
            "[recipe] sister factory returned NULL (OOM branch, "
            "libffmpegJNI.c:2412-2415) — construction fails\n");
    goto out;
  }
  blob_len = (*env)->GetArrayLength(env, blob);
  if (blob_len <= 0 || blob_len > 4096) {
    fprintf(stderr,
            "[recipe] sister blob length %d outside cereal domain (0,4096] "
            "(sister argument_plan.md return row) — construction fails\n",
            (int)blob_len);
    goto out;
  }
  fprintf(stderr, "[recipe] sister blob len=%d (expected ~120B, "
          "input-independent)\n", (int)blob_len);

  
  codec_name = (*env)->NewStringUTF(env, OMEN_ADEC_CODEC_NAME);
  if (codec_name == NULL ||
      (*env)->ExceptionCheck(env) == JNI_TRUE) {
    fprintf(stderr, "[recipe] NewStringUTF(\"%s\") failed\n",
            OMEN_ADEC_CODEC_NAME);
    if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
      (*env)->ExceptionDescribe(env);
      (*env)->ExceptionClear(env);
    }
    goto out;
  }

  


  factory = (omen_adec_init_fn)((uintptr_t)base + OMEN_ADEC_FACTORY_OFFSET);
  ctx = factory(env, NULL, codec_name, NULL, blob);
  if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
    fprintf(stderr, "[recipe] exception from ffmpegInitialize\n");
    (*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);
    ctx = (jlong)0;
    goto out;
  }
  if (ctx == (jlong)0) {
    fprintf(stderr,
            "[recipe] factory returned 0 (codec not found :2015-2017 / "
            "alloc failure :7861 / open2 failure :7872) — construction "
            "fails, ctx==0 must never reach the target (runtime case)\n");
    goto out;
  }
  if ((ctx & 7) != 0) {
    fprintf(stderr, "[recipe] context %llx not 8-byte aligned\n",
            (unsigned long long)ctx);
    ctx = (jlong)0;
    goto out;
  }

  

  {
    const uint8_t* p = (const uint8_t*)(uintptr_t)ctx;
    int32_t sample_rate, channels, sample_fmt;
    int64_t ch_layout;
    memcpy(&sample_rate, p + OMEN_ADEC_CTX_SAMPLE_RATE_OFF, 4);
    memcpy(&channels, p + OMEN_ADEC_CTX_CHANNELS_OFF, 4);
    memcpy(&sample_fmt, p + OMEN_ADEC_CTX_SAMPLE_FMT_OFF, 4);
    memcpy(&ch_layout, p + OMEN_ADEC_CTX_CH_LAYOUT_OFF, 8);
    fprintf(stderr,
            "[recipe] ctx %llx readback: sample_rate=%d channels=%d "
            "sample_fmt=%d channel_layout=%lld\n",
            (unsigned long long)ctx, (int)sample_rate, (int)channels,
            (int)sample_fmt, (long long)ch_layout);
    if (sample_rate != OMEN_ADEC_SAMPLE_RATE) {
      fprintf(stderr,
              "[recipe] ctx->sample_rate=%d != pinned %d (offset 0x190, "
              "ffmpegGetSampleRate :2226) — cross-target chain broken\n",
              (int)sample_rate, OMEN_ADEC_SAMPLE_RATE);
      ctx = (jlong)0;
      goto out;
    }
    if (channels != OMEN_ADEC_CHANNELS) {
      fprintf(stderr,
              "[recipe] ctx->channels=%d != pinned %d (offset 0x194, "
              "target :2139) — cross-target chain broken\n",
              (int)channels, OMEN_ADEC_CHANNELS);
      ctx = (jlong)0;
      goto out;
    }
    if (ch_layout == 0) {
      fprintf(stderr,
              "[recipe] ctx->channel_layout==0 (offset 0x1b0, target "
              ":2140) — default-layout fixup :7989-7992 did not apply\n");
      ctx = (jlong)0;
      goto out;
    }
  }

  *out_ctx = ctx;
  fprintf(stderr, "[recipe] FfmpegAudioDecoder context %llx verified "
          "(mp3, 2ch, 44100Hz, codecParametersData via sister 0x6E398)\n",
          (unsigned long long)ctx);
  rc = 0;

out:
  


  if (codec_name != NULL) {
    (*env)->DeleteLocalRef(env, codec_name);
  }
  if (blob != NULL) {
    (*env)->DeleteLocalRef(env, blob);
  }
  return rc;
}




void destruct_FfmpegAudioDecoder_context(JNIEnv* env, jlong ctx) {
  void* base;
  omen_adec_release_fn release;
  if (ctx == (jlong)0) {
    return;
  }
  if (dlopen(OMEN_ADEC_LIB, RTLD_NOW) == NULL) {
    return;
  }
  base = get_library_base(OMEN_ADEC_LIB);
  if (base == NULL) {
    return;
  }
  release = (omen_adec_release_fn)((uintptr_t)base + OMEN_ADEC_RELEASE_OFFSET);
  release(env, NULL, ctx);
  fprintf(stderr, "[recipe] ffmpegRelease(%llx) done\n",
          (unsigned long long)ctx);
}
