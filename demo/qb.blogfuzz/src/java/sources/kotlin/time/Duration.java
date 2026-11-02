package kotlin.time;

import androidx.constraintlayout.widget.ConstraintLayout;
import kotlin.Deprecated;
import kotlin.DeprecatedSinceKotlin;
import kotlin.Metadata;
import kotlin.ReplaceWith;
import kotlin.comparisons.ComparisonsKt;
import kotlin.jvm.JvmInline;
import kotlin.jvm.functions.Function2;
import kotlin.jvm.functions.Function3;
import kotlin.jvm.functions.Function4;
import kotlin.jvm.functions.Function5;
import kotlin.jvm.internal.DefaultConstructorMarker;
import kotlin.jvm.internal.Intrinsics;
import kotlin.math.MathKt;
import kotlin.ranges.LongRange;
import kotlin.ranges.RangesKt;
import kotlin.text.StringsKt;

/* JADX INFO: compiled from: Duration.kt */
/* JADX INFO: loaded from: classes.dex */
@Metadata(d1 = {"\u0000n\n\u0002\u0018\u0002\n\u0002\u0010\u000f\n\u0000\n\u0002\u0010\t\n\u0002\b\u0005\n\u0002\u0010\b\n\u0002\b\u0005\n\u0002\u0010\u0006\n\u0002\b-\n\u0002\u0018\u0002\n\u0002\b\u0017\n\u0002\u0010\u000b\n\u0002\u0010\u0000\n\u0002\b\u001b\n\u0002\u0018\u0002\n\u0002\u0018\u0002\n\u0002\b\b\n\u0002\u0018\u0002\n\u0000\n\u0002\u0018\u0002\n\u0000\n\u0002\u0018\u0002\n\u0002\b\t\n\u0002\u0010\u000e\n\u0002\b\u0010\n\u0002\u0010\u0002\n\u0002\u0018\u0002\n\u0002\u0018\u0002\n\u0002\b\b\b\u0087@\u0018\u0000 ¤\u00012\b\u0012\u0004\u0012\u00020\u00000\u0001:\u0002¤\u0001B\u0014\b\u0000\u0012\u0006\u0010\u0002\u001a\u00020\u0003ø\u0001\u0000¢\u0006\u0004\b\u0004\u0010\u0005J%\u0010D\u001a\u00020\u00002\u0006\u0010E\u001a\u00020\u00032\u0006\u0010F\u001a\u00020\u0003H\u0002ø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\bG\u0010HJ\u001b\u0010I\u001a\u00020\t2\u0006\u0010J\u001a\u00020\u0000H\u0096\u0002ø\u0001\u0000¢\u0006\u0004\bK\u0010LJ\u001e\u0010M\u001a\u00020\u00002\u0006\u0010N\u001a\u00020\u000fH\u0086\u0002ø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\bO\u0010PJ\u001e\u0010M\u001a\u00020\u00002\u0006\u0010N\u001a\u00020\tH\u0086\u0002ø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\bO\u0010QJ\u001b\u0010M\u001a\u00020\u000f2\u0006\u0010J\u001a\u00020\u0000H\u0086\u0002ø\u0001\u0000¢\u0006\u0004\bR\u0010SJ\u001a\u0010T\u001a\u00020U2\b\u0010J\u001a\u0004\u0018\u00010VHÖ\u0003¢\u0006\u0004\bW\u0010XJ\u0010\u0010Y\u001a\u00020\tHÖ\u0001¢\u0006\u0004\bZ\u0010\rJ\r\u0010[\u001a\u00020U¢\u0006\u0004\b\\\u0010]J\u000f\u0010^\u001a\u00020UH\u0002¢\u0006\u0004\b_\u0010]J\u000f\u0010`\u001a\u00020UH\u0002¢\u0006\u0004\ba\u0010]J\r\u0010b\u001a\u00020U¢\u0006\u0004\bc\u0010]J\r\u0010d\u001a\u00020U¢\u0006\u0004\be\u0010]J\r\u0010f\u001a\u00020U¢\u0006\u0004\bg\u0010]J\u001b\u0010h\u001a\u00020\u00002\u0006\u0010J\u001a\u00020\u0000H\u0086\u0002ø\u0001\u0000¢\u0006\u0004\bi\u0010jJ\u001b\u0010k\u001a\u00020\u00002\u0006\u0010J\u001a\u00020\u0000H\u0086\u0002ø\u0001\u0000¢\u0006\u0004\bl\u0010jJ\u001e\u0010m\u001a\u00020\u00002\u0006\u0010N\u001a\u00020\u000fH\u0086\u0002ø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\bn\u0010PJ\u001e\u0010m\u001a\u00020\u00002\u0006\u0010N\u001a\u00020\tH\u0086\u0002ø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\bn\u0010QJ\u009d\u0001\u0010o\u001a\u0002Hp\"\u0004\b\u0000\u0010p2u\u0010q\u001aq\u0012\u0013\u0012\u00110\u0003¢\u0006\f\bs\u0012\b\bt\u0012\u0004\b\b(u\u0012\u0013\u0012\u00110\t¢\u0006\f\bs\u0012\b\bt\u0012\u0004\b\b(v\u0012\u0013\u0012\u00110\t¢\u0006\f\bs\u0012\b\bt\u0012\u0004\b\b(w\u0012\u0013\u0012\u00110\t¢\u0006\f\bs\u0012\b\bt\u0012\u0004\b\b(x\u0012\u0013\u0012\u00110\t¢\u0006\f\bs\u0012\b\bt\u0012\u0004\b\b(y\u0012\u0004\u0012\u0002Hp0rH\u0086\bø\u0001\u0002\u0082\u0002\n\n\b\b\u0001\u0012\u0002\u0010\u0001 \u0001¢\u0006\u0004\bz\u0010{J\u0088\u0001\u0010o\u001a\u0002Hp\"\u0004\b\u0000\u0010p2`\u0010q\u001a\\\u0012\u0013\u0012\u00110\u0003¢\u0006\f\bs\u0012\b\bt\u0012\u0004\b\b(v\u0012\u0013\u0012\u00110\t¢\u0006\f\bs\u0012\b\bt\u0012\u0004\b\b(w\u0012\u0013\u0012\u00110\t¢\u0006\f\bs\u0012\b\bt\u0012\u0004\b\b(x\u0012\u0013\u0012\u00110\t¢\u0006\f\bs\u0012\b\bt\u0012\u0004\b\b(y\u0012\u0004\u0012\u0002Hp0|H\u0086\bø\u0001\u0002\u0082\u0002\n\n\b\b\u0001\u0012\u0002\u0010\u0001 \u0001¢\u0006\u0004\bz\u0010}Js\u0010o\u001a\u0002Hp\"\u0004\b\u0000\u0010p2K\u0010q\u001aG\u0012\u0013\u0012\u00110\u0003¢\u0006\f\bs\u0012\b\bt\u0012\u0004\b\b(w\u0012\u0013\u0012\u00110\t¢\u0006\f\bs\u0012\b\bt\u0012\u0004\b\b(x\u0012\u0013\u0012\u00110\t¢\u0006\f\bs\u0012\b\bt\u0012\u0004\b\b(y\u0012\u0004\u0012\u0002Hp0~H\u0086\bø\u0001\u0002\u0082\u0002\n\n\b\b\u0001\u0012\u0002\u0010\u0001 \u0001¢\u0006\u0004\bz\u0010\u007fJ`\u0010o\u001a\u0002Hp\"\u0004\b\u0000\u0010p27\u0010q\u001a3\u0012\u0013\u0012\u00110\u0003¢\u0006\f\bs\u0012\b\bt\u0012\u0004\b\b(x\u0012\u0013\u0012\u00110\t¢\u0006\f\bs\u0012\b\bt\u0012\u0004\b\b(y\u0012\u0004\u0012\u0002Hp0\u0080\u0001H\u0086\bø\u0001\u0002\u0082\u0002\n\n\b\b\u0001\u0012\u0002\u0010\u0001 \u0001¢\u0006\u0005\bz\u0010\u0081\u0001J\u0019\u0010\u0082\u0001\u001a\u00020\u000f2\u0007\u0010\u0083\u0001\u001a\u00020=¢\u0006\u0006\b\u0084\u0001\u0010\u0085\u0001J\u0019\u0010\u0086\u0001\u001a\u00020\t2\u0007\u0010\u0083\u0001\u001a\u00020=¢\u0006\u0006\b\u0087\u0001\u0010\u0088\u0001J\u0011\u0010\u0089\u0001\u001a\u00030\u008a\u0001¢\u0006\u0006\b\u008b\u0001\u0010\u008c\u0001J\u0019\u0010\u008d\u0001\u001a\u00020\u00032\u0007\u0010\u0083\u0001\u001a\u00020=¢\u0006\u0006\b\u008e\u0001\u0010\u008f\u0001J\u0011\u0010\u0090\u0001\u001a\u00020\u0003H\u0007¢\u0006\u0005\b\u0091\u0001\u0010\u0005J\u0011\u0010\u0092\u0001\u001a\u00020\u0003H\u0007¢\u0006\u0005\b\u0093\u0001\u0010\u0005J\u0013\u0010\u0094\u0001\u001a\u00030\u008a\u0001H\u0016¢\u0006\u0006\b\u0095\u0001\u0010\u008c\u0001J%\u0010\u0094\u0001\u001a\u00030\u008a\u00012\u0007\u0010\u0083\u0001\u001a\u00020=2\t\b\u0002\u0010\u0096\u0001\u001a\u00020\t¢\u0006\u0006\b\u0095\u0001\u0010\u0097\u0001J\u0018\u0010\u0098\u0001\u001a\u00020\u0000H\u0086\u0002ø\u0001\u0001ø\u0001\u0000¢\u0006\u0005\b\u0099\u0001\u0010\u0005JK\u0010\u009a\u0001\u001a\u00030\u009b\u0001*\b0\u009c\u0001j\u0003`\u009d\u00012\u0007\u0010\u009e\u0001\u001a\u00020\t2\u0007\u0010\u009f\u0001\u001a\u00020\t2\u0007\u0010 \u0001\u001a\u00020\t2\b\u0010\u0083\u0001\u001a\u00030\u008a\u00012\u0007\u0010¡\u0001\u001a\u00020UH\u0002¢\u0006\u0006\b¢\u0001\u0010£\u0001R\u0017\u0010\u0006\u001a\u00020\u00008Fø\u0001\u0000ø\u0001\u0001¢\u0006\u0006\u001a\u0004\b\u0007\u0010\u0005R\u001a\u0010\b\u001a\u00020\t8@X\u0081\u0004¢\u0006\f\u0012\u0004\b\n\u0010\u000b\u001a\u0004\b\f\u0010\rR\u001a\u0010\u000e\u001a\u00020\u000f8FX\u0087\u0004¢\u0006\f\u0012\u0004\b\u0010\u0010\u000b\u001a\u0004\b\u0011\u0010\u0012R\u001a\u0010\u0013\u001a\u00020\u000f8FX\u0087\u0004¢\u0006\f\u0012\u0004\b\u0014\u0010\u000b\u001a\u0004\b\u0015\u0010\u0012R\u001a\u0010\u0016\u001a\u00020\u000f8FX\u0087\u0004¢\u0006\f\u0012\u0004\b\u0017\u0010\u000b\u001a\u0004\b\u0018\u0010\u0012R\u001a\u0010\u0019\u001a\u00020\u000f8FX\u0087\u0004¢\u0006\f\u0012\u0004\b\u001a\u0010\u000b\u001a\u0004\b\u001b\u0010\u0012R\u001a\u0010\u001c\u001a\u00020\u000f8FX\u0087\u0004¢\u0006\f\u0012\u0004\b\u001d\u0010\u000b\u001a\u0004\b\u001e\u0010\u0012R\u001a\u0010\u001f\u001a\u00020\u000f8FX\u0087\u0004¢\u0006\f\u0012\u0004\b \u0010\u000b\u001a\u0004\b!\u0010\u0012R\u001a\u0010\"\u001a\u00020\u000f8FX\u0087\u0004¢\u0006\f\u0012\u0004\b#\u0010\u000b\u001a\u0004\b$\u0010\u0012R\u0011\u0010%\u001a\u00020\u00038F¢\u0006\u0006\u001a\u0004\b&\u0010\u0005R\u0011\u0010'\u001a\u00020\u00038F¢\u0006\u0006\u001a\u0004\b(\u0010\u0005R\u0011\u0010)\u001a\u00020\u00038F¢\u0006\u0006\u001a\u0004\b*\u0010\u0005R\u0011\u0010+\u001a\u00020\u00038F¢\u0006\u0006\u001a\u0004\b,\u0010\u0005R\u0011\u0010-\u001a\u00020\u00038F¢\u0006\u0006\u001a\u0004\b.\u0010\u0005R\u0011\u0010/\u001a\u00020\u00038F¢\u0006\u0006\u001a\u0004\b0\u0010\u0005R\u0011\u00101\u001a\u00020\u00038F¢\u0006\u0006\u001a\u0004\b2\u0010\u0005R\u001a\u00103\u001a\u00020\t8@X\u0081\u0004¢\u0006\f\u0012\u0004\b4\u0010\u000b\u001a\u0004\b5\u0010\rR\u001a\u00106\u001a\u00020\t8@X\u0081\u0004¢\u0006\f\u0012\u0004\b7\u0010\u000b\u001a\u0004\b8\u0010\rR\u000e\u0010\u0002\u001a\u00020\u0003X\u0082\u0004¢\u0006\u0002\n\u0000R\u001a\u00109\u001a\u00020\t8@X\u0081\u0004¢\u0006\f\u0012\u0004\b:\u0010\u000b\u001a\u0004\b;\u0010\rR\u0014\u0010<\u001a\u00020=8BX\u0082\u0004¢\u0006\u0006\u001a\u0004\b>\u0010?R\u0015\u0010@\u001a\u00020\t8Â\u0002X\u0082\u0004¢\u0006\u0006\u001a\u0004\bA\u0010\rR\u0014\u0010B\u001a\u00020\u00038BX\u0082\u0004¢\u0006\u0006\u001a\u0004\bC\u0010\u0005\u0088\u0001\u0002\u0092\u0001\u00020\u0003ø\u0001\u0000\u0082\u0002\u000f\n\u0002\b\u0019\n\u0002\b!\n\u0005\b\u009920\u0001¨\u0006¥\u0001"}, d2 = {"Lkotlin/time/Duration;", "", "rawValue", "", "constructor-impl", "(J)J", "absoluteValue", "getAbsoluteValue-UwyO8pc", "hoursComponent", "", "getHoursComponent$annotations", "()V", "getHoursComponent-impl", "(J)I", "inDays", "", "getInDays$annotations", "getInDays-impl", "(J)D", "inHours", "getInHours$annotations", "getInHours-impl", "inMicroseconds", "getInMicroseconds$annotations", "getInMicroseconds-impl", "inMilliseconds", "getInMilliseconds$annotations", "getInMilliseconds-impl", "inMinutes", "getInMinutes$annotations", "getInMinutes-impl", "inNanoseconds", "getInNanoseconds$annotations", "getInNanoseconds-impl", "inSeconds", "getInSeconds$annotations", "getInSeconds-impl", "inWholeDays", "getInWholeDays-impl", "inWholeHours", "getInWholeHours-impl", "inWholeMicroseconds", "getInWholeMicroseconds-impl", "inWholeMilliseconds", "getInWholeMilliseconds-impl", "inWholeMinutes", "getInWholeMinutes-impl", "inWholeNanoseconds", "getInWholeNanoseconds-impl", "inWholeSeconds", "getInWholeSeconds-impl", "minutesComponent", "getMinutesComponent$annotations", "getMinutesComponent-impl", "nanosecondsComponent", "getNanosecondsComponent$annotations", "getNanosecondsComponent-impl", "secondsComponent", "getSecondsComponent$annotations", "getSecondsComponent-impl", "storageUnit", "Lkotlin/time/DurationUnit;", "getStorageUnit-impl", "(J)Lkotlin/time/DurationUnit;", "unitDiscriminator", "getUnitDiscriminator-impl", "value", "getValue-impl", "addValuesMixedRanges", "thisMillis", "otherNanos", "addValuesMixedRanges-UwyO8pc", "(JJJ)J", "compareTo", "other", "compareTo-LRDsOJo", "(JJ)I", "div", "scale", "div-UwyO8pc", "(JD)J", "(JI)J", "div-LRDsOJo", "(JJ)D", "equals", "", "", "equals-impl", "(JLjava/lang/Object;)Z", "hashCode", "hashCode-impl", "isFinite", "isFinite-impl", "(J)Z", "isInMillis", "isInMillis-impl", "isInNanos", "isInNanos-impl", "isInfinite", "isInfinite-impl", "isNegative", "isNegative-impl", "isPositive", "isPositive-impl", "minus", "minus-LRDsOJo", "(JJ)J", "plus", "plus-LRDsOJo", "times", "times-UwyO8pc", "toComponents", "T", "action", "Lkotlin/Function5;", "Lkotlin/ParameterName;", "name", "days", "hours", "minutes", "seconds", "nanoseconds", "toComponents-impl", "(JLkotlin/jvm/functions/Function5;)Ljava/lang/Object;", "Lkotlin/Function4;", "(JLkotlin/jvm/functions/Function4;)Ljava/lang/Object;", "Lkotlin/Function3;", "(JLkotlin/jvm/functions/Function3;)Ljava/lang/Object;", "Lkotlin/Function2;", "(JLkotlin/jvm/functions/Function2;)Ljava/lang/Object;", "toDouble", "unit", "toDouble-impl", "(JLkotlin/time/DurationUnit;)D", "toInt", "toInt-impl", "(JLkotlin/time/DurationUnit;)I", "toIsoString", "", "toIsoString-impl", "(J)Ljava/lang/String;", "toLong", "toLong-impl", "(JLkotlin/time/DurationUnit;)J", "toLongMilliseconds", "toLongMilliseconds-impl", "toLongNanoseconds", "toLongNanoseconds-impl", "toString", "toString-impl", "decimals", "(JLkotlin/time/DurationUnit;I)Ljava/lang/String;", "unaryMinus", "unaryMinus-UwyO8pc", "appendFractional", "", "Ljava/lang/StringBuilder;", "Lkotlin/text/StringBuilder;", "whole", "fractional", "fractionalSize", "isoZeroes", "appendFractional-impl", "(JLjava/lang/StringBuilder;IIILjava/lang/String;Z)V", "Companion", "kotlin-stdlib"}, k = 1, mv = {1, 7, 1}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
@JvmInline
public final class Duration implements Comparable<Duration> {
    private final long rawValue;

    /* JADX INFO: renamed from: Companion, reason: from kotlin metadata */
    public static final Companion INSTANCE = new Companion(null);
    private static final long ZERO = m1413constructorimpl(0);
    private static final long INFINITE = DurationKt.durationOfMillis(DurationKt.MAX_MILLIS);
    private static final long NEG_INFINITE = DurationKt.durationOfMillis(-4611686018427387903L);

    /* JADX INFO: renamed from: box-impl, reason: not valid java name */
    public static final /* synthetic */ Duration m1411boximpl(long j) {
        return new Duration(j);
    }

    /* JADX INFO: renamed from: equals-impl, reason: not valid java name */
    public static boolean m1417equalsimpl(long j, Object obj) {
        return (obj instanceof Duration) && j == ((Duration) obj).getRawValue();
    }

    /* JADX INFO: renamed from: equals-impl0, reason: not valid java name */
    public static final boolean m1418equalsimpl0(long j, long j2) {
        return j == j2;
    }

    public static /* synthetic */ void getHoursComponent$annotations() {
    }

    @Deprecated(message = "Use inWholeDays property instead or convert toDouble(DAYS) if a double value is required.", replaceWith = @ReplaceWith(expression = "toDouble(DurationUnit.DAYS)", imports = {}))
    public static /* synthetic */ void getInDays$annotations() {
    }

    @Deprecated(message = "Use inWholeHours property instead or convert toDouble(HOURS) if a double value is required.", replaceWith = @ReplaceWith(expression = "toDouble(DurationUnit.HOURS)", imports = {}))
    public static /* synthetic */ void getInHours$annotations() {
    }

    @Deprecated(message = "Use inWholeMicroseconds property instead or convert toDouble(MICROSECONDS) if a double value is required.", replaceWith = @ReplaceWith(expression = "toDouble(DurationUnit.MICROSECONDS)", imports = {}))
    public static /* synthetic */ void getInMicroseconds$annotations() {
    }

    @Deprecated(message = "Use inWholeMilliseconds property instead or convert toDouble(MILLISECONDS) if a double value is required.", replaceWith = @ReplaceWith(expression = "toDouble(DurationUnit.MILLISECONDS)", imports = {}))
    public static /* synthetic */ void getInMilliseconds$annotations() {
    }

    @Deprecated(message = "Use inWholeMinutes property instead or convert toDouble(MINUTES) if a double value is required.", replaceWith = @ReplaceWith(expression = "toDouble(DurationUnit.MINUTES)", imports = {}))
    public static /* synthetic */ void getInMinutes$annotations() {
    }

    @Deprecated(message = "Use inWholeNanoseconds property instead or convert toDouble(NANOSECONDS) if a double value is required.", replaceWith = @ReplaceWith(expression = "toDouble(DurationUnit.NANOSECONDS)", imports = {}))
    public static /* synthetic */ void getInNanoseconds$annotations() {
    }

    @Deprecated(message = "Use inWholeSeconds property instead or convert toDouble(SECONDS) if a double value is required.", replaceWith = @ReplaceWith(expression = "toDouble(DurationUnit.SECONDS)", imports = {}))
    public static /* synthetic */ void getInSeconds$annotations() {
    }

    public static /* synthetic */ void getMinutesComponent$annotations() {
    }

    public static /* synthetic */ void getNanosecondsComponent$annotations() {
    }

    public static /* synthetic */ void getSecondsComponent$annotations() {
    }

    /* JADX INFO: renamed from: hashCode-impl, reason: not valid java name */
    public static int m1441hashCodeimpl(long j) {
        return (int) ((j >>> 32) ^ j);
    }

    public boolean equals(Object obj) {
        return m1417equalsimpl(this.rawValue, obj);
    }

    public int hashCode() {
        return m1441hashCodeimpl(this.rawValue);
    }

    /* JADX INFO: renamed from: unbox-impl, reason: not valid java name and from getter */
    public final /* synthetic */ long getRawValue() {
        return this.rawValue;
    }

    @Override // java.lang.Comparable
    public /* bridge */ /* synthetic */ int compareTo(Duration duration) {
        return m1466compareToLRDsOJo(duration.getRawValue());
    }

    private /* synthetic */ Duration(long rawValue) {
        this.rawValue = rawValue;
    }

    /* JADX INFO: renamed from: getValue-impl, reason: not valid java name */
    private static final long m1440getValueimpl(long arg0) {
        return arg0 >> 1;
    }

    /* JADX INFO: renamed from: getUnitDiscriminator-impl, reason: not valid java name */
    private static final int m1439getUnitDiscriminatorimpl(long arg0) {
        return ((int) arg0) & 1;
    }

    /* JADX INFO: renamed from: isInNanos-impl, reason: not valid java name */
    private static final boolean m1444isInNanosimpl(long arg0) {
        return (((int) arg0) & 1) == 0;
    }

    /* JADX INFO: renamed from: isInMillis-impl, reason: not valid java name */
    private static final boolean m1443isInMillisimpl(long arg0) {
        return (((int) arg0) & 1) == 1;
    }

    /* JADX INFO: renamed from: getStorageUnit-impl, reason: not valid java name */
    private static final DurationUnit m1438getStorageUnitimpl(long arg0) {
        return m1444isInNanosimpl(arg0) ? DurationUnit.NANOSECONDS : DurationUnit.MILLISECONDS;
    }

    /* JADX INFO: renamed from: constructor-impl, reason: not valid java name */
    public static long m1413constructorimpl(long rawValue) {
        if (DurationJvmKt.getDurationAssertionsEnabled()) {
            if (m1444isInNanosimpl(rawValue)) {
                if (!new LongRange(-4611686018426999999L, DurationKt.MAX_NANOS).contains(m1440getValueimpl(rawValue))) {
                    throw new AssertionError(m1440getValueimpl(rawValue) + " ns is out of nanoseconds range");
                }
            } else {
                if (!new LongRange(-4611686018427387903L, DurationKt.MAX_MILLIS).contains(m1440getValueimpl(rawValue))) {
                    throw new AssertionError(m1440getValueimpl(rawValue) + " ms is out of milliseconds range");
                }
                if (new LongRange(-4611686018426L, 4611686018426L).contains(m1440getValueimpl(rawValue))) {
                    throw new AssertionError(m1440getValueimpl(rawValue) + " ms is denormalized");
                }
            }
        }
        return rawValue;
    }

    /* JADX INFO: compiled from: Duration.kt */
    @Metadata(d1 = {"\u0000<\n\u0002\u0018\u0002\n\u0002\u0010\u0000\n\u0002\b\u0002\n\u0002\u0018\u0002\n\u0002\b\b\n\u0002\u0010\u0006\n\u0002\b\u0004\n\u0002\u0010\b\n\u0002\b\u0002\n\u0002\u0010\t\n\u0002\b\u0017\n\u0002\u0018\u0002\n\u0002\b\b\n\u0002\u0010\u000e\n\u0002\b\n\b\u0086\u0003\u0018\u00002\u00020\u0001B\u0007\b\u0002¢\u0006\u0002\u0010\u0002J \u0010*\u001a\u00020\r2\u0006\u0010+\u001a\u00020\r2\u0006\u0010,\u001a\u00020-2\u0006\u0010.\u001a\u00020-H\u0007J\u001d\u0010\f\u001a\u00020\u00042\u0006\u0010+\u001a\u00020\rH\u0007ø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\b/\u0010\u0011J\u001d\u0010\f\u001a\u00020\u00042\u0006\u0010+\u001a\u00020\u0012H\u0007ø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\b/\u0010\u0014J\u001d\u0010\f\u001a\u00020\u00042\u0006\u0010+\u001a\u00020\u0015H\u0007ø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\b/\u0010\u0017J\u001d\u0010\u0018\u001a\u00020\u00042\u0006\u0010+\u001a\u00020\rH\u0007ø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\b0\u0010\u0011J\u001d\u0010\u0018\u001a\u00020\u00042\u0006\u0010+\u001a\u00020\u0012H\u0007ø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\b0\u0010\u0014J\u001d\u0010\u0018\u001a\u00020\u00042\u0006\u0010+\u001a\u00020\u0015H\u0007ø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\b0\u0010\u0017J\u001d\u0010\u001b\u001a\u00020\u00042\u0006\u0010+\u001a\u00020\rH\u0007ø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\b1\u0010\u0011J\u001d\u0010\u001b\u001a\u00020\u00042\u0006\u0010+\u001a\u00020\u0012H\u0007ø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\b1\u0010\u0014J\u001d\u0010\u001b\u001a\u00020\u00042\u0006\u0010+\u001a\u00020\u0015H\u0007ø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\b1\u0010\u0017J\u001d\u0010\u001e\u001a\u00020\u00042\u0006\u0010+\u001a\u00020\rH\u0007ø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\b2\u0010\u0011J\u001d\u0010\u001e\u001a\u00020\u00042\u0006\u0010+\u001a\u00020\u0012H\u0007ø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\b2\u0010\u0014J\u001d\u0010\u001e\u001a\u00020\u00042\u0006\u0010+\u001a\u00020\u0015H\u0007ø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\b2\u0010\u0017J\u001d\u0010!\u001a\u00020\u00042\u0006\u0010+\u001a\u00020\rH\u0007ø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\b3\u0010\u0011J\u001d\u0010!\u001a\u00020\u00042\u0006\u0010+\u001a\u00020\u0012H\u0007ø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\b3\u0010\u0014J\u001d\u0010!\u001a\u00020\u00042\u0006\u0010+\u001a\u00020\u0015H\u0007ø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\b3\u0010\u0017J\u001d\u0010$\u001a\u00020\u00042\u0006\u0010+\u001a\u00020\rH\u0007ø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\b4\u0010\u0011J\u001d\u0010$\u001a\u00020\u00042\u0006\u0010+\u001a\u00020\u0012H\u0007ø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\b4\u0010\u0014J\u001d\u0010$\u001a\u00020\u00042\u0006\u0010+\u001a\u00020\u0015H\u0007ø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\b4\u0010\u0017J\u001b\u00105\u001a\u00020\u00042\u0006\u0010+\u001a\u000206ø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\b7\u00108J\u001b\u00109\u001a\u00020\u00042\u0006\u0010+\u001a\u000206ø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\b:\u00108J\u001b\u0010;\u001a\u0004\u0018\u00010\u00042\u0006\u0010+\u001a\u000206ø\u0001\u0001ø\u0001\u0000¢\u0006\u0002\b<J\u001b\u0010=\u001a\u0004\u0018\u00010\u00042\u0006\u0010+\u001a\u000206ø\u0001\u0001ø\u0001\u0000¢\u0006\u0002\b>J\u001d\u0010'\u001a\u00020\u00042\u0006\u0010+\u001a\u00020\rH\u0007ø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\b?\u0010\u0011J\u001d\u0010'\u001a\u00020\u00042\u0006\u0010+\u001a\u00020\u0012H\u0007ø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\b?\u0010\u0014J\u001d\u0010'\u001a\u00020\u00042\u0006\u0010+\u001a\u00020\u0015H\u0007ø\u0001\u0001ø\u0001\u0000¢\u0006\u0004\b?\u0010\u0017R\u0019\u0010\u0003\u001a\u00020\u0004ø\u0001\u0000ø\u0001\u0001¢\u0006\n\n\u0002\u0010\u0007\u001a\u0004\b\u0005\u0010\u0006R\u001c\u0010\b\u001a\u00020\u0004X\u0080\u0004ø\u0001\u0000ø\u0001\u0001¢\u0006\n\n\u0002\u0010\u0007\u001a\u0004\b\t\u0010\u0006R\u0019\u0010\n\u001a\u00020\u0004ø\u0001\u0000ø\u0001\u0001¢\u0006\n\n\u0002\u0010\u0007\u001a\u0004\b\u000b\u0010\u0006R%\u0010\f\u001a\u00020\u0004*\u00020\r8Æ\u0002X\u0087\u0004ø\u0001\u0000ø\u0001\u0001¢\u0006\f\u0012\u0004\b\u000e\u0010\u000f\u001a\u0004\b\u0010\u0010\u0011R%\u0010\f\u001a\u00020\u0004*\u00020\u00128Æ\u0002X\u0087\u0004ø\u0001\u0000ø\u0001\u0001¢\u0006\f\u0012\u0004\b\u000e\u0010\u0013\u001a\u0004\b\u0010\u0010\u0014R%\u0010\f\u001a\u00020\u0004*\u00020\u00158Æ\u0002X\u0087\u0004ø\u0001\u0000ø\u0001\u0001¢\u0006\f\u0012\u0004\b\u000e\u0010\u0016\u001a\u0004\b\u0010\u0010\u0017R%\u0010\u0018\u001a\u00020\u0004*\u00020\r8Æ\u0002X\u0087\u0004ø\u0001\u0000ø\u0001\u0001¢\u0006\f\u0012\u0004\b\u0019\u0010\u000f\u001a\u0004\b\u001a\u0010\u0011R%\u0010\u0018\u001a\u00020\u0004*\u00020\u00128Æ\u0002X\u0087\u0004ø\u0001\u0000ø\u0001\u0001¢\u0006\f\u0012\u0004\b\u0019\u0010\u0013\u001a\u0004\b\u001a\u0010\u0014R%\u0010\u0018\u001a\u00020\u0004*\u00020\u00158Æ\u0002X\u0087\u0004ø\u0001\u0000ø\u0001\u0001¢\u0006\f\u0012\u0004\b\u0019\u0010\u0016\u001a\u0004\b\u001a\u0010\u0017R%\u0010\u001b\u001a\u00020\u0004*\u00020\r8Æ\u0002X\u0087\u0004ø\u0001\u0000ø\u0001\u0001¢\u0006\f\u0012\u0004\b\u001c\u0010\u000f\u001a\u0004\b\u001d\u0010\u0011R%\u0010\u001b\u001a\u00020\u0004*\u00020\u00128Æ\u0002X\u0087\u0004ø\u0001\u0000ø\u0001\u0001¢\u0006\f\u0012\u0004\b\u001c\u0010\u0013\u001a\u0004\b\u001d\u0010\u0014R%\u0010\u001b\u001a\u00020\u0004*\u00020\u00158Æ\u0002X\u0087\u0004ø\u0001\u0000ø\u0001\u0001¢\u0006\f\u0012\u0004\b\u001c\u0010\u0016\u001a\u0004\b\u001d\u0010\u0017R%\u0010\u001e\u001a\u00020\u0004*\u00020\r8Æ\u0002X\u0087\u0004ø\u0001\u0000ø\u0001\u0001¢\u0006\f\u0012\u0004\b\u001f\u0010\u000f\u001a\u0004\b \u0010\u0011R%\u0010\u001e\u001a\u00020\u0004*\u00020\u00128Æ\u0002X\u0087\u0004ø\u0001\u0000ø\u0001\u0001¢\u0006\f\u0012\u0004\b\u001f\u0010\u0013\u001a\u0004\b \u0010\u0014R%\u0010\u001e\u001a\u00020\u0004*\u00020\u00158Æ\u0002X\u0087\u0004ø\u0001\u0000ø\u0001\u0001¢\u0006\f\u0012\u0004\b\u001f\u0010\u0016\u001a\u0004\b \u0010\u0017R%\u0010!\u001a\u00020\u0004*\u00020\r8Æ\u0002X\u0087\u0004ø\u0001\u0000ø\u0001\u0001¢\u0006\f\u0012\u0004\b\"\u0010\u000f\u001a\u0004\b#\u0010\u0011R%\u0010!\u001a\u00020\u0004*\u00020\u00128Æ\u0002X\u0087\u0004ø\u0001\u0000ø\u0001\u0001¢\u0006\f\u0012\u0004\b\"\u0010\u0013\u001a\u0004\b#\u0010\u0014R%\u0010!\u001a\u00020\u0004*\u00020\u00158Æ\u0002X\u0087\u0004ø\u0001\u0000ø\u0001\u0001¢\u0006\f\u0012\u0004\b\"\u0010\u0016\u001a\u0004\b#\u0010\u0017R%\u0010$\u001a\u00020\u0004*\u00020\r8Æ\u0002X\u0087\u0004ø\u0001\u0000ø\u0001\u0001¢\u0006\f\u0012\u0004\b%\u0010\u000f\u001a\u0004\b&\u0010\u0011R%\u0010$\u001a\u00020\u0004*\u00020\u00128Æ\u0002X\u0087\u0004ø\u0001\u0000ø\u0001\u0001¢\u0006\f\u0012\u0004\b%\u0010\u0013\u001a\u0004\b&\u0010\u0014R%\u0010$\u001a\u00020\u0004*\u00020\u00158Æ\u0002X\u0087\u0004ø\u0001\u0000ø\u0001\u0001¢\u0006\f\u0012\u0004\b%\u0010\u0016\u001a\u0004\b&\u0010\u0017R%\u0010'\u001a\u00020\u0004*\u00020\r8Æ\u0002X\u0087\u0004ø\u0001\u0000ø\u0001\u0001¢\u0006\f\u0012\u0004\b(\u0010\u000f\u001a\u0004\b)\u0010\u0011R%\u0010'\u001a\u00020\u0004*\u00020\u00128Æ\u0002X\u0087\u0004ø\u0001\u0000ø\u0001\u0001¢\u0006\f\u0012\u0004\b(\u0010\u0013\u001a\u0004\b)\u0010\u0014R%\u0010'\u001a\u00020\u0004*\u00020\u00158Æ\u0002X\u0087\u0004ø\u0001\u0000ø\u0001\u0001¢\u0006\f\u0012\u0004\b(\u0010\u0016\u001a\u0004\b)\u0010\u0017\u0082\u0002\b\n\u0002\b\u0019\n\u0002\b!¨\u0006@"}, d2 = {"Lkotlin/time/Duration$Companion;", "", "()V", "INFINITE", "Lkotlin/time/Duration;", "getINFINITE-UwyO8pc", "()J", "J", "NEG_INFINITE", "getNEG_INFINITE-UwyO8pc$kotlin_stdlib", "ZERO", "getZERO-UwyO8pc", "days", "", "getDays-UwyO8pc$annotations", "(D)V", "getDays-UwyO8pc", "(D)J", "", "(I)V", "(I)J", "", "(J)V", "(J)J", "hours", "getHours-UwyO8pc$annotations", "getHours-UwyO8pc", "microseconds", "getMicroseconds-UwyO8pc$annotations", "getMicroseconds-UwyO8pc", "milliseconds", "getMilliseconds-UwyO8pc$annotations", "getMilliseconds-UwyO8pc", "minutes", "getMinutes-UwyO8pc$annotations", "getMinutes-UwyO8pc", "nanoseconds", "getNanoseconds-UwyO8pc$annotations", "getNanoseconds-UwyO8pc", "seconds", "getSeconds-UwyO8pc$annotations", "getSeconds-UwyO8pc", "convert", "value", "sourceUnit", "Lkotlin/time/DurationUnit;", "targetUnit", "days-UwyO8pc", "hours-UwyO8pc", "microseconds-UwyO8pc", "milliseconds-UwyO8pc", "minutes-UwyO8pc", "nanoseconds-UwyO8pc", "parse", "", "parse-UwyO8pc", "(Ljava/lang/String;)J", "parseIsoString", "parseIsoString-UwyO8pc", "parseIsoStringOrNull", "parseIsoStringOrNull-FghU774", "parseOrNull", "parseOrNull-FghU774", "seconds-UwyO8pc", "kotlin-stdlib"}, k = 1, mv = {1, 7, 1}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    public static final class Companion {
        public /* synthetic */ Companion(DefaultConstructorMarker defaultConstructorMarker) {
            this();
        }

        /* JADX INFO: renamed from: getDays-UwyO8pc$annotations, reason: not valid java name */
        public static /* synthetic */ void m1471getDaysUwyO8pc$annotations(double d) {
        }

        /* JADX INFO: renamed from: getDays-UwyO8pc$annotations, reason: not valid java name */
        public static /* synthetic */ void m1472getDaysUwyO8pc$annotations(int i) {
        }

        /* JADX INFO: renamed from: getDays-UwyO8pc$annotations, reason: not valid java name */
        public static /* synthetic */ void m1473getDaysUwyO8pc$annotations(long j) {
        }

        /* JADX INFO: renamed from: getHours-UwyO8pc$annotations, reason: not valid java name */
        public static /* synthetic */ void m1477getHoursUwyO8pc$annotations(double d) {
        }

        /* JADX INFO: renamed from: getHours-UwyO8pc$annotations, reason: not valid java name */
        public static /* synthetic */ void m1478getHoursUwyO8pc$annotations(int i) {
        }

        /* JADX INFO: renamed from: getHours-UwyO8pc$annotations, reason: not valid java name */
        public static /* synthetic */ void m1479getHoursUwyO8pc$annotations(long j) {
        }

        /* JADX INFO: renamed from: getMicroseconds-UwyO8pc$annotations, reason: not valid java name */
        public static /* synthetic */ void m1483getMicrosecondsUwyO8pc$annotations(double d) {
        }

        /* JADX INFO: renamed from: getMicroseconds-UwyO8pc$annotations, reason: not valid java name */
        public static /* synthetic */ void m1484getMicrosecondsUwyO8pc$annotations(int i) {
        }

        /* JADX INFO: renamed from: getMicroseconds-UwyO8pc$annotations, reason: not valid java name */
        public static /* synthetic */ void m1485getMicrosecondsUwyO8pc$annotations(long j) {
        }

        /* JADX INFO: renamed from: getMilliseconds-UwyO8pc$annotations, reason: not valid java name */
        public static /* synthetic */ void m1489getMillisecondsUwyO8pc$annotations(double d) {
        }

        /* JADX INFO: renamed from: getMilliseconds-UwyO8pc$annotations, reason: not valid java name */
        public static /* synthetic */ void m1490getMillisecondsUwyO8pc$annotations(int i) {
        }

        /* JADX INFO: renamed from: getMilliseconds-UwyO8pc$annotations, reason: not valid java name */
        public static /* synthetic */ void m1491getMillisecondsUwyO8pc$annotations(long j) {
        }

        /* JADX INFO: renamed from: getMinutes-UwyO8pc$annotations, reason: not valid java name */
        public static /* synthetic */ void m1495getMinutesUwyO8pc$annotations(double d) {
        }

        /* JADX INFO: renamed from: getMinutes-UwyO8pc$annotations, reason: not valid java name */
        public static /* synthetic */ void m1496getMinutesUwyO8pc$annotations(int i) {
        }

        /* JADX INFO: renamed from: getMinutes-UwyO8pc$annotations, reason: not valid java name */
        public static /* synthetic */ void m1497getMinutesUwyO8pc$annotations(long j) {
        }

        /* JADX INFO: renamed from: getNanoseconds-UwyO8pc$annotations, reason: not valid java name */
        public static /* synthetic */ void m1501getNanosecondsUwyO8pc$annotations(double d) {
        }

        /* JADX INFO: renamed from: getNanoseconds-UwyO8pc$annotations, reason: not valid java name */
        public static /* synthetic */ void m1502getNanosecondsUwyO8pc$annotations(int i) {
        }

        /* JADX INFO: renamed from: getNanoseconds-UwyO8pc$annotations, reason: not valid java name */
        public static /* synthetic */ void m1503getNanosecondsUwyO8pc$annotations(long j) {
        }

        /* JADX INFO: renamed from: getSeconds-UwyO8pc$annotations, reason: not valid java name */
        public static /* synthetic */ void m1507getSecondsUwyO8pc$annotations(double d) {
        }

        /* JADX INFO: renamed from: getSeconds-UwyO8pc$annotations, reason: not valid java name */
        public static /* synthetic */ void m1508getSecondsUwyO8pc$annotations(int i) {
        }

        /* JADX INFO: renamed from: getSeconds-UwyO8pc$annotations, reason: not valid java name */
        public static /* synthetic */ void m1509getSecondsUwyO8pc$annotations(long j) {
        }

        private Companion() {
        }

        /* JADX INFO: renamed from: getZERO-UwyO8pc, reason: not valid java name */
        public final long m1515getZEROUwyO8pc() {
            return Duration.ZERO;
        }

        /* JADX INFO: renamed from: getINFINITE-UwyO8pc, reason: not valid java name */
        public final long m1513getINFINITEUwyO8pc() {
            return Duration.INFINITE;
        }

        /* JADX INFO: renamed from: getNEG_INFINITE-UwyO8pc$kotlin_stdlib, reason: not valid java name */
        public final long m1514getNEG_INFINITEUwyO8pc$kotlin_stdlib() {
            return Duration.NEG_INFINITE;
        }

        public final double convert(double value, DurationUnit sourceUnit, DurationUnit targetUnit) {
            Intrinsics.checkNotNullParameter(sourceUnit, "sourceUnit");
            Intrinsics.checkNotNullParameter(targetUnit, "targetUnit");
            return DurationUnitKt.convertDurationUnit(value, sourceUnit, targetUnit);
        }

        /* JADX INFO: renamed from: getNanoseconds-UwyO8pc, reason: not valid java name */
        private final long m1499getNanosecondsUwyO8pc(int $this$nanoseconds) {
            return DurationKt.toDuration($this$nanoseconds, DurationUnit.NANOSECONDS);
        }

        /* JADX INFO: renamed from: getNanoseconds-UwyO8pc, reason: not valid java name */
        private final long m1500getNanosecondsUwyO8pc(long $this$nanoseconds) {
            return DurationKt.toDuration($this$nanoseconds, DurationUnit.NANOSECONDS);
        }

        /* JADX INFO: renamed from: getNanoseconds-UwyO8pc, reason: not valid java name */
        private final long m1498getNanosecondsUwyO8pc(double $this$nanoseconds) {
            return DurationKt.toDuration($this$nanoseconds, DurationUnit.NANOSECONDS);
        }

        /* JADX INFO: renamed from: getMicroseconds-UwyO8pc, reason: not valid java name */
        private final long m1481getMicrosecondsUwyO8pc(int $this$microseconds) {
            return DurationKt.toDuration($this$microseconds, DurationUnit.MICROSECONDS);
        }

        /* JADX INFO: renamed from: getMicroseconds-UwyO8pc, reason: not valid java name */
        private final long m1482getMicrosecondsUwyO8pc(long $this$microseconds) {
            return DurationKt.toDuration($this$microseconds, DurationUnit.MICROSECONDS);
        }

        /* JADX INFO: renamed from: getMicroseconds-UwyO8pc, reason: not valid java name */
        private final long m1480getMicrosecondsUwyO8pc(double $this$microseconds) {
            return DurationKt.toDuration($this$microseconds, DurationUnit.MICROSECONDS);
        }

        /* JADX INFO: renamed from: getMilliseconds-UwyO8pc, reason: not valid java name */
        private final long m1487getMillisecondsUwyO8pc(int $this$milliseconds) {
            return DurationKt.toDuration($this$milliseconds, DurationUnit.MILLISECONDS);
        }

        /* JADX INFO: renamed from: getMilliseconds-UwyO8pc, reason: not valid java name */
        private final long m1488getMillisecondsUwyO8pc(long $this$milliseconds) {
            return DurationKt.toDuration($this$milliseconds, DurationUnit.MILLISECONDS);
        }

        /* JADX INFO: renamed from: getMilliseconds-UwyO8pc, reason: not valid java name */
        private final long m1486getMillisecondsUwyO8pc(double $this$milliseconds) {
            return DurationKt.toDuration($this$milliseconds, DurationUnit.MILLISECONDS);
        }

        /* JADX INFO: renamed from: getSeconds-UwyO8pc, reason: not valid java name */
        private final long m1505getSecondsUwyO8pc(int $this$seconds) {
            return DurationKt.toDuration($this$seconds, DurationUnit.SECONDS);
        }

        /* JADX INFO: renamed from: getSeconds-UwyO8pc, reason: not valid java name */
        private final long m1506getSecondsUwyO8pc(long $this$seconds) {
            return DurationKt.toDuration($this$seconds, DurationUnit.SECONDS);
        }

        /* JADX INFO: renamed from: getSeconds-UwyO8pc, reason: not valid java name */
        private final long m1504getSecondsUwyO8pc(double $this$seconds) {
            return DurationKt.toDuration($this$seconds, DurationUnit.SECONDS);
        }

        /* JADX INFO: renamed from: getMinutes-UwyO8pc, reason: not valid java name */
        private final long m1493getMinutesUwyO8pc(int $this$minutes) {
            return DurationKt.toDuration($this$minutes, DurationUnit.MINUTES);
        }

        /* JADX INFO: renamed from: getMinutes-UwyO8pc, reason: not valid java name */
        private final long m1494getMinutesUwyO8pc(long $this$minutes) {
            return DurationKt.toDuration($this$minutes, DurationUnit.MINUTES);
        }

        /* JADX INFO: renamed from: getMinutes-UwyO8pc, reason: not valid java name */
        private final long m1492getMinutesUwyO8pc(double $this$minutes) {
            return DurationKt.toDuration($this$minutes, DurationUnit.MINUTES);
        }

        /* JADX INFO: renamed from: getHours-UwyO8pc, reason: not valid java name */
        private final long m1475getHoursUwyO8pc(int $this$hours) {
            return DurationKt.toDuration($this$hours, DurationUnit.HOURS);
        }

        /* JADX INFO: renamed from: getHours-UwyO8pc, reason: not valid java name */
        private final long m1476getHoursUwyO8pc(long $this$hours) {
            return DurationKt.toDuration($this$hours, DurationUnit.HOURS);
        }

        /* JADX INFO: renamed from: getHours-UwyO8pc, reason: not valid java name */
        private final long m1474getHoursUwyO8pc(double $this$hours) {
            return DurationKt.toDuration($this$hours, DurationUnit.HOURS);
        }

        /* JADX INFO: renamed from: getDays-UwyO8pc, reason: not valid java name */
        private final long m1469getDaysUwyO8pc(int $this$days) {
            return DurationKt.toDuration($this$days, DurationUnit.DAYS);
        }

        /* JADX INFO: renamed from: getDays-UwyO8pc, reason: not valid java name */
        private final long m1470getDaysUwyO8pc(long $this$days) {
            return DurationKt.toDuration($this$days, DurationUnit.DAYS);
        }

        /* JADX INFO: renamed from: getDays-UwyO8pc, reason: not valid java name */
        private final long m1468getDaysUwyO8pc(double $this$days) {
            return DurationKt.toDuration($this$days, DurationUnit.DAYS);
        }

        @Deprecated(message = "Use 'Int.nanoseconds' extension property from Duration.Companion instead.", replaceWith = @ReplaceWith(expression = "value.nanoseconds", imports = {"kotlin.time.Duration.Companion.nanoseconds"}))
        @DeprecatedSinceKotlin(warningSince = "1.6")
        /* JADX INFO: renamed from: nanoseconds-UwyO8pc, reason: not valid java name */
        public final long m1529nanosecondsUwyO8pc(int value) {
            return DurationKt.toDuration(value, DurationUnit.NANOSECONDS);
        }

        @Deprecated(message = "Use 'Long.nanoseconds' extension property from Duration.Companion instead.", replaceWith = @ReplaceWith(expression = "value.nanoseconds", imports = {"kotlin.time.Duration.Companion.nanoseconds"}))
        @DeprecatedSinceKotlin(warningSince = "1.6")
        /* JADX INFO: renamed from: nanoseconds-UwyO8pc, reason: not valid java name */
        public final long m1530nanosecondsUwyO8pc(long value) {
            return DurationKt.toDuration(value, DurationUnit.NANOSECONDS);
        }

        @Deprecated(message = "Use 'Double.nanoseconds' extension property from Duration.Companion instead.", replaceWith = @ReplaceWith(expression = "value.nanoseconds", imports = {"kotlin.time.Duration.Companion.nanoseconds"}))
        @DeprecatedSinceKotlin(warningSince = "1.6")
        /* JADX INFO: renamed from: nanoseconds-UwyO8pc, reason: not valid java name */
        public final long m1528nanosecondsUwyO8pc(double value) {
            return DurationKt.toDuration(value, DurationUnit.NANOSECONDS);
        }

        @Deprecated(message = "Use 'Int.microseconds' extension property from Duration.Companion instead.", replaceWith = @ReplaceWith(expression = "value.microseconds", imports = {"kotlin.time.Duration.Companion.microseconds"}))
        @DeprecatedSinceKotlin(warningSince = "1.6")
        /* JADX INFO: renamed from: microseconds-UwyO8pc, reason: not valid java name */
        public final long m1520microsecondsUwyO8pc(int value) {
            return DurationKt.toDuration(value, DurationUnit.MICROSECONDS);
        }

        @Deprecated(message = "Use 'Long.microseconds' extension property from Duration.Companion instead.", replaceWith = @ReplaceWith(expression = "value.microseconds", imports = {"kotlin.time.Duration.Companion.microseconds"}))
        @DeprecatedSinceKotlin(warningSince = "1.6")
        /* JADX INFO: renamed from: microseconds-UwyO8pc, reason: not valid java name */
        public final long m1521microsecondsUwyO8pc(long value) {
            return DurationKt.toDuration(value, DurationUnit.MICROSECONDS);
        }

        @Deprecated(message = "Use 'Double.microseconds' extension property from Duration.Companion instead.", replaceWith = @ReplaceWith(expression = "value.microseconds", imports = {"kotlin.time.Duration.Companion.microseconds"}))
        @DeprecatedSinceKotlin(warningSince = "1.6")
        /* JADX INFO: renamed from: microseconds-UwyO8pc, reason: not valid java name */
        public final long m1519microsecondsUwyO8pc(double value) {
            return DurationKt.toDuration(value, DurationUnit.MICROSECONDS);
        }

        @Deprecated(message = "Use 'Int.milliseconds' extension property from Duration.Companion instead.", replaceWith = @ReplaceWith(expression = "value.milliseconds", imports = {"kotlin.time.Duration.Companion.milliseconds"}))
        @DeprecatedSinceKotlin(warningSince = "1.6")
        /* JADX INFO: renamed from: milliseconds-UwyO8pc, reason: not valid java name */
        public final long m1523millisecondsUwyO8pc(int value) {
            return DurationKt.toDuration(value, DurationUnit.MILLISECONDS);
        }

        @Deprecated(message = "Use 'Long.milliseconds' extension property from Duration.Companion instead.", replaceWith = @ReplaceWith(expression = "value.milliseconds", imports = {"kotlin.time.Duration.Companion.milliseconds"}))
        @DeprecatedSinceKotlin(warningSince = "1.6")
        /* JADX INFO: renamed from: milliseconds-UwyO8pc, reason: not valid java name */
        public final long m1524millisecondsUwyO8pc(long value) {
            return DurationKt.toDuration(value, DurationUnit.MILLISECONDS);
        }

        @Deprecated(message = "Use 'Double.milliseconds' extension property from Duration.Companion instead.", replaceWith = @ReplaceWith(expression = "value.milliseconds", imports = {"kotlin.time.Duration.Companion.milliseconds"}))
        @DeprecatedSinceKotlin(warningSince = "1.6")
        /* JADX INFO: renamed from: milliseconds-UwyO8pc, reason: not valid java name */
        public final long m1522millisecondsUwyO8pc(double value) {
            return DurationKt.toDuration(value, DurationUnit.MILLISECONDS);
        }

        @Deprecated(message = "Use 'Int.seconds' extension property from Duration.Companion instead.", replaceWith = @ReplaceWith(expression = "value.seconds", imports = {"kotlin.time.Duration.Companion.seconds"}))
        @DeprecatedSinceKotlin(warningSince = "1.6")
        /* JADX INFO: renamed from: seconds-UwyO8pc, reason: not valid java name */
        public final long m1536secondsUwyO8pc(int value) {
            return DurationKt.toDuration(value, DurationUnit.SECONDS);
        }

        @Deprecated(message = "Use 'Long.seconds' extension property from Duration.Companion instead.", replaceWith = @ReplaceWith(expression = "value.seconds", imports = {"kotlin.time.Duration.Companion.seconds"}))
        @DeprecatedSinceKotlin(warningSince = "1.6")
        /* JADX INFO: renamed from: seconds-UwyO8pc, reason: not valid java name */
        public final long m1537secondsUwyO8pc(long value) {
            return DurationKt.toDuration(value, DurationUnit.SECONDS);
        }

        @Deprecated(message = "Use 'Double.seconds' extension property from Duration.Companion instead.", replaceWith = @ReplaceWith(expression = "value.seconds", imports = {"kotlin.time.Duration.Companion.seconds"}))
        @DeprecatedSinceKotlin(warningSince = "1.6")
        /* JADX INFO: renamed from: seconds-UwyO8pc, reason: not valid java name */
        public final long m1535secondsUwyO8pc(double value) {
            return DurationKt.toDuration(value, DurationUnit.SECONDS);
        }

        @Deprecated(message = "Use 'Int.minutes' extension property from Duration.Companion instead.", replaceWith = @ReplaceWith(expression = "value.minutes", imports = {"kotlin.time.Duration.Companion.minutes"}))
        @DeprecatedSinceKotlin(warningSince = "1.6")
        /* JADX INFO: renamed from: minutes-UwyO8pc, reason: not valid java name */
        public final long m1526minutesUwyO8pc(int value) {
            return DurationKt.toDuration(value, DurationUnit.MINUTES);
        }

        @Deprecated(message = "Use 'Long.minutes' extension property from Duration.Companion instead.", replaceWith = @ReplaceWith(expression = "value.minutes", imports = {"kotlin.time.Duration.Companion.minutes"}))
        @DeprecatedSinceKotlin(warningSince = "1.6")
        /* JADX INFO: renamed from: minutes-UwyO8pc, reason: not valid java name */
        public final long m1527minutesUwyO8pc(long value) {
            return DurationKt.toDuration(value, DurationUnit.MINUTES);
        }

        @Deprecated(message = "Use 'Double.minutes' extension property from Duration.Companion instead.", replaceWith = @ReplaceWith(expression = "value.minutes", imports = {"kotlin.time.Duration.Companion.minutes"}))
        @DeprecatedSinceKotlin(warningSince = "1.6")
        /* JADX INFO: renamed from: minutes-UwyO8pc, reason: not valid java name */
        public final long m1525minutesUwyO8pc(double value) {
            return DurationKt.toDuration(value, DurationUnit.MINUTES);
        }

        @Deprecated(message = "Use 'Int.hours' extension property from Duration.Companion instead.", replaceWith = @ReplaceWith(expression = "value.hours", imports = {"kotlin.time.Duration.Companion.hours"}))
        @DeprecatedSinceKotlin(warningSince = "1.6")
        /* JADX INFO: renamed from: hours-UwyO8pc, reason: not valid java name */
        public final long m1517hoursUwyO8pc(int value) {
            return DurationKt.toDuration(value, DurationUnit.HOURS);
        }

        @Deprecated(message = "Use 'Long.hours' extension property from Duration.Companion instead.", replaceWith = @ReplaceWith(expression = "value.hours", imports = {"kotlin.time.Duration.Companion.hours"}))
        @DeprecatedSinceKotlin(warningSince = "1.6")
        /* JADX INFO: renamed from: hours-UwyO8pc, reason: not valid java name */
        public final long m1518hoursUwyO8pc(long value) {
            return DurationKt.toDuration(value, DurationUnit.HOURS);
        }

        @Deprecated(message = "Use 'Double.hours' extension property from Duration.Companion instead.", replaceWith = @ReplaceWith(expression = "value.hours", imports = {"kotlin.time.Duration.Companion.hours"}))
        @DeprecatedSinceKotlin(warningSince = "1.6")
        /* JADX INFO: renamed from: hours-UwyO8pc, reason: not valid java name */
        public final long m1516hoursUwyO8pc(double value) {
            return DurationKt.toDuration(value, DurationUnit.HOURS);
        }

        @Deprecated(message = "Use 'Int.days' extension property from Duration.Companion instead.", replaceWith = @ReplaceWith(expression = "value.days", imports = {"kotlin.time.Duration.Companion.days"}))
        @DeprecatedSinceKotlin(warningSince = "1.6")
        /* JADX INFO: renamed from: days-UwyO8pc, reason: not valid java name */
        public final long m1511daysUwyO8pc(int value) {
            return DurationKt.toDuration(value, DurationUnit.DAYS);
        }

        @Deprecated(message = "Use 'Long.days' extension property from Duration.Companion instead.", replaceWith = @ReplaceWith(expression = "value.days", imports = {"kotlin.time.Duration.Companion.days"}))
        @DeprecatedSinceKotlin(warningSince = "1.6")
        /* JADX INFO: renamed from: days-UwyO8pc, reason: not valid java name */
        public final long m1512daysUwyO8pc(long value) {
            return DurationKt.toDuration(value, DurationUnit.DAYS);
        }

        @Deprecated(message = "Use 'Double.days' extension property from Duration.Companion instead.", replaceWith = @ReplaceWith(expression = "value.days", imports = {"kotlin.time.Duration.Companion.days"}))
        @DeprecatedSinceKotlin(warningSince = "1.6")
        /* JADX INFO: renamed from: days-UwyO8pc, reason: not valid java name */
        public final long m1510daysUwyO8pc(double value) {
            return DurationKt.toDuration(value, DurationUnit.DAYS);
        }

        /* JADX INFO: renamed from: parse-UwyO8pc, reason: not valid java name */
        public final long m1531parseUwyO8pc(String value) {
            Intrinsics.checkNotNullParameter(value, "value");
            try {
                return DurationKt.parseDuration(value, false);
            } catch (IllegalArgumentException e) {
                throw new IllegalArgumentException("Invalid duration string format: '" + value + "'.", e);
            }
        }

        /* JADX INFO: renamed from: parseIsoString-UwyO8pc, reason: not valid java name */
        public final long m1532parseIsoStringUwyO8pc(String value) {
            Intrinsics.checkNotNullParameter(value, "value");
            try {
                return DurationKt.parseDuration(value, true);
            } catch (IllegalArgumentException e) {
                throw new IllegalArgumentException("Invalid ISO duration string format: '" + value + "'.", e);
            }
        }

        /* JADX INFO: renamed from: parseOrNull-FghU774, reason: not valid java name */
        public final Duration m1534parseOrNullFghU774(String value) {
            Intrinsics.checkNotNullParameter(value, "value");
            try {
                return Duration.m1411boximpl(DurationKt.parseDuration(value, false));
            } catch (IllegalArgumentException e) {
                return null;
            }
        }

        /* JADX INFO: renamed from: parseIsoStringOrNull-FghU774, reason: not valid java name */
        public final Duration m1533parseIsoStringOrNullFghU774(String value) {
            Intrinsics.checkNotNullParameter(value, "value");
            try {
                return Duration.m1411boximpl(DurationKt.parseDuration(value, true));
            } catch (IllegalArgumentException e) {
                return null;
            }
        }
    }

    /* JADX INFO: renamed from: unaryMinus-UwyO8pc, reason: not valid java name */
    public static final long m1465unaryMinusUwyO8pc(long arg0) {
        return DurationKt.durationOf(-m1440getValueimpl(arg0), ((int) arg0) & 1);
    }

    /* JADX INFO: renamed from: plus-LRDsOJo, reason: not valid java name */
    public static final long m1449plusLRDsOJo(long arg0, long other) {
        if (m1445isInfiniteimpl(arg0)) {
            if (m1442isFiniteimpl(other) || (arg0 ^ other) >= 0) {
                return arg0;
            }
            throw new IllegalArgumentException("Summing infinite durations of different signs yields an undefined result.");
        }
        if (m1445isInfiniteimpl(other)) {
            return other;
        }
        if ((((int) arg0) & 1) == (((int) other) & 1)) {
            long result = m1440getValueimpl(arg0) + m1440getValueimpl(other);
            return m1444isInNanosimpl(arg0) ? DurationKt.durationOfNanosNormalized(result) : DurationKt.durationOfMillisNormalized(result);
        }
        if (m1443isInMillisimpl(arg0)) {
            return m1409addValuesMixedRangesUwyO8pc(arg0, m1440getValueimpl(arg0), m1440getValueimpl(other));
        }
        return m1409addValuesMixedRangesUwyO8pc(arg0, m1440getValueimpl(other), m1440getValueimpl(arg0));
    }

    /* JADX INFO: renamed from: addValuesMixedRanges-UwyO8pc, reason: not valid java name */
    private static final long m1409addValuesMixedRangesUwyO8pc(long arg0, long thisMillis, long otherNanos) {
        long otherMillis = DurationKt.nanosToMillis(otherNanos);
        long resultMillis = thisMillis + otherMillis;
        if (new LongRange(-4611686018426L, 4611686018426L).contains(resultMillis)) {
            long otherNanoRemainder = otherNanos - DurationKt.millisToNanos(otherMillis);
            return DurationKt.durationOfNanos(DurationKt.millisToNanos(resultMillis) + otherNanoRemainder);
        }
        return DurationKt.durationOfMillis(RangesKt.coerceIn(resultMillis, -4611686018427387903L, DurationKt.MAX_MILLIS));
    }

    /* JADX INFO: renamed from: minus-LRDsOJo, reason: not valid java name */
    public static final long m1448minusLRDsOJo(long arg0, long other) {
        return m1449plusLRDsOJo(arg0, m1465unaryMinusUwyO8pc(other));
    }

    /* JADX INFO: renamed from: times-UwyO8pc, reason: not valid java name */
    public static final long m1451timesUwyO8pc(long arg0, int scale) {
        if (m1445isInfiniteimpl(arg0)) {
            if (scale != 0) {
                return scale > 0 ? arg0 : m1465unaryMinusUwyO8pc(arg0);
            }
            throw new IllegalArgumentException("Multiplying infinite duration by zero yields an undefined result.");
        }
        if (scale == 0) {
            return ZERO;
        }
        long value = m1440getValueimpl(arg0);
        long result = ((long) scale) * value;
        if (!m1444isInNanosimpl(arg0)) {
            if (result / ((long) scale) == value) {
                return DurationKt.durationOfMillis(RangesKt.coerceIn(result, new LongRange(-4611686018427387903L, DurationKt.MAX_MILLIS)));
            }
            return MathKt.getSign(value) * MathKt.getSign(scale) > 0 ? INFINITE : NEG_INFINITE;
        }
        if (new LongRange(-2147483647L, 2147483647L).contains(value)) {
            return DurationKt.durationOfNanos(result);
        }
        if (result / ((long) scale) == value) {
            return DurationKt.durationOfNanosNormalized(result);
        }
        long millis = DurationKt.nanosToMillis(value);
        long remNanos = value - DurationKt.millisToNanos(millis);
        long resultMillis = ((long) scale) * millis;
        long totalMillis = DurationKt.nanosToMillis(((long) scale) * remNanos) + resultMillis;
        if (resultMillis / ((long) scale) != millis || (totalMillis ^ resultMillis) < 0) {
            return MathKt.getSign(value) * MathKt.getSign(scale) > 0 ? INFINITE : NEG_INFINITE;
        }
        return DurationKt.durationOfMillis(RangesKt.coerceIn(totalMillis, new LongRange(-4611686018427387903L, DurationKt.MAX_MILLIS)));
    }

    /* JADX INFO: renamed from: times-UwyO8pc, reason: not valid java name */
    public static final long m1450timesUwyO8pc(long arg0, double scale) {
        int intScale = MathKt.roundToInt(scale);
        if (((double) intScale) == scale) {
            return m1451timesUwyO8pc(arg0, intScale);
        }
        DurationUnit unit = m1438getStorageUnitimpl(arg0);
        double result = m1456toDoubleimpl(arg0, unit) * scale;
        return DurationKt.toDuration(result, unit);
    }

    /* JADX INFO: renamed from: div-UwyO8pc, reason: not valid java name */
    public static final long m1416divUwyO8pc(long arg0, int scale) {
        if (scale == 0) {
            if (m1447isPositiveimpl(arg0)) {
                return INFINITE;
            }
            if (m1446isNegativeimpl(arg0)) {
                return NEG_INFINITE;
            }
            throw new IllegalArgumentException("Dividing zero duration by zero yields an undefined result.");
        }
        if (m1444isInNanosimpl(arg0)) {
            return DurationKt.durationOfNanos(m1440getValueimpl(arg0) / ((long) scale));
        }
        if (m1445isInfiniteimpl(arg0)) {
            return m1451timesUwyO8pc(arg0, MathKt.getSign(scale));
        }
        long result = m1440getValueimpl(arg0) / ((long) scale);
        if (new LongRange(-4611686018426L, 4611686018426L).contains(result)) {
            long rem = DurationKt.millisToNanos(m1440getValueimpl(arg0) - (((long) scale) * result)) / ((long) scale);
            return DurationKt.durationOfNanos(DurationKt.millisToNanos(result) + rem);
        }
        long rem2 = DurationKt.durationOfMillis(result);
        return rem2;
    }

    /* JADX INFO: renamed from: div-UwyO8pc, reason: not valid java name */
    public static final long m1415divUwyO8pc(long arg0, double scale) {
        int intScale = MathKt.roundToInt(scale);
        if ((((double) intScale) == scale) && intScale != 0) {
            return m1416divUwyO8pc(arg0, intScale);
        }
        DurationUnit unit = m1438getStorageUnitimpl(arg0);
        double result = m1456toDoubleimpl(arg0, unit) / scale;
        return DurationKt.toDuration(result, unit);
    }

    /* JADX INFO: renamed from: div-LRDsOJo, reason: not valid java name */
    public static final double m1414divLRDsOJo(long arg0, long other) {
        DurationUnit coarserUnit = (DurationUnit) ComparisonsKt.maxOf(m1438getStorageUnitimpl(arg0), m1438getStorageUnitimpl(other));
        return m1456toDoubleimpl(arg0, coarserUnit) / m1456toDoubleimpl(other, coarserUnit);
    }

    /* JADX INFO: renamed from: isNegative-impl, reason: not valid java name */
    public static final boolean m1446isNegativeimpl(long arg0) {
        return arg0 < 0;
    }

    /* JADX INFO: renamed from: isPositive-impl, reason: not valid java name */
    public static final boolean m1447isPositiveimpl(long arg0) {
        return arg0 > 0;
    }

    /* JADX INFO: renamed from: isInfinite-impl, reason: not valid java name */
    public static final boolean m1445isInfiniteimpl(long arg0) {
        return arg0 == INFINITE || arg0 == NEG_INFINITE;
    }

    /* JADX INFO: renamed from: isFinite-impl, reason: not valid java name */
    public static final boolean m1442isFiniteimpl(long arg0) {
        return !m1445isInfiniteimpl(arg0);
    }

    /* JADX INFO: renamed from: getAbsoluteValue-UwyO8pc, reason: not valid java name */
    public static final long m1419getAbsoluteValueUwyO8pc(long arg0) {
        return m1446isNegativeimpl(arg0) ? m1465unaryMinusUwyO8pc(arg0) : arg0;
    }

    /* JADX INFO: renamed from: compareTo-LRDsOJo, reason: not valid java name */
    public int m1466compareToLRDsOJo(long other) {
        return m1412compareToLRDsOJo(this.rawValue, other);
    }

    /* JADX INFO: renamed from: compareTo-LRDsOJo, reason: not valid java name */
    public static int m1412compareToLRDsOJo(long arg0, long other) {
        long compareBits = arg0 ^ other;
        if (compareBits < 0 || (((int) compareBits) & 1) == 0) {
            return Intrinsics.compare(arg0, other);
        }
        int r = (((int) arg0) & 1) - (((int) other) & 1);
        return m1446isNegativeimpl(arg0) ? -r : r;
    }

    /* JADX INFO: renamed from: toComponents-impl, reason: not valid java name */
    public static final <T> T m1455toComponentsimpl(long arg0, Function5<? super Long, ? super Integer, ? super Integer, ? super Integer, ? super Integer, ? extends T> action) {
        Intrinsics.checkNotNullParameter(action, "action");
        return action.invoke(Long.valueOf(m1428getInWholeDaysimpl(arg0)), Integer.valueOf(m1420getHoursComponentimpl(arg0)), Integer.valueOf(m1435getMinutesComponentimpl(arg0)), Integer.valueOf(m1437getSecondsComponentimpl(arg0)), Integer.valueOf(m1436getNanosecondsComponentimpl(arg0)));
    }

    /* JADX INFO: renamed from: toComponents-impl, reason: not valid java name */
    public static final <T> T m1454toComponentsimpl(long arg0, Function4<? super Long, ? super Integer, ? super Integer, ? super Integer, ? extends T> action) {
        Intrinsics.checkNotNullParameter(action, "action");
        return action.invoke(Long.valueOf(m1429getInWholeHoursimpl(arg0)), Integer.valueOf(m1435getMinutesComponentimpl(arg0)), Integer.valueOf(m1437getSecondsComponentimpl(arg0)), Integer.valueOf(m1436getNanosecondsComponentimpl(arg0)));
    }

    /* JADX INFO: renamed from: toComponents-impl, reason: not valid java name */
    public static final <T> T m1453toComponentsimpl(long arg0, Function3<? super Long, ? super Integer, ? super Integer, ? extends T> action) {
        Intrinsics.checkNotNullParameter(action, "action");
        return action.invoke(Long.valueOf(m1432getInWholeMinutesimpl(arg0)), Integer.valueOf(m1437getSecondsComponentimpl(arg0)), Integer.valueOf(m1436getNanosecondsComponentimpl(arg0)));
    }

    /* JADX INFO: renamed from: toComponents-impl, reason: not valid java name */
    public static final <T> T m1452toComponentsimpl(long arg0, Function2<? super Long, ? super Integer, ? extends T> action) {
        Intrinsics.checkNotNullParameter(action, "action");
        return action.invoke(Long.valueOf(m1434getInWholeSecondsimpl(arg0)), Integer.valueOf(m1436getNanosecondsComponentimpl(arg0)));
    }

    /* JADX INFO: renamed from: getHoursComponent-impl, reason: not valid java name */
    public static final int m1420getHoursComponentimpl(long arg0) {
        if (m1445isInfiniteimpl(arg0)) {
            return 0;
        }
        return (int) (m1429getInWholeHoursimpl(arg0) % ((long) 24));
    }

    /* JADX INFO: renamed from: getMinutesComponent-impl, reason: not valid java name */
    public static final int m1435getMinutesComponentimpl(long arg0) {
        if (m1445isInfiniteimpl(arg0)) {
            return 0;
        }
        return (int) (m1432getInWholeMinutesimpl(arg0) % ((long) 60));
    }

    /* JADX INFO: renamed from: getSecondsComponent-impl, reason: not valid java name */
    public static final int m1437getSecondsComponentimpl(long arg0) {
        if (m1445isInfiniteimpl(arg0)) {
            return 0;
        }
        return (int) (m1434getInWholeSecondsimpl(arg0) % ((long) 60));
    }

    /* JADX INFO: renamed from: getNanosecondsComponent-impl, reason: not valid java name */
    public static final int m1436getNanosecondsComponentimpl(long arg0) {
        if (m1445isInfiniteimpl(arg0)) {
            return 0;
        }
        if (m1443isInMillisimpl(arg0)) {
            return (int) DurationKt.millisToNanos(m1440getValueimpl(arg0) % ((long) 1000));
        }
        return (int) (m1440getValueimpl(arg0) % ((long) 1000000000));
    }

    /* JADX INFO: renamed from: toDouble-impl, reason: not valid java name */
    public static final double m1456toDoubleimpl(long arg0, DurationUnit unit) {
        Intrinsics.checkNotNullParameter(unit, "unit");
        if (arg0 == INFINITE) {
            return Double.POSITIVE_INFINITY;
        }
        if (arg0 == NEG_INFINITE) {
            return Double.NEGATIVE_INFINITY;
        }
        return DurationUnitKt.convertDurationUnit(m1440getValueimpl(arg0), m1438getStorageUnitimpl(arg0), unit);
    }

    /* JADX INFO: renamed from: toLong-impl, reason: not valid java name */
    public static final long m1459toLongimpl(long arg0, DurationUnit unit) {
        Intrinsics.checkNotNullParameter(unit, "unit");
        if (arg0 == INFINITE) {
            return Long.MAX_VALUE;
        }
        if (arg0 == NEG_INFINITE) {
            return Long.MIN_VALUE;
        }
        return DurationUnitKt.convertDurationUnit(m1440getValueimpl(arg0), m1438getStorageUnitimpl(arg0), unit);
    }

    /* JADX INFO: renamed from: toInt-impl, reason: not valid java name */
    public static final int m1457toIntimpl(long arg0, DurationUnit unit) {
        Intrinsics.checkNotNullParameter(unit, "unit");
        return (int) RangesKt.coerceIn(m1459toLongimpl(arg0, unit), -2147483648L, 2147483647L);
    }

    /* JADX INFO: renamed from: getInDays-impl, reason: not valid java name */
    public static final double m1421getInDaysimpl(long arg0) {
        return m1456toDoubleimpl(arg0, DurationUnit.DAYS);
    }

    /* JADX INFO: renamed from: getInHours-impl, reason: not valid java name */
    public static final double m1422getInHoursimpl(long arg0) {
        return m1456toDoubleimpl(arg0, DurationUnit.HOURS);
    }

    /* JADX INFO: renamed from: getInMinutes-impl, reason: not valid java name */
    public static final double m1425getInMinutesimpl(long arg0) {
        return m1456toDoubleimpl(arg0, DurationUnit.MINUTES);
    }

    /* JADX INFO: renamed from: getInSeconds-impl, reason: not valid java name */
    public static final double m1427getInSecondsimpl(long arg0) {
        return m1456toDoubleimpl(arg0, DurationUnit.SECONDS);
    }

    /* JADX INFO: renamed from: getInMilliseconds-impl, reason: not valid java name */
    public static final double m1424getInMillisecondsimpl(long arg0) {
        return m1456toDoubleimpl(arg0, DurationUnit.MILLISECONDS);
    }

    /* JADX INFO: renamed from: getInMicroseconds-impl, reason: not valid java name */
    public static final double m1423getInMicrosecondsimpl(long arg0) {
        return m1456toDoubleimpl(arg0, DurationUnit.MICROSECONDS);
    }

    /* JADX INFO: renamed from: getInNanoseconds-impl, reason: not valid java name */
    public static final double m1426getInNanosecondsimpl(long arg0) {
        return m1456toDoubleimpl(arg0, DurationUnit.NANOSECONDS);
    }

    /* JADX INFO: renamed from: getInWholeDays-impl, reason: not valid java name */
    public static final long m1428getInWholeDaysimpl(long arg0) {
        return m1459toLongimpl(arg0, DurationUnit.DAYS);
    }

    /* JADX INFO: renamed from: getInWholeHours-impl, reason: not valid java name */
    public static final long m1429getInWholeHoursimpl(long arg0) {
        return m1459toLongimpl(arg0, DurationUnit.HOURS);
    }

    /* JADX INFO: renamed from: getInWholeMinutes-impl, reason: not valid java name */
    public static final long m1432getInWholeMinutesimpl(long arg0) {
        return m1459toLongimpl(arg0, DurationUnit.MINUTES);
    }

    /* JADX INFO: renamed from: getInWholeSeconds-impl, reason: not valid java name */
    public static final long m1434getInWholeSecondsimpl(long arg0) {
        return m1459toLongimpl(arg0, DurationUnit.SECONDS);
    }

    /* JADX INFO: renamed from: getInWholeMilliseconds-impl, reason: not valid java name */
    public static final long m1431getInWholeMillisecondsimpl(long arg0) {
        return (m1443isInMillisimpl(arg0) && m1442isFiniteimpl(arg0)) ? m1440getValueimpl(arg0) : m1459toLongimpl(arg0, DurationUnit.MILLISECONDS);
    }

    /* JADX INFO: renamed from: getInWholeMicroseconds-impl, reason: not valid java name */
    public static final long m1430getInWholeMicrosecondsimpl(long arg0) {
        return m1459toLongimpl(arg0, DurationUnit.MICROSECONDS);
    }

    /* JADX INFO: renamed from: getInWholeNanoseconds-impl, reason: not valid java name */
    public static final long m1433getInWholeNanosecondsimpl(long arg0) {
        long value = m1440getValueimpl(arg0);
        if (m1444isInNanosimpl(arg0)) {
            return value;
        }
        if (value > 9223372036854L) {
            return Long.MAX_VALUE;
        }
        if (value < -9223372036854L) {
            return Long.MIN_VALUE;
        }
        return DurationKt.millisToNanos(value);
    }

    @Deprecated(message = "Use inWholeNanoseconds property instead.", replaceWith = @ReplaceWith(expression = "this.inWholeNanoseconds", imports = {}))
    /* JADX INFO: renamed from: toLongNanoseconds-impl, reason: not valid java name */
    public static final long m1461toLongNanosecondsimpl(long arg0) {
        return m1433getInWholeNanosecondsimpl(arg0);
    }

    @Deprecated(message = "Use inWholeMilliseconds property instead.", replaceWith = @ReplaceWith(expression = "this.inWholeMilliseconds", imports = {}))
    /* JADX INFO: renamed from: toLongMilliseconds-impl, reason: not valid java name */
    public static final long m1460toLongMillisecondsimpl(long arg0) {
        return m1431getInWholeMillisecondsimpl(arg0);
    }

    /* JADX INFO: renamed from: toString-impl, reason: not valid java name */
    public static String m1462toStringimpl(long arg0) {
        if (arg0 == 0) {
            return "0s";
        }
        if (arg0 == INFINITE) {
            return "Infinity";
        }
        if (arg0 == NEG_INFINITE) {
            return "-Infinity";
        }
        boolean isNegative = m1446isNegativeimpl(arg0);
        StringBuilder $this$toString_impl_u24lambda_u2d5 = new StringBuilder();
        if (isNegative) {
            $this$toString_impl_u24lambda_u2d5.append('-');
        }
        long arg0$iv = m1419getAbsoluteValueUwyO8pc(arg0);
        long days = m1428getInWholeDaysimpl(arg0$iv);
        int hours = m1420getHoursComponentimpl(arg0$iv);
        int minutes = m1435getMinutesComponentimpl(arg0$iv);
        int seconds = m1437getSecondsComponentimpl(arg0$iv);
        int nanoseconds = m1436getNanosecondsComponentimpl(arg0$iv);
        boolean hasDays = days != 0;
        boolean hasHours = hours != 0;
        boolean hasMinutes = minutes != 0;
        boolean hasSeconds = (seconds == 0 && nanoseconds == 0) ? false : true;
        int components = 0;
        if (hasDays) {
            $this$toString_impl_u24lambda_u2d5.append(days).append('d');
            components = 0 + 1;
        }
        if (hasHours || (hasDays && (hasMinutes || hasSeconds))) {
            int components2 = components + 1;
            if (components > 0) {
                $this$toString_impl_u24lambda_u2d5.append(' ');
            }
            $this$toString_impl_u24lambda_u2d5.append(hours).append('h');
            components = components2;
        }
        if (hasMinutes || (hasSeconds && (hasHours || hasDays))) {
            int components3 = components + 1;
            if (components > 0) {
                $this$toString_impl_u24lambda_u2d5.append(' ');
            }
            $this$toString_impl_u24lambda_u2d5.append(minutes).append('m');
            components = components3;
        }
        if (hasSeconds) {
            int components4 = components + 1;
            if (components > 0) {
                $this$toString_impl_u24lambda_u2d5.append(' ');
            }
            if (seconds != 0 || hasDays || hasHours || hasMinutes) {
                int nanoseconds2 = nanoseconds;
                m1410appendFractionalimpl(arg0, $this$toString_impl_u24lambda_u2d5, seconds, nanoseconds2, 9, "s", false);
                components = components4;
            } else {
                if (nanoseconds >= 1000000) {
                    m1410appendFractionalimpl(arg0, $this$toString_impl_u24lambda_u2d5, nanoseconds / DurationKt.NANOS_IN_MILLIS, nanoseconds % DurationKt.NANOS_IN_MILLIS, 6, "ms", false);
                } else if (nanoseconds >= 1000) {
                    m1410appendFractionalimpl(arg0, $this$toString_impl_u24lambda_u2d5, nanoseconds / 1000, nanoseconds % 1000, 3, "us", false);
                } else {
                    $this$toString_impl_u24lambda_u2d5.append(nanoseconds).append("ns");
                }
                components = components4;
            }
        }
        if (isNegative && components > 1) {
            $this$toString_impl_u24lambda_u2d5.insert(1, '(').append(')');
        }
        String string = $this$toString_impl_u24lambda_u2d5.toString();
        Intrinsics.checkNotNullExpressionValue(string, "StringBuilder().apply(builderAction).toString()");
        return string;
    }

    public String toString() {
        return m1462toStringimpl(this.rawValue);
    }

    /* JADX INFO: renamed from: appendFractional-impl, reason: not valid java name */
    private static final void m1410appendFractionalimpl(long arg0, StringBuilder $this$appendFractional, int whole, int fractional, int fractionalSize, String unit, boolean isoZeroes) {
        $this$appendFractional.append(whole);
        if (fractional != 0) {
            $this$appendFractional.append('.');
            CharSequence fracString = StringsKt.padStart(String.valueOf(fractional), fractionalSize, '0');
            CharSequence $this$indexOfLast$iv = fracString;
            int i = -1;
            int length = $this$indexOfLast$iv.length() - 1;
            if (length >= 0) {
                while (true) {
                    int index$iv = length;
                    length--;
                    char it = $this$indexOfLast$iv.charAt(index$iv);
                    char it2 = it != '0' ? (char) 1 : (char) 0;
                    if (it2 == 0) {
                        if (length < 0) {
                            break;
                        }
                    } else {
                        i = index$iv;
                        break;
                    }
                }
            }
            int nonZeroDigits = i + 1;
            if (isoZeroes || nonZeroDigits >= 3) {
                Intrinsics.checkNotNullExpressionValue($this$appendFractional.append(fracString, 0, ((nonZeroDigits + 2) / 3) * 3), "this.append(value, startIndex, endIndex)");
            } else {
                Intrinsics.checkNotNullExpressionValue($this$appendFractional.append(fracString, 0, nonZeroDigits), "this.append(value, startIndex, endIndex)");
            }
        }
        $this$appendFractional.append(unit);
    }

    /* JADX INFO: renamed from: toString-impl$default, reason: not valid java name */
    public static /* synthetic */ String m1464toStringimpl$default(long j, DurationUnit durationUnit, int i, int i2, Object obj) {
        if ((i2 & 2) != 0) {
            i = 0;
        }
        return m1463toStringimpl(j, durationUnit, i);
    }

    /* JADX INFO: renamed from: toString-impl, reason: not valid java name */
    public static final String m1463toStringimpl(long arg0, DurationUnit unit, int decimals) {
        Intrinsics.checkNotNullParameter(unit, "unit");
        if (!(decimals >= 0)) {
            throw new IllegalArgumentException(("decimals must be not negative, but was " + decimals).toString());
        }
        double number = m1456toDoubleimpl(arg0, unit);
        return Double.isInfinite(number) ? String.valueOf(number) : DurationJvmKt.formatToExactDecimals(number, RangesKt.coerceAtMost(decimals, 12)) + DurationUnitKt.shortName(unit);
    }

    /* JADX INFO: renamed from: toIsoString-impl, reason: not valid java name */
    public static final String m1458toIsoStringimpl(long arg0) {
        long hours;
        StringBuilder $this$toIsoString_impl_u24lambda_u2d9 = new StringBuilder();
        if (m1446isNegativeimpl(arg0)) {
            $this$toIsoString_impl_u24lambda_u2d9.append('-');
        }
        $this$toIsoString_impl_u24lambda_u2d9.append("PT");
        long arg0$iv = m1419getAbsoluteValueUwyO8pc(arg0);
        long hours2 = m1429getInWholeHoursimpl(arg0$iv);
        int minutes = m1435getMinutesComponentimpl(arg0$iv);
        int seconds = m1437getSecondsComponentimpl(arg0$iv);
        int nanoseconds = m1436getNanosecondsComponentimpl(arg0$iv);
        if (!m1445isInfiniteimpl(arg0)) {
            hours = hours2;
        } else {
            hours = 9999999999999L;
        }
        boolean z = true;
        boolean hasHours = hours != 0;
        boolean hasSeconds = (seconds == 0 && nanoseconds == 0) ? false : true;
        if (minutes == 0 && (!hasSeconds || !hasHours)) {
            z = false;
        }
        boolean hasMinutes = z;
        if (hasHours) {
            $this$toIsoString_impl_u24lambda_u2d9.append(hours).append('H');
        }
        if (hasMinutes) {
            $this$toIsoString_impl_u24lambda_u2d9.append(minutes).append('M');
        }
        if (hasSeconds || (!hasHours && !hasMinutes)) {
            m1410appendFractionalimpl(arg0, $this$toIsoString_impl_u24lambda_u2d9, seconds, nanoseconds, 9, "S", true);
        }
        String string = $this$toIsoString_impl_u24lambda_u2d9.toString();
        Intrinsics.checkNotNullExpressionValue(string, "StringBuilder().apply(builderAction).toString()");
        return string;
    }
}
