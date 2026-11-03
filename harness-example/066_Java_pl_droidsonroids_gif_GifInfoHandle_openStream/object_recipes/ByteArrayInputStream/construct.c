



































#include <jni.h>
#include <stdio.h>

#define TAG "[recipe:ByteArrayInputStream] "

static void clear_pending(JNIEnv* env, const char* where) {
    fprintf(stderr, TAG "JNI exception at %s\n", where);
    (*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);
}





jobject construct_ByteArrayInputStream(JNIEnv* env,
                                       const uint8_t* buffer,
                                       size_t length,
                                       size_t* offset) {
    if (offset == NULL) {
        fprintf(stderr, TAG "offset pointer is required\n");
        return NULL;
    }
    if (*offset > length) {
        fprintf(stderr, TAG "offset %zu exceeds buffer length %zu\n", *offset, length);
        return NULL;
    }
    size_t start = *offset;
    size_t avail = length - start;

    
    jbyteArray bytes = (*env)->NewByteArray(env, (jsize)avail);
    if (bytes == NULL) {
        fprintf(stderr, TAG "NewByteArray(%zu) returned NULL\n", avail);
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "NewByteArray");
        }
        return NULL;
    }
    if (avail > 0) {
        (*env)->SetByteArrayRegion(env, bytes, 0, (jsize)avail, (const jbyte*)(buffer + start));
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "SetByteArrayRegion");
            (*env)->DeleteLocalRef(env, bytes);
            return NULL;
        }
    }
    *offset = length; 

    
    jclass cls = (*env)->FindClass(env, "java/io/ByteArrayInputStream");
    if (cls == NULL) {
        fprintf(stderr, TAG "FindClass(java/io/ByteArrayInputStream) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "FindClass ByteArrayInputStream");
        }
        (*env)->DeleteLocalRef(env, bytes);
        return NULL;
    }
    jmethodID ctor = (*env)->GetMethodID(env, cls, "<init>", "([B)V");
    if (ctor == NULL) {
        fprintf(stderr, TAG "GetMethodID(ByteArrayInputStream.<init>) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "GetMethodID <init>");
        }
        (*env)->DeleteLocalRef(env, cls);
        (*env)->DeleteLocalRef(env, bytes);
        return NULL;
    }

    





    jmethodID mMark = (*env)->GetMethodID(env, cls, "mark", "(I)V");
    jmethodID mRead = (*env)->GetMethodID(env, cls, "read", "([BII)I");
    jmethodID mReset = (*env)->GetMethodID(env, cls, "reset", "()V");
    jmethodID mClose = (*env)->GetMethodID(env, cls, "close", "()V");
    if (mMark == NULL || mRead == NULL || mReset == NULL || mClose == NULL) {
        fprintf(stderr, TAG "native-invoked virtual missing (mark=%p read=%p reset=%p close=%p)\n",
                (void*)mMark, (void*)mRead, (void*)mReset, (void*)mClose);
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "GetMethodID native-invoked virtuals");
        }
        (*env)->DeleteLocalRef(env, cls);
        (*env)->DeleteLocalRef(env, bytes);
        return NULL;
    }

    jobject stream = (*env)->NewObject(env, cls, ctor, bytes);
    if (stream == NULL || (*env)->ExceptionCheck(env)) {
        fprintf(stderr, TAG "NewObject(ByteArrayInputStream) failed\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "NewObject ByteArrayInputStream");
        }
        (*env)->DeleteLocalRef(env, cls);
        (*env)->DeleteLocalRef(env, bytes);
        return NULL;
    }

    
    jclass iface = (*env)->FindClass(env, "java/io/InputStream");
    if (iface == NULL) {
        fprintf(stderr, TAG "FindClass(java/io/InputStream) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "FindClass InputStream");
        }
        (*env)->DeleteLocalRef(env, stream);
        (*env)->DeleteLocalRef(env, cls);
        (*env)->DeleteLocalRef(env, bytes);
        return NULL;
    }
    if ((*env)->IsInstanceOf(env, stream, iface) != JNI_TRUE) {
        fprintf(stderr, TAG "constructed object is not a java.io.InputStream\n");
        (*env)->DeleteLocalRef(env, iface);
        (*env)->DeleteLocalRef(env, stream);
        (*env)->DeleteLocalRef(env, cls);
        (*env)->DeleteLocalRef(env, bytes);
        return NULL;
    }

    







    (*env)->DeleteLocalRef(env, iface);
    (*env)->DeleteLocalRef(env, cls);
    (*env)->DeleteLocalRef(env, bytes);
    return stream; 
}
