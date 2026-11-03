










































#include <jni.h>
#include <stdint.h>
#include <stdio.h>

jobject construct_SipAccessNetwork(JNIEnv* env, const uint8_t* buffer,
                                   size_t length, size_t* offset) {
  jclass cls = NULL;
  jobject obj = NULL;
  jmethodID ctor;
  jfieldID fid;

  (void)length; 
  if (env == NULL || buffer == NULL || offset == NULL) {
    fprintf(stderr, "[recipe:SipAccessNetwork] invalid arguments\n");
    return NULL;
  }

  cls = (*env)->FindClass(env,
                          "com/cmdc/rcsprotocol/config/SipAccessNetwork");
  if (cls == NULL) {
    fprintf(stderr, "[recipe:SipAccessNetwork] FindClass failed\n");
    

    return NULL;
  }
  ctor = (*env)->GetMethodID(
      env, cls, "<init>",
      "(Landroid/net/NetworkInfo;Landroid/content/Context;)V");
  if (ctor == NULL) {
    fprintf(stderr, "[recipe:SipAccessNetwork] GetMethodID "
                    "<init>(Landroid/net/NetworkInfo;"
                    "Landroid/content/Context;)V failed\n");
    (*env)->DeleteLocalRef(env, cls);
    return NULL;
  }

  
  obj = (*env)->NewObject(env, cls, ctor, NULL, NULL);
  if (obj == NULL || (*env)->ExceptionCheck(env) == JNI_TRUE) {
    fprintf(stderr, "[recipe:SipAccessNetwork] NewObject failed\n");
    if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
      (*env)->ExceptionDescribe(env);
      (*env)->ExceptionClear(env);
    }
    (*env)->DeleteLocalRef(env, cls);
    return NULL;
  }

  fid = (*env)->GetFieldID(env, cls, "mStandaloneSwitchOverSize", "I");
  if (fid == NULL) {
    fprintf(stderr, "[recipe:SipAccessNetwork] GetFieldID "
                    "mStandaloneSwitchOverSize:I failed\n");
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return NULL;
  }
  (*env)->SetIntField(env, obj, fid, (jint)0);
  if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
    fprintf(stderr, "[recipe:SipAccessNetwork] SetIntField raised\n");
    (*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return NULL;
  }

  (*env)->DeleteLocalRef(env, cls);
  return obj;
}
