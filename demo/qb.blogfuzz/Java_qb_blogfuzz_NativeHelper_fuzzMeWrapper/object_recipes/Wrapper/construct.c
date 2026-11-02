jobject construct_Wrapper(JNIEnv* env, const uint8_t* buffer, size_t length, size_t* offset) {
  size_t remaining = (length > *offset) ? (length - *offset) : 0;
  if (remaining == 0) {
    remaining = 1;
    if (length == 0) {
      static const uint8_t dummy = 0;
      jbyteArray jBuffer = (*env)->NewByteArray(env, 1);
      (*env)->SetByteArrayRegion(env, jBuffer, 0, 1, (const jbyte*)&dummy);

      jclass cls = (*env)->FindClass(env, "qb/blogfuzz/Wrapper");
      if (!cls) { (*env)->DeleteLocalRef(env, jBuffer); return NULL; }

      jmethodID ctor = (*env)->GetMethodID(env, cls, "<init>", "([B)V");
      if (!ctor) { (*env)->DeleteLocalRef(env, cls); (*env)->DeleteLocalRef(env, jBuffer); return NULL; }

      jobject obj = (*env)->NewObject(env, cls, ctor, jBuffer);
      (*env)->DeleteLocalRef(env, cls);
      (*env)->DeleteLocalRef(env, jBuffer);
      return obj;
    }
  }

  jbyteArray jBuffer = (*env)->NewByteArray(env, remaining);
  if (!jBuffer) return NULL;
  (*env)->SetByteArrayRegion(env, jBuffer, 0, remaining, (const jbyte*)(buffer + *offset));
  *offset += remaining;

  jclass cls = (*env)->FindClass(env, "qb/blogfuzz/Wrapper");
  if (!cls) { (*env)->DeleteLocalRef(env, jBuffer); return NULL; }

  jmethodID ctor = (*env)->GetMethodID(env, cls, "<init>", "([B)V");
  if (!ctor) { (*env)->DeleteLocalRef(env, cls); (*env)->DeleteLocalRef(env, jBuffer); return NULL; }

  jobject obj = (*env)->NewObject(env, cls, ctor, jBuffer);
  (*env)->DeleteLocalRef(env, cls);
  (*env)->DeleteLocalRef(env, jBuffer);
  return obj;
}
