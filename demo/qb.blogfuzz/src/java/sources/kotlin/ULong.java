package kotlin;

import androidx.constraintlayout.widget.ConstraintLayout;
import kotlin.jvm.JvmInline;
import kotlin.ranges.ULongRange;

/* JADX INFO: compiled from: ULong.kt */
/* JADX INFO: loaded from: classes.dex */
@Metadata(d1 = {"\u0000j\n\u0002\u0018\u0002\n\u0002\u0010\u000f\n\u0000\n\u0002\u0010\t\n\u0002\b\t\n\u0002\u0010\b\n\u0002\u0018\u0002\n\u0002\b\u0002\n\u0002\u0018\u0002\n\u0002\b\u0004\n\u0002\u0018\u0002\n\u0002\b\r\n\u0002\u0010\u000b\n\u0002\u0010\u0000\n\u0002\b\"\n\u0002\u0018\u0002\n\u0002\b\u0012\n\u0002\u0010\u0005\n\u0002\b\u0003\n\u0002\u0010\u0006\n\u0002\b\u0003\n\u0002\u0010\u0007\n\u0002\b\u0007\n\u0002\u0010\n\n\u0002\b\u0003\n\u0002\u0010\u000e\n\u0002\b\u000e\b\u0087@\u0018\u0000 |2\b\u0012\u0004\u0012\u00020\u00000\u0001:\u0001|B\u0014\b\u0001\u0012\u0006\u0010\u0002\u001a\u00020\u0003ø\u0001\u0000¢\u0006\u0004\b\u0004\u0010\u0005J\u001b\u0010\b\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0000H\u0087\fø\u0001\u0000¢\u0006\u0004\b\n\u0010\u000bJ\u001b\u0010\f\u001a\u00020\r2\u0006\u0010\t\u001a\u00020\u000eH\u0087\nø\u0001\u0000¢\u0006\u0004\b\u000f\u0010\u0010J\u001b\u0010\f\u001a\u00020\r2\u0006\u0010\t\u001a\u00020\u0011H\u0087\nø\u0001\u0000¢\u0006\u0004\b\u0012\u0010\u0013J\u001b\u0010\f\u001a\u00020\r2\u0006\u0010\t\u001a\u00020\u0000H\u0097\nø\u0001\u0000¢\u0006\u0004\b\u0014\u0010\u0015J\u001b\u0010\f\u001a\u00020\r2\u0006\u0010\t\u001a\u00020\u0016H\u0087\nø\u0001\u0000¢\u0006\u0004\b\u0017\u0010\u0018J\u0016\u0010\u0019\u001a\u00020\u0000H\u0087\nø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\b\u001a\u0010\u0005J\u001b\u0010\u001b\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u000eH\u0087\nø\u0001\u0000¢\u0006\u0004\b\u001c\u0010\u001dJ\u001b\u0010\u001b\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0011H\u0087\nø\u0001\u0000¢\u0006\u0004\b\u001e\u0010\u001fJ\u001b\u0010\u001b\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0000H\u0087\nø\u0001\u0000¢\u0006\u0004\b \u0010\u000bJ\u001b\u0010\u001b\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0016H\u0087\nø\u0001\u0000¢\u0006\u0004\b!\u0010\"J\u001a\u0010#\u001a\u00020$2\b\u0010\t\u001a\u0004\u0018\u00010%HÖ\u0003¢\u0006\u0004\b&\u0010'J\u001b\u0010(\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u000eH\u0087\bø\u0001\u0000¢\u0006\u0004\b)\u0010\u001dJ\u001b\u0010(\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0011H\u0087\bø\u0001\u0000¢\u0006\u0004\b*\u0010\u001fJ\u001b\u0010(\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0000H\u0087\bø\u0001\u0000¢\u0006\u0004\b+\u0010\u000bJ\u001b\u0010(\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0016H\u0087\bø\u0001\u0000¢\u0006\u0004\b,\u0010\"J\u0010\u0010-\u001a\u00020\rHÖ\u0001¢\u0006\u0004\b.\u0010/J\u0016\u00100\u001a\u00020\u0000H\u0087\nø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\b1\u0010\u0005J\u0016\u00102\u001a\u00020\u0000H\u0087\bø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\b3\u0010\u0005J\u001b\u00104\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u000eH\u0087\nø\u0001\u0000¢\u0006\u0004\b5\u0010\u001dJ\u001b\u00104\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0011H\u0087\nø\u0001\u0000¢\u0006\u0004\b6\u0010\u001fJ\u001b\u00104\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0000H\u0087\nø\u0001\u0000¢\u0006\u0004\b7\u0010\u000bJ\u001b\u00104\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0016H\u0087\nø\u0001\u0000¢\u0006\u0004\b8\u0010\"J\u001b\u00109\u001a\u00020\u000e2\u0006\u0010\t\u001a\u00020\u000eH\u0087\bø\u0001\u0000¢\u0006\u0004\b:\u0010;J\u001b\u00109\u001a\u00020\u00112\u0006\u0010\t\u001a\u00020\u0011H\u0087\bø\u0001\u0000¢\u0006\u0004\b<\u0010\u0013J\u001b\u00109\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0000H\u0087\bø\u0001\u0000¢\u0006\u0004\b=\u0010\u000bJ\u001b\u00109\u001a\u00020\u00162\u0006\u0010\t\u001a\u00020\u0016H\u0087\bø\u0001\u0000¢\u0006\u0004\b>\u0010?J\u001b\u0010@\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0000H\u0087\fø\u0001\u0000¢\u0006\u0004\bA\u0010\u000bJ\u001b\u0010B\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u000eH\u0087\nø\u0001\u0000¢\u0006\u0004\bC\u0010\u001dJ\u001b\u0010B\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0011H\u0087\nø\u0001\u0000¢\u0006\u0004\bD\u0010\u001fJ\u001b\u0010B\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0000H\u0087\nø\u0001\u0000¢\u0006\u0004\bE\u0010\u000bJ\u001b\u0010B\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0016H\u0087\nø\u0001\u0000¢\u0006\u0004\bF\u0010\"J\u001b\u0010G\u001a\u00020H2\u0006\u0010\t\u001a\u00020\u0000H\u0087\nø\u0001\u0000¢\u0006\u0004\bI\u0010JJ\u001b\u0010K\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u000eH\u0087\nø\u0001\u0000¢\u0006\u0004\bL\u0010\u001dJ\u001b\u0010K\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0011H\u0087\nø\u0001\u0000¢\u0006\u0004\bM\u0010\u001fJ\u001b\u0010K\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0000H\u0087\nø\u0001\u0000¢\u0006\u0004\bN\u0010\u000bJ\u001b\u0010K\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0016H\u0087\nø\u0001\u0000¢\u0006\u0004\bO\u0010\"J\u001e\u0010P\u001a\u00020\u00002\u0006\u0010Q\u001a\u00020\rH\u0087\fø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\bR\u0010\u001fJ\u001e\u0010S\u001a\u00020\u00002\u0006\u0010Q\u001a\u00020\rH\u0087\fø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\bT\u0010\u001fJ\u001b\u0010U\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u000eH\u0087\nø\u0001\u0000¢\u0006\u0004\bV\u0010\u001dJ\u001b\u0010U\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0011H\u0087\nø\u0001\u0000¢\u0006\u0004\bW\u0010\u001fJ\u001b\u0010U\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0000H\u0087\nø\u0001\u0000¢\u0006\u0004\bX\u0010\u000bJ\u001b\u0010U\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0016H\u0087\nø\u0001\u0000¢\u0006\u0004\bY\u0010\"J\u0010\u0010Z\u001a\u00020[H\u0087\b¢\u0006\u0004\b\\\u0010]J\u0010\u0010^\u001a\u00020_H\u0087\b¢\u0006\u0004\b`\u0010aJ\u0010\u0010b\u001a\u00020cH\u0087\b¢\u0006\u0004\bd\u0010eJ\u0010\u0010f\u001a\u00020\rH\u0087\b¢\u0006\u0004\bg\u0010/J\u0010\u0010h\u001a\u00020\u0003H\u0087\b¢\u0006\u0004\bi\u0010\u0005J\u0010\u0010j\u001a\u00020kH\u0087\b¢\u0006\u0004\bl\u0010mJ\u000f\u0010n\u001a\u00020oH\u0016¢\u0006\u0004\bp\u0010qJ\u0016\u0010r\u001a\u00020\u000eH\u0087\bø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\bs\u0010]J\u0016\u0010t\u001a\u00020\u0011H\u0087\bø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\bu\u0010/J\u0016\u0010v\u001a\u00020\u0000H\u0087\bø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\bw\u0010\u0005J\u0016\u0010x\u001a\u00020\u0016H\u0087\bø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\by\u0010mJ\u001b\u0010z\u001a\u00020\u00002\u0006\u0010\t\u001a\u00020\u0000H\u0087\fø\u0001\u0000¢\u0006\u0004\b{\u0010\u000bR\u0016\u0010\u0002\u001a\u00020\u00038\u0000X\u0081\u0004¢\u0006\b\n\u0000\u0012\u0004\b\u0006\u0010\u0007\u0088\u0001\u0002\u0092\u0001\u00020\u0003ø\u0001\u0000\u0082\u0002\b\n\u0002\b\u0019\n\u0002\b!¨\u0006}"}, d2 = {"Lkotlin/ULong;", "", "data", "", "constructor-impl", "(J)J", "getData$annotations", "()V", "and", "other", "and-VKZWuLQ", "(JJ)J", "compareTo", "", "Lkotlin/UByte;", "compareTo-7apg3OU", "(JB)I", "Lkotlin/UInt;", "compareTo-WZ4Q5Ns", "(JI)I", "compareTo-VKZWuLQ", "(JJ)I", "Lkotlin/UShort;", "compareTo-xj2QHRw", "(JS)I", "dec", "dec-s-VKNKU", "div", "div-7apg3OU", "(JB)J", "div-WZ4Q5Ns", "(JI)J", "div-VKZWuLQ", "div-xj2QHRw", "(JS)J", "equals", "", "", "equals-impl", "(JLjava/lang/Object;)Z", "floorDiv", "floorDiv-7apg3OU", "floorDiv-WZ4Q5Ns", "floorDiv-VKZWuLQ", "floorDiv-xj2QHRw", "hashCode", "hashCode-impl", "(J)I", "inc", "inc-s-VKNKU", "inv", "inv-s-VKNKU", "minus", "minus-7apg3OU", "minus-WZ4Q5Ns", "minus-VKZWuLQ", "minus-xj2QHRw", "mod", "mod-7apg3OU", "(JB)B", "mod-WZ4Q5Ns", "mod-VKZWuLQ", "mod-xj2QHRw", "(JS)S", "or", "or-VKZWuLQ", "plus", "plus-7apg3OU", "plus-WZ4Q5Ns", "plus-VKZWuLQ", "plus-xj2QHRw", "rangeTo", "Lkotlin/ranges/ULongRange;", "rangeTo-VKZWuLQ", "(JJ)Lkotlin/ranges/ULongRange;", "rem", "rem-7apg3OU", "rem-WZ4Q5Ns", "rem-VKZWuLQ", "rem-xj2QHRw", "shl", "bitCount", "shl-s-VKNKU", "shr", "shr-s-VKNKU", "times", "times-7apg3OU", "times-WZ4Q5Ns", "times-VKZWuLQ", "times-xj2QHRw", "toByte", "", "toByte-impl", "(J)B", "toDouble", "", "toDouble-impl", "(J)D", "toFloat", "", "toFloat-impl", "(J)F", "toInt", "toInt-impl", "toLong", "toLong-impl", "toShort", "", "toShort-impl", "(J)S", "toString", "", "toString-impl", "(J)Ljava/lang/String;", "toUByte", "toUByte-w2LRezQ", "toUInt", "toUInt-pVg5ArA", "toULong", "toULong-s-VKNKU", "toUShort", "toUShort-Mh2AYeg", "xor", "xor-VKZWuLQ", "Companion", "kotlin-stdlib"}, k = 1, mv = {1, 7, 1}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
@JvmInline
public final class ULong implements Comparable<ULong> {
    public static final long MAX_VALUE = -1;
    public static final long MIN_VALUE = 0;
    public static final int SIZE_BITS = 64;
    public static final int SIZE_BYTES = 8;
    private final long data;

    /* JADX INFO: renamed from: box-impl, reason: not valid java name */
    public static final /* synthetic */ ULong m264boximpl(long j) {
        return new ULong(j);
    }

    /* JADX INFO: renamed from: constructor-impl, reason: not valid java name */
    public static long m270constructorimpl(long j) {
        return j;
    }

    /* JADX INFO: renamed from: equals-impl, reason: not valid java name */
    public static boolean m276equalsimpl(long j, Object obj) {
        return (obj instanceof ULong) && j == ((ULong) obj).getData();
    }

    /* JADX INFO: renamed from: equals-impl0, reason: not valid java name */
    public static final boolean m277equalsimpl0(long j, long j2) {
        return j == j2;
    }

    public static /* synthetic */ void getData$annotations() {
    }

    /* JADX INFO: renamed from: hashCode-impl, reason: not valid java name */
    public static int m282hashCodeimpl(long j) {
        return (int) ((j >>> 32) ^ j);
    }

    public boolean equals(Object obj) {
        return m276equalsimpl(this.data, obj);
    }

    public int hashCode() {
        return m282hashCodeimpl(this.data);
    }

    /* JADX INFO: renamed from: unbox-impl, reason: not valid java name and from getter */
    public final /* synthetic */ long getData() {
        return this.data;
    }

    @Override // java.lang.Comparable
    public /* bridge */ /* synthetic */ int compareTo(ULong uLong) {
        return UnsignedKt.ulongCompare(getData(), uLong.getData());
    }

    private /* synthetic */ ULong(long data) {
        this.data = data;
    }

    /* JADX INFO: renamed from: compareTo-7apg3OU, reason: not valid java name */
    private static final int m265compareTo7apg3OU(long arg0, byte other) {
        return UnsignedKt.ulongCompare(arg0, m270constructorimpl(((long) other) & 255));
    }

    /* JADX INFO: renamed from: compareTo-xj2QHRw, reason: not valid java name */
    private static final int m269compareToxj2QHRw(long arg0, short other) {
        return UnsignedKt.ulongCompare(arg0, m270constructorimpl(((long) other) & 65535));
    }

    /* JADX INFO: renamed from: compareTo-WZ4Q5Ns, reason: not valid java name */
    private static final int m268compareToWZ4Q5Ns(long arg0, int other) {
        return UnsignedKt.ulongCompare(arg0, m270constructorimpl(((long) other) & 4294967295L));
    }

    /* JADX INFO: renamed from: compareTo-VKZWuLQ, reason: not valid java name */
    private int m266compareToVKZWuLQ(long other) {
        return UnsignedKt.ulongCompare(getData(), other);
    }

    /* JADX INFO: renamed from: compareTo-VKZWuLQ, reason: not valid java name */
    private static int m267compareToVKZWuLQ(long arg0, long other) {
        return UnsignedKt.ulongCompare(arg0, other);
    }

    /* JADX INFO: renamed from: plus-7apg3OU, reason: not valid java name */
    private static final long m294plus7apg3OU(long arg0, byte other) {
        return m270constructorimpl(m270constructorimpl(((long) other) & 255) + arg0);
    }

    /* JADX INFO: renamed from: plus-xj2QHRw, reason: not valid java name */
    private static final long m297plusxj2QHRw(long arg0, short other) {
        return m270constructorimpl(m270constructorimpl(((long) other) & 65535) + arg0);
    }

    /* JADX INFO: renamed from: plus-WZ4Q5Ns, reason: not valid java name */
    private static final long m296plusWZ4Q5Ns(long arg0, int other) {
        return m270constructorimpl(m270constructorimpl(((long) other) & 4294967295L) + arg0);
    }

    /* JADX INFO: renamed from: plus-VKZWuLQ, reason: not valid java name */
    private static final long m295plusVKZWuLQ(long arg0, long other) {
        return m270constructorimpl(arg0 + other);
    }

    /* JADX INFO: renamed from: minus-7apg3OU, reason: not valid java name */
    private static final long m285minus7apg3OU(long arg0, byte other) {
        return m270constructorimpl(arg0 - m270constructorimpl(((long) other) & 255));
    }

    /* JADX INFO: renamed from: minus-xj2QHRw, reason: not valid java name */
    private static final long m288minusxj2QHRw(long arg0, short other) {
        return m270constructorimpl(arg0 - m270constructorimpl(((long) other) & 65535));
    }

    /* JADX INFO: renamed from: minus-WZ4Q5Ns, reason: not valid java name */
    private static final long m287minusWZ4Q5Ns(long arg0, int other) {
        return m270constructorimpl(arg0 - m270constructorimpl(((long) other) & 4294967295L));
    }

    /* JADX INFO: renamed from: minus-VKZWuLQ, reason: not valid java name */
    private static final long m286minusVKZWuLQ(long arg0, long other) {
        return m270constructorimpl(arg0 - other);
    }

    /* JADX INFO: renamed from: times-7apg3OU, reason: not valid java name */
    private static final long m305times7apg3OU(long arg0, byte other) {
        return m270constructorimpl(m270constructorimpl(((long) other) & 255) * arg0);
    }

    /* JADX INFO: renamed from: times-xj2QHRw, reason: not valid java name */
    private static final long m308timesxj2QHRw(long arg0, short other) {
        return m270constructorimpl(m270constructorimpl(((long) other) & 65535) * arg0);
    }

    /* JADX INFO: renamed from: times-WZ4Q5Ns, reason: not valid java name */
    private static final long m307timesWZ4Q5Ns(long arg0, int other) {
        return m270constructorimpl(m270constructorimpl(((long) other) & 4294967295L) * arg0);
    }

    /* JADX INFO: renamed from: times-VKZWuLQ, reason: not valid java name */
    private static final long m306timesVKZWuLQ(long arg0, long other) {
        return m270constructorimpl(arg0 * other);
    }

    /* JADX INFO: renamed from: div-7apg3OU, reason: not valid java name */
    private static final long m272div7apg3OU(long arg0, byte other) {
        return UnsignedKt.m447ulongDivideeb3DHEI(arg0, m270constructorimpl(((long) other) & 255));
    }

    /* JADX INFO: renamed from: div-xj2QHRw, reason: not valid java name */
    private static final long m275divxj2QHRw(long arg0, short other) {
        return UnsignedKt.m447ulongDivideeb3DHEI(arg0, m270constructorimpl(((long) other) & 65535));
    }

    /* JADX INFO: renamed from: div-WZ4Q5Ns, reason: not valid java name */
    private static final long m274divWZ4Q5Ns(long arg0, int other) {
        return UnsignedKt.m447ulongDivideeb3DHEI(arg0, m270constructorimpl(((long) other) & 4294967295L));
    }

    /* JADX INFO: renamed from: div-VKZWuLQ, reason: not valid java name */
    private static final long m273divVKZWuLQ(long arg0, long other) {
        return UnsignedKt.m447ulongDivideeb3DHEI(arg0, other);
    }

    /* JADX INFO: renamed from: rem-7apg3OU, reason: not valid java name */
    private static final long m299rem7apg3OU(long arg0, byte other) {
        return UnsignedKt.m448ulongRemaindereb3DHEI(arg0, m270constructorimpl(((long) other) & 255));
    }

    /* JADX INFO: renamed from: rem-xj2QHRw, reason: not valid java name */
    private static final long m302remxj2QHRw(long arg0, short other) {
        return UnsignedKt.m448ulongRemaindereb3DHEI(arg0, m270constructorimpl(((long) other) & 65535));
    }

    /* JADX INFO: renamed from: rem-WZ4Q5Ns, reason: not valid java name */
    private static final long m301remWZ4Q5Ns(long arg0, int other) {
        return UnsignedKt.m448ulongRemaindereb3DHEI(arg0, m270constructorimpl(((long) other) & 4294967295L));
    }

    /* JADX INFO: renamed from: rem-VKZWuLQ, reason: not valid java name */
    private static final long m300remVKZWuLQ(long arg0, long other) {
        return UnsignedKt.m448ulongRemaindereb3DHEI(arg0, other);
    }

    /* JADX INFO: renamed from: floorDiv-7apg3OU, reason: not valid java name */
    private static final long m278floorDiv7apg3OU(long arg0, byte other) {
        return UnsignedKt.m447ulongDivideeb3DHEI(arg0, m270constructorimpl(((long) other) & 255));
    }

    /* JADX INFO: renamed from: floorDiv-xj2QHRw, reason: not valid java name */
    private static final long m281floorDivxj2QHRw(long arg0, short other) {
        return UnsignedKt.m447ulongDivideeb3DHEI(arg0, m270constructorimpl(((long) other) & 65535));
    }

    /* JADX INFO: renamed from: floorDiv-WZ4Q5Ns, reason: not valid java name */
    private static final long m280floorDivWZ4Q5Ns(long arg0, int other) {
        return UnsignedKt.m447ulongDivideeb3DHEI(arg0, m270constructorimpl(((long) other) & 4294967295L));
    }

    /* JADX INFO: renamed from: floorDiv-VKZWuLQ, reason: not valid java name */
    private static final long m279floorDivVKZWuLQ(long arg0, long other) {
        return UnsignedKt.m447ulongDivideeb3DHEI(arg0, other);
    }

    /* JADX INFO: renamed from: mod-7apg3OU, reason: not valid java name */
    private static final byte m289mod7apg3OU(long arg0, byte other) {
        return UByte.m116constructorimpl((byte) UnsignedKt.m448ulongRemaindereb3DHEI(arg0, m270constructorimpl(((long) other) & 255)));
    }

    /* JADX INFO: renamed from: mod-xj2QHRw, reason: not valid java name */
    private static final short m292modxj2QHRw(long arg0, short other) {
        return UShort.m376constructorimpl((short) UnsignedKt.m448ulongRemaindereb3DHEI(arg0, m270constructorimpl(((long) other) & 65535)));
    }

    /* JADX INFO: renamed from: mod-WZ4Q5Ns, reason: not valid java name */
    private static final int m291modWZ4Q5Ns(long arg0, int other) {
        return UInt.m192constructorimpl((int) UnsignedKt.m448ulongRemaindereb3DHEI(arg0, m270constructorimpl(((long) other) & 4294967295L)));
    }

    /* JADX INFO: renamed from: mod-VKZWuLQ, reason: not valid java name */
    private static final long m290modVKZWuLQ(long arg0, long other) {
        return UnsignedKt.m448ulongRemaindereb3DHEI(arg0, other);
    }

    /* JADX INFO: renamed from: inc-s-VKNKU, reason: not valid java name */
    private static final long m283incsVKNKU(long arg0) {
        return m270constructorimpl(1 + arg0);
    }

    /* JADX INFO: renamed from: dec-s-VKNKU, reason: not valid java name */
    private static final long m271decsVKNKU(long arg0) {
        return m270constructorimpl((-1) + arg0);
    }

    /* JADX INFO: renamed from: rangeTo-VKZWuLQ, reason: not valid java name */
    private static final ULongRange m298rangeToVKZWuLQ(long arg0, long other) {
        return new ULongRange(arg0, other, null);
    }

    /* JADX INFO: renamed from: shl-s-VKNKU, reason: not valid java name */
    private static final long m303shlsVKNKU(long arg0, int bitCount) {
        return m270constructorimpl(arg0 << bitCount);
    }

    /* JADX INFO: renamed from: shr-s-VKNKU, reason: not valid java name */
    private static final long m304shrsVKNKU(long arg0, int bitCount) {
        return m270constructorimpl(arg0 >>> bitCount);
    }

    /* JADX INFO: renamed from: and-VKZWuLQ, reason: not valid java name */
    private static final long m263andVKZWuLQ(long arg0, long other) {
        return m270constructorimpl(arg0 & other);
    }

    /* JADX INFO: renamed from: or-VKZWuLQ, reason: not valid java name */
    private static final long m293orVKZWuLQ(long arg0, long other) {
        return m270constructorimpl(arg0 | other);
    }

    /* JADX INFO: renamed from: xor-VKZWuLQ, reason: not valid java name */
    private static final long m320xorVKZWuLQ(long arg0, long other) {
        return m270constructorimpl(arg0 ^ other);
    }

    /* JADX INFO: renamed from: inv-s-VKNKU, reason: not valid java name */
    private static final long m284invsVKNKU(long arg0) {
        return m270constructorimpl(~arg0);
    }

    /* JADX INFO: renamed from: toByte-impl, reason: not valid java name */
    private static final byte m309toByteimpl(long arg0) {
        return (byte) arg0;
    }

    /* JADX INFO: renamed from: toShort-impl, reason: not valid java name */
    private static final short m314toShortimpl(long arg0) {
        return (short) arg0;
    }

    /* JADX INFO: renamed from: toInt-impl, reason: not valid java name */
    private static final int m312toIntimpl(long arg0) {
        return (int) arg0;
    }

    /* JADX INFO: renamed from: toLong-impl, reason: not valid java name */
    private static final long m313toLongimpl(long arg0) {
        return arg0;
    }

    /* JADX INFO: renamed from: toUByte-w2LRezQ, reason: not valid java name */
    private static final byte m316toUBytew2LRezQ(long arg0) {
        return UByte.m116constructorimpl((byte) arg0);
    }

    /* JADX INFO: renamed from: toUShort-Mh2AYeg, reason: not valid java name */
    private static final short m319toUShortMh2AYeg(long arg0) {
        return UShort.m376constructorimpl((short) arg0);
    }

    /* JADX INFO: renamed from: toUInt-pVg5ArA, reason: not valid java name */
    private static final int m317toUIntpVg5ArA(long arg0) {
        return UInt.m192constructorimpl((int) arg0);
    }

    /* JADX INFO: renamed from: toULong-s-VKNKU, reason: not valid java name */
    private static final long m318toULongsVKNKU(long arg0) {
        return arg0;
    }

    /* JADX INFO: renamed from: toFloat-impl, reason: not valid java name */
    private static final float m311toFloatimpl(long arg0) {
        return (float) UnsignedKt.ulongToDouble(arg0);
    }

    /* JADX INFO: renamed from: toDouble-impl, reason: not valid java name */
    private static final double m310toDoubleimpl(long arg0) {
        return UnsignedKt.ulongToDouble(arg0);
    }

    /* JADX INFO: renamed from: toString-impl, reason: not valid java name */
    public static String m315toStringimpl(long arg0) {
        return UnsignedKt.ulongToString(arg0);
    }

    public String toString() {
        return m315toStringimpl(this.data);
    }
}
