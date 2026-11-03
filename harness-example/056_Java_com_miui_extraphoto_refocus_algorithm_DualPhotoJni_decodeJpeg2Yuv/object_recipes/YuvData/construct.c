
#include <jni.h>
#include <stdint.h>
#include <stdio.h>

jobject construct_YuvData(JNIEnv* env, const uint8_t* buffer, size_t length,
                          size_t* offset) {
  jclass cls;
  jmethodID ctor;
  jobject obj;

  (void)buffer; 
  (void)length;
  (void)offset; 

  cls = (*env)->FindClass(env, "com/miui/extraphoto/refocus/model/YuvData");
  if (cls == NULL) {
    fprintf(stderr, "[recipe:YuvData] FindClass failed\n");
    if ((*env)->ExceptionCheck(env) != JNI_FALSE) {
      (*env)->ExceptionDescribe(env);
      (*env)->ExceptionClear(env);
    }
    return NULL;
  }

  ctor = (*env)->GetMethodID(env, cls, "<init>", "(JIII)V");
  if (ctor == NULL) {
    fprintf(stderr, "[recipe:YuvData] GetMethodID <init>(JIII)V failed\n");
    if ((*env)->ExceptionCheck(env) != JNI_FALSE) {
      (*env)->ExceptionDescribe(env);
      (*env)->ExceptionClear(env);
    }
    (*env)->DeleteLocalRef(env, cls);
    return NULL;
  }

  
  obj = (*env)->NewObject(env, cls, ctor, (jlong)-1, (jint)-1, (jint)-1,
                          (jint)0);
  (*env)->DeleteLocalRef(env, cls);
  if ((*env)->ExceptionCheck(env) != JNI_FALSE) {
    fprintf(stderr, "[recipe:YuvData] NewObject raised exception\n");
    (*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);
    return NULL;
  }
  if (obj == NULL) {
    fprintf(stderr, "[recipe:YuvData] NewObject returned NULL\n");
    return NULL;
  }
  return obj; 
}
