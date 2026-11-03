



























#include <jni.h>
#include <stdio.h>
#include <string.h>

#define TAG "[recipe:RandomAccessFileInStream] "

static void clear_pending(JNIEnv* env, const char* where) {
    fprintf(stderr, TAG "JNI exception at %s\n", where);
    (*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);
}






static jobject construct_RandomAccessFile(JNIEnv* env, const char* path, const char* mode) {
    jstring jpath = (*env)->NewStringUTF(env, path);
    if (jpath == NULL) {
        fprintf(stderr, TAG "NewStringUTF(path) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "NewStringUTF path");
        }
        return NULL;
    }
    jstring jmode = (*env)->NewStringUTF(env, mode);
    if (jmode == NULL) {
        fprintf(stderr, TAG "NewStringUTF(mode) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "NewStringUTF mode");
        }
        (*env)->DeleteLocalRef(env, jpath);
        return NULL;
    }

    jclass rafCls = (*env)->FindClass(env, "java/io/RandomAccessFile");
    if (rafCls == NULL) {
        fprintf(stderr, TAG "FindClass(java/io/RandomAccessFile) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "FindClass RandomAccessFile");
        }
        (*env)->DeleteLocalRef(env, jmode);
        (*env)->DeleteLocalRef(env, jpath);
        return NULL;
    }
    jmethodID rafCtor =
        (*env)->GetMethodID(env, rafCls, "<init>", "(Ljava/lang/String;Ljava/lang/String;)V");
    if (rafCtor == NULL) {
        fprintf(stderr, TAG "GetMethodID(RandomAccessFile.<init>) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "GetMethodID RandomAccessFile <init>");
        }
        (*env)->DeleteLocalRef(env, rafCls);
        (*env)->DeleteLocalRef(env, jmode);
        (*env)->DeleteLocalRef(env, jpath);
        return NULL;
    }

    jobject raf = (*env)->NewObject(env, rafCls, rafCtor, jpath, jmode);
    if (raf == NULL || (*env)->ExceptionCheck(env)) {
        fprintf(stderr, TAG "NewObject(RandomAccessFile) failed\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "NewObject RandomAccessFile");
        }
        (*env)->DeleteLocalRef(env, rafCls);
        (*env)->DeleteLocalRef(env, jmode);
        (*env)->DeleteLocalRef(env, jpath);
        return NULL;
    }

    

    (*env)->DeleteLocalRef(env, rafCls);
    (*env)->DeleteLocalRef(env, jmode);
    (*env)->DeleteLocalRef(env, jpath);
    return raf;
}





jobject construct_RandomAccessFileInStream(JNIEnv* env,
                                           const uint8_t* buffer,
                                           size_t length,
                                           size_t* offset,
                                           const char* staged_archive_path) {
    if (staged_archive_path == NULL || staged_archive_path[0] == '\0') {
        fprintf(stderr, TAG "staged_archive_path is required\n");
        return NULL;
    }
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

    FILE* f = fopen(staged_archive_path, "wb");
    if (f == NULL) {
        fprintf(stderr, TAG "fopen(%s, wb) failed\n", staged_archive_path);
        return NULL;
    }
    if (avail > 0 && fwrite(buffer + start, 1, avail, f) != avail) {
        fprintf(stderr, TAG "fwrite of %zu bytes failed\n", avail);
        fclose(f);
        return NULL;
    }
    if (fclose(f) != 0) {
        fprintf(stderr, TAG "fclose failed\n");
        return NULL;
    }
    *offset = length; 

    
    jobject raf = construct_RandomAccessFile(env, staged_archive_path, "r");
    if (raf == NULL) {
        return NULL;
    }

    
    jclass streamCls =
        (*env)->FindClass(env, "net/sf/sevenzipjbinding/impl/RandomAccessFileInStream");
    if (streamCls == NULL) {
        fprintf(stderr, TAG "FindClass(RandomAccessFileInStream) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "FindClass RandomAccessFileInStream");
        }
        (*env)->DeleteLocalRef(env, raf);
        return NULL;
    }
    jmethodID ctor =
        (*env)->GetMethodID(env, streamCls, "<init>", "(Ljava/io/RandomAccessFile;)V");
    if (ctor == NULL) {
        fprintf(stderr, TAG "GetMethodID(RandomAccessFileInStream.<init>) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "GetMethodID <init>");
        }
        (*env)->DeleteLocalRef(env, streamCls);
        (*env)->DeleteLocalRef(env, raf);
        return NULL;
    }

    
    jmethodID mRead = (*env)->GetMethodID(env, streamCls, "read", "([B)I");
    if (mRead == NULL) {
        fprintf(stderr, TAG "GetMethodID(read([B)I) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "GetMethodID read");
        }
        (*env)->DeleteLocalRef(env, streamCls);
        (*env)->DeleteLocalRef(env, raf);
        return NULL;
    }
    jmethodID mSeek = (*env)->GetMethodID(env, streamCls, "seek", "(JI)J");
    if (mSeek == NULL) {
        fprintf(stderr, TAG "GetMethodID(seek(JI)J) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "GetMethodID seek");
        }
        (*env)->DeleteLocalRef(env, streamCls);
        (*env)->DeleteLocalRef(env, raf);
        return NULL;
    }

    jobject stream = (*env)->NewObject(env, streamCls, ctor, raf);
    if (stream == NULL || (*env)->ExceptionCheck(env)) {
        fprintf(stderr, TAG "NewObject(RandomAccessFileInStream) failed\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "NewObject RandomAccessFileInStream");
        }
        (*env)->DeleteLocalRef(env, streamCls);
        (*env)->DeleteLocalRef(env, raf);
        return NULL;
    }

    
    jclass iface = (*env)->FindClass(env, "net/sf/sevenzipjbinding/IInStream");
    if (iface == NULL) {
        fprintf(stderr, TAG "FindClass(IInStream) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "FindClass IInStream");
        }
        (*env)->DeleteLocalRef(env, stream);
        (*env)->DeleteLocalRef(env, streamCls);
        (*env)->DeleteLocalRef(env, raf);
        return NULL;
    }
    if ((*env)->IsInstanceOf(env, stream, iface) != JNI_TRUE) {
        fprintf(stderr, TAG "constructed object is not an IInStream\n");
        (*env)->DeleteLocalRef(env, iface);
        (*env)->DeleteLocalRef(env, stream);
        (*env)->DeleteLocalRef(env, streamCls);
        (*env)->DeleteLocalRef(env, raf);
        return NULL;
    }

    

    (*env)->DeleteLocalRef(env, iface);
    (*env)->DeleteLocalRef(env, streamCls);
    (*env)->DeleteLocalRef(env, raf);
    return stream; 
}
