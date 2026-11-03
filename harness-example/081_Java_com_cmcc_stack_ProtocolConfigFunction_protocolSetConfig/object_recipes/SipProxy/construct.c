






































#include <jni.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int need_bytes(size_t length, size_t offset, size_t n) {
  return (length >= offset) && ((length - offset) >= n);
}


static jstring read_jstring(JNIEnv* env, const uint8_t* buffer, size_t length,
                            size_t* offset, const char* what) {
  size_t L;
  uint8_t* tmp;
  jstring s;

  if (!need_bytes(length, *offset, 2)) {
    fprintf(stderr, "[recipe:SipProxy] FAIL_CONSTRUCTION: %s needs u16 "
                    "length prefix\n", what);
    return NULL;
  }
  L = (size_t)buffer[*offset] | ((size_t)buffer[*offset + 1] << 8);
  *offset += 2;
  if (!need_bytes(length, *offset, L)) {
    fprintf(stderr, "[recipe:SipProxy] FAIL_CONSTRUCTION: %s payload declares "
                    "%zu bytes, have %zu\n", what, L, length - *offset);
    return NULL;
  }
  tmp = (uint8_t*)malloc(L + 1);
  if (tmp == NULL) {
    fprintf(stderr, "[recipe:SipProxy] OOM staging %s payload\n", what);
    return NULL;
  }
  memcpy(tmp, buffer + *offset, L);
  tmp[L] = 0;
  *offset += L;
  s = (*env)->NewStringUTF(env, (const char*)tmp);
  free(tmp);
  if (s == NULL || (*env)->ExceptionCheck(env) == JNI_TRUE) {
    fprintf(stderr, "[recipe:SipProxy] NewStringUTF(%s) failed\n", what);
    if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
      (*env)->ExceptionDescribe(env);
      (*env)->ExceptionClear(env);
    }
    return NULL;
  }
  return s;
}

jobject construct_SipProxy(JNIEnv* env, const uint8_t* buffer,
                           size_t length, size_t* offset) {
  jstring addr = NULL, dom = NULL, trans = NULL;
  jclass cls = NULL;
  jobject obj = NULL;
  size_t saved_offset = 0;

  if (env == NULL || buffer == NULL || offset == NULL) {
    fprintf(stderr, "[recipe:SipProxy] invalid arguments\n");
    return NULL;
  }
  saved_offset = *offset;

  
  addr = read_jstring(env, buffer, length, offset, "address");
  if (addr == NULL) goto consume_fail;
  dom = read_jstring(env, buffer, length, offset, "imsDomain");
  if (dom == NULL) goto consume_fail;
  trans = read_jstring(env, buffer, length, offset, "transport");
  if (trans == NULL) goto consume_fail;

  

  cls = (*env)->FindClass(env, "com/cmdc/rcsprotocol/config/SipProxy");
  if (cls == NULL) {
    fprintf(stderr, "[recipe:SipProxy] FindClass failed\n");
    goto consume_fail; 
  }
  obj = (*env)->NewObject(env, cls,
                          (*env)->GetMethodID(env, cls, "<init>",
                                              "(Ljava/lang/String;"
                                              "Ljava/lang/String;"
                                              "Ljava/lang/String;)V"),
                          addr, dom, trans);
  if (obj == NULL || (*env)->ExceptionCheck(env) == JNI_TRUE) {
    

    fprintf(stderr, "[recipe:SipProxy] NewObject(<init>(String,String,String)V) "
                    "failed (ctor parse semantics)\n");
    if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
      (*env)->ExceptionDescribe(env);
      (*env)->ExceptionClear(env);
    }
    goto build_fail;
  }

  (*env)->DeleteLocalRef(env, addr);
  (*env)->DeleteLocalRef(env, dom);
  (*env)->DeleteLocalRef(env, trans);
  (*env)->DeleteLocalRef(env, cls);
  return obj;

consume_fail:
  *offset = saved_offset; 
build_fail:
  if (addr != NULL) (*env)->DeleteLocalRef(env, addr);
  if (dom != NULL) (*env)->DeleteLocalRef(env, dom);
  if (trans != NULL) (*env)->DeleteLocalRef(env, trans);
  if (cls != NULL) (*env)->DeleteLocalRef(env, cls);
  if (obj != NULL) (*env)->DeleteLocalRef(env, obj);
  return NULL;
}
