












































#include <jni.h>
#include <stdio.h>

#define TAG "[recipe:ExtractSlowCallback] "

static void clear_pending(JNIEnv* env, const char* where) {
    fprintf(stderr, TAG "JNI exception at %s\n", where);
    (*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);
}







jobject construct_ExtractSlowCallback(JNIEnv* env,
                                      const uint8_t* buffer,
                                      size_t length,
                                      size_t* offset,
                                      jobject out_stream) {
    (void)buffer; 
    (void)length;

    if (offset == NULL) {
        fprintf(stderr, TAG "offset pointer is required\n");
        return NULL;
    }
    if (out_stream == NULL) {
        fprintf(stderr,
                TAG "out_stream (ISequentialOutStream sink) is required: a NULL sink makes "
                    "getStream return null in EXTRACT mode and disables the decompression "
                    "write path (not an app-real value; refusing instead of masking)\n");
        return NULL;
    }

    
    jclass sinkIface = (*env)->FindClass(env, "net/sf/sevenzipjbinding/ISequentialOutStream");
    if (sinkIface == NULL) {
        fprintf(stderr, TAG "FindClass(ISequentialOutStream) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "FindClass ISequentialOutStream");
        }
        return NULL;
    }
    if ((*env)->IsInstanceOf(env, out_stream, sinkIface) != JNI_TRUE) {
        fprintf(stderr, TAG "out_stream is not an ISequentialOutStream\n");
        (*env)->DeleteLocalRef(env, sinkIface);
        return NULL;
    }
    (*env)->DeleteLocalRef(env, sinkIface);

    jclass cbCls =
        (*env)->FindClass(env, "net/sf/sevenzipjbinding/impl/InArchiveImpl$ExtractSlowCallback");
    if (cbCls == NULL) {
        fprintf(stderr, TAG "FindClass(InArchiveImpl$ExtractSlowCallback) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "FindClass ExtractSlowCallback");
        }
        return NULL;
    }
    jmethodID ctor = (*env)->GetMethodID(
        env, cbCls, "<init>", "(Lnet/sf/sevenzipjbinding/ISequentialOutStream;)V");
    if (ctor == NULL) {
        fprintf(stderr, TAG "GetMethodID(ExtractSlowCallback.<init>) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "GetMethodID <init>");
        }
        (*env)->DeleteLocalRef(env, cbCls);
        return NULL;
    }

    
    struct {
        const char* name;
        const char* sig;
    } bridge[] = {
        {"getStream",
         "(ILnet/sf/sevenzipjbinding/ExtractAskMode;)Lnet/sf/sevenzipjbinding/ISequentialOutStream;"},
        {"prepareOperation", "(Lnet/sf/sevenzipjbinding/ExtractAskMode;)V"},
        {"setOperationResult", "(Lnet/sf/sevenzipjbinding/ExtractOperationResult;)V"},
        {"setTotal", "(J)V"},
        {"setCompleted", "(J)V"},
    };
    for (size_t i = 0; i < sizeof(bridge) / sizeof(bridge[0]); i++) {
        jmethodID m = (*env)->GetMethodID(env, cbCls, bridge[i].name, bridge[i].sig);
        if (m == NULL) {
            fprintf(stderr, TAG "GetMethodID(%s %s) returned NULL\n", bridge[i].name,
                    bridge[i].sig);
            if ((*env)->ExceptionCheck(env)) {
                clear_pending(env, "GetMethodID bridge method");
            }
            (*env)->DeleteLocalRef(env, cbCls);
            return NULL;
        }
    }

    jobject callback = (*env)->NewObject(env, cbCls, ctor, out_stream);
    if (callback == NULL || (*env)->ExceptionCheck(env)) {
        fprintf(stderr, TAG "NewObject(ExtractSlowCallback) failed\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "NewObject ExtractSlowCallback");
        }
        (*env)->DeleteLocalRef(env, cbCls);
        return NULL;
    }

    
    jclass iface = (*env)->FindClass(env, "net/sf/sevenzipjbinding/IArchiveExtractCallback");
    if (iface == NULL) {
        fprintf(stderr, TAG "FindClass(IArchiveExtractCallback) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "FindClass IArchiveExtractCallback");
        }
        (*env)->DeleteLocalRef(env, callback);
        (*env)->DeleteLocalRef(env, cbCls);
        return NULL;
    }
    if ((*env)->IsInstanceOf(env, callback, iface) != JNI_TRUE) {
        fprintf(stderr, TAG "constructed object is not an IArchiveExtractCallback\n");
        (*env)->DeleteLocalRef(env, iface);
        (*env)->DeleteLocalRef(env, callback);
        (*env)->DeleteLocalRef(env, cbCls);
        return NULL;
    }

    (*env)->DeleteLocalRef(env, iface);
    (*env)->DeleteLocalRef(env, cbCls);
    return callback; 
}
