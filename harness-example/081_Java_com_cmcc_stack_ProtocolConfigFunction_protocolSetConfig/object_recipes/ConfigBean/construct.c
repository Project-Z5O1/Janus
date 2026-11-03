
#include <jni.h>
#include <stdint.h>
#include <stdio.h>

jobject construct_SipAccount(JNIEnv* env, const uint8_t* buffer,
                             size_t length, size_t* offset);
jobject construct_SipNetwork(JNIEnv* env, const uint8_t* buffer,
                             size_t length, size_t* offset);
jobject construct_SipProxy(JNIEnv* env, const uint8_t* buffer,
                           size_t length, size_t* offset);
jobject construct_SipAccessNetwork(JNIEnv* env, const uint8_t* buffer,
                                   size_t length, size_t* offset);

jobject construct_ConfigBean(JNIEnv* env, const uint8_t* buffer,
                             size_t length, size_t* offset) {
  jobject sip_account = NULL, sip_network = NULL, sip_proxy = NULL;
  jobject sip_access = NULL, bean = NULL;
  jclass cls = NULL;
  size_t saved_offset;
  jfieldID f;

  if (env == NULL || buffer == NULL || offset == NULL) {
    fprintf(stderr, "[recipe:ConfigBean] invalid arguments\n");
    return NULL;
  }
  saved_offset = *offset;

  

  sip_account = construct_SipAccount(env, buffer, length, offset);
  if (sip_account == NULL) {
    fprintf(stderr, "[recipe:ConfigBean] child SipAccount FAIL_CONSTRUCTION\n");
    goto fail_reset;
  }
  sip_network = construct_SipNetwork(env, buffer, length, offset);
  if (sip_network == NULL) {
    fprintf(stderr, "[recipe:ConfigBean] child SipNetwork FAIL_CONSTRUCTION\n");
    goto fail_reset;
  }
  sip_proxy = construct_SipProxy(env, buffer, length, offset);
  if (sip_proxy == NULL) {
    fprintf(stderr, "[recipe:ConfigBean] child SipProxy FAIL_CONSTRUCTION\n");
    goto fail_reset;
  }
  sip_access = construct_SipAccessNetwork(env, buffer, length, offset);
  if (sip_access == NULL) {
    fprintf(stderr, "[recipe:ConfigBean] child SipAccessNetwork "
                    "FAIL_CONSTRUCTION\n");
    goto fail_reset;
  }

  

  cls = (*env)->FindClass(env, "com/cmdc/rcsprotocol/config/ConfigBean");
  if (cls == NULL) {
    fprintf(stderr, "[recipe:ConfigBean] FindClass failed\n");
    goto fail_reset; 
  }
  bean = (*env)->NewObject(env, cls,
                           (*env)->GetMethodID(env, cls, "<init>", "()V"));
  if (bean == NULL || (*env)->ExceptionCheck(env) == JNI_TRUE) {
    fprintf(stderr, "[recipe:ConfigBean] NewObject failed\n");
    if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
      (*env)->ExceptionDescribe(env);
      (*env)->ExceptionClear(env);
    }
    goto fail;
  }

  

  f = (*env)->GetFieldID(env, cls, "mSipAccount",
                         "Lcom/cmdc/rcsprotocol/config/SipAccount;");
  if (f == NULL) goto fail;
  (*env)->SetObjectField(env, bean, f, sip_account);
  if ((*env)->ExceptionCheck(env) == JNI_TRUE) goto fail_set;
  (*env)->DeleteLocalRef(env, sip_account);
  sip_account = NULL;

  f = (*env)->GetFieldID(env, cls, "mSipNetwork",
                         "Lcom/cmdc/rcsprotocol/config/SipNetwork;");
  if (f == NULL) goto fail;
  (*env)->SetObjectField(env, bean, f, sip_network);
  if ((*env)->ExceptionCheck(env) == JNI_TRUE) goto fail_set;
  (*env)->DeleteLocalRef(env, sip_network);
  sip_network = NULL;

  f = (*env)->GetFieldID(env, cls, "mSipProxy",
                         "Lcom/cmdc/rcsprotocol/config/SipProxy;");
  if (f == NULL) goto fail;
  (*env)->SetObjectField(env, bean, f, sip_proxy);
  if ((*env)->ExceptionCheck(env) == JNI_TRUE) goto fail_set;
  (*env)->DeleteLocalRef(env, sip_proxy);
  sip_proxy = NULL;

  f = (*env)->GetFieldID(env, cls, "mSipAccessNetwork",
                         "Lcom/cmdc/rcsprotocol/config/SipAccessNetwork;");
  if (f == NULL) goto fail;
  (*env)->SetObjectField(env, bean, f, sip_access);
  if ((*env)->ExceptionCheck(env) == JNI_TRUE) goto fail_set;
  (*env)->DeleteLocalRef(env, sip_access);
  sip_access = NULL;

  f = (*env)->GetFieldID(env, cls, "mIsRunnerEnvironment", "Z");
  if (f == NULL) goto fail;
  (*env)->SetBooleanField(env, bean, f, (jboolean)0);
  if ((*env)->ExceptionCheck(env) == JNI_TRUE) goto fail_set;

  (*env)->DeleteLocalRef(env, cls);
  return bean;

fail_set:
  (*env)->ExceptionDescribe(env);
  (*env)->ExceptionClear(env);
fail:
  if (sip_account != NULL) (*env)->DeleteLocalRef(env, sip_account);
  if (sip_network != NULL) (*env)->DeleteLocalRef(env, sip_network);
  if (sip_proxy != NULL) (*env)->DeleteLocalRef(env, sip_proxy);
  if (sip_access != NULL) (*env)->DeleteLocalRef(env, sip_access);
  if (bean != NULL) (*env)->DeleteLocalRef(env, bean);
  if (cls != NULL) (*env)->DeleteLocalRef(env, cls);
  return NULL;

fail_reset:
  *offset = saved_offset; 
  goto fail;
}
