

#include <jni.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define TAG "[recipe:RegisterBean] "

#define RB_MAX_FEATURE 16
#define RB_MAX_ELEM    64
#define RB_MAX_INST    64
#define RB_MAX_CAPS    64
#define RB_MAX_IMSI    32
#define RB_BUF_MAX     1880


jobject construct_Authorization(JNIEnv* env, const uint8_t* buffer,
                                size_t length, size_t* offset);

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


static int emit_string(JNIEnv* env, jobject bean, jfieldID fid,
                       const char* value, const char* field_name) {
  jstring s = (*env)->NewStringUTF(env, value);
  if (s == NULL || (*env)->ExceptionCheck(env)) {
    fprintf(stderr, TAG "FAIL: NewStringUTF %s\n", field_name);
    if ((*env)->ExceptionCheck(env)) {
      clear_pending(env, "NewStringUTF field");
    }
    return -1;
  }
  (*env)->SetObjectField(env, bean, fid, s);
  if ((*env)->ExceptionCheck(env)) {
    fprintf(stderr, TAG "FAIL: SetObjectField %s\n", field_name);
    clear_pending(env, "SetObjectField");
    (*env)->DeleteLocalRef(env, s);
    return -1;
  }
  (*env)->DeleteLocalRef(env, s); 
  return 0;
}

