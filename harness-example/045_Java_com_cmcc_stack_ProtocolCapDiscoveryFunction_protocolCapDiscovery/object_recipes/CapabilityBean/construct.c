
#include <jni.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CB_CAP_URI        256u
#define CB_CAP_NUMBER     256u
#define CB_CAP_ONELINE    512u
#define CB_CAP_ELEM       128u
#define CB_LIST_MAX       8u
#define CB_LIST_NULL_MODE 0xFFu


#define CB_MAX_STR_CAP \
  ((size_t)(CB_CAP_ONELINE > CB_CAP_URI ? CB_CAP_ONELINE : CB_CAP_URI))




static jstring cb_take_string(JNIEnv* env, const uint8_t* buffer,
                              size_t length, size_t* offset, size_t cap,
                              const char* tag) {
  size_t avail, need, i;
  uint16_t slen;
  char out[CB_MAX_STR_CAP + 1];
  jstring s;

  if (cap + 1 > sizeof(out)) {
    

    fprintf(stderr, "[recipe:CapabilityBean] %s: cap %zu exceeds decode "
                    "buffer %zu\n", tag, cap, sizeof(out));
    return NULL;
  }
  if (length < *offset || length - *offset < 2) {
    fprintf(stderr, "[recipe:CapabilityBean] %s: length prefix exhausted\n",
            tag);
    return NULL;
  }
  slen = (uint16_t)(buffer[*offset] | ((uint16_t)buffer[*offset + 1] << 8));
  *offset += 2;
  if ((size_t)slen > cap) {
    fprintf(stderr, "[recipe:CapabilityBean] %s: len %u over cap %zu\n", tag,
            (unsigned)slen, cap);
    return NULL;
  }
  avail = (*offset <= length) ? (length - *offset) : 0;
  if ((size_t)slen > avail) {
    fprintf(stderr, "[recipe:CapabilityBean] %s: payload exhausted "
                    "(need %u, avail %zu)\n", tag, (unsigned)slen, avail);
    return NULL;
  }
  need = (size_t)slen;
  for (i = 0; i < need; i++) {
    uint8_t b = (uint8_t)(buffer[*offset + i] & 0x7F);
    if (b == 0x00) b = (uint8_t)' '; 
    out[i] = (char)b;
  }
  *offset += need;
  out[need] = '\0';
  s = (*env)->NewStringUTF(env, out);
  if (s == NULL || (*env)->ExceptionCheck(env) == JNI_TRUE) {
    fprintf(stderr, "[recipe:CapabilityBean] %s: NewStringUTF(\"%s\") "
                    "failed\n", tag, out);
    if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
      (*env)->ExceptionDescribe(env);
      (*env)->ExceptionClear(env);
    }
    return NULL;
  }
  return s;
}

static int cb_take_i32(const uint8_t* buffer, size_t length,
                       size_t* offset, jint* out, const char* tag) {
  if (length < *offset || length - *offset < 4) {
    fprintf(stderr, "[recipe:CapabilityBean] %s: i32 exhausted\n", tag);
    return 0;
  }
  *out = (jint)((uint32_t)buffer[*offset] |
                ((uint32_t)buffer[*offset + 1] << 8) |
                ((uint32_t)buffer[*offset + 2] << 16) |
                ((uint32_t)buffer[*offset + 3] << 24));
  *offset += 4;
  return 1;
}

