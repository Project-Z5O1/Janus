












































#include <jni.h>
#include <stdio.h>

#define TAG "[recipe:OutArchive7zImpl] "

static void clear_pending(JNIEnv* env, const char* where) {
    fprintf(stderr, TAG "JNI exception at %s\n", where);
    (*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);
}






jobject construct_OutArchive7zImpl(JNIEnv* env,
                                   const uint8_t* buffer,
                                   size_t length,
                                   size_t* offset) {
    (void)buffer; 
    (void)length;

    if (offset == NULL) {
        fprintf(stderr, TAG "offset pointer is required\n");
        return NULL;
    }

    jclass cls = (*env)->FindClass(env, "net/sf/sevenzipjbinding/impl/OutArchive7zImpl");
    if (cls == NULL) {
        fprintf(stderr, TAG "FindClass(OutArchive7zImpl) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "FindClass OutArchive7zImpl");
        }
        return NULL;
    }
    jmethodID ctor = (*env)->GetMethodID(env, cls, "<init>", "()V");
    if (ctor == NULL) {
        fprintf(stderr, TAG "GetMethodID(OutArchive7zImpl.<init>) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "GetMethodID OutArchive7zImpl <init>");
        }
        (*env)->DeleteLocalRef(env, cls);
        return NULL;
    }

    jobject instance = (*env)->NewObject(env, cls, ctor);
    if (instance == NULL || (*env)->ExceptionCheck(env)) {
        fprintf(stderr, TAG "NewObject(OutArchive7zImpl) failed\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "NewObject OutArchive7zImpl");
        }
        (*env)->DeleteLocalRef(env, cls);
        return NULL;
    }

    


    jclass superCls = (*env)->FindClass(env, "net/sf/sevenzipjbinding/impl/OutArchiveImpl");
    if (superCls == NULL) {
        fprintf(stderr, TAG "FindClass(OutArchiveImpl) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "FindClass OutArchiveImpl");
        }
        (*env)->DeleteLocalRef(env, instance);
        (*env)->DeleteLocalRef(env, cls);
        return NULL;
    }
    if ((*env)->IsInstanceOf(env, instance, superCls) != JNI_TRUE) {
        fprintf(stderr, TAG "constructed object is not an OutArchiveImpl\n");
        (*env)->DeleteLocalRef(env, superCls);
        (*env)->DeleteLocalRef(env, instance);
        (*env)->DeleteLocalRef(env, cls);
        return NULL;
    }

    


    (*env)->DeleteLocalRef(env, superCls);
    (*env)->DeleteLocalRef(env, cls);
    return instance; 
}
