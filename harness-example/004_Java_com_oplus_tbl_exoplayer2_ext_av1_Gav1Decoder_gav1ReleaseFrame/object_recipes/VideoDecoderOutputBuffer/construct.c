








































#include <jni.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

 jobject construct_VideoDecoderOutputBuffer(JNIEnv* env,
                                            const uint8_t* buffer,
                                            size_t length,
                                            size_t* offset) {
  (void)buffer;
  (void)length;
  (void)offset; 

  jclass ownerPayloadClass = NULL;
  jobject ownerPayload = NULL;
  jclass ownerClass = NULL;
  jmethodID ownerCtor = NULL;
  jobject owner = NULL;
  jclass bufferClass = NULL;
  jmethodID bufferCtor = NULL;
  jobject out = NULL;
  jfieldID modeField = NULL;

  
  ownerPayloadClass = (*env)->FindClass(env, "java/lang/Object");
  if (ownerPayloadClass == NULL) {
    fprintf(stderr, "[recipe] FindClass(java/lang/Object) failed\n");
    goto fail;
  }
  ownerPayload = (*env)->NewObject(env, ownerPayloadClass,
                                   (*env)->GetMethodID(env, ownerPayloadClass,
                                                       "<init>", "()V"));
  if (ownerPayload == NULL || (*env)->ExceptionCheck(env) == JNI_TRUE) {
    fprintf(stderr, "[recipe] NewObject(java/lang/Object) failed\n");
    goto fail;
  }

  
  ownerClass = (*env)->FindClass(env, "q8/e");
  if (ownerClass == NULL) {
    fprintf(stderr, "[recipe] FindClass(q8/e) failed\n");
    goto fail;
  }
  ownerCtor = (*env)->GetMethodID(env, ownerClass, "<init>",
                                  "(Ljava/lang/Object;)V");
  if (ownerCtor == NULL) {
    fprintf(stderr, "[recipe] GetMethodID(q8/e.<init>) failed\n");
    goto fail;
  }
  owner = (*env)->NewObject(env, ownerClass, ownerCtor, ownerPayload);
  if (owner == NULL || (*env)->ExceptionCheck(env) == JNI_TRUE) {
    fprintf(stderr, "[recipe] NewObject(q8/e) failed\n");
    goto fail;
  }

  
  bufferClass = (*env)->FindClass(
      env, "com/oplus/tbl/exoplayer2/video/VideoDecoderOutputBuffer");
  if (bufferClass == NULL) {
    fprintf(stderr,
            "[recipe] FindClass(VideoDecoderOutputBuffer) failed\n");
    goto fail;
  }
  bufferCtor = (*env)->GetMethodID(
      env, bufferClass, "<init>",
      "(Lcom/oplus/tbl/exoplayer2/decoder/OutputBuffer$Owner;)V");
  if (bufferCtor == NULL) {
    fprintf(stderr, "[recipe] GetMethodID(VideoDecoderOutputBuffer.<init>) "
                    "failed\n");
    goto fail;
  }
  out = (*env)->NewObject(env, bufferClass, bufferCtor, owner);
  if (out == NULL || (*env)->ExceptionCheck(env) == JNI_TRUE) {
    fprintf(stderr, "[recipe] NewObject(VideoDecoderOutputBuffer) failed\n");
    goto fail;
  }
  (*env)->DeleteLocalRef(env, owner);
  owner = NULL;
  (*env)->DeleteLocalRef(env, ownerPayload);
  ownerPayload = NULL;

  


  modeField = (*env)->GetFieldID(env, bufferClass, "mode", "I");
  if (modeField == NULL) {
    fprintf(stderr, "[recipe] GetFieldID(mode) failed\n");
    goto fail;
  }
  (*env)->SetIntField(env, out, modeField, 0);
  if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
    fprintf(stderr, "[recipe] SetIntField(mode) raised\n");
    goto fail;
  }

  (*env)->DeleteLocalRef(env, bufferClass);
  return out;

fail:
  if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
    (*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);
  }
  if (owner != NULL) (*env)->DeleteLocalRef(env, owner);
  if (ownerPayload != NULL) (*env)->DeleteLocalRef(env, ownerPayload);
  if (out != NULL) (*env)->DeleteLocalRef(env, out);
  if (bufferClass != NULL) (*env)->DeleteLocalRef(env, bufferClass);
  if (ownerClass != NULL) (*env)->DeleteLocalRef(env, ownerClass);
  if (ownerPayloadClass != NULL)
    (*env)->DeleteLocalRef(env, ownerPayloadClass);
  return NULL;
}
