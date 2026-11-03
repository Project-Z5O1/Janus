
#include <jni.h>
#include <stdint.h>
#include <stdio.h>

jobject construct_YuvData_input(JNIEnv* env, jlong real_ptr, jint w, jint h) {
  jclass cls;
  jmethodID ctor;
  jobject obj;

  if (real_ptr == 0) {
    


    fprintf(stderr,
            "[recipe:YuvData-input] refused: real_ptr==0 (sentinel/NULL form "
            "is forbidden for input carriers)\n");
    return NULL;
  }

  cls = (*env)->FindClass(env, "com/miui/extraphoto/refocus/model/YuvData");
  if (cls == NULL) {
    fprintf(stderr, "[recipe:YuvData-input] FindClass failed\n");
    if ((*env)->ExceptionCheck(env) != JNI_FALSE) {
      (*env)->ExceptionDescribe(env);
      (*env)->ExceptionClear(env);
    }
    return NULL;
  }

  ctor = (*env)->GetMethodID(env, cls, "<init>", "(JIII)V");
  if (ctor == NULL) {
    fprintf(stderr, "[recipe:YuvData-input] GetMethodID <init>(JIII)V failed\n");
    if ((*env)->ExceptionCheck(env) != JNI_FALSE) {
      (*env)->ExceptionDescribe(env);
      (*env)->ExceptionClear(env);
    }
    (*env)->DeleteLocalRef(env, cls);
    return NULL;
  }

  

  obj = (*env)->NewObject(env, cls, ctor, real_ptr, w, h, (jint)0);
  (*env)->DeleteLocalRef(env, cls);
  if ((*env)->ExceptionCheck(env) != JNI_FALSE) {
    fprintf(stderr, "[recipe:YuvData-input] NewObject raised exception\n");
    (*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);
    return NULL;
  }
  if (obj == NULL) {
    fprintf(stderr, "[recipe:YuvData-input] NewObject returned NULL\n");
    return NULL;
  }
  return obj; 
}
