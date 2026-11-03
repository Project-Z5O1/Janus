
















































#include <jni.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define TAG "[recipe:Authorization] "

#define AUTH_MAX_STR   64
#define AUTH_BUF_MAX   654



static const char* const kStringFields[10] = {
    "mUsername", "mRealm", "mNonce", "mUri", "mResponse", "mAlgorithm",
    "mCnonce", "mQop", "mAuthType", "mAuthMethod",
};

static void clear_pending(JNIEnv* env, const char* where) {
  fprintf(stderr, TAG "JNI exception at %s\n", where);
  (*env)->ExceptionDescribe(env);
  (*env)->ExceptionClear(env);
}

static int take_bytes(JNIEnv* env, const uint8_t* buffer, size_t length,
                      size_t* offset, size_t want, const uint8_t** out) {
  if (buffer == NULL || length < want || *offset > length - want) {
    fprintf(stderr, TAG "FAIL_CONSTRUCTION: input exhausted (need %zu)\n",
            want);
    if ((*env)->ExceptionCheck(env)) {
      clear_pending(env, "input exhaustion");
    }
    return -1;
  }
  *out = buffer + *offset;
  *offset += want;
  return 0;
}



static void sanitize_copy(char* dst, const uint8_t* src, size_t count) {
  size_t i;
  for (i = 0; i < count; i++) {
    char c = (char)(src[i] & 0x7f);
    if (c == '\0') {
      c = ' ';
    }
    dst[i] = c;
  }
  dst[count] = '\0';
}


static int take_string(char* dst, size_t dst_cap, JNIEnv* env,
                       const uint8_t* buffer, size_t length, size_t* offset,
                       const char* field_name) {
  const uint8_t* p;
  size_t n;
  if (take_bytes(env, buffer, length, offset, 1, &p) != 0) {
    fprintf(stderr, TAG "FAIL_CONSTRUCTION: %s length prefix missing\n",
            field_name);
    return -1;
  }
  n = p[0];
  if (n > dst_cap - 1) {
    fprintf(stderr, TAG "FAIL_CONSTRUCTION: %s len %zu > cap %zu\n",
            field_name, n, dst_cap - 1);
    return -1;
  }
  if (take_bytes(env, buffer, length, offset, n, &p) != 0) {
    fprintf(stderr, TAG "FAIL_CONSTRUCTION: %s payload exhausted\n",
            field_name);
    return -1;
  }
  sanitize_copy(dst, p, n);
  return 0;
}


static int emit_string(JNIEnv* env, jobject auth, jfieldID fid,
                       const char* value, const char* field_name) {
  jstring s = (*env)->NewStringUTF(env, value);
  if (s == NULL || (*env)->ExceptionCheck(env)) {
    fprintf(stderr, TAG "FAIL: NewStringUTF %s\n", field_name);
    if ((*env)->ExceptionCheck(env)) {
      clear_pending(env, "NewStringUTF field");
    }
    return -1;
  }
  (*env)->SetObjectField(env, auth, fid, s);
  if ((*env)->ExceptionCheck(env)) {
    fprintf(stderr, TAG "FAIL: SetObjectField %s\n", field_name);
    clear_pending(env, "SetObjectField");
    (*env)->DeleteLocalRef(env, s);
    return -1;
  }
  (*env)->DeleteLocalRef(env, s); 
  return 0;
}

jobject construct_Authorization(JNIEnv* env, const uint8_t* buffer,
                                size_t length, size_t* offset) {
  jclass cls = NULL;
  jmethodID ctor = NULL;
  jobject auth = NULL;
  jfieldID fid_str[10];
  jfieldID fid_ncount;
  jint nonce_count;
  char strs[10][AUTH_MAX_STR + 1];
  size_t i;
  const uint8_t* p;

  
  for (i = 0; i < 10; i++) {
    if (take_string(strs[i], sizeof(strs[i]), env, buffer, length, offset,
                    kStringFields[i]) != 0) {
      return NULL;
    }
  }
  if (take_bytes(env, buffer, length, offset, 4, &p) != 0) {
    fprintf(stderr, TAG "FAIL_CONSTRUCTION: mNonceCount exhausted\n");
    return NULL;
  }
  nonce_count = (jint)((uint32_t)p[0] | ((uint32_t)p[1] << 8) |
                       ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24));

  

  cls = (*env)->FindClass(env, "com/cmdc/rcsprotocol/bean/Authorization");
  if (cls == NULL) {
    fprintf(stderr, TAG "FAIL: FindClass com/cmdc/rcsprotocol/bean/"
                    "Authorization (classpath must expose base.apk)\n");
    if ((*env)->ExceptionCheck(env)) {
      clear_pending(env, "FindClass Authorization");
    }
    return NULL;
  }
  ctor = (*env)->GetMethodID(env, cls, "<init>", "()V");
  if (ctor == NULL) {
    fprintf(stderr, TAG "FAIL: GetMethodID <init>()V\n");
    if ((*env)->ExceptionCheck(env)) {
      clear_pending(env, "GetMethodID <init>()V");
    }
    (*env)->DeleteLocalRef(env, cls);
    return NULL;
  }
  auth = (*env)->NewObject(env, cls, ctor);
  if (auth == NULL || (*env)->ExceptionCheck(env)) {
    fprintf(stderr, TAG "FAIL: NewObject Authorization\n");
    if ((*env)->ExceptionCheck(env)) {
      clear_pending(env, "NewObject Authorization");
    }
    (*env)->DeleteLocalRef(env, cls);
    return NULL;
  }

  
  for (i = 0; i < 10; i++) {
    fid_str[i] = (*env)->GetFieldID(env, cls, kStringFields[i],
                                    "Ljava/lang/String;");
    if (fid_str[i] == NULL) {
      fprintf(stderr, TAG "FAIL: GetFieldID %s\n", kStringFields[i]);
      if ((*env)->ExceptionCheck(env)) {
        clear_pending(env, kStringFields[i]);
      }
      goto fail;
    }
  }
  fid_ncount = (*env)->GetFieldID(env, cls, "mNonceCount", "I");
  if (fid_ncount == NULL) {
    fprintf(stderr, TAG "FAIL: GetFieldID mNonceCount\n");
    if ((*env)->ExceptionCheck(env)) {
      clear_pending(env, "mNonceCount");
    }
    goto fail;
  }

  
  for (i = 0; i < 10; i++) {
    if (emit_string(env, auth, fid_str[i], strs[i], kStringFields[i]) != 0) {
      goto fail;
    }
  }

  
  (*env)->SetIntField(env, auth, fid_ncount, nonce_count);
  if ((*env)->ExceptionCheck(env)) {
    fprintf(stderr, TAG "FAIL: SetIntField mNonceCount\n");
    clear_pending(env, "SetIntField mNonceCount");
    goto fail;
  }

  (*env)->DeleteLocalRef(env, cls);
  fprintf(stderr, TAG "Authorization constructed: nonce_count=%d "
                  "username_len=%zu response_len=%zu (all 11 fields fuzz-set)\n",
          nonce_count, strnlen(strs[0], sizeof(strs[0])),
          strnlen(strs[4], sizeof(strs[4])));
  return auth;

fail:
  if (auth != NULL) (*env)->DeleteLocalRef(env, auth);
  if (cls != NULL) (*env)->DeleteLocalRef(env, cls);
  return NULL;
}
