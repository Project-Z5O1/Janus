




























#include <jni.h>
#include <android/log.h>

#define LOG_TAG "OmenRecipe.NativeSdk"

jobject construct_NativeSdk(JNIEnv* env,
                            const unsigned char* buffer,
                            size_t length,
                            size_t* offset) {
  const char* cls_desc = "com/xiaomi/scanner/nativecode/NativeSdk";
  const char* factory_sig = "()Lcom/xiaomi/scanner/nativecode/NativeSdk;";
  jclass cls = NULL;
  jmethodID mid = NULL;
  jobject instance = NULL;
  jobject result = NULL;

  (void)buffer; 
  (void)length;
  (void)offset; 

  if (env == NULL) {
    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "env is NULL");
    return NULL;
  }

  cls = (*env)->FindClass(env, cls_desc);
  if (cls == NULL || (*env)->ExceptionCheck(env) == JNI_TRUE) {
    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG,
                        "FindClass(%s) failed", cls_desc);
    if ((*env)->ExceptionCheck(env) == JNI_TRUE) (*env)->ExceptionClear(env);
    goto done;
  }

  mid = (*env)->GetStaticMethodID(env, cls, "getInstance", factory_sig);
  if (mid == NULL || (*env)->ExceptionCheck(env) == JNI_TRUE) {
    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG,
                        "GetStaticMethodID(getInstance %s) failed", factory_sig);
    if ((*env)->ExceptionCheck(env) == JNI_TRUE) (*env)->ExceptionClear(env);
    goto done;
  }

  instance = (*env)->CallStaticObjectMethod(env, cls, mid);
  if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG,
                        "getInstance() threw");
    (*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);
    goto done;
  }
  if (instance == NULL) {
    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG,
                        "getInstance() returned NULL (class init failed?)");
    goto done;
  }

  result = instance;
  instance = NULL; 

done:
  if (instance != NULL) (*env)->DeleteLocalRef(env, instance);
  if (cls != NULL) (*env)->DeleteLocalRef(env, cls);
  return result;
}
