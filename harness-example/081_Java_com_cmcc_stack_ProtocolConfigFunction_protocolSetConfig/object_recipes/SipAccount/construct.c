



































#include <jni.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIPACCOUNT_STRING_FIELDS 12

static int need_bytes(size_t length, size_t offset, size_t n) {
  return (length >= offset) && ((length - offset) >= n);
}


static jstring read_jstring(JNIEnv* env, const uint8_t* buffer, size_t length,
                            size_t* offset, const char* what) {
  size_t L;
  uint8_t* tmp;
  jstring s;

  if (!need_bytes(length, *offset, 2)) {
    fprintf(stderr, "[recipe:SipAccount] FAIL_CONSTRUCTION: %s needs u16 "
                    "length prefix, have %zu bytes\n", what,
            need_bytes(length, *offset, 1) ? (size_t)1 : (size_t)0);
    return NULL;
  }
  L = (size_t)buffer[*offset] | ((size_t)buffer[*offset + 1] << 8);
  *offset += 2;
  if (!need_bytes(length, *offset, L)) {
    fprintf(stderr, "[recipe:SipAccount] FAIL_CONSTRUCTION: %s payload "
                    "declares %zu bytes, have %zu\n", what, L,
            length - *offset);
    return NULL;
  }
  tmp = (uint8_t*)malloc(L + 1);
  if (tmp == NULL) {
    fprintf(stderr, "[recipe:SipAccount] OOM staging %s payload\n", what);
    return NULL;
  }
  memcpy(tmp, buffer + *offset, L);
  tmp[L] = 0;
  *offset += L;
  s = (*env)->NewStringUTF(env, (const char*)tmp);
  free(tmp);
  if (s == NULL || (*env)->ExceptionCheck(env) == JNI_TRUE) {
    fprintf(stderr, "[recipe:SipAccount] NewStringUTF(%s) failed\n", what);
    if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
      (*env)->ExceptionDescribe(env);
      (*env)->ExceptionClear(env);
    }
    return NULL;
  }
  return s;
}

jobject construct_SipAccount(JNIEnv* env, const uint8_t* buffer,
                             size_t length, size_t* offset) {
  static const char* kFields[SIPACCOUNT_STRING_FIELDS] = {
    "mPublicUserIdentity3", "mPublicUserIdentity2", "mPublicUserIdentity1",
    "mAuthUserName", "mAuthRealm", "mAuthUserPwd", "mConfFctyUri",
    "mPublicAccountAddr", "mMassFctyUri", "mAgentName",
    "mPublicUserIdentityPc", "mHomeNetworkDomainName"
  };
  static const char* kBools[5] = {
    "mChatAuth", "mGroupChatAuth", "mFtAuth", "mStandaloneMsgAuth",
    "mGeolocPushAuth"
  };
  jstring vals[SIPACCOUNT_STRING_FIELDS];
  jclass cls = NULL;
  jobject obj = NULL;
  size_t consumed_at[SIPACCOUNT_STRING_FIELDS];
  size_t saved_offset;
  int i;

  if (env == NULL || buffer == NULL || offset == NULL) {
    fprintf(stderr, "[recipe:SipAccount] invalid arguments\n");
    return NULL;
  }

  

  saved_offset = *offset;
  for (i = 0; i < SIPACCOUNT_STRING_FIELDS; i++) {
    consumed_at[i] = *offset;
    vals[i] = read_jstring(env, buffer, length, offset, kFields[i]);
    if (vals[i] == NULL) {
      int j;
      for (j = 0; j < i; j++) (*env)->DeleteLocalRef(env, vals[j]);
      *offset = saved_offset; 
      return NULL;
    }
  }
  (void)consumed_at;

  

  cls = (*env)->FindClass(env, "com/cmdc/rcsprotocol/config/SipAccount");
  if (cls == NULL) {
    fprintf(stderr, "[recipe:SipAccount] FindClass failed\n");
    

    for (i = 0; i < SIPACCOUNT_STRING_FIELDS; i++)
      (*env)->DeleteLocalRef(env, vals[i]);
    return NULL;
  }
  obj = (*env)->NewObject(env, cls, (*env)->GetMethodID(env, cls, "<init>",
                                                        "()V"));
  if (obj == NULL || (*env)->ExceptionCheck(env) == JNI_TRUE) {
    fprintf(stderr, "[recipe:SipAccount] NewObject failed\n");
    if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
      (*env)->ExceptionDescribe(env);
      (*env)->ExceptionClear(env);
    }
    goto fail;
  }

  

  for (i = 0; i < SIPACCOUNT_STRING_FIELDS; i++) {
    jfieldID fid = (*env)->GetFieldID(env, cls, kFields[i],
                                      "Ljava/lang/String;");
    if (fid == NULL) {
      fprintf(stderr, "[recipe:SipAccount] GetFieldID %s:String failed\n",
              kFields[i]);
      goto fail;
    }
    (*env)->SetObjectField(env, obj, fid, vals[i]);
    if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
      fprintf(stderr, "[recipe:SipAccount] SetObjectField %s raised\n",
              kFields[i]);
      (*env)->ExceptionDescribe(env);
      (*env)->ExceptionClear(env);
      goto fail;
    }
  }
  for (i = 0; i < 5; i++) {
    jfieldID fid = (*env)->GetFieldID(env, cls, kBools[i], "Z");
    if (fid == NULL) {
      fprintf(stderr, "[recipe:SipAccount] GetFieldID %s:Z failed\n",
              kBools[i]);
      goto fail;
    }
    (*env)->SetBooleanField(env, obj, fid, (jboolean)0);
    if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
      fprintf(stderr, "[recipe:SipAccount] SetBooleanField %s raised\n",
              kBools[i]);
      (*env)->ExceptionDescribe(env);
      (*env)->ExceptionClear(env);
      goto fail;
    }
  }

  for (i = 0; i < SIPACCOUNT_STRING_FIELDS; i++)
    (*env)->DeleteLocalRef(env, vals[i]);
  (*env)->DeleteLocalRef(env, cls);
  return obj;

fail:
  for (i = 0; i < SIPACCOUNT_STRING_FIELDS; i++)
    (*env)->DeleteLocalRef(env, vals[i]);
  if (obj != NULL) (*env)->DeleteLocalRef(env, obj);
  (*env)->DeleteLocalRef(env, cls);
  return NULL;
}
