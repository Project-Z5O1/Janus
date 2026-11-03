




































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
    fprintf(stderr, "[recipe:SipNetwork] FAIL_CONSTRUCTION: %s needs u16 "
                    "length prefix\n", what);
    return NULL;
  }
  L = (size_t)buffer[*offset] | ((size_t)buffer[*offset + 1] << 8);
  *offset += 2;
  if (!need_bytes(length, *offset, L)) {
    fprintf(stderr, "[recipe:SipNetwork] FAIL_CONSTRUCTION: %s payload "
                    "declares %zu bytes, have %zu\n", what, L,
            length - *offset);
    return NULL;
  }
  tmp = (uint8_t*)malloc(L + 1);
  if (tmp == NULL) {
    fprintf(stderr, "[recipe:SipNetwork] OOM staging %s payload\n", what);
    return NULL;
  }
  memcpy(tmp, buffer + *offset, L);
  tmp[L] = 0;
  *offset += L;
  s = (*env)->NewStringUTF(env, (const char*)tmp);
  free(tmp);
  if (s == NULL || (*env)->ExceptionCheck(env) == JNI_TRUE) {
    fprintf(stderr, "[recipe:SipNetwork] NewStringUTF(%s) failed\n", what);
    if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
      (*env)->ExceptionDescribe(env);
      (*env)->ExceptionClear(env);
    }
    return NULL;
  }
  return s;
}

jobject construct_SipNetwork(JNIEnv* env, const uint8_t* buffer,
                             size_t length, size_t* offset) {
  jclass cls = NULL;
  jclass str_cls = NULL;
  jobjectArray dns = NULL;
  jstring* elems = NULL;
  jstring stun = NULL, ssl = NULL;
  jobject obj = NULL;
  size_t saved_offset;
  unsigned N;
  unsigned i;

  if (env == NULL || buffer == NULL || offset == NULL) {
    fprintf(stderr, "[recipe:SipNetwork] invalid arguments\n");
    return NULL;
  }

  
  saved_offset = *offset;
  if (!need_bytes(length, *offset, 1)) {
    fprintf(stderr, "[recipe:SipNetwork] FAIL_CONSTRUCTION: need u8 DNS "
                    "count\n");
    return NULL;
  }
  N = (unsigned)buffer[*offset];
  *offset += 1;
  if (N > 0) {
    elems = (jstring*)calloc(N, sizeof(jstring));
    if (elems == NULL) {
      fprintf(stderr, "[recipe:SipNetwork] OOM staging %u DNS elements\n", N);
      return NULL;
    }
  }
  for (i = 0; i < N; i++) {
    elems[i] = read_jstring(env, buffer, length, offset, "dns_elem");
    if (elems[i] == NULL) {
      goto consume_fail;
    }
  }
  stun = read_jstring(env, buffer, length, offset, "mStunServerIp");
  if (stun == NULL) goto consume_fail;
  ssl = read_jstring(env, buffer, length, offset, "mSslKey");
  if (ssl == NULL) goto consume_fail;

  

  str_cls = (*env)->FindClass(env, "java/lang/String");
  if (str_cls == NULL) {
    fprintf(stderr, "[recipe:SipNetwork] FindClass java/lang/String failed\n");
    goto consume_fail;
  }
  dns = (*env)->NewObjectArray(env, (jsize)N, str_cls, NULL);
  if (dns == NULL || (*env)->ExceptionCheck(env) == JNI_TRUE) {
    fprintf(stderr, "[recipe:SipNetwork] NewObjectArray(%u) failed\n", N);
    if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
      (*env)->ExceptionDescribe(env);
      (*env)->ExceptionClear(env);
    }
    goto consume_fail;
  }
  for (i = 0; i < N; i++) {
    (*env)->SetObjectArrayElement(env, dns, (jsize)i, elems[i]);
    if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
      fprintf(stderr, "[recipe:SipNetwork] SetObjectArrayElement[%u]\n", i);
      (*env)->ExceptionDescribe(env);
      (*env)->ExceptionClear(env);
      goto build_fail;
    }
  }

  
  cls = (*env)->FindClass(env, "com/cmdc/rcsprotocol/config/SipNetwork");
  if (cls == NULL) {
    fprintf(stderr, "[recipe:SipNetwork] FindClass failed\n");
    goto build_fail; 
  }
  obj = (*env)->NewObject(env, cls,
                          (*env)->GetMethodID(env, cls, "<init>",
                                              "([Ljava/lang/String;)V"),
                          dns);
  if (obj == NULL || (*env)->ExceptionCheck(env) == JNI_TRUE) {
    fprintf(stderr, "[recipe:SipNetwork] NewObject(<init>([Ljava/lang/String;)V) "
                    "failed\n");
    if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
      (*env)->ExceptionDescribe(env);
      (*env)->ExceptionClear(env);
    }
    goto build_fail;
  }

  


  {
    jfieldID f;
    f = (*env)->GetFieldID(env, cls, "mStunServerIp", "Ljava/lang/String;");
    if (f == NULL) goto build_fail;
    (*env)->SetObjectField(env, obj, f, stun);
    if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
      (*env)->ExceptionDescribe(env);
      (*env)->ExceptionClear(env);
      goto build_fail;
    }
    f = (*env)->GetFieldID(env, cls, "mSslKey", "Ljava/lang/String;");
    if (f == NULL) goto build_fail;
    (*env)->SetObjectField(env, obj, f, ssl);
    if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
      (*env)->ExceptionDescribe(env);
      (*env)->ExceptionClear(env);
      goto build_fail;
    }
  }

  if (elems != NULL) {
    for (i = 0; i < N; i++) (*env)->DeleteLocalRef(env, elems[i]);
    free(elems);
  }
  (*env)->DeleteLocalRef(env, stun);
  (*env)->DeleteLocalRef(env, ssl);
  (*env)->DeleteLocalRef(env, dns);
  (*env)->DeleteLocalRef(env, str_cls);
  (*env)->DeleteLocalRef(env, cls);
  return obj;

consume_fail:
  *offset = saved_offset; 
  
build_fail:
  if (elems != NULL) {
    for (i = 0; i < N; i++)
      if (elems[i] != NULL) (*env)->DeleteLocalRef(env, elems[i]);
    free(elems);
  }
  if (stun != NULL) (*env)->DeleteLocalRef(env, stun);
  if (ssl != NULL) (*env)->DeleteLocalRef(env, ssl);
  if (dns != NULL) (*env)->DeleteLocalRef(env, dns);
  if (str_cls != NULL) (*env)->DeleteLocalRef(env, str_cls);
  if (cls != NULL) (*env)->DeleteLocalRef(env, cls);
  if (obj != NULL) (*env)->DeleteLocalRef(env, obj);
  return NULL;
}
