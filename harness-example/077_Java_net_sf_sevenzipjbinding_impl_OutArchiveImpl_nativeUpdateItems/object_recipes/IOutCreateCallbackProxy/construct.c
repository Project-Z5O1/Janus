



























































#include <jni.h>
#include <stdio.h>
#include <string.h>

#define TAG "[recipe:IOutCreateCallbackProxy] "

static void clear_pending(JNIEnv* env, const char* where) {
    fprintf(stderr, TAG "JNI exception at %s\n", where);
    (*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);
}











jobject construct_IOutCreateCallbackProxy(JNIEnv* env,
                                          const uint8_t* buffer,
                                          size_t length,
                                          size_t* offset) {
    if (offset == NULL) {
        fprintf(stderr, TAG "offset pointer is required\n");
        return NULL;
    }
    size_t start = *offset;
    size_t remaining = (length > start) ? (length - start) : 0;

    
    jbyteArray fuzzBytes = NULL;
    if (remaining > 0) {
        fuzzBytes = (*env)->NewByteArray(env, (jsize)remaining);
        if (fuzzBytes == NULL || (*env)->ExceptionCheck(env)) {
            fprintf(stderr, TAG "NewByteArray(%zu) failed\n", remaining);
            if ((*env)->ExceptionCheck(env)) {
                clear_pending(env, "NewByteArray");
            }
            return NULL;
        }
        
        void* pinned = (void*)(*env)->GetPrimitiveArrayCritical(env, fuzzBytes, NULL);
        if (pinned == NULL) {
            fprintf(stderr, TAG "GetPrimitiveArrayCritical failed\n");
            (*env)->DeleteLocalRef(env, fuzzBytes);
            return NULL;
        }
        memcpy(pinned, buffer + start, remaining);
        (*env)->ReleasePrimitiveArrayCritical(env, fuzzBytes, pinned, 0);
    }

    
    jclass handlerCls = (*env)->FindClass(env, "omen/jni/OmenUpdateCallbackHandler");
    if (handlerCls == NULL) {
        fprintf(stderr, TAG "FindClass(omen/jni/OmenUpdateCallbackHandler) returned NULL"
                " — stage omen-handler.dex on the device classpath\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "FindClass handler");
        }
        if (fuzzBytes != NULL) {
            (*env)->DeleteLocalRef(env, fuzzBytes);
        }
        return NULL;
    }
    jmethodID handlerCtor =
        (*env)->GetMethodID(env, handlerCls, "<init>", "([B)V");
    if (handlerCtor == NULL) {
        fprintf(stderr, TAG "GetMethodID(handler <init>([B)V) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "GetMethodID handler <init>");
        }
        (*env)->DeleteLocalRef(env, handlerCls);
        if (fuzzBytes != NULL) {
            (*env)->DeleteLocalRef(env, fuzzBytes);
        }
        return NULL;
    }
    jobject handler =
        (*env)->NewObject(env, handlerCls, handlerCtor, (jobject)fuzzBytes);
    if (handler == NULL || (*env)->ExceptionCheck(env)) {
        fprintf(stderr, TAG "NewObject(handler) failed\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "NewObject handler");
        }
        (*env)->DeleteLocalRef(env, handlerCls);
        if (fuzzBytes != NULL) {
            (*env)->DeleteLocalRef(env, fuzzBytes);
        }
        return NULL;
    }
    if (fuzzBytes != NULL) {
        (*env)->DeleteLocalRef(env, fuzzBytes); 
    }

    

    jclass ifaceCls = (*env)->FindClass(env, "net/sf/sevenzipjbinding/IOutCreateCallback");
    if (ifaceCls == NULL) {
        fprintf(stderr, TAG "FindClass(IOutCreateCallback) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "FindClass IOutCreateCallback");
        }
        (*env)->DeleteLocalRef(env, handler);
        (*env)->DeleteLocalRef(env, handlerCls);
        return NULL;
    }
    jclass classCls = (*env)->FindClass(env, "java/lang/Class");
    if (classCls == NULL) {
        fprintf(stderr, TAG "FindClass(java/lang/Class) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "FindClass Class");
        }
        (*env)->DeleteLocalRef(env, ifaceCls);
        (*env)->DeleteLocalRef(env, handler);
        (*env)->DeleteLocalRef(env, handlerCls);
        return NULL;
    }
    jobjectArray ifaceArray =
        (*env)->NewObjectArray(env, 1, classCls, (jobject)ifaceCls);
    if (ifaceArray == NULL || (*env)->ExceptionCheck(env)) {
        fprintf(stderr, TAG "NewObjectArray(interfaces) failed\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "NewObjectArray interfaces");
        }
        (*env)->DeleteLocalRef(env, classCls);
        (*env)->DeleteLocalRef(env, ifaceCls);
        (*env)->DeleteLocalRef(env, handler);
        (*env)->DeleteLocalRef(env, handlerCls);
        return NULL;
    }

    



    jclass proxyCls = (*env)->FindClass(env, "java/lang/reflect/Proxy");
    if (proxyCls == NULL) {
        fprintf(stderr, TAG "FindClass(java/lang/reflect/Proxy) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "FindClass Proxy");
        }
        (*env)->DeleteLocalRef(env, ifaceArray);
        (*env)->DeleteLocalRef(env, classCls);
        (*env)->DeleteLocalRef(env, ifaceCls);
        (*env)->DeleteLocalRef(env, handler);
        (*env)->DeleteLocalRef(env, handlerCls);
        return NULL;
    }
    jmethodID npi = (*env)->GetStaticMethodID(
        env, proxyCls, "newProxyInstance",
        "(Ljava/lang/ClassLoader;[Ljava/lang/Class;Ljava/lang/reflect/InvocationHandler;)Ljava/lang/Object;");
    if (npi == NULL) {
        fprintf(stderr, TAG "GetStaticMethodID(newProxyInstance) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "GetStaticMethodID newProxyInstance");
        }
        (*env)->DeleteLocalRef(env, proxyCls);
        (*env)->DeleteLocalRef(env, ifaceArray);
        (*env)->DeleteLocalRef(env, classCls);
        (*env)->DeleteLocalRef(env, ifaceCls);
        (*env)->DeleteLocalRef(env, handler);
        (*env)->DeleteLocalRef(env, handlerCls);
        return NULL;
    }
    jclass classLoaderCls = (*env)->FindClass(env, "java/lang/ClassLoader");
    if (classLoaderCls == NULL) {
        fprintf(stderr, TAG "FindClass(java/lang/ClassLoader) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "FindClass ClassLoader");
        }
        (*env)->DeleteLocalRef(env, proxyCls);
        (*env)->DeleteLocalRef(env, ifaceArray);
        (*env)->DeleteLocalRef(env, classCls);
        (*env)->DeleteLocalRef(env, ifaceCls);
        (*env)->DeleteLocalRef(env, handler);
        (*env)->DeleteLocalRef(env, handlerCls);
        return NULL;
    }
    jmethodID getClassLoader = (*env)->GetMethodID(
        env, classCls, "getClassLoader", "()Ljava/lang/ClassLoader;");
    if (getClassLoader == NULL) {
        fprintf(stderr, TAG "GetMethodID(Class.getClassLoader) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "GetMethodID getClassLoader");
        }
        (*env)->DeleteLocalRef(env, classLoaderCls);
        (*env)->DeleteLocalRef(env, proxyCls);
        (*env)->DeleteLocalRef(env, ifaceArray);
        (*env)->DeleteLocalRef(env, classCls);
        (*env)->DeleteLocalRef(env, ifaceCls);
        (*env)->DeleteLocalRef(env, handler);
        (*env)->DeleteLocalRef(env, handlerCls);
        return NULL;
    }
    jobject loader = (*env)->CallObjectMethod(env, ifaceCls, getClassLoader);
    if (loader == NULL && (*env)->ExceptionCheck(env)) {
        fprintf(stderr, TAG "getClassLoader() failed\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "CallObjectMethod getClassLoader");
        }
        (*env)->DeleteLocalRef(env, classLoaderCls);
        (*env)->DeleteLocalRef(env, proxyCls);
        (*env)->DeleteLocalRef(env, ifaceArray);
        (*env)->DeleteLocalRef(env, classCls);
        (*env)->DeleteLocalRef(env, ifaceCls);
        (*env)->DeleteLocalRef(env, handler);
        (*env)->DeleteLocalRef(env, handlerCls);
        return NULL;
    }

    jobject proxy = (*env)->CallStaticObjectMethod(
        env, proxyCls, npi, loader, ifaceArray, handler);
    if (proxy == NULL || (*env)->ExceptionCheck(env)) {
        fprintf(stderr, TAG "Proxy.newProxyInstance failed\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "Proxy.newProxyInstance");
        }
        if (loader != NULL) {
            (*env)->DeleteLocalRef(env, loader);
        }
        (*env)->DeleteLocalRef(env, classLoaderCls);
        (*env)->DeleteLocalRef(env, proxyCls);
        (*env)->DeleteLocalRef(env, ifaceArray);
        (*env)->DeleteLocalRef(env, classCls);
        (*env)->DeleteLocalRef(env, ifaceCls);
        (*env)->DeleteLocalRef(env, handler);
        (*env)->DeleteLocalRef(env, handlerCls);
        return NULL;
    }

    
    if ((*env)->IsInstanceOf(env, proxy, ifaceCls) != JNI_TRUE) {
        fprintf(stderr, TAG "proxy is not an IOutCreateCallback\n");
        (*env)->DeleteLocalRef(env, proxy);
        if (loader != NULL) {
            (*env)->DeleteLocalRef(env, loader);
        }
        (*env)->DeleteLocalRef(env, classLoaderCls);
        (*env)->DeleteLocalRef(env, proxyCls);
        (*env)->DeleteLocalRef(env, ifaceArray);
        (*env)->DeleteLocalRef(env, classCls);
        (*env)->DeleteLocalRef(env, ifaceCls);
        (*env)->DeleteLocalRef(env, handler);
        (*env)->DeleteLocalRef(env, handlerCls);
        return NULL;
    }

    


    if (loader != NULL) {
        (*env)->DeleteLocalRef(env, loader);
    }
    (*env)->DeleteLocalRef(env, classLoaderCls);
    (*env)->DeleteLocalRef(env, proxyCls);
    (*env)->DeleteLocalRef(env, ifaceArray);
    (*env)->DeleteLocalRef(env, classCls);
    (*env)->DeleteLocalRef(env, ifaceCls);
    (*env)->DeleteLocalRef(env, handler);
    (*env)->DeleteLocalRef(env, handlerCls);
    return proxy; 
}
