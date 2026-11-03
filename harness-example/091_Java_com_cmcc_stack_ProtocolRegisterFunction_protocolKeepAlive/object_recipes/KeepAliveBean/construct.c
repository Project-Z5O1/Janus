



























#include <jni.h>
#include <stdint.h>
#include <stdio.h>

jobject construct_KeepAliveBean(JNIEnv* env, const uint8_t* buffer,
                                size_t length, size_t* offset) {
  jclass cls = NULL;
  jmethodID ctor = NULL;
  jfieldID fid = NULL;
  jobject bean = NULL;
  jint check_ping_timeout_ms = 0;

  if (env == NULL || buffer == NULL || offset == NULL) {
    fprintf(stderr, "[recipe:KeepAliveBean] invalid arguments\n");
    return NULL;
  }

  
  if (length < *offset || (length - *offset) < 4) {
    fprintf(stderr,
            "[recipe:KeepAliveBean] FAIL_CONSTRUCTION: need 4 bytes for "
            "mCheckPingTimeoutMs, have %zu\n",
            (length >= *offset) ? (length - *offset) : (size_t)0);
    return NULL;
  }
  check_ping_timeout_ms =
      (jint)((uint32_t)buffer[*offset] |
             ((uint32_t)buffer[*offset + 1] << 8) |
             ((uint32_t)buffer[*offset + 2] << 16) |
             ((uint32_t)buffer[*offset + 3] << 24));
  *offset += 4;

  cls = (*env)->FindClass(env, "com/cmdc/rcsprotocol/bean/KeepAliveBean");
  if (cls == NULL) {
    fprintf(stderr, "[recipe:KeepAliveBean] FindClass failed\n");
    return NULL; 

  }
  ctor = (*env)->GetMethodID(env, cls, "<init>", "(II)V");
  if (ctor == NULL) {
    fprintf(stderr, "[recipe:KeepAliveBean] GetMethodID <init>(II)V failed\n");
    (*env)->DeleteLocalRef(env, cls);
    return NULL;
  }

  




  bean = (*env)->NewObject(env, cls, ctor, (jint)0, (jint)0);
  if (bean == NULL || (*env)->ExceptionCheck(env) == JNI_TRUE) {
    fprintf(stderr, "[recipe:KeepAliveBean] NewObject failed\n");
    if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
      (*env)->ExceptionDescribe(env);
      (*env)->ExceptionClear(env);
    }
    if (cls != NULL) (*env)->DeleteLocalRef(env, cls);
    return NULL;
  }

  


  fid = (*env)->GetFieldID(env, cls, "mCheckPingTimeoutMs", "I");
  if (fid == NULL) {
    fprintf(stderr, "[recipe:KeepAliveBean] GetFieldID mCheckPingTimeoutMs:I "
                    "failed\n");
    (*env)->DeleteLocalRef(env, bean);
    (*env)->DeleteLocalRef(env, cls);
    return NULL;
  }
  (*env)->SetIntField(env, bean, fid, check_ping_timeout_ms);
  if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
    fprintf(stderr, "[recipe:KeepAliveBean] SetIntField raised\n");
    (*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);
    (*env)->DeleteLocalRef(env, bean);
    (*env)->DeleteLocalRef(env, cls);
    return NULL;
  }

  (*env)->DeleteLocalRef(env, cls);
  return bean;
}
