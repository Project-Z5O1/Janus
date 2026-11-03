




































#include <jni.h>
#include <stdio.h>

#define TAG "[recipe:SequentialOutStream] "

static void clear_pending(JNIEnv* env, const char* where) {
    fprintf(stderr, TAG "JNI exception at %s\n", where);
    (*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);
}






jobject construct_SequentialOutStream(JNIEnv* env,
                                      const uint8_t* buffer,
                                      size_t length,
                                      size_t* offset,
                                      const char* staged_out_path) {
    (void)buffer; 
    (void)length;

    if (staged_out_path == NULL || staged_out_path[0] == '\0') {
        fprintf(stderr, TAG "staged_out_path is required\n");
        return NULL;
    }
    if (offset == NULL) {
        fprintf(stderr, TAG "offset pointer is required\n");
        return NULL;
    }

    
    jstring jpath = (*env)->NewStringUTF(env, staged_out_path);
    if (jpath == NULL) {
        fprintf(stderr, TAG "NewStringUTF(path) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "NewStringUTF path");
        }
        return NULL;
    }

    
    jclass weakCls = (*env)->FindClass(env, "java/lang/ref/WeakReference");
    if (weakCls == NULL) {
        fprintf(stderr, TAG "FindClass(java/lang/ref/WeakReference) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "FindClass WeakReference");
        }
        (*env)->DeleteLocalRef(env, jpath);
        return NULL;
    }
    jmethodID weakCtor =
        (*env)->GetMethodID(env, weakCls, "<init>", "(Ljava/lang/Object;)V");
    if (weakCtor == NULL) {
        fprintf(stderr, TAG "GetMethodID(WeakReference.<init>) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "GetMethodID WeakReference <init>");
        }
        (*env)->DeleteLocalRef(env, weakCls);
        (*env)->DeleteLocalRef(env, jpath);
        return NULL;
    }
    jobject weakRef = (*env)->NewObject(env, weakCls, weakCtor, NULL);
    if (weakRef == NULL || (*env)->ExceptionCheck(env)) {
        fprintf(stderr, TAG "NewObject(WeakReference, null) failed\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "NewObject WeakReference");
        }
        (*env)->DeleteLocalRef(env, weakCls);
        (*env)->DeleteLocalRef(env, jpath);
        return NULL;
    }

    
    jclass streamCls =
        (*env)->FindClass(env, "com/android/fileexplorer/model/SevenZHelper$SequentialOutStream");
    if (streamCls == NULL) {
        fprintf(stderr, TAG "FindClass(SevenZHelper$SequentialOutStream) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "FindClass SequentialOutStream");
        }
        (*env)->DeleteLocalRef(env, weakRef);
        (*env)->DeleteLocalRef(env, weakCls);
        (*env)->DeleteLocalRef(env, jpath);
        return NULL;
    }
    jmethodID ctor = (*env)->GetMethodID(
        env, streamCls, "<init>", "(Ljava/lang/String;Ljava/lang/ref/WeakReference;)V");
    if (ctor == NULL) {
        fprintf(stderr, TAG "GetMethodID(SequentialOutStream.<init>) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "GetMethodID <init>");
        }
        (*env)->DeleteLocalRef(env, streamCls);
        (*env)->DeleteLocalRef(env, weakRef);
        (*env)->DeleteLocalRef(env, weakCls);
        (*env)->DeleteLocalRef(env, jpath);
        return NULL;
    }

    
    jmethodID mWrite = (*env)->GetMethodID(env, streamCls, "write", "([B)I");
    if (mWrite == NULL) {
        fprintf(stderr, TAG "GetMethodID(write([B)I) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "GetMethodID write");
        }
        (*env)->DeleteLocalRef(env, streamCls);
        (*env)->DeleteLocalRef(env, weakRef);
        (*env)->DeleteLocalRef(env, weakCls);
        (*env)->DeleteLocalRef(env, jpath);
        return NULL;
    }

    
    jobject stream = (*env)->NewObject(env, streamCls, ctor, jpath, weakRef);
    if (stream == NULL || (*env)->ExceptionCheck(env)) {
        fprintf(stderr, TAG "NewObject(SequentialOutStream) failed\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "NewObject SequentialOutStream");
        }
        (*env)->DeleteLocalRef(env, streamCls);
        (*env)->DeleteLocalRef(env, weakRef);
        (*env)->DeleteLocalRef(env, weakCls);
        (*env)->DeleteLocalRef(env, jpath);
        return NULL;
    }

    
    jclass iface = (*env)->FindClass(env, "net/sf/sevenzipjbinding/ISequentialOutStream");
    if (iface == NULL) {
        fprintf(stderr, TAG "FindClass(ISequentialOutStream) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "FindClass ISequentialOutStream");
        }
        (*env)->DeleteLocalRef(env, stream);
        (*env)->DeleteLocalRef(env, streamCls);
        (*env)->DeleteLocalRef(env, weakRef);
        (*env)->DeleteLocalRef(env, weakCls);
        (*env)->DeleteLocalRef(env, jpath);
        return NULL;
    }
    if ((*env)->IsInstanceOf(env, stream, iface) != JNI_TRUE) {
        fprintf(stderr, TAG "constructed object is not an ISequentialOutStream\n");
        (*env)->DeleteLocalRef(env, iface);
        (*env)->DeleteLocalRef(env, stream);
        (*env)->DeleteLocalRef(env, streamCls);
        (*env)->DeleteLocalRef(env, weakRef);
        (*env)->DeleteLocalRef(env, weakCls);
        (*env)->DeleteLocalRef(env, jpath);
        return NULL;
    }

    

    (*env)->DeleteLocalRef(env, iface);
    (*env)->DeleteLocalRef(env, streamCls);
    (*env)->DeleteLocalRef(env, weakRef);
    (*env)->DeleteLocalRef(env, weakCls);
    (*env)->DeleteLocalRef(env, jpath);
    return stream; 
}