jobject construct_RegisterBean(JNIEnv* env, const uint8_t* buffer,
                               size_t length, size_t* offset) {
  jclass cls = NULL, list_cls = NULL;
  jmethodID ctor = NULL, list_ctor = NULL, list_add = NULL;
  jobject bean = NULL, feature = NULL, auth = NULL;
  jfieldID fid_instance, fid_caps, fid_imsi, fid_feature, fid_auth;
  jfieldID fid_challenge;
  jfieldID fid_expire;
  jint ictors[3]; 
  jboolean is_challenge;
  jlong expire_period;
  uint8_t scalar_flags, auth_flags;
  char instance[RB_MAX_INST + 1];
  char caps[RB_MAX_CAPS + 1];
  char imsi[RB_MAX_IMSI + 1];
  char elems[RB_MAX_FEATURE][RB_MAX_ELEM + 1];
  size_t n_feature, i;
  const uint8_t* p;

  
  if (take_bytes(env, buffer, length, offset, 1, &p) != 0) {
    return NULL;
  }
  n_feature = p[0];
  if (n_feature > RB_MAX_FEATURE) {
    fprintf(stderr, TAG "FAIL_CONSTRUCTION: mFeatureTag count %zu > cap %d\n",
            n_feature, RB_MAX_FEATURE);
    return NULL;
  }
  for (i = 0; i < n_feature; i++) {
    if (take_string(elems[i], sizeof(elems[i]), env, buffer, length, offset,
                    "mFeatureTag[i]") != 0) {
      return NULL;
    }
  }
  if (take_string(instance, sizeof(instance), env, buffer, length, offset,
                  "mInstanceId") != 0) {
    return NULL;
  }
  if (take_string(caps, sizeof(caps), env, buffer, length, offset,
                  "mCaps") != 0) {
    return NULL;
  }
  if (take_string(imsi, sizeof(imsi), env, buffer, length, offset,
                  "mImsi") != 0) {
    return NULL;
  }
  for (i = 0; i < 3; i++) {
    if (take_bytes(env, buffer, length, offset, 4, &p) != 0) {
      fprintf(stderr, TAG "FAIL_CONSTRUCTION: ctor int arg %zu exhausted\n", i);
      return NULL;
    }
    ictors[i] = (jint)((uint32_t)p[0] | ((uint32_t)p[1] << 8) |
                       ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24));
  }
  if (take_bytes(env, buffer, length, offset, 1, &p) != 0) {
    fprintf(stderr, TAG "FAIL_CONSTRUCTION: scalar flags exhausted\n");
    return NULL;
  }
  scalar_flags = p[0];
  is_challenge = (scalar_flags & 1) ? JNI_TRUE : JNI_FALSE;
  if (take_bytes(env, buffer, length, offset, 8, &p) != 0) {
    fprintf(stderr, TAG "FAIL_CONSTRUCTION: mExpirePeriod exhausted\n");
    return NULL;
  }
  {
    uint64_t u = 0;
    size_t b;
    for (b = 0; b < 8; b++) {
      u |= ((uint64_t)p[b]) << (8 * b);
    }
    expire_period = (jlong)u;
  }
  if (take_bytes(env, buffer, length, offset, 1, &p) != 0) {
    fprintf(stderr, TAG "FAIL_CONSTRUCTION: auth flags exhausted\n");
    return NULL;
  }
  auth_flags = p[0];

  




  cls = (*env)->FindClass(env, "com/cmdc/rcsprotocol/bean/RegisterBean");
  if (cls == NULL) {
    fprintf(stderr, TAG "FAIL: FindClass com/cmdc/rcsprotocol/bean/"
                    "RegisterBean (classpath must expose base.apk)\n");
    if ((*env)->ExceptionCheck(env)) {
      clear_pending(env, "FindClass RegisterBean");
    }
    return NULL;
  }
  ctor = (*env)->GetMethodID(env, cls, "<init>", "(III)V");
  if (ctor == NULL) {
    fprintf(stderr, TAG "FAIL: GetMethodID <init>(III)V\n");
    if ((*env)->ExceptionCheck(env)) {
      clear_pending(env, "GetMethodID <init>(III)V");
    }
    (*env)->DeleteLocalRef(env, cls);
    return NULL;
  }
  bean = (*env)->NewObject(env, cls, ctor, ictors[0], ictors[1], ictors[2]);
  if (bean == NULL || (*env)->ExceptionCheck(env)) {
    fprintf(stderr, TAG "FAIL: NewObject RegisterBean(%d,%d,%d)\n", ictors[0],
            ictors[1], ictors[2]);
    if ((*env)->ExceptionCheck(env)) {
      clear_pending(env, "NewObject RegisterBean");
    }
    (*env)->DeleteLocalRef(env, cls);
    return NULL;
  }

  
  fid_instance = (*env)->GetFieldID(env, cls, "mInstanceId",
                                    "Ljava/lang/String;");
  fid_caps = (*env)->GetFieldID(env, cls, "mCaps", "Ljava/lang/String;");
  fid_imsi = (*env)->GetFieldID(env, cls, "mImsi", "Ljava/lang/String;");
  fid_feature = (*env)->GetFieldID(env, cls, "mFeatureTag",
                                   "Ljava/util/List;");
  fid_auth = (*env)->GetFieldID(env, cls, "mAuth",
                                "Lcom/cmdc/rcsprotocol/bean/Authorization;");
  fid_challenge = (*env)->GetFieldID(env, cls, "mIsChallenge", "Z");
  fid_expire = (*env)->GetFieldID(env, cls, "mExpirePeriod", "J");
  if (fid_instance == NULL || fid_caps == NULL || fid_imsi == NULL ||
      fid_feature == NULL || fid_auth == NULL || fid_challenge == NULL ||
      fid_expire == NULL) {
    fprintf(stderr, TAG "FAIL: GetFieldID core fields\n");
    if ((*env)->ExceptionCheck(env)) {
      clear_pending(env, "GetFieldID core");
    }
    goto fail;
  }

  
  if (emit_string(env, bean, fid_instance, instance, "mInstanceId") != 0) {
    goto fail;
  }
  if (emit_string(env, bean, fid_caps, caps, "mCaps") != 0) {
    goto fail;
  }
  if (emit_string(env, bean, fid_imsi, imsi, "mImsi") != 0) {
    goto fail;
  }

  


  list_cls = (*env)->FindClass(env, "java/util/ArrayList");
  if (list_cls == NULL) {
    fprintf(stderr, TAG "FAIL: FindClass java/util/ArrayList\n");
    if ((*env)->ExceptionCheck(env)) {
      clear_pending(env, "FindClass ArrayList");
    }
    goto fail;
  }
  list_ctor = (*env)->GetMethodID(env, list_cls, "<init>", "()V");
  list_add = (*env)->GetMethodID(env, list_cls, "add",
                                 "(Ljava/lang/Object;)Z");
  if (list_ctor == NULL || list_add == NULL) {
    fprintf(stderr, TAG "FAIL: ArrayList <init>()V / add(Object)Z\n");
    if ((*env)->ExceptionCheck(env)) {
      clear_pending(env, "ArrayList methods");
    }
    (*env)->DeleteLocalRef(env, list_cls);
    list_cls = NULL;
    goto fail;
  }
  feature = (*env)->NewObject(env, list_cls, list_ctor);
  (*env)->DeleteLocalRef(env, list_cls); 
  list_cls = NULL;
  if (feature == NULL || (*env)->ExceptionCheck(env)) {
    fprintf(stderr, TAG "FAIL: NewObject ArrayList\n");
    if ((*env)->ExceptionCheck(env)) {
      clear_pending(env, "NewObject ArrayList");
    }
    goto fail;
  }
  for (i = 0; i < n_feature; i++) {
    jstring s = (*env)->NewStringUTF(env, elems[i]);
    if (s == NULL || (*env)->ExceptionCheck(env)) {
      fprintf(stderr, TAG "FAIL: NewStringUTF mFeatureTag[%zu]\n", i);
      if ((*env)->ExceptionCheck(env)) {
        clear_pending(env, "NewStringUTF element");
      }
      (*env)->DeleteLocalRef(env, feature);
      feature = NULL;
      goto fail;
    }
    (*env)->CallBooleanMethod(env, feature, list_add, s);
    if ((*env)->ExceptionCheck(env)) {
      fprintf(stderr, TAG "FAIL: ArrayList.add[%zu]\n", i);
      clear_pending(env, "ArrayList.add");
      (*env)->DeleteLocalRef(env, s);
      (*env)->DeleteLocalRef(env, feature);
      feature = NULL;
      goto fail;
    }
    (*env)->DeleteLocalRef(env, s); 
  }
  (*env)->SetObjectField(env, bean, fid_feature, feature);
  if ((*env)->ExceptionCheck(env)) {
    fprintf(stderr, TAG "FAIL: SetObjectField mFeatureTag\n");
    clear_pending(env, "SetObjectField mFeatureTag");
    (*env)->DeleteLocalRef(env, feature);
    feature = NULL;
    goto fail;
  }
  (*env)->DeleteLocalRef(env, feature);
  feature = NULL;

  

  (*env)->SetBooleanField(env, bean, fid_challenge, is_challenge);
  (*env)->SetLongField(env, bean, fid_expire, expire_period);
  if ((*env)->ExceptionCheck(env)) {
    fprintf(stderr, TAG "FAIL: Set own scalar fields\n");
    clear_pending(env, "Set own scalars");
    goto fail;
  }

  



  if (auth_flags & 1) {
    auth = construct_Authorization(env, buffer, length, offset);
    if (auth == NULL || (*env)->ExceptionCheck(env)) {
      fprintf(stderr, TAG "FAIL: construct_Authorization child\n");
      if ((*env)->ExceptionCheck(env)) {
        clear_pending(env, "construct_Authorization");
      }
      goto fail;
    }
    (*env)->SetObjectField(env, bean, fid_auth, auth);
    if ((*env)->ExceptionCheck(env)) {
      fprintf(stderr, TAG "FAIL: SetObjectField mAuth\n");
      clear_pending(env, "SetObjectField mAuth");
      (*env)->DeleteLocalRef(env, auth);
      auth = NULL;
      goto fail;
    }
    (*env)->DeleteLocalRef(env, auth); 
    auth = NULL;
  }

  (*env)->DeleteLocalRef(env, cls);
  fprintf(stderr, TAG "RegisterBean constructed: sender=%d receiver=%d "
                  "regType=%d challenge=%d expire=%lld feature=%zu "
                  "instance_len=%zu caps_len=%zu imsi_len=%zu auth=%s "
                  "beanType=101 (ctor-set)\n",
          ictors[0], ictors[1], ictors[2], (int)is_challenge,
          (long long)expire_period, n_feature,
          strnlen(instance, sizeof(instance)),
          strnlen(caps, sizeof(caps)), strnlen(imsi, sizeof(imsi)),
          (auth_flags & 1) ? "constructed" : "null(app-real)");
  return bean;

fail:
  if (list_cls != NULL) (*env)->DeleteLocalRef(env, list_cls);
  if (feature != NULL) (*env)->DeleteLocalRef(env, feature);
  if (auth != NULL) (*env)->DeleteLocalRef(env, auth);
  if (bean != NULL) (*env)->DeleteLocalRef(env, bean);
  if (cls != NULL) (*env)->DeleteLocalRef(env, cls);
  return NULL;
}
