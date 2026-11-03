

#include <jni.h>
#include <stdint.h>
#include <stdio.h>
#include <dlfcn.h>

#include "jenv.h" 

#define OMEN_PCM_LIB "libffmpegJNI.so"
#define OMEN_PCM_CREATE_CTX_OFFSET 0x79C78u
#define OMEN_PCM_DESTROY_CTX_OFFSET 0x79CC8u 
#define OMEN_PCM_IN_SAMPLE_SIZE 3            
#define OMEN_PCM_OUT_SAMPLE_SIZE 2           

typedef jlong (*omen_pcm_create_ctx_fn)(JNIEnv*, jobject, jint, jint);
typedef void (*omen_pcm_destroy_ctx_fn)(JNIEnv*, jobject, jlong);

int construct_PCMConvertor_context(JNIEnv* env, jlong* out_ctx) {
  if (out_ctx == NULL) {
    fprintf(stderr, "[recipe] out_ctx is NULL\n");
    return -1;
  }
  *out_ctx = (jlong)0;

  
  void* lib = dlopen(OMEN_PCM_LIB, RTLD_NOW);
  if (lib == NULL) {
    fprintf(stderr, "[recipe] dlopen(%s) failed: %s\n", OMEN_PCM_LIB,
            dlerror());
    return -1;
  }

  void* base = get_library_base(OMEN_PCM_LIB);
  if (base == NULL) {
    fprintf(stderr, "[recipe] get_library_base(%s) failed\n", OMEN_PCM_LIB);
    return -1;
  }

  omen_pcm_create_ctx_fn factory =
      (omen_pcm_create_ctx_fn)((uintptr_t)base + OMEN_PCM_CREATE_CTX_OFFSET);
  fprintf(stderr, "[recipe] pcmCreateContext = %s base %p + 0x%lx\n",
          OMEN_PCM_LIB, base, (unsigned long)OMEN_PCM_CREATE_CTX_OFFSET);

  jlong ctx = factory(env, NULL, (jint)OMEN_PCM_IN_SAMPLE_SIZE,
                      (jint)OMEN_PCM_OUT_SAMPLE_SIZE);
  if (ctx == (jlong)0) {
    fprintf(stderr,
            "[recipe] factory returned 0 (av_mallocz failure path, "
            "libffmpegJNI.c:12122) — construction fails, ctx==0 must never "
            "reach the target\n");
    return -1;
  }
  if ((ctx & 7) != 0) {
    fprintf(stderr, "[recipe] context %llx not 8-byte aligned\n",
            (unsigned long long)ctx);
    return -1;
  }

  const int32_t* fields = (const int32_t*)(uintptr_t)ctx;
  if (fields[0] != OMEN_PCM_IN_SAMPLE_SIZE ||
      fields[1] != OMEN_PCM_OUT_SAMPLE_SIZE) {
    fprintf(stderr,
            "[recipe] guard mismatch: ctx={%d,%d} expected {3,2} "
            "(target loop guard libffmpegJNI.c:12165 would reject every "
            "case)\n",
            fields[0], fields[1]);
    return -1;
  }

  *out_ctx = ctx;
  fprintf(stderr, "[recipe] PCMConvertor context %llx verified {3,2}\n",
          (unsigned long long)ctx);
  return 0;
}





void destruct_PCMConvertor_context(JNIEnv* env, jlong ctx) {
  if (ctx == (jlong)0) {
    return;
  }
  void* base = get_library_base(OMEN_PCM_LIB);
  if (base == NULL) {
    return;
  }
  omen_pcm_destroy_ctx_fn destroy =
      (omen_pcm_destroy_ctx_fn)((uintptr_t)base + OMEN_PCM_DESTROY_CTX_OFFSET);
  destroy(env, NULL, ctx);
}
