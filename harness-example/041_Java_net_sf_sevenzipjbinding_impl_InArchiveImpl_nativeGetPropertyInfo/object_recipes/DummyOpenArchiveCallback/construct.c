























#include <jni.h>
#include <stdio.h>

#define TAG "[recipe:DummyOpenArchiveCallback] "

static void clear_pending(JNIEnv* env, const char* where) {
    fprintf(stderr, TAG "JNI exception at %s\n", where);
    (*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);
}

jobject construct_DummyOpenArchiveCallback(JNIEnv* env,
                                           const uint8_t* buffer,
                                           size_t length,
                                           size_t* offset) {
    
    (void)buffer;
    (void)length;
    (void)offset;

    jclass cls = (*env)->FindClass(
        env, "net/sf/sevenzipjbinding/SevenZip$DummyOpenArchiveCallback");
    if (cls == NULL) {
        fprintf(stderr, TAG "FindClass(SevenZip$DummyOpenArchiveCallback) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "FindClass");
        }
        return NULL;
    }

    
    jmethodID ctor = (*env)->GetMethodID(env, cls, "<init>", "(I)V");
    if (ctor == NULL) {
        fprintf(stderr, TAG "GetMethodID(<init>(I)V) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "GetMethodID <init>");
        }
        (*env)->DeleteLocalRef(env, cls);
        return NULL;
    }

    
    jmethodID mSetCompleted =
        (*env)->GetMethodID(env, cls, "setCompleted", "(Ljava/lang/Long;Ljava/lang/Long;)V");
    if (mSetCompleted == NULL) {
        fprintf(stderr, TAG "GetMethodID(setCompleted) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "GetMethodID setCompleted");
        }
        (*env)->DeleteLocalRef(env, cls);
        return NULL;
    }
    jmethodID mSetTotal =
        (*env)->GetMethodID(env, cls, "setTotal", "(Ljava/lang/Long;Ljava/lang/Long;)V");
    if (mSetTotal == NULL) {
        fprintf(stderr, TAG "GetMethodID(setTotal) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "GetMethodID setTotal");
        }
        (*env)->DeleteLocalRef(env, cls);
        return NULL;
    }

    jobject obj = (*env)->NewObject(env, cls, ctor, (jint)0);
    if (obj == NULL || (*env)->ExceptionCheck(env)) {
        fprintf(stderr, TAG "NewObject failed\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "NewObject");
        }
        (*env)->DeleteLocalRef(env, cls);
        return NULL;
    }

    
    jclass iface = (*env)->FindClass(env, "net/sf/sevenzipjbinding/IArchiveOpenCallback");
    if (iface == NULL) {
        fprintf(stderr, TAG "FindClass(IArchiveOpenCallback) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "FindClass IArchiveOpenCallback");
        }
        (*env)->DeleteLocalRef(env, obj);
        (*env)->DeleteLocalRef(env, cls);
        return NULL;
    }
    if ((*env)->IsInstanceOf(env, obj, iface) != JNI_TRUE) {
        fprintf(stderr, TAG "constructed object is not an IArchiveOpenCallback\n");
        (*env)->DeleteLocalRef(env, iface);
        (*env)->DeleteLocalRef(env, obj);
        (*env)->DeleteLocalRef(env, cls);
        return NULL;
    }

    (*env)->DeleteLocalRef(env, iface);
    (*env)->DeleteLocalRef(env, cls);
    return obj; 
}
