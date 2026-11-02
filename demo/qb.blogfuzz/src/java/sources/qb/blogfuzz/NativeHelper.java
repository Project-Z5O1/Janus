package qb.blogfuzz;

/* JADX INFO: loaded from: classes3.dex */
public class NativeHelper {
    public native void fuzzMeArray(byte[] bArr);

    public native void fuzzMeWrapper(Wrapper wrapper);

    static {
        System.loadLibrary("blogfuzz");
    }
}
