


























#include <jni.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


#define RECIPE_TAG "[RECIPE] CONSTRUCT_FAIL(android.graphics.Bitmap)"

static void recipe_log(const char* msg)
{
  fprintf(stderr, "%s %s\n", RECIPE_TAG, msg);
  fflush(stderr);
}






jobject construct_android_graphics_Bitmap(JNIEnv* env,
                                          jint width, jint height,
                                          const uint8_t* pixels,
                                          size_t pixel_len)
{
  jclass bitmapClass = NULL;
  jclass configClass = NULL;
  jobject configArgb8888 = NULL;
  jobject bitmap = NULL;
  jobject byteBuffer = NULL;
  jfieldID argbField = NULL;
  jmethodID createBitmapMid = NULL;
  jmethodID copyMid = NULL;
  jboolean exceptionRaised;
  size_t need;

  if (env == NULL || pixels == NULL) {
    recipe_log("env==NULL or pixels==NULL");
    return NULL;
  }

  



  if (width < 1 || height < 1) {
    recipe_log("width<1 or height<1 (construction-axis precondition)");
    return NULL;
  }

  


  need = (size_t)width * (size_t)height * 4u;
  if (pixel_len < need) {
    fprintf(stderr, "%s pixel_len=%zu < width*height*4=%zu\n",
            RECIPE_TAG, pixel_len, need);
    fflush(stderr);
    return NULL;
  }

  bitmapClass = (*env)->FindClass(env, "android/graphics/Bitmap");
  if (bitmapClass == NULL || (*env)->ExceptionCheck(env) == JNI_TRUE) {
    recipe_log("FindClass(android/graphics/Bitmap)");
    goto fail;
  }

  configClass = (*env)->FindClass(env, "android/graphics/Bitmap$Config");
  if (configClass == NULL || (*env)->ExceptionCheck(env) == JNI_TRUE) {
    recipe_log("FindClass(android/graphics/Bitmap$Config)");
    goto fail;
  }

  argbField = (*env)->GetStaticFieldID(env, configClass, "ARGB_8888",
                                       "Landroid/graphics/Bitmap$Config;");
  if (argbField == NULL || (*env)->ExceptionCheck(env) == JNI_TRUE) {
    recipe_log("GetStaticFieldID(Config.ARGB_8888)");
    goto fail;
  }
  configArgb8888 = (*env)->GetStaticObjectField(env, configClass, argbField);
  if (configArgb8888 == NULL ||
      (*env)->ExceptionCheck(env) == JNI_TRUE) {
    recipe_log("GetStaticObjectField(Config.ARGB_8888)");
    goto fail;
  }

  


  createBitmapMid = (*env)->GetStaticMethodID(env, bitmapClass, "createBitmap",
      "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
  if (createBitmapMid == NULL || (*env)->ExceptionCheck(env) == JNI_TRUE) {
    recipe_log("GetStaticMethodID(createBitmap(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;)");
    goto fail;
  }

  bitmap = (*env)->CallStaticObjectMethod(env, bitmapClass, createBitmapMid,
                                          width, height, configArgb8888);
  exceptionRaised = (*env)->ExceptionCheck(env);
  if (exceptionRaised == JNI_TRUE) {
    recipe_log("CallStaticObjectMethod(createBitmap) threw");
    goto fail;
  }
  if (bitmap == NULL) {
    recipe_log("createBitmap returned NULL without exception");
    goto fail;
  }

  




  byteBuffer = (*env)->NewDirectByteBuffer(env, (void*)pixels, (jlong)need);
  if (byteBuffer == NULL || (*env)->ExceptionCheck(env) == JNI_TRUE) {
    recipe_log("NewDirectByteBuffer(payload)");
    goto fail;
  }

  copyMid = (*env)->GetMethodID(env, bitmapClass, "copyPixelsFromBuffer",
                                "(Ljava/nio/Buffer;)V");
  if (copyMid == NULL || (*env)->ExceptionCheck(env) == JNI_TRUE) {
    recipe_log("GetMethodID(copyPixelsFromBuffer(Ljava/nio/Buffer;)V)");
    goto fail;
  }

  (*env)->CallVoidMethod(env, bitmap, copyMid, byteBuffer);
  if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
    


    recipe_log("copyPixelsFromBuffer threw");
    goto fail;
  }

  

  (*env)->DeleteLocalRef(env, byteBuffer);
  (*env)->DeleteLocalRef(env, configArgb8888);
  (*env)->DeleteLocalRef(env, configClass);
  (*env)->DeleteLocalRef(env, bitmapClass);
  return bitmap;

fail:
  if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
    (*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);
  }
  if (byteBuffer != NULL) {
    (*env)->DeleteLocalRef(env, byteBuffer);
  }
  if (bitmap != NULL) {
    (*env)->DeleteLocalRef(env, bitmap);
  }
  if (configArgb8888 != NULL) {
    (*env)->DeleteLocalRef(env, configArgb8888);
  }
  if (configClass != NULL) {
    (*env)->DeleteLocalRef(env, configClass);
  }
  if (bitmapClass != NULL) {
    (*env)->DeleteLocalRef(env, bitmapClass);
  }
  return NULL;
}
