package kotlin;

import androidx.constraintlayout.widget.ConstraintLayout;
import kotlin.jvm.JvmInline;
import kotlin.ranges.UIntRange;

/* JADX INFO: compiled from: UInt.kt */
/* JADX INFO: loaded from: classes.dex */
@Metadata(d1 = {"\u0000n\n\u0002\u0018\u0002\n\u0002\u0010\u000f\n\u0000\n\u0002\u0010\b\n\u0002\b\t\n\u0002\u0018\u0002\n\u0002\b\u0003\n\u0002\u0018\u0002\n\u0002\b\u0002\n\u0002\u0018\u0002\n\u0002\b\u000b\n\u0002\u0010\u000b\n\u0002\u0010\u0000\n\u0002\b!\n\u0002\u0018\u0002\n\u0002\b\u0012\n\u0002\u0010\u0005\n\u0002\b\u0003\n\u0002\u0010\u0006\n\u0002\b\u0003\n\u0002\u0010\u0007\n\u0002\b\u0005\n\u0002\u0010\t\n\u0002\b\u0003\n\u0002\u0010\n\n\u0002\b\u0003\n\u0002\u0010\u000e\n\u0002\b\u000e\b\u0087@\u0018\u0000 y2\b\u0012\u0004\u0012\u00020\u00000\u0001:\u0001yB\u0014\b\u0001\u0012\u0006\u0010\u0002\u001a\u00020\u0003ø\u0001\u0000¢\u0006\u0004\b\u0004\u0010\u0005J\u001b\u0010\b\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0000H\u0087\fø\u0001\u0000¢\u0006\u0004\b\n\u0010\u000bJ\u001b\u0010\f\u001a\u00020\u00032\u0006\u0010\t\u001a\u00020\rH\u0087\nø\u0001\u0000¢\u0006\u0004\b\u000e\u0010\u000fJ\u001b\u0010\f\u001a\u00020\u00032\u0006\u0010\t\u001a\u00020\u0000H\u0097\nø\u0001\u0000¢\u0006\u0004\b\u0010\u0010\u000bJ\u001b\u0010\f\u001a\u00020\u00032\u0006\u0010\t\u001a\u00020\u0011H\u0087\nø\u0001\u0000¢\u0006\u0004\b\u0012\u0010\u0013J\u001b\u0010\f\u001a\u00020\u00032\u0006\u0010\t\u001a\u00020\u0014H\u0087\nø\u0001\u0000¢\u0006\u0004\b\u0015\u0010\u0016J\u0016\u0010\u0017\u001a\u00020\u0000H\u0087\nø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\b\u0018\u0010\u0005J\u001b\u0010\u0019\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\rH\u0087\nø\u0001\u0000¢\u0006\u0004\b\u001a\u0010\u000fJ\u001b\u0010\u0019\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0000H\u0087\nø\u0001\u0000¢\u0006\u0004\b\u001b\u0010\u000bJ\u001b\u0010\u0019\u001a\u00020\u00112\u0006\u0010\t\u001a\u00020\u0011H\u0087\nø\u0001\u0000¢\u0006\u0004\b\u001c\u0010\u001dJ\u001b\u0010\u0019\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0014H\u0087\nø\u0001\u0000¢\u0006\u0004\b\u001e\u0010\u0016J\u001a\u0010\u001f\u001a\u00020 2\b\u0010\t\u001a\u0004\u0018\u00010!HÖ\u0003¢\u0006\u0004\b\"\u0010#J\u001b\u0010$\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\rH\u0087\bø\u0001\u0000¢\u0006\u0004\b%\u0010\u000fJ\u001b\u0010$\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0000H\u0087\bø\u0001\u0000¢\u0006\u0004\b&\u0010\u000bJ\u001b\u0010$\u001a\u00020\u00112\u0006\u0010\t\u001a\u00020\u0011H\u0087\bø\u0001\u0000¢\u0006\u0004\b'\u0010\u001dJ\u001b\u0010$\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0014H\u0087\bø\u0001\u0000¢\u0006\u0004\b(\u0010\u0016J\u0010\u0010)\u001a\u00020\u0003HÖ\u0001¢\u0006\u0004\b*\u0010\u0005J\u0016\u0010+\u001a\u00020\u0000H\u0087\nø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\b,\u0010\u0005J\u0016\u0010-\u001a\u00020\u0000H\u0087\bø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\b.\u0010\u0005J\u001b\u0010/\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\rH\u0087\nø\u0001\u0000¢\u0006\u0004\b0\u0010\u000fJ\u001b\u0010/\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0000H\u0087\nø\u0001\u0000¢\u0006\u0004\b1\u0010\u000bJ\u001b\u0010/\u001a\u00020\u00112\u0006\u0010\t\u001a\u00020\u0011H\u0087\nø\u0001\u0000¢\u0006\u0004\b2\u0010\u001dJ\u001b\u0010/\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0014H\u0087\nø\u0001\u0000¢\u0006\u0004\b3\u0010\u0016J\u001b\u00104\u001a\u00020\r2\u0006\u0010\t\u001a\u00020\rH\u0087\bø\u0001\u0000¢\u0006\u0004\b5\u00106J\u001b\u00104\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0000H\u0087\bø\u0001\u0000¢\u0006\u0004\b7\u0010\u000bJ\u001b\u00104\u001a\u00020\u00112\u0006\u0010\t\u001a\u00020\u0011H\u0087\bø\u0001\u0000¢\u0006\u0004\b8\u0010\u001dJ\u001b\u00104\u001a\u00020\u00142\u0006\u0010\t\u001a\u00020\u0014H\u0087\bø\u0001\u0000¢\u0006\u0004\b9\u0010:J\u001b\u0010;\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0000H\u0087\fø\u0001\u0000¢\u0006\u0004\b<\u0010\u000bJ\u001b\u0010=\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\rH\u0087\nø\u0001\u0000¢\u0006\u0004\b>\u0010\u000fJ\u001b\u0010=\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0000H\u0087\nø\u0001\u0000¢\u0006\u0004\b?\u0010\u000bJ\u001b\u0010=\u001a\u00020\u00112\u0006\u0010\t\u001a\u00020\u0011H\u0087\nø\u0001\u0000¢\u0006\u0004\b@\u0010\u001dJ\u001b\u0010=\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0014H\u0087\nø\u0001\u0000¢\u0006\u0004\bA\u0010\u0016J\u001b\u0010B\u001a\u00020C2\u0006\u0010\t\u001a\u00020\u0000H\u0087\nø\u0001\u0000¢\u0006\u0004\bD\u0010EJ\u001b\u0010F\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\rH\u0087\nø\u0001\u0000¢\u0006\u0004\bG\u0010\u000fJ\u001b\u0010F\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0000H\u0087\nø\u0001\u0000¢\u0006\u0004\bH\u0010\u000bJ\u001b\u0010F\u001a\u00020\u00112\u0006\u0010\t\u001a\u00020\u0011H\u0087\nø\u0001\u0000¢\u0006\u0004\bI\u0010\u001dJ\u001b\u0010F\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0014H\u0087\nø\u0001\u0000¢\u0006\u0004\bJ\u0010\u0016J\u001e\u0010K\u001a\u00020\u00002\u0006\u0010L\u001a\u00020\u0003H\u0087\fø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\bM\u0010\u000bJ\u001e\u0010N\u001a\u00020\u00002\u0006\u0010L\u001a\u00020\u0003H\u0087\fø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\bO\u0010\u000bJ\u001b\u0010P\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\rH\u0087\nø\u0001\u0000¢\u0006\u0004\bQ\u0010\u000fJ\u001b\u0010P\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0000H\u0087\nø\u0001\u0000¢\u0006\u0004\bR\u0010\u000bJ\u001b\u0010P\u001a\u00020\u00112\u0006\u0010\t\u001a\u00020\u0011H\u0087\nø\u0001\u0000¢\u0006\u0004\bS\u0010\u001dJ\u001b\u0010P\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0014H\u0087\nø\u0001\u0000¢\u0006\u0004\bT\u0010\u0016J\u0010\u0010U\u001a\u00020VH\u0087\b¢\u0006\u0004\bW\u0010XJ\u0010\u0010Y\u001a\u00020ZH\u0087\b¢\u0006\u0004\b[\u0010\\J\u0010\u0010]\u001a\u00020^H\u0087\b¢\u0006\u0004\b_\u0010`J\u0010\u0010a\u001a\u00020\u0003H\u0087\b¢\u0006\u0004\bb\u0010\u0005J\u0010\u0010c\u001a\u00020dH\u0087\b¢\u0006\u0004\be\u0010fJ\u0010\u0010g\u001a\u00020hH\u0087\b¢\u0006\u0004\bi\u0010jJ\u000f\u0010k\u001a\u00020lH\u0016¢\u0006\u0004\bm\u0010nJ\u0016\u0010o\u001a\u00020\rH\u0087\bø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\bp\u0010XJ\u0016\u0010q\u001a\u00020\u0000H\u0087\bø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\br\u0010\u0005J\u0016\u0010s\u001a\u00020\u0011H\u0087\bø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\bt\u0010fJ\u0016\u0010u\u001a\u00020\u0014H\u0087\bø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\bv\u0010jJ\u001b\u0010w\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0000H\u0087\fø\u0001\u0000¢\u0006\u0004\bx\u0010\u000bR\u0016\u0010\u0002\u001a\u00020\u00038\u0000X\u0081\u0004¢\u0006\b\n\u0000\u0012\u0004\b\u0006\u0010\u0007\u0088\u0001\u0002\u0092\u0001\u00020\u0003ø\u0001\u0000\u0082\u0002\b\n\u0002\b\u0019\n\u0002\b!¨\u0006z"}, d2 = {"Lkotlin/UInt;", "", "data", "", "constructor-impl", "(I)I", "getData$annotations", "()V", "and", "other", "and-WZ4Q5Ns", "(II)I", "compareTo", "Lkotlin/UByte;", "compareTo-7apg3OU", "(IB)I", "compareTo-WZ4Q5Ns", "Lkotlin/ULong;", "compareTo-VKZWuLQ", "(IJ)I", "Lkotlin/UShort;", "compareTo-xj2QHRw", "(IS)I", "dec", "dec-pVg5ArA", "div", "div-7apg3OU", "div-WZ4Q5Ns", "div-VKZWuLQ", "(IJ)J", "div-xj2QHRw", "equals", "", "", "equals-impl", "(ILjava/lang/Object;)Z", "floorDiv", "floorDiv-7apg3OU", "floorDiv-WZ4Q5Ns", "floorDiv-VKZWuLQ", "floorDiv-xj2QHRw", "hashCode", "hashCode-impl", "inc", "inc-pVg5ArA", "inv", "inv-pVg5ArA", "minus", "minus-7apg3OU", "minus-WZ4Q5Ns", "minus-VKZWuLQ", "minus-xj2QHRw", "mod", "mod-7apg3OU", "(IB)B", "mod-WZ4Q5Ns", "mod-VKZWuLQ", "mod-xj2QHRw", "(IS)S", "or", "or-WZ4Q5Ns", "plus", "plus-7apg3OU", "plus-WZ4Q5Ns", "plus-VKZWuLQ", "plus-xj2QHRw", "rangeTo", "Lkotlin/ranges/UIntRange;", "rangeTo-WZ4Q5Ns", "(II)Lkotlin/ranges/UIntRange;", "rem", "rem-7apg3OU", "rem-WZ4Q5Ns", "rem-VKZWuLQ", "rem-xj2QHRw", "shl", "bitCount", "shl-pVg5ArA", "shr", "shr-pVg5ArA", "times", "times-7apg3OU", "times-WZ4Q5Ns", "times-VKZWuLQ", "times-xj2QHRw", "toByte", "", "toByte-impl", "(I)B", "toDouble", "", "toDouble-impl", "(I)D", "toFloat", "", "toFloat-impl", "(I)F", "toInt", "toInt-impl", "toLong", "", "toLong-impl", "(I)J", "toShort", "", "toShort-impl", "(I)S", "toString", "", "toString-impl", "(I)Ljava/lang/String;", "toUByte", "toUByte-w2LRezQ", "toUInt", "toUInt-pVg5ArA", "toULong", "toULong-s-VKNKU", "toUShort", "toUShort-Mh2AYeg", "xor", "xor-WZ4Q5Ns", "Companion", "kotlin-stdlib"}, k = 1, mv = {1, 7, 1}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
@JvmInline
public final class UInt implements Comparable<UInt> {
    public static final int MAX_VALUE = -1;
    public static final int MIN_VALUE = 0;
    public static final int SIZE_BITS = 32;
    public static final int SIZE_BYTES = 4;
    private final int data;

    /* JADX INFO: renamed from: box-impl, reason: not valid java name */
    public static final /* synthetic */ UInt m186boximpl(int i) {
        return new UInt(i);
    }

    /* JADX INFO: renamed from: constructor-impl, reason: not valid java name */
    public static int m192constructorimpl(int i) {
        return i;
    }

    /* JADX INFO: renamed from: equals-impl, reason: not valid java name */
    public static boolean m198equalsimpl(int i, Object obj) {
        return (obj instanceof UInt) && i == ((UInt) obj).getData();
    }

    /* JADX INFO: renamed from: equals-impl0, reason: not valid java name */
    public static final boolean m199equalsimpl0(int i, int i2) {
        return i == i2;
    }

    public static /* synthetic */ void getData$annotations() {
    }

    /* JADX INFO: renamed from: hashCode-impl, reason: not valid java name */
    public static int m204hashCodeimpl(int i) {
        return i;
    }

    public boolean equals(Object obj) {
        return m198equalsimpl(this.data, obj);
    }

    public int hashCode() {
        return m204hashCodeimpl(this.data);
    }

    /* JADX INFO: renamed from: unbox-impl, reason: not valid java name and from getter */
    public final /* synthetic */ int getData() {
        return this.data;
    }

    @Override // java.lang.Comparable
    public /* bridge */ /* synthetic */ int compareTo(UInt uInt) {
        return UnsignedKt.uintCompare(getData(), uInt.getData());
    }

    private /* synthetic */ UInt(int data) {
        this.data = data;
    }

    /* JADX INFO: renamed from: compareTo-7apg3OU, reason: not valid java name */
    private static final int m187compareTo7apg3OU(int arg0, byte other) {
        return UnsignedKt.uintCompare(arg0, m192constructorimpl(other & UByte.MAX_VALUE));
    }

    /* JADX INFO: renamed from: compareTo-xj2QHRw, reason: not valid java name */
    private static final int m191compareToxj2QHRw(int arg0, short other) {
        return UnsignedKt.uintCompare(arg0, m192constructorimpl(65535 & other));
    }

    /* JADX INFO: renamed from: compareTo-WZ4Q5Ns, reason: not valid java name */
    private int m189compareToWZ4Q5Ns(int other) {
        return UnsignedKt.uintCompare(getData(), other);
    }

    /* JADX INFO: renamed from: compareTo-WZ4Q5Ns, reason: not valid java name */
    private static int m190compareToWZ4Q5Ns(int arg0, int other) {
        return UnsignedKt.uintCompare(arg0, other);
    }

    /* JADX INFO: renamed from: compareTo-VKZWuLQ, reason: not valid java name */
    private static final int m188compareToVKZWuLQ(int arg0, long other) {
        return UnsignedKt.ulongCompare(ULong.m270constructorimpl(((long) arg0) & 4294967295L), other);
    }

    /* JADX INFO: renamed from: plus-7apg3OU, reason: not valid java name */
    private static final int m216plus7apg3OU(int arg0, byte other) {
        return m192constructorimpl(m192constructorimpl(other & UByte.MAX_VALUE) + arg0);
    }

    /* JADX INFO: renamed from: plus-xj2QHRw, reason: not valid java name */
    private static final int m219plusxj2QHRw(int arg0, short other) {
        return m192constructorimpl(m192constructorimpl(65535 & other) + arg0);
    }

    /* JADX INFO: renamed from: plus-WZ4Q5Ns, reason: not valid java name */
    private static final int m218plusWZ4Q5Ns(int arg0, int other) {
        return m192constructorimpl(arg0 + other);
    }

    /* JADX INFO: renamed from: plus-VKZWuLQ, reason: not valid java name */
    private static final long m217plusVKZWuLQ(int arg0, long other) {
        return ULong.m270constructorimpl(ULong.m270constructorimpl(((long) arg0) & 4294967295L) + other);
    }

    /* JADX INFO: renamed from: minus-7apg3OU, reason: not valid java name */
    private static final int m207minus7apg3OU(int arg0, byte other) {
        return m192constructorimpl(arg0 - m192constructorimpl(other & UByte.MAX_VALUE));
    }

    /* JADX INFO: renamed from: minus-xj2QHRw, reason: not valid java name */
    private static final int m210minusxj2QHRw(int arg0, short other) {
        return m192constructorimpl(arg0 - m192constructorimpl(65535 & other));
    }

    /* JADX INFO: renamed from: minus-WZ4Q5Ns, reason: not valid java name */
    private static final int m209minusWZ4Q5Ns(int arg0, int other) {
        return m192constructorimpl(arg0 - other);
    }

    /* JADX INFO: renamed from: minus-VKZWuLQ, reason: not valid java name */
    private static final long m208minusVKZWuLQ(int arg0, long other) {
        return ULong.m270constructorimpl(ULong.m270constructorimpl(((long) arg0) & 4294967295L) - other);
    }

    /* JADX INFO: renamed from: times-7apg3OU, reason: not valid java name */
    private static final int m227times7apg3OU(int arg0, byte other) {
        return m192constructorimpl(m192constructorimpl(other & UByte.MAX_VALUE) * arg0);
    }

    /* JADX INFO: renamed from: times-xj2QHRw, reason: not valid java name */
    private static final int m230timesxj2QHRw(int arg0, short other) {
        return m192constructorimpl(m192constructorimpl(65535 & other) * arg0);
    }

    /* JADX INFO: renamed from: times-WZ4Q5Ns, reason: not valid java name */
    private static final int m229timesWZ4Q5Ns(int arg0, int other) {
        return m192constructorimpl(arg0 * other);
    }

    /* JADX INFO: renamed from: times-VKZWuLQ, reason: not valid java name */
    private static final long m228timesVKZWuLQ(int arg0, long other) {
        return ULong.m270constructorimpl(ULong.m270constructorimpl(((long) arg0) & 4294967295L) * other);
    }

    /* JADX INFO: renamed from: div-7apg3OU, reason: not valid java name */
    private static final int m194div7apg3OU(int arg0, byte other) {
        return UnsignedKt.m445uintDivideJ1ME1BU(arg0, m192constructorimpl(other & UByte.MAX_VALUE));
    }

    /* JADX INFO: renamed from: div-xj2QHRw, reason: not valid java name */
    private static final int m197divxj2QHRw(int arg0, short other) {
        return UnsignedKt.m445uintDivideJ1ME1BU(arg0, m192constructorimpl(65535 & other));
    }

    /* JADX INFO: renamed from: div-WZ4Q5Ns, reason: not valid java name */
    private static final int m196divWZ4Q5Ns(int arg0, int other) {
        return UnsignedKt.m445uintDivideJ1ME1BU(arg0, other);
    }

    /* JADX INFO: renamed from: div-VKZWuLQ, reason: not valid java name */
    private static final long m195divVKZWuLQ(int arg0, long other) {
        return UnsignedKt.m447ulongDivideeb3DHEI(ULong.m270constructorimpl(((long) arg0) & 4294967295L), other);
    }

    /* JADX INFO: renamed from: rem-7apg3OU, reason: not valid java name */
    private static final int m221rem7apg3OU(int arg0, byte other) {
        return UnsignedKt.m446uintRemainderJ1ME1BU(arg0, m192constructorimpl(other & UByte.MAX_VALUE));
    }

    /* JADX INFO: renamed from: rem-xj2QHRw, reason: not valid java name */
    private static final int m224remxj2QHRw(int arg0, short other) {
        return UnsignedKt.m446uintRemainderJ1ME1BU(arg0, m192constructorimpl(65535 & other));
    }

    /* JADX INFO: renamed from: rem-WZ4Q5Ns, reason: not valid java name */
    private static final int m223remWZ4Q5Ns(int arg0, int other) {
        return UnsignedKt.m446uintRemainderJ1ME1BU(arg0, other);
    }

    /* JADX INFO: renamed from: rem-VKZWuLQ, reason: not valid java name */
    private static final long m222remVKZWuLQ(int arg0, long other) {
        return UnsignedKt.m448ulongRemaindereb3DHEI(ULong.m270constructorimpl(((long) arg0) & 4294967295L), other);
    }

    /* JADX INFO: renamed from: floorDiv-7apg3OU, reason: not valid java name */
    private static final int m200floorDiv7apg3OU(int arg0, byte other) {
        return UnsignedKt.m445uintDivideJ1ME1BU(arg0, m192constructorimpl(other & UByte.MAX_VALUE));
    }

    /* JADX INFO: renamed from: floorDiv-xj2QHRw, reason: not valid java name */
    private static final int m203floorDivxj2QHRw(int arg0, short other) {
        return UnsignedKt.m445uintDivideJ1ME1BU(arg0, m192constructorimpl(65535 & other));
    }

    /* JADX INFO: renamed from: floorDiv-WZ4Q5Ns, reason: not valid java name */
    private static final int m202floorDivWZ4Q5Ns(int arg0, int other) {
        return UnsignedKt.m445uintDivideJ1ME1BU(arg0, other);
    }

    /* JADX INFO: renamed from: floorDiv-VKZWuLQ, reason: not valid java name */
    private static final long m201floorDivVKZWuLQ(int arg0, long other) {
        return UnsignedKt.m447ulongDivideeb3DHEI(ULong.m270constructorimpl(((long) arg0) & 4294967295L), other);
    }

    /* JADX INFO: renamed from: mod-7apg3OU, reason: not valid java name */
    private static final byte m211mod7apg3OU(int arg0, byte other) {
        return UByte.m116constructorimpl((byte) UnsignedKt.m446uintRemainderJ1ME1BU(arg0, m192constructorimpl(other & UByte.MAX_VALUE)));
    }

    /* JADX INFO: renamed from: mod-xj2QHRw, reason: not valid java name */
    private static final short m214modxj2QHRw(int arg0, short other) {
        return UShort.m376constructorimpl((short) UnsignedKt.m446uintRemainderJ1ME1BU(arg0, m192constructorimpl(65535 & other)));
    }

    /* JADX INFO: renamed from: mod-WZ4Q5Ns, reason: not valid java name */
    private static final int m213modWZ4Q5Ns(int arg0, int other) {
        return UnsignedKt.m446uintRemainderJ1ME1BU(arg0, other);
    }

    /* JADX INFO: renamed from: mod-VKZWuLQ, reason: not valid java name */
    private static final long m212modVKZWuLQ(int arg0, long other) {
        return UnsignedKt.m448ulongRemaindereb3DHEI(ULong.m270constructorimpl(((long) arg0) & 4294967295L), other);
    }

    /* JADX INFO: renamed from: inc-pVg5ArA, reason: not valid java name */
    private static final int m205incpVg5ArA(int arg0) {
        return m192constructorimpl(arg0 + 1);
    }

    /* JADX INFO: renamed from: dec-pVg5ArA, reason: not valid java name */
    private static final int m193decpVg5ArA(int arg0) {
        return m192constructorimpl(arg0 - 1);
    }

    /* JADX INFO: renamed from: rangeTo-WZ4Q5Ns, reason: not valid java name */
    private static final UIntRange m220rangeToWZ4Q5Ns(int arg0, int other) {
        return new UIntRange(arg0, other, null);
    }

    /* JADX INFO: renamed from: shl-pVg5ArA, reason: not valid java name */
    private static final int m225shlpVg5ArA(int arg0, int bitCount) {
        return m192constructorimpl(arg0 << bitCount);
    }

    /* JADX INFO: renamed from: shr-pVg5ArA, reason: not valid java name */
    private static final int m226shrpVg5ArA(int arg0, int bitCount) {
        return m192constructorimpl(arg0 >>> bitCount);
    }

    /* JADX INFO: renamed from: and-WZ4Q5Ns, reason: not valid java name */
    private static final int m185andWZ4Q5Ns(int arg0, int other) {
        return m192constructorimpl(arg0 & other);
    }

    /* JADX INFO: renamed from: or-WZ4Q5Ns, reason: not valid java name */
    private static final int m215orWZ4Q5Ns(int arg0, int other) {
        return m192constructorimpl(arg0 | other);
    }

    /* JADX INFO: renamed from: xor-WZ4Q5Ns, reason: not valid java name */
    private static final int m242xorWZ4Q5Ns(int arg0, int other) {
        return m192constructorimpl(arg0 ^ other);
    }

    /* JADX INFO: renamed from: inv-pVg5ArA, reason: not valid java name */
    private static final int m206invpVg5ArA(int arg0) {
        return m192constructorimpl(~arg0);
    }

    /* JADX INFO: renamed from: toByte-impl, reason: not valid java name */
    private static final byte m231toByteimpl(int arg0) {
        return (byte) arg0;
    }

    /* JADX INFO: renamed from: toShort-impl, reason: not valid java name */
    private static final short m236toShortimpl(int arg0) {
        return (short) arg0;
    }

    /* JADX INFO: renamed from: toInt-impl, reason: not valid java name */
    private static final int m234toIntimpl(int arg0) {
        return arg0;
    }

    /* JADX INFO: renamed from: toLong-impl, reason: not valid java name */
    private static final long m235toLongimpl(int arg0) {
        return ((long) arg0) & 4294967295L;
    }

    /* JADX INFO: renamed from: toUByte-w2LRezQ, reason: not valid java name */
    private static final byte m238toUBytew2LRezQ(int arg0) {
        return UByte.m116constructorimpl((byte) arg0);
    }

    /* JADX INFO: renamed from: toUShort-Mh2AYeg, reason: not valid java name */
    private static final short m241toUShortMh2AYeg(int arg0) {
        return UShort.m376constructorimpl((short) arg0);
    }

    /* JADX INFO: renamed from: toUInt-pVg5ArA, reason: not valid java name */
    private static final int m239toUIntpVg5ArA(int arg0) {
        return arg0;
    }

    /* JADX INFO: renamed from: toULong-s-VKNKU, reason: not valid java name */
    private static final long m240toULongsVKNKU(int arg0) {
        return ULong.m270constructorimpl(((long) arg0) & 4294967295L);
    }

    /* JADX INFO: renamed from: toFloat-impl, reason: not valid java name */
    private static final float m233toFloatimpl(int arg0) {
        return (float) UnsignedKt.uintToDouble(arg0);
    }

    /* JADX INFO: renamed from: toDouble-impl, reason: not valid java name */
    private static final double m232toDoubleimpl(int arg0) {
        return UnsignedKt.uintToDouble(arg0);
    }

    /* JADX INFO: renamed from: toString-impl, reason: not valid java name */
    public static String m237toStringimpl(int arg0) {
        return String.valueOf(((long) arg0) & 4294967295L);
    }

    public String toString() {
        return m237toStringimpl(this.data);
    }
}
