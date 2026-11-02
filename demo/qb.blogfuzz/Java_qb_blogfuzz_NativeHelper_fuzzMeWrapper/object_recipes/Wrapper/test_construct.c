#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jenv.h"

static JavaCTX ctx;

jobject construct_Wrapper(JNIEnv* env, const uint8_t* buffer, size_t length, size_t* offset) {
  size_t remaining = (length > *offset) ? (length - *offset) : 0;
  if (remaining == 0) {
    remaining = 1;
    if (length == 0) {
      static const uint8_t dummy = 0;
      jbyteArray jBuffer = (*env)->NewByteArray(env, 1);
      (*env)->SetByteArrayRegion(env, jBuffer, 0, 1, (const jbyte*)&dummy);

      jclass cls = (*env)->FindClass(env, "qb/blogfuzz/Wrapper");
      if (!cls) { (*env)->DeleteLocalRef(env, jBuffer); return NULL; }

      jmethodID ctor = (*env)->GetMethodID(env, cls, "<init>", "([B)V");
      if (!ctor) { (*env)->DeleteLocalRef(env, cls); (*env)->DeleteLocalRef(env, jBuffer); return NULL; }

      jobject obj = (*env)->NewObject(env, cls, ctor, jBuffer);
      (*env)->DeleteLocalRef(env, cls);
      (*env)->DeleteLocalRef(env, jBuffer);
      return obj;
    }
  }

  jbyteArray jBuffer = (*env)->NewByteArray(env, remaining);
  if (!jBuffer) return NULL;
  (*env)->SetByteArrayRegion(env, jBuffer, 0, remaining, (const jbyte*)(buffer + *offset));
  *offset += remaining;

  jclass cls = (*env)->FindClass(env, "qb/blogfuzz/Wrapper");
  if (!cls) { (*env)->DeleteLocalRef(env, jBuffer); return NULL; }

  jmethodID ctor = (*env)->GetMethodID(env, cls, "<init>", "([B)V");
  if (!ctor) { (*env)->DeleteLocalRef(env, cls); (*env)->DeleteLocalRef(env, jBuffer); return NULL; }

  jobject obj = (*env)->NewObject(env, cls, ctor, jBuffer);
  (*env)->DeleteLocalRef(env, cls);
  (*env)->DeleteLocalRef(env, jBuffer);
  return obj;
}

int main(void) {
  int status;
  uint8_t buffer[64];
  char* options = "-Djava.class.path=./base.apk";
  ssize_t rlength;

  rlength = fread(buffer, 1, sizeof(buffer), stdin);
  if (rlength == -1) return 1;

  if ((status = init_java_env(&ctx, &options, 1)) != 0) {
    fprintf(stderr, "[RECIPE] init_java_env failed: %d\n", status);
    return status;
  }

  size_t offset = 0;
  jobject wrapper = construct_Wrapper(ctx.env, buffer, (size_t)rlength, &offset);
  if (wrapper) {
    fprintf(stderr, "[RECIPE] construct_Wrapper succeeded\n");
    (*ctx.env)->DeleteLocalRef(ctx.env, wrapper);
  } else {
    fprintf(stderr, "[RECIPE] construct_Wrapper returned NULL\n");
    return 1;
  }

  return 0;
}
