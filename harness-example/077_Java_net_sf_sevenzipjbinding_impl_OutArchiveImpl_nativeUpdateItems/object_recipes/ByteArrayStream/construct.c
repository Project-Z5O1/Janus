
















































#include <jni.h>
#include <stdio.h>

#define TAG "[recipe:ByteArrayStream] "

static void clear_pending(JNIEnv* env, const char* where) {
    fprintf(stderr, TAG "JNI exception at %s\n", where);
    (*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);
}






jobject construct_ByteArrayStream(JNIEnv* env,
                                  const uint8_t* buffer,
                                  size_t length,
                                  size_t* offset) {
    if (offset == NULL) {
        fprintf(stderr, TAG "offset pointer is required\n");
        return NULL;
    }
    if (length - *offset < 4) {
        fprintf(stderr, TAG "fuzz buffer exhausted before maxSize (need 4 bytes)\n");
        return NULL;
    }
    uint32_t raw = (uint32_t)buffer[*offset]
                 | ((uint32_t)buffer[*offset + 1] << 8)
                 | ((uint32_t)buffer[*offset + 2] << 16)
                 | ((uint32_t)buffer[*offset + 3] << 24);
    *offset += 4;

    
    int32_t maxSize = (int32_t)((raw % (16u * 1024u * 1024u)) + 1u);
    const int initialSize = 1024; 

    jclass cls = (*env)->FindClass(env, "net/sf/sevenzipjbinding/util/ByteArrayStream");
    if (cls == NULL) {
        fprintf(stderr, TAG "FindClass(ByteArrayStream) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "FindClass ByteArrayStream");
        }
        return NULL;
    }
    jmethodID ctor = (*env)->GetMethodID(env, cls, "<init>", "(II)V");
    if (ctor == NULL) {
        fprintf(stderr, TAG "GetMethodID(ByteArrayStream.<init>(II)V) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "GetMethodID <init>(II)V");
        }
        (*env)->DeleteLocalRef(env, cls);
        return NULL;
    }

    
    struct {
        const char* name;
        const char* sig;
    } bridge[] = {
        {"write", "([B)I"},                  
        {"setSize", "(J)V"},                 
        {"seek", "(JI)J"},                   
        {"read", "([B)I"},                   
    };
    for (size_t i = 0; i < sizeof(bridge) / sizeof(bridge[0]); i++) {
        jmethodID m = (*env)->GetMethodID(env, cls, bridge[i].name, bridge[i].sig);
        if (m == NULL) {
            fprintf(stderr, TAG "GetMethodID(%s %s) returned NULL\n", bridge[i].name,
                    bridge[i].sig);
            if ((*env)->ExceptionCheck(env)) {
                clear_pending(env, "GetMethodID bridge method");
            }
            (*env)->DeleteLocalRef(env, cls);
            return NULL;
        }
    }

    jobject stream = (*env)->NewObject(env, cls, ctor, (jint)initialSize, (jint)maxSize);
    if (stream == NULL || (*env)->ExceptionCheck(env)) {
        fprintf(stderr, TAG "NewObject(ByteArrayStream) failed\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "NewObject ByteArrayStream");
        }
        (*env)->DeleteLocalRef(env, cls);
        return NULL;
    }

    
    jclass iface = (*env)->FindClass(env, "net/sf/sevenzipjbinding/ISequentialOutStream");
    if (iface == NULL) {
        fprintf(stderr, TAG "FindClass(ISequentialOutStream) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "FindClass ISequentialOutStream");
        }
        (*env)->DeleteLocalRef(env, stream);
        (*env)->DeleteLocalRef(env, cls);
        return NULL;
    }
    if ((*env)->IsInstanceOf(env, stream, iface) != JNI_TRUE) {
        fprintf(stderr, TAG "constructed object is not an ISequentialOutStream\n");
        (*env)->DeleteLocalRef(env, iface);
        (*env)->DeleteLocalRef(env, stream);
        (*env)->DeleteLocalRef(env, cls);
        return NULL;
    }

    (*env)->DeleteLocalRef(env, iface);
    (*env)->DeleteLocalRef(env, cls);
    return stream; 
}
