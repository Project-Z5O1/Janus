


























































#include <jni.h>
#include <stdio.h>
#include <stdint.h>

#define TAG "[recipe:Bitmap] "







#define OMEN_GIFLIB_GET_WIDTH_OFFSET  0x8270u
#define OMEN_GIFLIB_GET_HEIGHT_OFFSET 0x8288u

typedef jint (*omen_fn_get_width)(JNIEnv*, jclass, jlong);
typedef jint (*omen_fn_get_height)(JNIEnv*, jclass, jlong);



typedef struct {
    void* lib_base;    
    jlong  gif_handle; 
} omen_bitmap_gif_context;

static void clear_pending(JNIEnv* env, const char* where) {
    fprintf(stderr, TAG "JNI exception at %s\n", where);
    (*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);
}







jobject construct_Bitmap_wh(JNIEnv* env, jint width, jint height) {
    


    if (width <= 0 || height <= 0) {
        fprintf(stderr, TAG "refusing createBitmap(w=%d, h=%d): non-positive "
                        "dimensions are not app-real (degenerate LSD is "
                        "rejected by the openByteArray factory)\n",
                (int)width, (int)height);
        return NULL;
    }

    jclass cfgCls = (*env)->FindClass(env, "android/graphics/Bitmap$Config");
    if (cfgCls == NULL) {
        fprintf(stderr, TAG "FindClass(android/graphics/Bitmap$Config) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "FindClass Bitmap$Config");
        }
        return NULL;
    }
    jclass bmpCls = (*env)->FindClass(env, "android/graphics/Bitmap");
    if (bmpCls == NULL) {
        fprintf(stderr, TAG "FindClass(android/graphics/Bitmap) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "FindClass Bitmap");
        }
        (*env)->DeleteLocalRef(env, cfgCls);
        return NULL;
    }

    

    jmethodID createBitmap = (*env)->GetStaticMethodID(
        env, bmpCls, "createBitmap",
        "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
    if (createBitmap == NULL) {
        fprintf(stderr, TAG "GetStaticMethodID(Bitmap.createBitmap) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "GetStaticMethodID createBitmap");
        }
        (*env)->DeleteLocalRef(env, bmpCls);
        (*env)->DeleteLocalRef(env, cfgCls);
        return NULL;
    }

    

    jfieldID argb8888 = (*env)->GetStaticFieldID(
        env, cfgCls, "ARGB_8888", "Landroid/graphics/Bitmap$Config;");
    if (argb8888 == NULL) {
        fprintf(stderr, TAG "GetStaticFieldID(Bitmap$Config.ARGB_8888) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "GetStaticFieldID ARGB_8888");
        }
        (*env)->DeleteLocalRef(env, bmpCls);
        (*env)->DeleteLocalRef(env, cfgCls);
        return NULL;
    }
    jobject cfgObj = (*env)->GetStaticObjectField(env, cfgCls, argb8888);
    if (cfgObj == NULL) {
        fprintf(stderr, TAG "GetStaticObjectField(ARGB_8888) returned NULL\n");
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "GetStaticObjectField ARGB_8888");
        }
        (*env)->DeleteLocalRef(env, bmpCls);
        (*env)->DeleteLocalRef(env, cfgCls);
        return NULL;
    }

    jobject bitmap = (*env)->CallStaticObjectMethod(
        env, bmpCls, createBitmap, width, height, cfgObj);
    if (bitmap == NULL || (*env)->ExceptionCheck(env)) {
        




        fprintf(stderr, TAG "createBitmap(w=%d, h=%d, ARGB_8888) failed "
                        "(%.1f MiB request) — dimension boundary, exception "
                        "observed and cleared\n",
                (int)width, (int)height,
                ((double)width * (double)height * 4.0) / (1024.0 * 1024.0));
        if ((*env)->ExceptionCheck(env)) {
            clear_pending(env, "CallStaticObjectMethod createBitmap");
        }
        (*env)->DeleteLocalRef(env, cfgObj);
        (*env)->DeleteLocalRef(env, bmpCls);
        (*env)->DeleteLocalRef(env, cfgCls);
        return NULL;
    }

    if ((*env)->IsInstanceOf(env, bitmap, bmpCls) != JNI_TRUE) {
        fprintf(stderr, TAG "created object is not an android.graphics.Bitmap\n");
        (*env)->DeleteLocalRef(env, bitmap);
        (*env)->DeleteLocalRef(env, cfgObj);
        (*env)->DeleteLocalRef(env, bmpCls);
        (*env)->DeleteLocalRef(env, cfgCls);
        return NULL;
    }

    



    (*env)->DeleteLocalRef(env, cfgObj);
    (*env)->DeleteLocalRef(env, bmpCls);
    (*env)->DeleteLocalRef(env, cfgCls);
    return bitmap; 
}







jobject construct_Bitmap(JNIEnv* env, const omen_bitmap_gif_context* ctx) {
    if (ctx == NULL || ctx->lib_base == NULL) {
        fprintf(stderr, TAG "context requires the runtime base of "
                        "libpl_droidsonroids_gif.so (dlopen + get_library_base)\n");
        return NULL;
    }
    if (ctx->gif_handle == 0) {
        

        fprintf(stderr, TAG "zero GifInfo handle: openByteArray failed, "
                        "nothing to size a bitmap from\n");
        return NULL;
    }

    omen_fn_get_width get_width =
        (omen_fn_get_width)((uintptr_t)ctx->lib_base + OMEN_GIFLIB_GET_WIDTH_OFFSET);
    omen_fn_get_height get_height =
        (omen_fn_get_height)((uintptr_t)ctx->lib_base + OMEN_GIFLIB_GET_HEIGHT_OFFSET);

    

    jint w = get_width(env, NULL, ctx->gif_handle);
    if ((*env)->ExceptionCheck(env)) {
        clear_pending(env, "getWidth");
        return NULL;
    }
    jint h = get_height(env, NULL, ctx->gif_handle);
    if ((*env)->ExceptionCheck(env)) {
        clear_pending(env, "getHeight");
        return NULL;
    }
    fprintf(stderr, TAG "GIF logical screen from handle 0x%llx: %d x %d\n",
            (unsigned long long)ctx->gif_handle, (int)w, (int)h);

    return construct_Bitmap_wh(env, w, h);
}