jobject construct_CapabilityBean(JNIEnv* env, const uint8_t* buffer,
                                 size_t length, size_t* offset) {
  jclass cls = NULL, list_cls = NULL;
  jmethodID list_ctor, list_add;
  jobject bean = NULL, list_obj = NULL;
  jstring s_uri = NULL, s_number = NULL, s_oneline = NULL, s_elem = NULL;
  jfieldID f;
  jint sip_code = 0, sender_id = 0, receiver_id = 0;
  uint8_t list_mode;
  size_t saved_offset;
  unsigned i;

  if (env == NULL || buffer == NULL || offset == NULL) {
    fprintf(stderr, "[recipe:CapabilityBean] invalid arguments\n");
    return NULL;
  }
  saved_offset = *offset;

  


  s_uri = cb_take_string(env, buffer, length, offset, CB_CAP_URI, "mUri");
  if (s_uri == NULL) goto fail_reset;
  s_number = cb_take_string(env, buffer, length, offset, CB_CAP_NUMBER,
                            "mNumber");
  if (s_number == NULL) goto fail_reset;

  if (length < *offset || length - *offset < 1) {
    fprintf(stderr, "[recipe:CapabilityBean] mFeatureTags: mode exhausted\n");
    goto fail_reset;
  }
  list_mode = buffer[*offset];
  *offset += 1;
  if (list_mode != CB_LIST_NULL_MODE && list_mode > CB_LIST_MAX) {
    fprintf(stderr, "[recipe:CapabilityBean] mFeatureTags: illegal mode "
                    "%u (0xFF or 0..%u)\n", (unsigned)list_mode,
            (unsigned)CB_LIST_MAX);
    goto fail_reset;
  }
  if (list_mode != CB_LIST_NULL_MODE) {
    


    list_cls = (*env)->FindClass(env, "java/util/ArrayList");
    if (list_cls == NULL) {
      fprintf(stderr, "[recipe:CapabilityBean] FindClass ArrayList failed\n");
      goto fail_reset;
    }
    list_ctor = (*env)->GetMethodID(env, list_cls, "<init>", "()V");
    list_add = (*env)->GetMethodID(env, list_cls, "add",
                                   "(Ljava/lang/Object;)Z");
    if (list_ctor == NULL || list_add == NULL) {
      fprintf(stderr, "[recipe:CapabilityBean] ArrayList method lookup "
                      "failed\n");
      if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
      }
      goto fail_reset;
    }
    list_obj = (*env)->NewObject(env, list_cls, list_ctor);
    if (list_obj == NULL || (*env)->ExceptionCheck(env) == JNI_TRUE) {
      fprintf(stderr, "[recipe:CapabilityBean] NewObject ArrayList failed\n");
      if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
      }
      goto fail_reset;
    }
    for (i = 0; i < (unsigned)list_mode; i++) {
      jboolean added;
      char tag[16];
      snprintf(tag, sizeof(tag), "mFeatureTags[%u]", i);
      s_elem = cb_take_string(env, buffer, length, offset, CB_CAP_ELEM, tag);
      if (s_elem == NULL) goto fail_reset;
      added = (*env)->CallBooleanMethod(env, list_obj, list_add, s_elem);
      if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
        fprintf(stderr, "[recipe:CapabilityBean] ArrayList.add(%u) threw\n",
                i);
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        goto fail_reset;
      }
      if (!added) {
        fprintf(stderr, "[recipe:CapabilityBean] ArrayList.add(%u) returned "
                        "false\n", i);
        goto fail_reset;
      }
      (*env)->DeleteLocalRef(env, s_elem);
      s_elem = NULL;
    }
    (*env)->DeleteLocalRef(env, list_cls);
    list_cls = NULL;
  }

  s_oneline = cb_take_string(env, buffer, length, offset, CB_CAP_ONELINE,
                             "mFeatureTagsOneLine");
  if (s_oneline == NULL) goto fail_reset;
  {
    uint8_t is_answer = 0;
    if (length < *offset || length - *offset < 1) {
      fprintf(stderr, "[recipe:CapabilityBean] mIsAnswer exhausted\n");
      goto fail_reset;
    }
    is_answer = buffer[*offset];
    *offset += 1;
    if (!cb_take_i32(buffer, length, offset, &sip_code, "mSipCode"))
      goto fail_reset;
    if (!cb_take_i32(buffer, length, offset, &sender_id, "mSenderId"))
      goto fail_reset;
    if (!cb_take_i32(buffer, length, offset, &receiver_id, "mReceiverId"))
      goto fail_reset;

    


    cls = (*env)->FindClass(env, "com/cmdc/rcsprotocol/bean/CapabilityBean");
    if (cls == NULL) {
      fprintf(stderr, "[recipe:CapabilityBean] FindClass failed\n");
      goto fail_reset; 
    }
    bean = (*env)->NewObject(env, cls,
                             (*env)->GetMethodID(env, cls, "<init>", "()V"));
    if (bean == NULL || (*env)->ExceptionCheck(env) == JNI_TRUE) {
      fprintf(stderr, "[recipe:CapabilityBean] NewObject failed\n");
      if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
      }
      goto fail;
    }

    


    f = (*env)->GetFieldID(env, cls, "mUri", "Ljava/lang/String;");
    if (f == NULL) goto fail_set;
    (*env)->SetObjectField(env, bean, f, s_uri);
    if ((*env)->ExceptionCheck(env) == JNI_TRUE) goto fail_set;
    (*env)->DeleteLocalRef(env, s_uri);
    s_uri = NULL;

    f = (*env)->GetFieldID(env, cls, "mNumber", "Ljava/lang/String;");
    if (f == NULL) goto fail_set;
    (*env)->SetObjectField(env, bean, f, s_number);
    if ((*env)->ExceptionCheck(env) == JNI_TRUE) goto fail_set;
    (*env)->DeleteLocalRef(env, s_number);
    s_number = NULL;

    if (list_obj != NULL) {
      f = (*env)->GetFieldID(env, cls, "mFeatureTags", "Ljava/util/List;");
      if (f == NULL) goto fail_set;
      (*env)->SetObjectField(env, bean, f, list_obj);
      if ((*env)->ExceptionCheck(env) == JNI_TRUE) goto fail_set;
      (*env)->DeleteLocalRef(env, list_obj);
      list_obj = NULL;
    }
    




    f = (*env)->GetFieldID(env, cls, "mFeatureTagsOneLine",
                           "Ljava/lang/String;");
    if (f == NULL) goto fail_set;
    (*env)->SetObjectField(env, bean, f, s_oneline);
    if ((*env)->ExceptionCheck(env) == JNI_TRUE) goto fail_set;
    (*env)->DeleteLocalRef(env, s_oneline);
    s_oneline = NULL;

    f = (*env)->GetFieldID(env, cls, "mIsAnswer", "Z");
    if (f == NULL) goto fail_set;
    (*env)->SetBooleanField(env, bean, f, (jboolean)(is_answer ? 1 : 0));
    if ((*env)->ExceptionCheck(env) == JNI_TRUE) goto fail_set;

    f = (*env)->GetFieldID(env, cls, "mSipCode", "I");
    if (f == NULL) goto fail_set;
    (*env)->SetIntField(env, bean, f, sip_code);
    if ((*env)->ExceptionCheck(env) == JNI_TRUE) goto fail_set;

    f = (*env)->GetFieldID(env, cls, "mSenderId", "I");
    if (f == NULL) goto fail_set;
    (*env)->SetIntField(env, bean, f, sender_id);
    if ((*env)->ExceptionCheck(env) == JNI_TRUE) goto fail_set;

    f = (*env)->GetFieldID(env, cls, "mReceiverId", "I");
    if (f == NULL) goto fail_set;
    (*env)->SetIntField(env, bean, f, receiver_id);
    if ((*env)->ExceptionCheck(env) == JNI_TRUE) goto fail_set;

    (*env)->DeleteLocalRef(env, cls);
    return bean;
  }

fail_set:
  (*env)->ExceptionDescribe(env);
  (*env)->ExceptionClear(env);
fail:
  if (s_uri != NULL) (*env)->DeleteLocalRef(env, s_uri);
  if (s_number != NULL) (*env)->DeleteLocalRef(env, s_number);
  if (s_elem != NULL) (*env)->DeleteLocalRef(env, s_elem);
  if (s_oneline != NULL) (*env)->DeleteLocalRef(env, s_oneline);
  if (list_obj != NULL) (*env)->DeleteLocalRef(env, list_obj);
  if (list_cls != NULL) (*env)->DeleteLocalRef(env, list_cls);
  if (bean != NULL) (*env)->DeleteLocalRef(env, bean);
  if (cls != NULL) (*env)->DeleteLocalRef(env, cls);
  return NULL;

fail_reset:
  *offset = saved_offset; 
  goto fail;
}
