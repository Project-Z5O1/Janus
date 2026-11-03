

#include <jni.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define TAG "[recipe:MessageBean] "

#define MB_MAX_NUMBERS 16
#define MB_MAX_ELEM    64
#define MB_MAX_CONTENT 128
#define MB_MAX_STR     32
#define MB_BUF_MAX     1488



static const char* const kStringFields[9] = {
    "mContentType", "mContentEncoding", "mMessageId", "mConversationId",
    "mContributeId", "mDispositionType", "mTrafficType",
    "mInReplyToContributionId", "mDate",
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

jobject construct_MessageBean(JNIEnv* env, const uint8_t* buffer,
                              size_t length, size_t* offset) {
  jclass cls = NULL, list_cls = NULL;
  jmethodID ctor = NULL, list_ctor = NULL, list_add = NULL;
  jobject bean = NULL, numbers = NULL;
  jfieldID fid_content, fid_numbers;
  jfieldID fid_str[9];
  jfieldID fid_mstatus, fid_sipcode, fid_sender, fid_receiver;
  jfieldID fid_bflags[5];
  static const char* const bflag_names[5] = {
      "mIsFileMessage", "mIsAnonymous", "mIsPublish", "mIsChatBot",
      "mIsAnswer"};
  jboolean bvals[5];
  jint ivalues[4]; 
  jint message_type;
  char content[MB_MAX_CONTENT + 1];
  char strs[9][MB_MAX_STR + 1];
  char elems[MB_MAX_NUMBERS][MB_MAX_ELEM + 1];
  size_t n_numbers, i;
  uint8_t flags;
  const uint8_t* p;

  
  if (take_bytes(env, buffer, length, offset, 1, &p) != 0) {
    return NULL;
  }
  n_numbers = p[0];
  if (n_numbers > MB_MAX_NUMBERS) {
    fprintf(stderr, TAG "FAIL_CONSTRUCTION: mNumbers count %zu > cap %d\n",
            n_numbers, MB_MAX_NUMBERS);
    return NULL;
  }
  for (i = 0; i < n_numbers; i++) {
    if (take_string(elems[i], sizeof(elems[i]), env, buffer, length, offset,
                    "mNumbers[i]") != 0) {
      return NULL;
    }
  }
  if (take_string(content, sizeof(content), env, buffer, length, offset,
                  "mContent") != 0) {
    return NULL;
  }
  for (i = 0; i < 9; i++) {
    if (take_string(strs[i], sizeof(strs[i]), env, buffer, length, offset,
                    kStringFields[i]) != 0) {
      return NULL;
    }
  }
  for (i = 0; i < 4; i++) {
    if (take_bytes(env, buffer, length, offset, 4, &p) != 0) {
      fprintf(stderr, TAG "FAIL_CONSTRUCTION: int field %zu exhausted\n", i);
      return NULL;
    }
    ivalues[i] = (jint)((uint32_t)p[0] | ((uint32_t)p[1] << 8) |
                        ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24));
  }
  if (take_bytes(env, buffer, length, offset, 4, &p) != 0) {
    fprintf(stderr, TAG "FAIL_CONSTRUCTION: mMessageType exhausted\n");
    return NULL;
  }
  message_type = (jint)((uint32_t)p[0] | ((uint32_t)p[1] << 8) |
                        ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24));
  if (take_bytes(env, buffer, length, offset, 1, &p) != 0) {
    fprintf(stderr, TAG "FAIL_CONSTRUCTION: flags byte exhausted\n");
    return NULL;
  }
  flags = p[0];
  for (i = 0; i < 5; i++) {
    bvals[i] = ((flags >> i) & 1) ? JNI_TRUE : JNI_FALSE;
  }

  
  cls = (*env)->FindClass(env, "com/cmdc/rcsprotocol/bean/MessageBean");
  if (cls == NULL) {
    fprintf(stderr, TAG "FAIL: FindClass com/cmdc/rcsprotocol/bean/"
                    "MessageBean (classpath must expose base.apk)\n");
    if ((*env)->ExceptionCheck(env)) {
      clear_pending(env, "FindClass MessageBean");
    }
    return NULL;
  }
  ctor = (*env)->GetMethodID(env, cls, "<init>", "(I)V");
  if (ctor == NULL) {
    fprintf(stderr, TAG "FAIL: GetMethodID <init>(I)V\n");
    if ((*env)->ExceptionCheck(env)) {
      clear_pending(env, "GetMethodID <init>(I)V");
    }
    (*env)->DeleteLocalRef(env, cls);
    return NULL;
  }
  


  bean = (*env)->NewObject(env, cls, ctor, message_type);
  if (bean == NULL || (*env)->ExceptionCheck(env)) {
    fprintf(stderr, TAG "FAIL: NewObject MessageBean(%d)\n", message_type);
    if ((*env)->ExceptionCheck(env)) {
      clear_pending(env, "NewObject MessageBean");
    }
    (*env)->DeleteLocalRef(env, cls);
    return NULL;
  }

  
  fid_content = (*env)->GetFieldID(env, cls, "mContent", "Ljava/lang/String;");
  fid_numbers = (*env)->GetFieldID(env, cls, "mNumbers", "Ljava/util/List;");
  fid_mstatus = (*env)->GetFieldID(env, cls, "mMessageStatus", "I");
  fid_sipcode = (*env)->GetFieldID(env, cls, "mSipCode", "I");
  fid_sender = (*env)->GetFieldID(env, cls, "mSenderId", "I");
  fid_receiver = (*env)->GetFieldID(env, cls, "mReceiverId", "I");
  if (fid_content == NULL || fid_numbers == NULL || fid_mstatus == NULL ||
      fid_sipcode == NULL || fid_sender == NULL || fid_receiver == NULL) {
    fprintf(stderr, TAG "FAIL: GetFieldID core fields\n");
    if ((*env)->ExceptionCheck(env)) {
      clear_pending(env, "GetFieldID core");
    }
    goto fail;
  }
  for (i = 0; i < 9; i++) {
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
  for (i = 0; i < 5; i++) {
    fid_bflags[i] = (*env)->GetFieldID(env, cls, bflag_names[i], "Z");
    if (fid_bflags[i] == NULL) {
      fprintf(stderr, TAG "FAIL: GetFieldID %s\n", bflag_names[i]);
      if ((*env)->ExceptionCheck(env)) {
        clear_pending(env, bflag_names[i]);
      }
      goto fail;
    }
  }

  
  if (emit_string(env, bean, fid_content, content, "mContent") != 0) {
    goto fail;
  }
  for (i = 0; i < 9; i++) {
    if (emit_string(env, bean, fid_str[i], strs[i], kStringFields[i]) != 0) {
      goto fail;
    }
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
  numbers = (*env)->NewObject(env, list_cls, list_ctor);
  (*env)->DeleteLocalRef(env, list_cls); 
  list_cls = NULL;
  if (numbers == NULL || (*env)->ExceptionCheck(env)) {
    fprintf(stderr, TAG "FAIL: NewObject ArrayList\n");
    if ((*env)->ExceptionCheck(env)) {
      clear_pending(env, "NewObject ArrayList");
    }
    goto fail;
  }
  for (i = 0; i < n_numbers; i++) {
    jstring s = (*env)->NewStringUTF(env, elems[i]);
    if (s == NULL || (*env)->ExceptionCheck(env)) {
      fprintf(stderr, TAG "FAIL: NewStringUTF mNumbers[%zu]\n", i);
      if ((*env)->ExceptionCheck(env)) {
        clear_pending(env, "NewStringUTF element");
      }
      (*env)->DeleteLocalRef(env, numbers);
      numbers = NULL;
      goto fail;
    }
    (*env)->CallBooleanMethod(env, numbers, list_add, s);
    if ((*env)->ExceptionCheck(env)) {
      fprintf(stderr, TAG "FAIL: ArrayList.add[%zu]\n", i);
      clear_pending(env, "ArrayList.add");
      (*env)->DeleteLocalRef(env, s);
      (*env)->DeleteLocalRef(env, numbers);
      numbers = NULL;
      goto fail;
    }
    (*env)->DeleteLocalRef(env, s); 
  }
  (*env)->SetObjectField(env, bean, fid_numbers, numbers);
  if ((*env)->ExceptionCheck(env)) {
    fprintf(stderr, TAG "FAIL: SetObjectField mNumbers\n");
    clear_pending(env, "SetObjectField mNumbers");
    (*env)->DeleteLocalRef(env, numbers);
    numbers = NULL;
    goto fail;
  }
  (*env)->DeleteLocalRef(env, numbers);
  numbers = NULL;

  
  (*env)->SetIntField(env, bean, fid_mstatus, ivalues[0]);
  (*env)->SetIntField(env, bean, fid_sipcode, ivalues[1]);
  (*env)->SetIntField(env, bean, fid_sender, ivalues[2]);
  (*env)->SetIntField(env, bean, fid_receiver, ivalues[3]);
  for (i = 0; i < 5; i++) {
    (*env)->SetBooleanField(env, bean, fid_bflags[i], bvals[i]);
  }
  if ((*env)->ExceptionCheck(env)) {
    fprintf(stderr, TAG "FAIL: Set scalar/boolean fields\n");
    clear_pending(env, "Set scalars");
    goto fail;
  }

  (*env)->DeleteLocalRef(env, cls);
  fprintf(stderr, TAG "MessageBean constructed: type=%d numbers=%zu "
                  "content_len=%zu beanType=102 (ctor-set)\n",
          message_type, n_numbers, strnlen(content, sizeof(content)));
  return bean;

fail:
  if (list_cls != NULL) (*env)->DeleteLocalRef(env, list_cls);
  if (numbers != NULL) (*env)->DeleteLocalRef(env, numbers);
  if (bean != NULL) (*env)->DeleteLocalRef(env, bean);
  if (cls != NULL) (*env)->DeleteLocalRef(env, cls);
  return NULL;
}
