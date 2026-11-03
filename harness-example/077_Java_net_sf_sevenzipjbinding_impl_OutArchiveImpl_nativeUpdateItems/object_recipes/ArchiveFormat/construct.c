











































#include <jni.h>
#include <stdio.h>

#define TAG "[recipe:ArchiveFormat] "

static void clear_pending(JNIEnv* env, const char* where) {
    fprintf(stderr, TAG "JNI exception at %s\n", where);
    (*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);
}







jobject construct_ArchiveFormat_SevenZip(JNIEnv* env,
                                         const uint8_t* buffer,
                                         size_t length,
                                         size_t* offset) {
    (void)buffer; 
    (void)length;

    if (offset == NULL) {
        fprintf(stderr, TAG "offset pointer is required\n");
        return NULL;
    }

    jclass cls = (*env)->FindClass(env, "net/sf/sevenzipjbinding/ArchiveFormat");
    if (cls == NULL) {
        fprintf(stderr, TAG "FindClass(ArchiveFormat) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "FindClass ArchiveFormat");
        }
        return NULL;
    }

    jfieldID fid = (*env)->GetStaticFieldID(
        env, cls, "SEVEN_ZIP", "Lnet/sf/sevenzipjbinding/ArchiveFormat;");
    if (fid == NULL) {
        fprintf(stderr, TAG "GetStaticFieldID(SEVEN_ZIP) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "GetStaticFieldID SEVEN_ZIP");
        }
        (*env)->DeleteLocalRef(env, cls);
        return NULL;
    }

    jobject constant = (*env)->GetStaticObjectField(env, cls, fid);
    if (constant == NULL || (*env)->ExceptionCheck(env)) {
        fprintf(stderr, TAG "GetStaticObjectField(SEVEN_ZIP) failed\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "GetStaticObjectField SEVEN_ZIP");
        }
        (*env)->DeleteLocalRef(env, cls);
        return NULL;
    }

    



    if ((*env)->IsInstanceOf(env, constant, cls) != JNI_TRUE) {
        fprintf(stderr, TAG "SEVEN_ZIP static field is not an ArchiveFormat\n");
        (*env)->DeleteLocalRef(env, constant);
        (*env)->DeleteLocalRef(env, cls);
        return NULL;
    }

    




    jfieldID fidCodec = (*env)->GetFieldID(env, cls, "codecIndex", "I");
    if (fidCodec == NULL) {
        fprintf(stderr, TAG "GetFieldID(codecIndex:I) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "GetFieldID codecIndex");
        }
        (*env)->DeleteLocalRef(env, constant);
        (*env)->DeleteLocalRef(env, cls);
        return NULL;
    }
    jint codecIndex = (*env)->GetIntField(env, constant, fidCodec);
    if ((*env)->ExceptionCheck(env)) {
        clear_pending(env, "GetIntField codecIndex");
        (*env)->DeleteLocalRef(env, constant);
        (*env)->DeleteLocalRef(env, cls);
        return NULL;
    }
    if (codecIndex != -2) {
        fprintf(stderr,
                TAG "unexpected codecIndex=%d (expected -2: un-memoized SEVEN_ZIP)\n",
                (int)codecIndex);
        (*env)->DeleteLocalRef(env, constant);
        (*env)->DeleteLocalRef(env, cls);
        return NULL;
    }
    jfieldID fidMethod = (*env)->GetFieldID(env, cls, "methodName",
                                            "Ljava/lang/String;");
    if (fidMethod == NULL) {
        fprintf(stderr, TAG "GetFieldID(methodName) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "GetFieldID methodName");
        }
        (*env)->DeleteLocalRef(env, constant);
        (*env)->DeleteLocalRef(env, cls);
        return NULL;
    }
    jstring methodName = (jstring)(*env)->GetObjectField(env, constant, fidMethod);
    if (methodName == NULL || (*env)->ExceptionCheck(env)) {
        fprintf(stderr, TAG "GetStringField(methodName) failed\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "GetObjectField methodName");
        }
        (*env)->DeleteLocalRef(env, methodName);
        (*env)->DeleteLocalRef(env, constant);
        (*env)->DeleteLocalRef(env, cls);
        return NULL;
    }
    const char* nameUtf = (*env)->GetStringUTFChars(env, methodName, NULL);
    if (nameUtf == NULL) {
        fprintf(stderr, TAG "GetStringUTFChars(methodName) failed\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "GetStringUTFChars methodName");
        }
        (*env)->DeleteLocalRef(env, methodName);
        (*env)->DeleteLocalRef(env, constant);
        (*env)->DeleteLocalRef(env, cls);
        return NULL;
    }
    int nameOk = (nameUtf[0] == '7' && nameUtf[1] == 'z' && nameUtf[2] == '\0');
    if (!nameOk) {
        fprintf(stderr, TAG "unexpected methodName='%s' (expected \"7z\")\n", nameUtf);
    }
    (*env)->ReleaseStringUTFChars(env, methodName, nameUtf);
    (*env)->DeleteLocalRef(env, methodName);
    if (!nameOk) {
        (*env)->DeleteLocalRef(env, constant);
        (*env)->DeleteLocalRef(env, cls);
        return NULL;
    }

    


    (*env)->DeleteLocalRef(env, cls);
    return constant; 
}
