package qb.blogfuzz;

import java.util.Arrays;

/* JADX INFO: loaded from: classes3.dex */
public class Wrapper {
    public static final long TIME_TO_WAIT = 2000;
    private final byte[] mBuffer;

    public Wrapper(byte[] buffer) throws InterruptedException {
        byte[] bArr = new byte[buffer.length];
        this.mBuffer = bArr;
        System.arraycopy(buffer, 0, bArr, 0, bArr.length);
    }

    public byte[] getBuffer() {
        byte[] bArr = this.mBuffer;
        return Arrays.copyOf(bArr, bArr.length);
    }
}
