

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

#define OMEN_ADEC_CODEC_NAME "truehd"  
#define OMEN_ADEC_CHANNELS 6           
#define OMEN_ADEC_SAMPLE_RATE 48000    

#define OMEN_ADEC_CODEC_ID_TRUEHD 0x1502cu 


#define OMEN_ADEC_CTX_CODEC_ID_OFF   0x18u  
#define OMEN_ADEC_CTX_SAMPLE_RATE_OFF 0x190u
#define OMEN_ADEC_CTX_CHANNELS_OFF    0x194u
#define OMEN_ADEC_CTX_CH_LAYOUT_OFF   0x1b0u

typedef jbyteArray (*omen_adec_create_params_fn)(JNIEnv*, jclass, jint, jint);
typedef jlong (*omen_adec_init_fn)(JNIEnv*, jobject, jstring, jbyteArray,
                                   jbyteArray);
typedef void (*omen_adec_release_fn)(JNIEnv*, jobject, jlong);

int construct_FfmpegAudioDecoder_context_truehd(JNIEnv* env, jlong* out_ctx) {
  jbyteArray blob = NULL;
  jstring codec_name = NULL;
  jsize blob_len = 0;
  jlong ctx = (jlong)0;
  void* base = NULL;
  omen_adec_create_params_fn sister;
  omen_adec_init_fn factory;
  int rc = -1;

  if (out_ctx == NULL) {
    fprintf(stderr, "[recipe-truehd] out_ctx is NULL\n");
    return -1;
  }
  *out_ctx = (jlong)0;

  
  if (dlopen(OMEN_ADEC_LIB, RTLD_NOW) == NULL) {
    fprintf(stderr, "[recipe-truehd] dlopen(%s) failed: %s\n", OMEN_ADEC_LIB,
            dlerror());
    return -1;
  }
  base = get_library_base(OMEN_ADEC_LIB);
  if (base == NULL) {
    fprintf(stderr, "[recipe-truehd] get_library_base(%s) failed\n",
            OMEN_ADEC_LIB);
    return -1;
  }
  fprintf(stderr, "[recipe-truehd] libffmpegJNI.so base %p: sister 0x%lx "
          "factory 0x%lx\n", base, (unsigned long)OMEN_ADEC_SISTER_OFFSET,
          (unsigned long)OMEN_ADEC_FACTORY_OFFSET);

  




  sister = (omen_adec_create_params_fn)((uintptr_t)base + OMEN_ADEC_SISTER_OFFSET);
  blob = sister(env, NULL, (jint)OMEN_ADEC_CHANNELS,
                (jint)OMEN_ADEC_SAMPLE_RATE);
  if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
    fprintf(stderr, "[recipe-truehd] exception from sister factory\n");
    (*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);
    goto out;
  }
  if (blob == NULL) {
    fprintf(stderr, "[recipe-truehd] sister factory returned NULL (OOM "
            "branch) — construction fails\n");
    goto out;
  }
  blob_len = (*env)->GetArrayLength(env, blob);
  if (blob_len <= 0 || blob_len > 4096) {
    fprintf(stderr, "[recipe-truehd] sister blob length %d outside cereal "
            "domain (0,4096] — construction fails\n", (int)blob_len);
    goto out;
  }
  fprintf(stderr, "[recipe-truehd] sister blob len=%d (expected 125B, "
          "input-independent)\n", (int)blob_len);

  
  codec_name = (*env)->NewStringUTF(env, OMEN_ADEC_CODEC_NAME);
  if (codec_name == NULL ||
      (*env)->ExceptionCheck(env) == JNI_TRUE) {
    fprintf(stderr, "[recipe-truehd] NewStringUTF(\"%s\") failed\n",
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
    fprintf(stderr, "[recipe-truehd] exception from ffmpegInitialize\n");
    (*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);
    ctx = (jlong)0;
    goto out;
  }
  if (ctx == (jlong)0) {
    fprintf(stderr, "[recipe-truehd] factory returned 0 (codec not found / "
            "alloc failure / open2 failure) — construction fails, ctx==0 "
            "must never reach the target (runtime case)\n");
    goto out;
  }
  if ((ctx & 7) != 0) {
    fprintf(stderr, "[recipe-truehd] context %llx not 8-byte aligned\n",
            (unsigned long long)ctx);
    ctx = (jlong)0;
    goto out;
  }

  

  {
    const uint8_t* p = (const uint8_t*)(uintptr_t)ctx;
    int32_t codec_id, sample_rate, channels;
    int64_t ch_layout;
    memcpy(&codec_id, p + OMEN_ADEC_CTX_CODEC_ID_OFF, 4);
    memcpy(&sample_rate, p + OMEN_ADEC_CTX_SAMPLE_RATE_OFF, 4);
    memcpy(&channels, p + OMEN_ADEC_CTX_CHANNELS_OFF, 4);
    memcpy(&ch_layout, p + OMEN_ADEC_CTX_CH_LAYOUT_OFF, 8);
    fprintf(stderr, "[recipe-truehd] ctx %llx readback: codec_id=0x%x "
            "sample_rate=%d channels=%d channel_layout=%lld\n",
            (unsigned long long)ctx, codec_id, sample_rate, channels,
            (long long)ch_layout);
    if ((uint32_t)codec_id != OMEN_ADEC_CODEC_ID_TRUEHD) {
      fprintf(stderr, "[recipe-truehd] ctx->codec_id=0x%x != 0x%x (offset "
              "0x18, ffmpegReset branch discriminator :2247) — context "
              "would never enter the recreate arm\n", codec_id,
              OMEN_ADEC_CODEC_ID_TRUEHD);
      ctx = (jlong)0;
      goto out;
    }
    if (sample_rate != OMEN_ADEC_SAMPLE_RATE) {
      fprintf(stderr, "[recipe-truehd] ctx->sample_rate=%d != pinned %d "
              "(offset 0x190) — cross-target chain broken\n",
              sample_rate, OMEN_ADEC_SAMPLE_RATE);
      ctx = (jlong)0;
      goto out;
    }
    if (channels != OMEN_ADEC_CHANNELS) {
      fprintf(stderr, "[recipe-truehd] ctx->channels=%d != pinned %d "
              "(offset 0x194) — cross-target chain broken\n", channels,
              OMEN_ADEC_CHANNELS);
      ctx = (jlong)0;
      goto out;
    }
    if (ch_layout == 0) {
      fprintf(stderr, "[recipe-truehd] ctx->channel_layout==0 (offset 0x1b0)"
              " — default-layout fixup :7989-7992 did not apply\n");
      ctx = (jlong)0;
      goto out;
    }
  }

  *out_ctx = ctx;
  fprintf(stderr, "[recipe-truehd] FfmpegAudioDecoder context %llx verified "
          "(truehd, 6ch, 48000Hz, codecParametersData via sister 0x6E398, "
          "recreate arm armed)\n", (unsigned long long)ctx);
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



void destruct_FfmpegAudioDecoder_context_truehd(JNIEnv* env, jlong ctx) {
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
  fprintf(stderr, "[recipe-truehd] ffmpegRelease(%llx) done\n",
          (unsigned long long)ctx);
}
