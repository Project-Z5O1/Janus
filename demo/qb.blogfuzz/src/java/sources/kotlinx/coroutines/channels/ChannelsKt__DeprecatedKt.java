package kotlinx.coroutines.channels;

import androidx.constraintlayout.core.motion.utils.TypedValues;
import androidx.constraintlayout.widget.ConstraintLayout;
import java.util.Collection;
import java.util.Comparator;
import java.util.HashSet;
import java.util.LinkedHashSet;
import java.util.Map;
import java.util.NoSuchElementException;
import java.util.Set;
import kotlin.Deprecated;
import kotlin.DeprecationLevel;
import kotlin.ExceptionsKt;
import kotlin.Metadata;
import kotlin.Pair;
import kotlin.ResultKt;
import kotlin.Unit;
import kotlin.collections.IndexedValue;
import kotlin.coroutines.Continuation;
import kotlin.coroutines.CoroutineContext;
import kotlin.coroutines.EmptyCoroutineContext;
import kotlin.coroutines.intrinsics.IntrinsicsKt;
import kotlin.coroutines.jvm.internal.Boxing;
import kotlin.coroutines.jvm.internal.ContinuationImpl;
import kotlin.coroutines.jvm.internal.DebugMetadata;
import kotlin.coroutines.jvm.internal.SuspendLambda;
import kotlin.jvm.functions.Function1;
import kotlin.jvm.functions.Function2;
import kotlin.jvm.functions.Function3;
import kotlin.jvm.internal.Intrinsics;
import kotlin.jvm.internal.Ref;
import kotlinx.coroutines.CoroutineStart;
import kotlinx.coroutines.Dispatchers;
import kotlinx.coroutines.GlobalScope;

/* JADX INFO: compiled from: Deprecated.kt */
/* JADX INFO: loaded from: classes.dex */
@Metadata(d1 = {"\u0000 \u0001\n\u0000\n\u0002\u0018\u0002\n\u0002\u0010\u0003\n\u0002\u0018\u0002\n\u0002\b\u0002\n\u0002\u0010\u0002\n\u0002\u0018\u0002\n\u0000\n\u0002\u0010\u0011\n\u0002\u0018\u0002\n\u0002\b\u0002\n\u0002\u0010\u000b\n\u0002\b\u0004\n\u0002\u0010\b\n\u0002\b\u0004\n\u0002\u0018\u0002\n\u0000\n\u0002\u0018\u0002\n\u0002\u0018\u0002\n\u0002\u0010\u0000\n\u0002\b\u000b\n\u0002\u0018\u0002\n\u0002\b\u0005\n\u0002\u0010\u001f\n\u0002\b\u0002\n\u0002\u0018\u0002\n\u0002\b\u0012\n\u0002\u0018\u0002\n\u0002\u0018\u0002\n\u0002\b\u000b\n\u0002\u0010$\n\u0000\n\u0002\u0018\u0002\n\u0000\n\u0002\u0010%\n\u0002\b\u0002\n\u0002\u0010!\n\u0000\n\u0002\u0010#\n\u0000\n\u0002\u0010\"\n\u0000\n\u0002\u0018\u0002\n\u0002\b\u0005\u001aJ\u0010\u0000\u001a#\u0012\u0015\u0012\u0013\u0018\u00010\u0002¢\u0006\f\b\u0003\u0012\b\b\u0004\u0012\u0004\b\b(\u0005\u0012\u0004\u0012\u00020\u00060\u0001j\u0002`\u00072\u001a\u0010\b\u001a\u000e\u0012\n\b\u0001\u0012\u0006\u0012\u0002\b\u00030\n0\t\"\u0006\u0012\u0002\b\u00030\nH\u0001¢\u0006\u0002\u0010\u000b\u001a!\u0010\f\u001a\u00020\r\"\u0004\b\u0000\u0010\u000e*\b\u0012\u0004\u0012\u0002H\u000e0\nH\u0087@ø\u0001\u0000¢\u0006\u0002\u0010\u000f\u001a1\u0010\u0010\u001a#\u0012\u0015\u0012\u0013\u0018\u00010\u0002¢\u0006\f\b\u0003\u0012\b\b\u0004\u0012\u0004\b\b(\u0005\u0012\u0004\u0012\u00020\u00060\u0001j\u0002`\u0007*\u0006\u0012\u0002\b\u00030\nH\u0001\u001a!\u0010\u0011\u001a\u00020\u0012\"\u0004\b\u0000\u0010\u000e*\b\u0012\u0004\u0012\u0002H\u000e0\nH\u0087@ø\u0001\u0000¢\u0006\u0002\u0010\u000f\u001a\u001e\u0010\u0013\u001a\b\u0012\u0004\u0012\u0002H\u000e0\n\"\u0004\b\u0000\u0010\u000e*\b\u0012\u0004\u0012\u0002H\u000e0\nH\u0007\u001aZ\u0010\u0014\u001a\b\u0012\u0004\u0012\u0002H\u000e0\n\"\u0004\b\u0000\u0010\u000e\"\u0004\b\u0001\u0010\u0015*\b\u0012\u0004\u0012\u0002H\u000e0\n2\b\b\u0002\u0010\u0016\u001a\u00020\u00172\"\u0010\u0018\u001a\u001e\b\u0001\u0012\u0004\u0012\u0002H\u000e\u0012\n\u0012\b\u0012\u0004\u0012\u0002H\u00150\u001a\u0012\u0006\u0012\u0004\u0018\u00010\u001b0\u0019H\u0001ø\u0001\u0000¢\u0006\u0002\u0010\u001c\u001a0\u0010\u001d\u001a\b\u0012\u0004\u0012\u0002H\u000e0\n\"\u0004\b\u0000\u0010\u000e*\b\u0012\u0004\u0012\u0002H\u000e0\n2\u0006\u0010\u001e\u001a\u00020\u00122\b\b\u0002\u0010\u0016\u001a\u00020\u0017H\u0007\u001aT\u0010\u001f\u001a\b\u0012\u0004\u0012\u0002H\u000e0\n\"\u0004\b\u0000\u0010\u000e*\b\u0012\u0004\u0012\u0002H\u000e0\n2\b\b\u0002\u0010\u0016\u001a\u00020\u00172\"\u0010 \u001a\u001e\b\u0001\u0012\u0004\u0012\u0002H\u000e\u0012\n\u0012\b\u0012\u0004\u0012\u00020\r0\u001a\u0012\u0006\u0012\u0004\u0018\u00010\u001b0\u0019H\u0007ø\u0001\u0000¢\u0006\u0002\u0010\u001c\u001a)\u0010!\u001a\u0002H\u000e\"\u0004\b\u0000\u0010\u000e*\b\u0012\u0004\u0012\u0002H\u000e0\n2\u0006\u0010\"\u001a\u00020\u0012H\u0087@ø\u0001\u0000¢\u0006\u0002\u0010#\u001a+\u0010$\u001a\u0004\u0018\u0001H\u000e\"\u0004\b\u0000\u0010\u000e*\b\u0012\u0004\u0012\u0002H\u000e0\n2\u0006\u0010\"\u001a\u00020\u0012H\u0087@ø\u0001\u0000¢\u0006\u0002\u0010#\u001aT\u0010%\u001a\b\u0012\u0004\u0012\u0002H\u000e0\n\"\u0004\b\u0000\u0010\u000e*\b\u0012\u0004\u0012\u0002H\u000e0\n2\b\b\u0002\u0010\u0016\u001a\u00020\u00172\"\u0010 \u001a\u001e\b\u0001\u0012\u0004\u0012\u0002H\u000e\u0012\n\u0012\b\u0012\u0004\u0012\u00020\r0\u001a\u0012\u0006\u0012\u0004\u0018\u00010\u001b0\u0019H\u0001ø\u0001\u0000¢\u0006\u0002\u0010\u001c\u001ai\u0010&\u001a\b\u0012\u0004\u0012\u0002H\u000e0\n\"\u0004\b\u0000\u0010\u000e*\b\u0012\u0004\u0012\u0002H\u000e0\n2\b\b\u0002\u0010\u0016\u001a\u00020\u001727\u0010 \u001a3\b\u0001\u0012\u0013\u0012\u00110\u0012¢\u0006\f\b\u0003\u0012\b\b\u0004\u0012\u0004\b\b(\"\u0012\u0004\u0012\u0002H\u000e\u0012\n\u0012\b\u0012\u0004\u0012\u00020\r0\u001a\u0012\u0006\u0012\u0004\u0018\u00010\u001b0'H\u0007ø\u0001\u0000¢\u0006\u0002\u0010(\u001aT\u0010)\u001a\b\u0012\u0004\u0012\u0002H\u000e0\n\"\u0004\b\u0000\u0010\u000e*\b\u0012\u0004\u0012\u0002H\u000e0\n2\b\b\u0002\u0010\u0016\u001a\u00020\u00172\"\u0010 \u001a\u001e\b\u0001\u0012\u0004\u0012\u0002H\u000e\u0012\n\u0012\b\u0012\u0004\u0012\u00020\r0\u001a\u0012\u0006\u0012\u0004\u0018\u00010\u001b0\u0019H\u0007ø\u0001\u0000¢\u0006\u0002\u0010\u001c\u001a$\u0010*\u001a\b\u0012\u0004\u0012\u0002H\u000e0\n\"\b\b\u0000\u0010\u000e*\u00020\u001b*\n\u0012\u0006\u0012\u0004\u0018\u0001H\u000e0\nH\u0001\u001aA\u0010+\u001a\u0002H,\"\b\b\u0000\u0010\u000e*\u00020\u001b\"\u0010\b\u0001\u0010,*\n\u0012\u0006\b\u0000\u0012\u0002H\u000e0-*\n\u0012\u0006\u0012\u0004\u0018\u0001H\u000e0\n2\u0006\u0010.\u001a\u0002H,H\u0087@ø\u0001\u0000¢\u0006\u0002\u0010/\u001a?\u0010+\u001a\u0002H,\"\b\b\u0000\u0010\u000e*\u00020\u001b\"\u000e\b\u0001\u0010,*\b\u0012\u0004\u0012\u0002H\u000e00*\n\u0012\u0006\u0012\u0004\u0018\u0001H\u000e0\n2\u0006\u0010.\u001a\u0002H,H\u0087@ø\u0001\u0000¢\u0006\u0002\u00101\u001a!\u00102\u001a\u0002H\u000e\"\u0004\b\u0000\u0010\u000e*\b\u0012\u0004\u0012\u0002H\u000e0\nH\u0087@ø\u0001\u0000¢\u0006\u0002\u0010\u000f\u001a#\u00103\u001a\u0004\u0018\u0001H\u000e\"\u0004\b\u0000\u0010\u000e*\b\u0012\u0004\u0012\u0002H\u000e0\nH\u0087@ø\u0001\u0000¢\u0006\u0002\u0010\u000f\u001a`\u00104\u001a\b\u0012\u0004\u0012\u0002H50\n\"\u0004\b\u0000\u0010\u000e\"\u0004\b\u0001\u00105*\b\u0012\u0004\u0012\u0002H\u000e0\n2\b\b\u0002\u0010\u0016\u001a\u00020\u00172(\u00106\u001a$\b\u0001\u0012\u0004\u0012\u0002H\u000e\u0012\u0010\u0012\u000e\u0012\n\u0012\b\u0012\u0004\u0012\u0002H50\n0\u001a\u0012\u0006\u0012\u0004\u0018\u00010\u001b0\u0019H\u0007ø\u0001\u0000¢\u0006\u0002\u0010\u001c\u001a)\u00107\u001a\u00020\u0012\"\u0004\b\u0000\u0010\u000e*\b\u0012\u0004\u0012\u0002H\u000e0\n2\u0006\u00108\u001a\u0002H\u000eH\u0087@ø\u0001\u0000¢\u0006\u0002\u00109\u001a!\u0010:\u001a\u0002H\u000e\"\u0004\b\u0000\u0010\u000e*\b\u0012\u0004\u0012\u0002H\u000e0\nH\u0087@ø\u0001\u0000¢\u0006\u0002\u0010\u000f\u001a)\u0010;\u001a\u00020\u0012\"\u0004\b\u0000\u0010\u000e*\b\u0012\u0004\u0012\u0002H\u000e0\n2\u0006\u00108\u001a\u0002H\u000eH\u0087@ø\u0001\u0000¢\u0006\u0002\u00109\u001a#\u0010<\u001a\u0004\u0018\u0001H\u000e\"\u0004\b\u0000\u0010\u000e*\b\u0012\u0004\u0012\u0002H\u000e0\nH\u0087@ø\u0001\u0000¢\u0006\u0002\u0010\u000f\u001aZ\u0010=\u001a\b\u0012\u0004\u0012\u0002H50\n\"\u0004\b\u0000\u0010\u000e\"\u0004\b\u0001\u00105*\b\u0012\u0004\u0012\u0002H\u000e0\n2\b\b\u0002\u0010\u0016\u001a\u00020\u00172\"\u00106\u001a\u001e\b\u0001\u0012\u0004\u0012\u0002H\u000e\u0012\n\u0012\b\u0012\u0004\u0012\u0002H50\u001a\u0012\u0006\u0012\u0004\u0018\u00010\u001b0\u0019H\u0001ø\u0001\u0000¢\u0006\u0002\u0010\u001c\u001ao\u0010>\u001a\b\u0012\u0004\u0012\u0002H50\n\"\u0004\b\u0000\u0010\u000e\"\u0004\b\u0001\u00105*\b\u0012\u0004\u0012\u0002H\u000e0\n2\b\b\u0002\u0010\u0016\u001a\u00020\u001727\u00106\u001a3\b\u0001\u0012\u0013\u0012\u00110\u0012¢\u0006\f\b\u0003\u0012\b\b\u0004\u0012\u0004\b\b(\"\u0012\u0004\u0012\u0002H\u000e\u0012\n\u0012\b\u0012\u0004\u0012\u0002H50\u001a\u0012\u0006\u0012\u0004\u0018\u00010\u001b0'H\u0001ø\u0001\u0000¢\u0006\u0002\u0010(\u001au\u0010?\u001a\b\u0012\u0004\u0012\u0002H50\n\"\u0004\b\u0000\u0010\u000e\"\b\b\u0001\u00105*\u00020\u001b*\b\u0012\u0004\u0012\u0002H\u000e0\n2\b\b\u0002\u0010\u0016\u001a\u00020\u001729\u00106\u001a5\b\u0001\u0012\u0013\u0012\u00110\u0012¢\u0006\f\b\u0003\u0012\b\b\u0004\u0012\u0004\b\b(\"\u0012\u0004\u0012\u0002H\u000e\u0012\f\u0012\n\u0012\u0006\u0012\u0004\u0018\u0001H50\u001a\u0012\u0006\u0012\u0004\u0018\u00010\u001b0'H\u0007ø\u0001\u0000¢\u0006\u0002\u0010(\u001a`\u0010@\u001a\b\u0012\u0004\u0012\u0002H50\n\"\u0004\b\u0000\u0010\u000e\"\b\b\u0001\u00105*\u00020\u001b*\b\u0012\u0004\u0012\u0002H\u000e0\n2\b\b\u0002\u0010\u0016\u001a\u00020\u00172$\u00106\u001a \b\u0001\u0012\u0004\u0012\u0002H\u000e\u0012\f\u0012\n\u0012\u0006\u0012\u0004\u0018\u0001H50\u001a\u0012\u0006\u0012\u0004\u0018\u00010\u001b0\u0019H\u0007ø\u0001\u0000¢\u0006\u0002\u0010\u001c\u001a?\u0010A\u001a\u0004\u0018\u0001H\u000e\"\u0004\b\u0000\u0010\u000e*\b\u0012\u0004\u0012\u0002H\u000e0\n2\u001a\u0010B\u001a\u0016\u0012\u0006\b\u0000\u0012\u0002H\u000e0Cj\n\u0012\u0006\b\u0000\u0012\u0002H\u000e`DH\u0087@ø\u0001\u0000¢\u0006\u0002\u0010E\u001a?\u0010F\u001a\u0004\u0018\u0001H\u000e\"\u0004\b\u0000\u0010\u000e*\b\u0012\u0004\u0012\u0002H\u000e0\n2\u001a\u0010B\u001a\u0016\u0012\u0006\b\u0000\u0012\u0002H\u000e0Cj\n\u0012\u0006\b\u0000\u0012\u0002H\u000e`DH\u0087@ø\u0001\u0000¢\u0006\u0002\u0010E\u001a!\u0010G\u001a\u00020\r\"\u0004\b\u0000\u0010\u000e*\b\u0012\u0004\u0012\u0002H\u000e0\nH\u0087@ø\u0001\u0000¢\u0006\u0002\u0010\u000f\u001a$\u0010H\u001a\b\u0012\u0004\u0012\u0002H\u000e0\n\"\b\b\u0000\u0010\u000e*\u00020\u001b*\n\u0012\u0006\u0012\u0004\u0018\u0001H\u000e0\nH\u0007\u001a!\u0010I\u001a\u0002H\u000e\"\u0004\b\u0000\u0010\u000e*\b\u0012\u0004\u0012\u0002H\u000e0\nH\u0087@ø\u0001\u0000¢\u0006\u0002\u0010\u000f\u001a#\u0010J\u001a\u0004\u0018\u0001H\u000e\"\u0004\b\u0000\u0010\u000e*\b\u0012\u0004\u0012\u0002H\u000e0\nH\u0087@ø\u0001\u0000¢\u0006\u0002\u0010\u000f\u001a0\u0010K\u001a\b\u0012\u0004\u0012\u0002H\u000e0\n\"\u0004\b\u0000\u0010\u000e*\b\u0012\u0004\u0012\u0002H\u000e0\n2\u0006\u0010\u001e\u001a\u00020\u00122\b\b\u0002\u0010\u0016\u001a\u00020\u0017H\u0007\u001aT\u0010L\u001a\b\u0012\u0004\u0012\u0002H\u000e0\n\"\u0004\b\u0000\u0010\u000e*\b\u0012\u0004\u0012\u0002H\u000e0\n2\b\b\u0002\u0010\u0016\u001a\u00020\u00172\"\u0010 \u001a\u001e\b\u0001\u0012\u0004\u0012\u0002H\u000e\u0012\n\u0012\b\u0012\u0004\u0012\u00020\r0\u001a\u0012\u0006\u0012\u0004\u0018\u00010\u001b0\u0019H\u0007ø\u0001\u0000¢\u0006\u0002\u0010\u001c\u001a9\u0010M\u001a\u0002H,\"\u0004\b\u0000\u0010\u000e\"\u000e\b\u0001\u0010,*\b\u0012\u0004\u0012\u0002H\u000e00*\b\u0012\u0004\u0012\u0002H\u000e0\n2\u0006\u0010.\u001a\u0002H,H\u0081@ø\u0001\u0000¢\u0006\u0002\u00101\u001a;\u0010N\u001a\u0002H,\"\u0004\b\u0000\u0010\u000e\"\u0010\b\u0001\u0010,*\n\u0012\u0006\b\u0000\u0012\u0002H\u000e0-*\b\u0012\u0004\u0012\u0002H\u000e0\n2\u0006\u0010.\u001a\u0002H,H\u0081@ø\u0001\u0000¢\u0006\u0002\u0010/\u001a?\u0010O\u001a\u000e\u0012\u0004\u0012\u0002H\u0015\u0012\u0004\u0012\u0002HQ0P\"\u0004\b\u0000\u0010\u0015\"\u0004\b\u0001\u0010Q*\u0014\u0012\u0010\u0012\u000e\u0012\u0004\u0012\u0002H\u0015\u0012\u0004\u0012\u0002HQ0R0\nH\u0087@ø\u0001\u0000¢\u0006\u0002\u0010\u000f\u001aU\u0010O\u001a\u0002HS\"\u0004\b\u0000\u0010\u0015\"\u0004\b\u0001\u0010Q\"\u0018\b\u0002\u0010S*\u0012\u0012\u0006\b\u0000\u0012\u0002H\u0015\u0012\u0006\b\u0000\u0012\u0002HQ0T*\u0014\u0012\u0010\u0012\u000e\u0012\u0004\u0012\u0002H\u0015\u0012\u0004\u0012\u0002HQ0R0\n2\u0006\u0010.\u001a\u0002HSH\u0081@ø\u0001\u0000¢\u0006\u0002\u0010U\u001a'\u0010V\u001a\b\u0012\u0004\u0012\u0002H\u000e0W\"\u0004\b\u0000\u0010\u000e*\b\u0012\u0004\u0012\u0002H\u000e0\nH\u0087@ø\u0001\u0000¢\u0006\u0002\u0010\u000f\u001a'\u0010X\u001a\b\u0012\u0004\u0012\u0002H\u000e0Y\"\u0004\b\u0000\u0010\u000e*\b\u0012\u0004\u0012\u0002H\u000e0\nH\u0081@ø\u0001\u0000¢\u0006\u0002\u0010\u000f\u001a'\u0010Z\u001a\b\u0012\u0004\u0012\u0002H\u000e0[\"\u0004\b\u0000\u0010\u000e*\b\u0012\u0004\u0012\u0002H\u000e0\nH\u0087@ø\u0001\u0000¢\u0006\u0002\u0010\u000f\u001a.\u0010\\\u001a\u000e\u0012\n\u0012\b\u0012\u0004\u0012\u0002H\u000e0]0\n\"\u0004\b\u0000\u0010\u000e*\b\u0012\u0004\u0012\u0002H\u000e0\n2\b\b\u0002\u0010\u0016\u001a\u00020\u0017H\u0007\u001a?\u0010^\u001a\u0014\u0012\u0010\u0012\u000e\u0012\u0004\u0012\u0002H\u000e\u0012\u0004\u0012\u0002H50R0\n\"\u0004\b\u0000\u0010\u000e\"\u0004\b\u0001\u00105*\b\u0012\u0004\u0012\u0002H\u000e0\n2\f\u0010_\u001a\b\u0012\u0004\u0012\u0002H50\nH\u0087\u0004\u001az\u0010^\u001a\b\u0012\u0004\u0012\u0002HQ0\n\"\u0004\b\u0000\u0010\u000e\"\u0004\b\u0001\u00105\"\u0004\b\u0002\u0010Q*\b\u0012\u0004\u0012\u0002H\u000e0\n2\f\u0010_\u001a\b\u0012\u0004\u0012\u0002H50\n2\b\b\u0002\u0010\u0016\u001a\u00020\u001726\u00106\u001a2\u0012\u0013\u0012\u0011H\u000e¢\u0006\f\b\u0003\u0012\b\b\u0004\u0012\u0004\b\b(`\u0012\u0013\u0012\u0011H5¢\u0006\f\b\u0003\u0012\b\b\u0004\u0012\u0004\b\b(a\u0012\u0004\u0012\u0002HQ0\u0019H\u0001\u0082\u0002\u0004\n\u0002\b\u0019¨\u0006b"}, d2 = {"consumesAll", "Lkotlin/Function1;", "", "Lkotlin/ParameterName;", "name", "cause", "", "Lkotlinx/coroutines/CompletionHandler;", "channels", "", "Lkotlinx/coroutines/channels/ReceiveChannel;", "([Lkotlinx/coroutines/channels/ReceiveChannel;)Lkotlin/jvm/functions/Function1;", "any", "", "E", "(Lkotlinx/coroutines/channels/ReceiveChannel;Lkotlin/coroutines/Continuation;)Ljava/lang/Object;", "consumes", "count", "", "distinct", "distinctBy", "K", "context", "Lkotlin/coroutines/CoroutineContext;", "selector", "Lkotlin/Function2;", "Lkotlin/coroutines/Continuation;", "", "(Lkotlinx/coroutines/channels/ReceiveChannel;Lkotlin/coroutines/CoroutineContext;Lkotlin/jvm/functions/Function2;)Lkotlinx/coroutines/channels/ReceiveChannel;", "drop", "n", "dropWhile", "predicate", "elementAt", "index", "(Lkotlinx/coroutines/channels/ReceiveChannel;ILkotlin/coroutines/Continuation;)Ljava/lang/Object;", "elementAtOrNull", "filter", "filterIndexed", "Lkotlin/Function3;", "(Lkotlinx/coroutines/channels/ReceiveChannel;Lkotlin/coroutines/CoroutineContext;Lkotlin/jvm/functions/Function3;)Lkotlinx/coroutines/channels/ReceiveChannel;", "filterNot", "filterNotNull", "filterNotNullTo", "C", "", "destination", "(Lkotlinx/coroutines/channels/ReceiveChannel;Ljava/util/Collection;Lkotlin/coroutines/Continuation;)Ljava/lang/Object;", "Lkotlinx/coroutines/channels/SendChannel;", "(Lkotlinx/coroutines/channels/ReceiveChannel;Lkotlinx/coroutines/channels/SendChannel;Lkotlin/coroutines/Continuation;)Ljava/lang/Object;", "first", "firstOrNull", "flatMap", "R", "transform", "indexOf", "element", "(Lkotlinx/coroutines/channels/ReceiveChannel;Ljava/lang/Object;Lkotlin/coroutines/Continuation;)Ljava/lang/Object;", "last", "lastIndexOf", "lastOrNull", "map", "mapIndexed", "mapIndexedNotNull", "mapNotNull", "maxWith", "comparator", "Ljava/util/Comparator;", "Lkotlin/Comparator;", "(Lkotlinx/coroutines/channels/ReceiveChannel;Ljava/util/Comparator;Lkotlin/coroutines/Continuation;)Ljava/lang/Object;", "minWith", "none", "requireNoNulls", "single", "singleOrNull", "take", "takeWhile", "toChannel", "toCollection", "toMap", "", "V", "Lkotlin/Pair;", "M", "", "(Lkotlinx/coroutines/channels/ReceiveChannel;Ljava/util/Map;Lkotlin/coroutines/Continuation;)Ljava/lang/Object;", "toMutableList", "", "toMutableSet", "", "toSet", "", "withIndex", "Lkotlin/collections/IndexedValue;", "zip", "other", "a", "b", "kotlinx-coroutines-core"}, k = 5, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE, xs = "kotlinx/coroutines/channels/ChannelsKt")
final /* synthetic */ class ChannelsKt__DeprecatedKt {

    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$any$1, reason: invalid class name */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt", f = "Deprecated.kt", i = {0}, l = {404}, m = "any", n = {"$this$consume$iv"}, s = {"L$0"})
    static final class AnonymousClass1<E> extends ContinuationImpl {
        Object L$0;
        int label;
        /* synthetic */ Object result;

        AnonymousClass1(Continuation<? super AnonymousClass1> continuation) {
            super(continuation);
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Object invokeSuspend(Object obj) {
            this.result = obj;
            this.label |= Integer.MIN_VALUE;
            return ChannelsKt__DeprecatedKt.any(null, this);
        }
    }

    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$count$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt", f = "Deprecated.kt", i = {0, 0}, l = {487}, m = "count", n = {"count", "$this$consume$iv$iv"}, s = {"L$0", "L$1"})
    static final class C00741<E> extends ContinuationImpl {
        Object L$0;
        Object L$1;
        Object L$2;
        int label;
        /* synthetic */ Object result;

        C00741(Continuation<? super C00741> continuation) {
            super(continuation);
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Object invokeSuspend(Object obj) {
            this.result = obj;
            this.label |= Integer.MIN_VALUE;
            return ChannelsKt__DeprecatedKt.count(null, this);
        }
    }

    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$elementAt$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt", f = "Deprecated.kt", i = {0, 0, 0}, l = {38}, m = "elementAt", n = {"$this$consume$iv", "index", "count"}, s = {"L$0", "I$0", "I$1"})
    static final class C00791<E> extends ContinuationImpl {
        int I$0;
        int I$1;
        Object L$0;
        Object L$1;
        int label;
        /* synthetic */ Object result;

        C00791(Continuation<? super C00791> continuation) {
            super(continuation);
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Object invokeSuspend(Object obj) {
            this.result = obj;
            this.label |= Integer.MIN_VALUE;
            return ChannelsKt__DeprecatedKt.elementAt(null, 0, this);
        }
    }

    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$elementAtOrNull$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt", f = "Deprecated.kt", i = {0, 0, 0}, l = {ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_BASELINE_TO_BOTTOM_OF}, m = "elementAtOrNull", n = {"$this$consume$iv", "index", "count"}, s = {"L$0", "I$0", "I$1"})
    static final class C00801<E> extends ContinuationImpl {
        int I$0;
        int I$1;
        Object L$0;
        Object L$1;
        int label;
        /* synthetic */ Object result;

        C00801(Continuation<? super C00801> continuation) {
            super(continuation);
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Object invokeSuspend(Object obj) {
            this.result = obj;
            this.label |= Integer.MIN_VALUE;
            return ChannelsKt__DeprecatedKt.elementAtOrNull(null, 0, this);
        }
    }

    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$filterNotNullTo$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt", f = "Deprecated.kt", i = {0, 0}, l = {487}, m = "filterNotNullTo", n = {"destination", "$this$consume$iv$iv"}, s = {"L$0", "L$1"})
    static final class C00851<E, C extends Collection<? super E>> extends ContinuationImpl {
        Object L$0;
        Object L$1;
        Object L$2;
        int label;
        /* synthetic */ Object result;

        C00851(Continuation<? super C00851> continuation) {
            super(continuation);
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Object invokeSuspend(Object obj) {
            this.result = obj;
            this.label |= Integer.MIN_VALUE;
            return ChannelsKt__DeprecatedKt.filterNotNullTo((ReceiveChannel) null, (Collection) null, this);
        }
    }

    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$filterNotNullTo$3, reason: invalid class name */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt", f = "Deprecated.kt", i = {0, 0, 1, 1}, l = {487, 242}, m = "filterNotNullTo", n = {"destination", "$this$consume$iv$iv", "destination", "$this$consume$iv$iv"}, s = {"L$0", "L$1", "L$0", "L$1"})
    static final class AnonymousClass3<E, C extends SendChannel<? super E>> extends ContinuationImpl {
        Object L$0;
        Object L$1;
        Object L$2;
        int label;
        /* synthetic */ Object result;

        AnonymousClass3(Continuation<? super AnonymousClass3> continuation) {
            super(continuation);
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Object invokeSuspend(Object obj) {
            this.result = obj;
            this.label |= Integer.MIN_VALUE;
            return ChannelsKt__DeprecatedKt.filterNotNullTo((ReceiveChannel) null, (SendChannel) null, this);
        }
    }

    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$first$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt", f = "Deprecated.kt", i = {0, 0}, l = {ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_HEIGHT}, m = "first", n = {"$this$consume$iv", "iterator"}, s = {"L$0", "L$1"})
    static final class C00861<E> extends ContinuationImpl {
        Object L$0;
        Object L$1;
        int label;
        /* synthetic */ Object result;

        C00861(Continuation<? super C00861> continuation) {
            super(continuation);
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Object invokeSuspend(Object obj) {
            this.result = obj;
            this.label |= Integer.MIN_VALUE;
            return ChannelsKt__DeprecatedKt.first(null, this);
        }
    }

    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$firstOrNull$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt", f = "Deprecated.kt", i = {0, 0}, l = {75}, m = "firstOrNull", n = {"$this$consume$iv", "iterator"}, s = {"L$0", "L$1"})
    static final class C00871<E> extends ContinuationImpl {
        Object L$0;
        Object L$1;
        int label;
        /* synthetic */ Object result;

        C00871(Continuation<? super C00871> continuation) {
            super(continuation);
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Object invokeSuspend(Object obj) {
            this.result = obj;
            this.label |= Integer.MIN_VALUE;
            return ChannelsKt__DeprecatedKt.firstOrNull(null, this);
        }
    }

    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$indexOf$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt", f = "Deprecated.kt", i = {0, 0, 0}, l = {487}, m = "indexOf", n = {"element", "index", "$this$consume$iv$iv"}, s = {"L$0", "L$1", "L$2"})
    static final class C00891<E> extends ContinuationImpl {
        Object L$0;
        Object L$1;
        Object L$2;
        Object L$3;
        int label;
        /* synthetic */ Object result;

        C00891(Continuation<? super C00891> continuation) {
            super(continuation);
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Object invokeSuspend(Object obj) {
            this.result = obj;
            this.label |= Integer.MIN_VALUE;
            return ChannelsKt__DeprecatedKt.indexOf(null, null, this);
        }
    }

    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$last$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt", f = "Deprecated.kt", i = {0, 0, 1, 1, 1}, l = {97, 100}, m = "last", n = {"$this$consume$iv", "iterator", "$this$consume$iv", "iterator", "last"}, s = {"L$0", "L$1", "L$0", "L$1", "L$2"})
    static final class C00901<E> extends ContinuationImpl {
        Object L$0;
        Object L$1;
        Object L$2;
        int label;
        /* synthetic */ Object result;

        C00901(Continuation<? super C00901> continuation) {
            super(continuation);
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Object invokeSuspend(Object obj) {
            this.result = obj;
            this.label |= Integer.MIN_VALUE;
            return ChannelsKt__DeprecatedKt.last(null, this);
        }
    }

    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$lastIndexOf$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt", f = "Deprecated.kt", i = {0, 0, 0, 0}, l = {487}, m = "lastIndexOf", n = {"element", "lastIndex", "index", "$this$consume$iv$iv"}, s = {"L$0", "L$1", "L$2", "L$3"})
    static final class C00911<E> extends ContinuationImpl {
        Object L$0;
        Object L$1;
        Object L$2;
        Object L$3;
        Object L$4;
        int label;
        /* synthetic */ Object result;

        C00911(Continuation<? super C00911> continuation) {
            super(continuation);
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Object invokeSuspend(Object obj) {
            this.result = obj;
            this.label |= Integer.MIN_VALUE;
            return ChannelsKt__DeprecatedKt.lastIndexOf(null, null, this);
        }
    }

    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$lastOrNull$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt", f = "Deprecated.kt", i = {0, 0, 1, 1, 1}, l = {123, 126}, m = "lastOrNull", n = {"$this$consume$iv", "iterator", "$this$consume$iv", "iterator", "last"}, s = {"L$0", "L$1", "L$0", "L$1", "L$2"})
    static final class C00921<E> extends ContinuationImpl {
        Object L$0;
        Object L$1;
        Object L$2;
        int label;
        /* synthetic */ Object result;

        C00921(Continuation<? super C00921> continuation) {
            super(continuation);
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Object invokeSuspend(Object obj) {
            this.result = obj;
            this.label |= Integer.MIN_VALUE;
            return ChannelsKt__DeprecatedKt.lastOrNull(null, this);
        }
    }

    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$maxWith$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt", f = "Deprecated.kt", i = {0, 0, 0, 1, 1, 1, 1}, l = {TypedValues.CycleType.TYPE_EASING, TypedValues.CycleType.TYPE_CUSTOM_WAVE_SHAPE}, m = "maxWith", n = {"comparator", "$this$consume$iv", "iterator", "comparator", "$this$consume$iv", "iterator", "max"}, s = {"L$0", "L$1", "L$2", "L$0", "L$1", "L$2", "L$3"})
    static final class C00951<E> extends ContinuationImpl {
        Object L$0;
        Object L$1;
        Object L$2;
        Object L$3;
        int label;
        /* synthetic */ Object result;

        C00951(Continuation<? super C00951> continuation) {
            super(continuation);
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Object invokeSuspend(Object obj) {
            this.result = obj;
            this.label |= Integer.MIN_VALUE;
            return ChannelsKt__DeprecatedKt.maxWith(null, null, this);
        }
    }

    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$minWith$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt", f = "Deprecated.kt", i = {0, 0, 0, 1, 1, 1, 1}, l = {434, 436}, m = "minWith", n = {"comparator", "$this$consume$iv", "iterator", "comparator", "$this$consume$iv", "iterator", "min"}, s = {"L$0", "L$1", "L$2", "L$0", "L$1", "L$2", "L$3"})
    static final class C00961<E> extends ContinuationImpl {
        Object L$0;
        Object L$1;
        Object L$2;
        Object L$3;
        int label;
        /* synthetic */ Object result;

        C00961(Continuation<? super C00961> continuation) {
            super(continuation);
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Object invokeSuspend(Object obj) {
            this.result = obj;
            this.label |= Integer.MIN_VALUE;
            return ChannelsKt__DeprecatedKt.minWith(null, null, this);
        }
    }

    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$none$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt", f = "Deprecated.kt", i = {0}, l = {447}, m = "none", n = {"$this$consume$iv"}, s = {"L$0"})
    static final class C00971<E> extends ContinuationImpl {
        Object L$0;
        int label;
        /* synthetic */ Object result;

        C00971(Continuation<? super C00971> continuation) {
            super(continuation);
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Object invokeSuspend(Object obj) {
            this.result = obj;
            this.label |= Integer.MIN_VALUE;
            return ChannelsKt__DeprecatedKt.none(null, this);
        }
    }

    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$single$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt", f = "Deprecated.kt", i = {0, 0, 1, 1}, l = {136, 139}, m = "single", n = {"$this$consume$iv", "iterator", "$this$consume$iv", "single"}, s = {"L$0", "L$1", "L$0", "L$1"})
    static final class C00991<E> extends ContinuationImpl {
        Object L$0;
        Object L$1;
        int label;
        /* synthetic */ Object result;

        C00991(Continuation<? super C00991> continuation) {
            super(continuation);
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Object invokeSuspend(Object obj) {
            this.result = obj;
            this.label |= Integer.MIN_VALUE;
            return ChannelsKt__DeprecatedKt.single(null, this);
        }
    }

    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$singleOrNull$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt", f = "Deprecated.kt", i = {0, 0, 1, 1}, l = {149, 152}, m = "singleOrNull", n = {"$this$consume$iv", "iterator", "$this$consume$iv", "single"}, s = {"L$0", "L$1", "L$0", "L$1"})
    static final class C01001<E> extends ContinuationImpl {
        Object L$0;
        Object L$1;
        int label;
        /* synthetic */ Object result;

        C01001(Continuation<? super C01001> continuation) {
            super(continuation);
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Object invokeSuspend(Object obj) {
            this.result = obj;
            this.label |= Integer.MIN_VALUE;
            return ChannelsKt__DeprecatedKt.singleOrNull(null, this);
        }
    }

    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$toChannel$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt", f = "Deprecated.kt", i = {0, 0, 1, 1}, l = {487, 278}, m = "toChannel", n = {"destination", "$this$consume$iv$iv", "destination", "$this$consume$iv$iv"}, s = {"L$0", "L$1", "L$0", "L$1"})
    static final class C01031<E, C extends SendChannel<? super E>> extends ContinuationImpl {
        Object L$0;
        Object L$1;
        Object L$2;
        int label;
        /* synthetic */ Object result;

        C01031(Continuation<? super C01031> continuation) {
            super(continuation);
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Object invokeSuspend(Object obj) {
            this.result = obj;
            this.label |= Integer.MIN_VALUE;
            return ChannelsKt.toChannel(null, null, this);
        }
    }

    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$toCollection$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt", f = "Deprecated.kt", i = {0, 0}, l = {487}, m = "toCollection", n = {"destination", "$this$consume$iv$iv"}, s = {"L$0", "L$1"})
    static final class C01041<E, C extends Collection<? super E>> extends ContinuationImpl {
        Object L$0;
        Object L$1;
        Object L$2;
        int label;
        /* synthetic */ Object result;

        C01041(Continuation<? super C01041> continuation) {
            super(continuation);
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Object invokeSuspend(Object obj) {
            this.result = obj;
            this.label |= Integer.MIN_VALUE;
            return ChannelsKt.toCollection(null, null, this);
        }
    }

    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$toMap$2, reason: invalid class name */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt", f = "Deprecated.kt", i = {0, 0}, l = {487}, m = "toMap", n = {"destination", "$this$consume$iv$iv"}, s = {"L$0", "L$1"})
    static final class AnonymousClass2<K, V, M extends Map<? super K, ? super V>> extends ContinuationImpl {
        Object L$0;
        Object L$1;
        Object L$2;
        int label;
        /* synthetic */ Object result;

        AnonymousClass2(Continuation<? super AnonymousClass2> continuation) {
            super(continuation);
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Object invokeSuspend(Object obj) {
            this.result = obj;
            this.label |= Integer.MIN_VALUE;
            return ChannelsKt.toMap(null, null, this);
        }
    }

    public static final Function1<Throwable, Unit> consumesAll(final ReceiveChannel<?>... receiveChannelArr) {
        return new Function1<Throwable, Unit>() { // from class: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt.consumesAll.1
            /* JADX WARN: 'super' call moved to the top of the method (can break code semantics) */
            {
                super(1);
            }

            @Override // kotlin.jvm.functions.Function1
            public /* bridge */ /* synthetic */ Unit invoke(Throwable th) throws Throwable {
                invoke2(th);
                return Unit.INSTANCE;
            }

            /* JADX INFO: renamed from: invoke, reason: avoid collision after fix types in other method */
            public final void invoke2(Throwable cause) throws Throwable {
                Throwable exception = null;
                ReceiveChannel<?>[] receiveChannelArr2 = receiveChannelArr;
                int length = receiveChannelArr2.length;
                int i = 0;
                while (i < length) {
                    ReceiveChannel<?> receiveChannel = receiveChannelArr2[i];
                    i++;
                    try {
                        ChannelsKt.cancelConsumed(receiveChannel, cause);
                    } catch (Throwable e) {
                        if (exception == null) {
                            exception = e;
                        } else {
                            Throwable $this$addSuppressedThrowable$iv = exception;
                            ExceptionsKt.addSuppressed($this$addSuppressedThrowable$iv, e);
                        }
                    }
                }
                if (exception != null) {
                    Throwable it = exception;
                    throw it;
                }
            }
        };
    }

    /* JADX WARN: Removed duplicated region for block: B:24:0x007a A[RETURN] */
    /* JADX WARN: Removed duplicated region for block: B:25:0x007b  */
    /* JADX WARN: Removed duplicated region for block: B:28:0x008c A[Catch: all -> 0x00c6, TRY_LEAVE, TryCatch #3 {all -> 0x00c6, blocks: (B:26:0x0084, B:28:0x008c), top: B:57:0x0084 }] */
    /* JADX WARN: Removed duplicated region for block: B:34:0x00a3  */
    /* JADX WARN: Removed duplicated region for block: B:7:0x0014  */
    /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:25:0x007b -> B:57:0x0084). Please report as a decompilation issue!!! */
    @Deprecated(level = DeprecationLevel.HIDDEN, message = "Binary compatibility")
    /*
        Code decompiled incorrectly, please refer to instructions dump.
    */
    public static final /* synthetic */ Object elementAt(ReceiveChannel $this$consume$iv, int i, Continuation continuation) throws Throwable {
        C00791 c00791;
        ReceiveChannel $this$consume$iv2;
        Throwable e$iv;
        Object $result;
        Throwable th;
        ReceiveChannel $this$consume$iv3;
        ChannelIterator channelIterator;
        int index;
        int index2;
        Object obj;
        if (continuation instanceof C00791) {
            c00791 = (C00791) continuation;
            if ((c00791.label & Integer.MIN_VALUE) != 0) {
                c00791.label -= Integer.MIN_VALUE;
            } else {
                c00791 = new C00791(continuation);
            }
        }
        C00791 c007912 = c00791;
        Object $result2 = c007912.result;
        Object $result3 = IntrinsicsKt.getCOROUTINE_SUSPENDED();
        switch (c007912.label) {
            case 0:
                ResultKt.throwOnFailure($result2);
                int index3 = i;
                try {
                    if (index3 < 0) {
                        throw new IndexOutOfBoundsException("ReceiveChannel doesn't contain element at index " + index3 + '.');
                    }
                    ChannelIterator it = $this$consume$iv.iterator();
                    ReceiveChannel $this$consume$iv4 = $this$consume$iv;
                    Throwable cause$iv = null;
                    int count = 0;
                    try {
                        c007912.L$0 = $this$consume$iv4;
                        c007912.L$1 = it;
                        c007912.I$0 = index3;
                        c007912.I$1 = count;
                        c007912.label = 1;
                        Object objHasNext = it.hasNext(c007912);
                        if (objHasNext != $result3) {
                            return $result3;
                        }
                        Object obj2 = $result3;
                        $result = $result2;
                        $result2 = objHasNext;
                        th = cause$iv;
                        $this$consume$iv3 = $this$consume$iv4;
                        channelIterator = it;
                        index = index3;
                        index2 = count;
                        obj = obj2;
                        try {
                            if (((Boolean) $result2).booleanValue()) {
                                ReceiveChannel $this$consume$iv5 = $this$consume$iv3;
                                try {
                                    throw new IndexOutOfBoundsException("ReceiveChannel doesn't contain element at index " + index + '.');
                                } catch (Throwable th2) {
                                    e$iv = th2;
                                    $this$consume$iv2 = $this$consume$iv5;
                                    Throwable cause$iv2 = e$iv;
                                    try {
                                        throw e$iv;
                                    } catch (Throwable e$iv2) {
                                        ChannelsKt.cancelConsumed($this$consume$iv2, cause$iv2);
                                        throw e$iv2;
                                    }
                                }
                            }
                            Object next = channelIterator.next();
                            int count2 = index2 + 1;
                            if (index == index2) {
                                ChannelsKt.cancelConsumed($this$consume$iv3, th);
                                return next;
                            }
                            ReceiveChannel $this$consume$iv6 = $this$consume$iv3;
                            cause$iv = th;
                            index3 = index;
                            it = channelIterator;
                            $this$consume$iv4 = $this$consume$iv6;
                            $result2 = $result;
                            $result3 = obj;
                            count = count2;
                            c007912.L$0 = $this$consume$iv4;
                            c007912.L$1 = it;
                            c007912.I$0 = index3;
                            c007912.I$1 = count;
                            c007912.label = 1;
                            Object objHasNext2 = it.hasNext(c007912);
                            if (objHasNext2 != $result3) {
                            }
                        } catch (Throwable th3) {
                            e$iv = th3;
                            $this$consume$iv2 = $this$consume$iv3;
                        }
                    } catch (Throwable th4) {
                        e$iv = th4;
                        $this$consume$iv2 = $this$consume$iv4;
                        Throwable cause$iv22 = e$iv;
                        throw e$iv;
                    }
                } catch (Throwable th5) {
                    $this$consume$iv2 = $this$consume$iv;
                    e$iv = th5;
                    Throwable cause$iv222 = e$iv;
                    throw e$iv;
                }
                break;
            case 1:
                int count3 = c007912.I$1;
                int index4 = c007912.I$0;
                ChannelIterator channelIterator2 = (ChannelIterator) c007912.L$1;
                ReceiveChannel receiveChannel = (ReceiveChannel) c007912.L$0;
                try {
                    ResultKt.throwOnFailure($result2);
                    th = null;
                    $this$consume$iv3 = receiveChannel;
                    channelIterator = channelIterator2;
                    index = index4;
                    index2 = count3;
                    obj = $result3;
                    $result = $result2;
                    if (((Boolean) $result2).booleanValue()) {
                    }
                } catch (Throwable th6) {
                    e$iv = th6;
                    $this$consume$iv2 = receiveChannel;
                    Throwable cause$iv2222 = e$iv;
                    throw e$iv;
                }
                break;
            default:
                throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
        }
    }

    /* JADX WARN: Removed duplicated region for block: B:26:0x0079 A[RETURN] */
    /* JADX WARN: Removed duplicated region for block: B:27:0x007a  */
    /* JADX WARN: Removed duplicated region for block: B:30:0x008a A[Catch: all -> 0x00a5, TRY_LEAVE, TryCatch #1 {all -> 0x00a5, blocks: (B:28:0x0082, B:30:0x008a), top: B:51:0x0082 }] */
    /* JADX WARN: Removed duplicated region for block: B:36:0x00a0  */
    /* JADX WARN: Removed duplicated region for block: B:7:0x0014  */
    /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:27:0x007a -> B:51:0x0082). Please report as a decompilation issue!!! */
    @Deprecated(level = DeprecationLevel.HIDDEN, message = "Binary compatibility")
    /*
        Code decompiled incorrectly, please refer to instructions dump.
    */
    public static final /* synthetic */ Object elementAtOrNull(ReceiveChannel $this$consume$iv, int i, Continuation continuation) throws Throwable {
        C00801 c00801;
        ReceiveChannel $this$consume$iv2;
        Throwable e$iv;
        int count;
        ReceiveChannel $this$consume$iv3;
        ChannelIterator it;
        Object $result;
        Throwable th;
        ChannelIterator channelIterator;
        int index;
        int index2;
        Object obj;
        if (continuation instanceof C00801) {
            c00801 = (C00801) continuation;
            if ((c00801.label & Integer.MIN_VALUE) != 0) {
                c00801.label -= Integer.MIN_VALUE;
            } else {
                c00801 = new C00801(continuation);
            }
        }
        C00801 c008012 = c00801;
        Object $result2 = c008012.result;
        Object $result3 = IntrinsicsKt.getCOROUTINE_SUSPENDED();
        switch (c008012.label) {
            case 0:
                ResultKt.throwOnFailure($result2);
                int index3 = i;
                if (index3 < 0) {
                    ChannelsKt.cancelConsumed($this$consume$iv, null);
                    return null;
                }
                Throwable cause$iv = null;
                try {
                    count = 0;
                    $this$consume$iv3 = $this$consume$iv;
                    it = $this$consume$iv.iterator();
                } catch (Throwable th2) {
                    $this$consume$iv2 = $this$consume$iv;
                    e$iv = th2;
                    Throwable cause$iv2 = e$iv;
                    try {
                        throw e$iv;
                    } catch (Throwable e$iv2) {
                        ChannelsKt.cancelConsumed($this$consume$iv2, cause$iv2);
                        throw e$iv2;
                    }
                }
                try {
                    c008012.L$0 = $this$consume$iv3;
                    c008012.L$1 = it;
                    c008012.I$0 = index3;
                    c008012.I$1 = count;
                    c008012.label = 1;
                    Object objHasNext = it.hasNext(c008012);
                    if (objHasNext != $result3) {
                        return $result3;
                    }
                    Object obj2 = $result3;
                    $result = $result2;
                    $result2 = objHasNext;
                    th = cause$iv;
                    channelIterator = it;
                    index = index3;
                    index2 = count;
                    obj = obj2;
                    try {
                        if (((Boolean) $result2).booleanValue()) {
                            ChannelsKt.cancelConsumed($this$consume$iv3, th);
                            return null;
                        }
                        Object next = channelIterator.next();
                        int count2 = index2 + 1;
                        if (index == index2) {
                            ChannelsKt.cancelConsumed($this$consume$iv3, th);
                            return next;
                        }
                        index3 = index;
                        it = channelIterator;
                        cause$iv = th;
                        $result2 = $result;
                        $result3 = obj;
                        count = count2;
                        c008012.L$0 = $this$consume$iv3;
                        c008012.L$1 = it;
                        c008012.I$0 = index3;
                        c008012.I$1 = count;
                        c008012.label = 1;
                        Object objHasNext2 = it.hasNext(c008012);
                        if (objHasNext2 != $result3) {
                        }
                    } catch (Throwable th3) {
                        e$iv = th3;
                        $this$consume$iv2 = $this$consume$iv3;
                        Throwable cause$iv22 = e$iv;
                        throw e$iv;
                    }
                } catch (Throwable th4) {
                    e$iv = th4;
                    $this$consume$iv2 = $this$consume$iv3;
                    Throwable cause$iv222 = e$iv;
                    throw e$iv;
                }
                break;
            case 1:
                int count3 = c008012.I$1;
                int index4 = c008012.I$0;
                ChannelIterator channelIterator2 = (ChannelIterator) c008012.L$1;
                ReceiveChannel receiveChannel = (ReceiveChannel) c008012.L$0;
                try {
                    ResultKt.throwOnFailure($result2);
                    th = null;
                    $this$consume$iv3 = receiveChannel;
                    channelIterator = channelIterator2;
                    index = index4;
                    index2 = count3;
                    obj = $result3;
                    $result = $result2;
                    if (((Boolean) $result2).booleanValue()) {
                    }
                } catch (Throwable th5) {
                    e$iv = th5;
                    $this$consume$iv2 = receiveChannel;
                    Throwable cause$iv2222 = e$iv;
                    throw e$iv;
                }
                break;
            default:
                throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
        }
    }

    /* JADX WARN: Removed duplicated region for block: B:24:0x0067 A[Catch: all -> 0x003d, TRY_LEAVE, TryCatch #2 {all -> 0x003d, blocks: (B:13:0x0038, B:22:0x005f, B:24:0x0067, B:27:0x006f, B:28:0x0076), top: B:40:0x0038 }] */
    /* JADX WARN: Removed duplicated region for block: B:27:0x006f A[Catch: all -> 0x003d, TRY_ENTER, TryCatch #2 {all -> 0x003d, blocks: (B:13:0x0038, B:22:0x005f, B:24:0x0067, B:27:0x006f, B:28:0x0076), top: B:40:0x0038 }] */
    /* JADX WARN: Removed duplicated region for block: B:7:0x0014  */
    @Deprecated(level = DeprecationLevel.HIDDEN, message = "Binary compatibility")
    /*
        Code decompiled incorrectly, please refer to instructions dump.
    */
    public static final /* synthetic */ Object first(ReceiveChannel $this$first, Continuation continuation) throws Throwable {
        C00861 c00861;
        ReceiveChannel $this$consume$iv;
        Throwable cause$iv;
        ReceiveChannel $this$consume$iv2;
        ChannelIterator iterator;
        Object objHasNext;
        if (continuation instanceof C00861) {
            c00861 = (C00861) continuation;
            if ((c00861.label & Integer.MIN_VALUE) != 0) {
                c00861.label -= Integer.MIN_VALUE;
            } else {
                c00861 = new C00861(continuation);
            }
        }
        C00861 c008612 = c00861;
        Object $result = c008612.result;
        Object coroutine_suspended = IntrinsicsKt.getCOROUTINE_SUSPENDED();
        switch (c008612.label) {
            case 0:
                ResultKt.throwOnFailure($result);
                $this$consume$iv = $this$first;
                cause$iv = null;
                try {
                    iterator = $this$consume$iv.iterator();
                    c008612.L$0 = $this$consume$iv;
                    c008612.L$1 = iterator;
                    c008612.label = 1;
                    objHasNext = iterator.hasNext(c008612);
                    if (objHasNext == coroutine_suspended) {
                        return coroutine_suspended;
                    }
                    if (((Boolean) objHasNext).booleanValue()) {
                        throw new NoSuchElementException("ReceiveChannel is empty.");
                    }
                    Object next = iterator.next();
                    ChannelsKt.cancelConsumed($this$consume$iv, cause$iv);
                    return next;
                } catch (Throwable th) {
                    e$iv = th;
                    $this$consume$iv2 = $this$consume$iv;
                    Throwable cause$iv2 = e$iv;
                    try {
                        throw e$iv;
                    } catch (Throwable e$iv) {
                        ChannelsKt.cancelConsumed($this$consume$iv2, cause$iv2);
                        throw e$iv;
                    }
                }
            case 1:
                iterator = (ChannelIterator) c008612.L$1;
                $this$consume$iv = (ReceiveChannel) c008612.L$0;
                cause$iv = null;
                try {
                    ResultKt.throwOnFailure($result);
                    objHasNext = $result;
                    if (((Boolean) objHasNext).booleanValue()) {
                    }
                } catch (Throwable th2) {
                    e$iv = th2;
                    $this$consume$iv2 = $this$consume$iv;
                    Throwable cause$iv22 = e$iv;
                    throw e$iv;
                }
                break;
            default:
                throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
        }
    }

    /* JADX WARN: Removed duplicated region for block: B:26:0x006d A[Catch: all -> 0x0072, TRY_ENTER, TRY_LEAVE, TryCatch #0 {all -> 0x0072, blocks: (B:22:0x0060, B:26:0x006d), top: B:37:0x0060 }] */
    /* JADX WARN: Removed duplicated region for block: B:7:0x0014  */
    @Deprecated(level = DeprecationLevel.HIDDEN, message = "Binary compatibility")
    /*
        Code decompiled incorrectly, please refer to instructions dump.
    */
    public static final /* synthetic */ Object firstOrNull(ReceiveChannel $this$firstOrNull, Continuation continuation) throws Throwable {
        C00871 c00871;
        ReceiveChannel $this$consume$iv;
        Throwable cause$iv;
        ReceiveChannel $this$consume$iv2;
        ChannelIterator iterator;
        Object objHasNext;
        if (continuation instanceof C00871) {
            c00871 = (C00871) continuation;
            if ((c00871.label & Integer.MIN_VALUE) != 0) {
                c00871.label -= Integer.MIN_VALUE;
            } else {
                c00871 = new C00871(continuation);
            }
        }
        C00871 c008712 = c00871;
        Object $result = c008712.result;
        Object coroutine_suspended = IntrinsicsKt.getCOROUTINE_SUSPENDED();
        Object next = null;
        switch (c008712.label) {
            case 0:
                ResultKt.throwOnFailure($result);
                $this$consume$iv = $this$firstOrNull;
                cause$iv = null;
                try {
                    iterator = $this$consume$iv.iterator();
                    c008712.L$0 = $this$consume$iv;
                    c008712.L$1 = iterator;
                    c008712.label = 1;
                    objHasNext = iterator.hasNext(c008712);
                    if (objHasNext == coroutine_suspended) {
                        return coroutine_suspended;
                    }
                    try {
                        if (((Boolean) objHasNext).booleanValue()) {
                            next = iterator.next();
                            break;
                        }
                        ChannelsKt.cancelConsumed($this$consume$iv, cause$iv);
                        return next;
                    } catch (Throwable th) {
                        e$iv = th;
                        $this$consume$iv2 = $this$consume$iv;
                        Throwable cause$iv2 = e$iv;
                        try {
                            throw e$iv;
                        } catch (Throwable e$iv) {
                            ChannelsKt.cancelConsumed($this$consume$iv2, cause$iv2);
                            throw e$iv;
                        }
                    }
                } catch (Throwable th2) {
                    e$iv = th2;
                    $this$consume$iv2 = $this$consume$iv;
                    Throwable cause$iv22 = e$iv;
                    throw e$iv;
                }
            case 1:
                ChannelIterator iterator2 = (ChannelIterator) c008712.L$1;
                $this$consume$iv = (ReceiveChannel) c008712.L$0;
                try {
                    ResultKt.throwOnFailure($result);
                    objHasNext = $result;
                    iterator = iterator2;
                    cause$iv = null;
                    if (((Boolean) objHasNext).booleanValue()) {
                    }
                    ChannelsKt.cancelConsumed($this$consume$iv, cause$iv);
                    return next;
                } catch (Throwable th3) {
                    e$iv = th3;
                    $this$consume$iv2 = $this$consume$iv;
                    Throwable cause$iv222 = e$iv;
                    throw e$iv;
                }
            default:
                throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
        }
    }

    /* JADX WARN: Removed duplicated region for block: B:22:0x0084 A[RETURN] */
    /* JADX WARN: Removed duplicated region for block: B:23:0x0085  */
    /* JADX WARN: Removed duplicated region for block: B:26:0x009a A[Catch: all -> 0x00fa, TRY_LEAVE, TryCatch #4 {all -> 0x00fa, blocks: (B:24:0x0092, B:26:0x009a), top: B:63:0x0092 }] */
    /* JADX WARN: Removed duplicated region for block: B:38:0x00df  */
    /* JADX WARN: Removed duplicated region for block: B:7:0x0016  */
    /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:23:0x0085 -> B:63:0x0092). Please report as a decompilation issue!!! */
    @Deprecated(level = DeprecationLevel.HIDDEN, message = "Binary compatibility")
    /*
        Code decompiled incorrectly, please refer to instructions dump.
    */
    public static final /* synthetic */ Object indexOf(ReceiveChannel $this$consume$iv$iv, Object obj, Continuation continuation) throws Throwable {
        C00891 c00891;
        ReceiveChannel $this$consume$iv$iv2;
        ReceiveChannel $this$consumeEach_u24lambda_u2d1$iv;
        Object $result;
        int $i$f$consumeEach;
        int $i$f$consume;
        Throwable cause$iv$iv;
        ChannelIterator it;
        Object objHasNext;
        Object $result2;
        Throwable th;
        Object element;
        Ref.IntRef index;
        ReceiveChannel receiveChannel;
        ChannelIterator channelIterator;
        int i;
        int $i$f$consume2;
        Object obj2;
        if (continuation instanceof C00891) {
            c00891 = (C00891) continuation;
            if ((c00891.label & Integer.MIN_VALUE) != 0) {
                c00891.label -= Integer.MIN_VALUE;
            } else {
                c00891 = new C00891(continuation);
            }
        }
        Object $result3 = c00891.result;
        Object coroutine_suspended = IntrinsicsKt.getCOROUTINE_SUSPENDED();
        switch (c00891.label) {
            case 0:
                ResultKt.throwOnFailure($result3);
                Object element2 = obj;
                Ref.IntRef index2 = new Ref.IntRef();
                try {
                    $this$consumeEach_u24lambda_u2d1$iv = $this$consume$iv$iv;
                    $result = null;
                    $i$f$consumeEach = 0;
                    $i$f$consume = 0;
                    cause$iv$iv = null;
                    it = $this$consume$iv$iv.iterator();
                    try {
                        c00891.L$0 = element2;
                        c00891.L$1 = index2;
                        c00891.L$2 = $this$consumeEach_u24lambda_u2d1$iv;
                        c00891.L$3 = it;
                        c00891.label = 1;
                        objHasNext = it.hasNext(c00891);
                    } catch (Throwable th2) {
                        e$iv$iv = th2;
                        $this$consume$iv$iv2 = $this$consumeEach_u24lambda_u2d1$iv;
                    }
                } catch (Throwable th3) {
                    e$iv$iv = th3;
                    $this$consume$iv$iv2 = $this$consume$iv$iv;
                }
                if (objHasNext != coroutine_suspended) {
                    return coroutine_suspended;
                }
                Object obj3 = $result;
                $result2 = $result3;
                $result3 = objHasNext;
                th = cause$iv$iv;
                element = element2;
                index = index2;
                receiveChannel = $this$consumeEach_u24lambda_u2d1$iv;
                channelIterator = it;
                i = $i$f$consume;
                $i$f$consume2 = $i$f$consumeEach;
                obj2 = obj3;
                try {
                    if (((Boolean) $result3).booleanValue()) {
                        $this$consume$iv$iv2 = receiveChannel;
                        Throwable cause$iv$iv2 = th;
                        try {
                            Unit unit = Unit.INSTANCE;
                            ChannelsKt.cancelConsumed($this$consume$iv$iv2, cause$iv$iv2);
                            return Boxing.boxInt(-1);
                        } catch (Throwable th4) {
                            e$iv$iv = th4;
                        }
                    } else {
                        Object e$iv = channelIterator.next();
                        if (Intrinsics.areEqual(element, e$iv)) {
                            try {
                                Integer numBoxInt = Boxing.boxInt(index.element);
                                ChannelsKt.cancelConsumed(receiveChannel, th);
                                return numBoxInt;
                            } catch (Throwable th5) {
                                e$iv$iv = th5;
                                $this$consume$iv$iv2 = receiveChannel;
                            }
                        } else {
                            ReceiveChannel $this$consume$iv$iv3 = receiveChannel;
                            Throwable cause$iv$iv3 = th;
                            try {
                                index.element++;
                                ChannelIterator channelIterator2 = channelIterator;
                                $this$consumeEach_u24lambda_u2d1$iv = $this$consume$iv$iv3;
                                $result3 = $result2;
                                $result = obj2;
                                $i$f$consumeEach = $i$f$consume2;
                                $i$f$consume = i;
                                it = channelIterator2;
                                Object obj4 = element;
                                cause$iv$iv = cause$iv$iv3;
                                index2 = index;
                                element2 = obj4;
                                c00891.L$0 = element2;
                                c00891.L$1 = index2;
                                c00891.L$2 = $this$consumeEach_u24lambda_u2d1$iv;
                                c00891.L$3 = it;
                                c00891.label = 1;
                                objHasNext = it.hasNext(c00891);
                                if (objHasNext != coroutine_suspended) {
                                }
                            } catch (Throwable th6) {
                                e$iv$iv = th6;
                                $this$consume$iv$iv2 = $this$consume$iv$iv3;
                            }
                        }
                    }
                } catch (Throwable th7) {
                    e$iv$iv = th7;
                    $this$consume$iv$iv2 = receiveChannel;
                }
                Throwable cause$iv$iv4 = e$iv$iv;
                try {
                    throw e$iv$iv;
                } catch (Throwable e$iv$iv) {
                    ChannelsKt.cancelConsumed($this$consume$iv$iv2, cause$iv$iv4);
                    throw e$iv$iv;
                }
                break;
            case 1:
                ChannelIterator channelIterator3 = (ChannelIterator) c00891.L$3;
                ReceiveChannel receiveChannel2 = (ReceiveChannel) c00891.L$2;
                Ref.IntRef index3 = (Ref.IntRef) c00891.L$1;
                Object element3 = c00891.L$0;
                try {
                    ResultKt.throwOnFailure($result3);
                    th = null;
                    element = element3;
                    index = index3;
                    receiveChannel = receiveChannel2;
                    channelIterator = channelIterator3;
                    i = 0;
                    $i$f$consume2 = 0;
                    obj2 = null;
                    $result2 = $result3;
                    if (((Boolean) $result3).booleanValue()) {
                    }
                } catch (Throwable th8) {
                    e$iv$iv = th8;
                    $this$consume$iv$iv2 = receiveChannel2;
                }
                Throwable cause$iv$iv42 = e$iv$iv;
                throw e$iv$iv;
            default:
                throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
        }
    }

    /* JADX WARN: Removed duplicated region for block: B:29:0x008b A[Catch: all -> 0x00de, TRY_LEAVE, TryCatch #5 {all -> 0x00de, blocks: (B:27:0x0083, B:29:0x008b, B:45:0x00d6, B:46:0x00dd), top: B:65:0x0083 }] */
    /* JADX WARN: Removed duplicated region for block: B:33:0x00a5 A[RETURN] */
    /* JADX WARN: Removed duplicated region for block: B:34:0x00a6  */
    /* JADX WARN: Removed duplicated region for block: B:37:0x00b7 A[Catch: all -> 0x00cb, TRY_LEAVE, TryCatch #4 {all -> 0x00cb, blocks: (B:35:0x00af, B:37:0x00b7), top: B:63:0x00af }] */
    /* JADX WARN: Removed duplicated region for block: B:39:0x00c6  */
    /* JADX WARN: Removed duplicated region for block: B:45:0x00d6 A[Catch: all -> 0x00de, TRY_ENTER, TryCatch #5 {all -> 0x00de, blocks: (B:27:0x0083, B:29:0x008b, B:45:0x00d6, B:46:0x00dd), top: B:65:0x0083 }] */
    /* JADX WARN: Removed duplicated region for block: B:7:0x0014  */
    /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:34:0x00a6 -> B:63:0x00af). Please report as a decompilation issue!!! */
    @Deprecated(level = DeprecationLevel.HIDDEN, message = "Binary compatibility")
    /*
        Code decompiled incorrectly, please refer to instructions dump.
    */
    public static final /* synthetic */ Object last(ReceiveChannel $this$last, Continuation continuation) throws Throwable {
        C00901 c00901;
        ReceiveChannel $this$consume$iv;
        int i;
        Object objHasNext;
        Throwable cause$iv;
        ChannelIterator iterator;
        Object $result;
        Throwable th;
        ReceiveChannel receiveChannel;
        ChannelIterator iterator2;
        Object obj;
        int i2;
        Object obj2;
        if (continuation instanceof C00901) {
            c00901 = (C00901) continuation;
            if ((c00901.label & Integer.MIN_VALUE) != 0) {
                c00901.label -= Integer.MIN_VALUE;
            } else {
                c00901 = new C00901(continuation);
            }
        }
        C00901 c009012 = c00901;
        Object $result2 = c009012.result;
        Object $result3 = IntrinsicsKt.getCOROUTINE_SUSPENDED();
        switch (c009012.label) {
            case 0:
                ResultKt.throwOnFailure($result2);
                $this$consume$iv = $this$last;
                i = 0;
                try {
                    ChannelIterator iterator3 = $this$consume$iv.iterator();
                    c009012.L$0 = $this$consume$iv;
                    c009012.L$1 = iterator3;
                    c009012.label = 1;
                    objHasNext = iterator3.hasNext(c009012);
                    if (objHasNext == $result3) {
                        return $result3;
                    }
                    cause$iv = null;
                    iterator = iterator3;
                    try {
                        if (((Boolean) objHasNext).booleanValue()) {
                            throw new NoSuchElementException("ReceiveChannel is empty.");
                        }
                        int i3 = i;
                        ReceiveChannel $this$consume$iv2 = $this$consume$iv;
                        int i4 = i3;
                        Throwable th2 = cause$iv;
                        ChannelIterator iterator4 = iterator;
                        Object last = iterator.next();
                        Throwable cause$iv2 = th2;
                        try {
                            c009012.L$0 = $this$consume$iv2;
                            c009012.L$1 = iterator4;
                            c009012.L$2 = last;
                            c009012.label = 2;
                            Object objHasNext2 = iterator4.hasNext(c009012);
                            if (objHasNext2 != $result3) {
                                return $result3;
                            }
                            Object obj3 = $result3;
                            $result = $result2;
                            $result2 = objHasNext2;
                            th = cause$iv2;
                            receiveChannel = $this$consume$iv2;
                            iterator2 = iterator4;
                            obj = last;
                            i2 = i4;
                            obj2 = obj3;
                            try {
                                if (((Boolean) $result2).booleanValue()) {
                                    ChannelsKt.cancelConsumed(receiveChannel, th);
                                    return obj;
                                }
                                ReceiveChannel $this$consume$iv3 = receiveChannel;
                                cause$iv2 = th;
                                int i5 = i2;
                                last = iterator2.next();
                                $result2 = $result;
                                $result3 = obj2;
                                i4 = i5;
                                ChannelIterator channelIterator = iterator2;
                                $this$consume$iv2 = $this$consume$iv3;
                                iterator4 = channelIterator;
                                c009012.L$0 = $this$consume$iv2;
                                c009012.L$1 = iterator4;
                                c009012.L$2 = last;
                                c009012.label = 2;
                                Object objHasNext22 = iterator4.hasNext(c009012);
                                if (objHasNext22 != $result3) {
                                }
                            } catch (Throwable th3) {
                                $this$consume$iv = receiveChannel;
                                e$iv = th3;
                                Throwable cause$iv3 = e$iv;
                                try {
                                    throw e$iv;
                                } catch (Throwable e$iv) {
                                    ChannelsKt.cancelConsumed($this$consume$iv, cause$iv3);
                                    throw e$iv;
                                }
                            }
                        } catch (Throwable th4) {
                            e$iv = th4;
                            $this$consume$iv = $this$consume$iv2;
                            Throwable cause$iv32 = e$iv;
                            throw e$iv;
                        }
                    } catch (Throwable th5) {
                        e$iv = th5;
                        Throwable cause$iv322 = e$iv;
                        throw e$iv;
                    }
                } catch (Throwable th6) {
                    e$iv = th6;
                    Throwable cause$iv3222 = e$iv;
                    throw e$iv;
                }
                break;
            case 1:
                iterator = (ChannelIterator) c009012.L$1;
                cause$iv = null;
                ReceiveChannel $this$consume$iv4 = (ReceiveChannel) c009012.L$0;
                try {
                    ResultKt.throwOnFailure($result2);
                    objHasNext = $result2;
                    i = 0;
                    $this$consume$iv = $this$consume$iv4;
                    if (((Boolean) objHasNext).booleanValue()) {
                    }
                } catch (Throwable th7) {
                    e$iv = th7;
                    $this$consume$iv = $this$consume$iv4;
                    Throwable cause$iv32222 = e$iv;
                    throw e$iv;
                }
                break;
            case 2:
                Object last2 = c009012.L$2;
                ChannelIterator iterator5 = (ChannelIterator) c009012.L$1;
                ReceiveChannel receiveChannel2 = (ReceiveChannel) c009012.L$0;
                try {
                    ResultKt.throwOnFailure($result2);
                    th = null;
                    receiveChannel = receiveChannel2;
                    iterator2 = iterator5;
                    obj = last2;
                    i2 = 0;
                    obj2 = $result3;
                    $result = $result2;
                    if (((Boolean) $result2).booleanValue()) {
                    }
                } catch (Throwable th8) {
                    e$iv = th8;
                    $this$consume$iv = receiveChannel2;
                    Throwable cause$iv322222 = e$iv;
                    throw e$iv;
                }
                break;
            default:
                throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
        }
    }

    /* JADX WARN: Removed duplicated region for block: B:22:0x0093 A[RETURN] */
    /* JADX WARN: Removed duplicated region for block: B:23:0x0094  */
    /* JADX WARN: Removed duplicated region for block: B:26:0x00aa A[Catch: all -> 0x00de, TryCatch #3 {all -> 0x00de, blocks: (B:24:0x00a2, B:26:0x00aa, B:28:0x00b6, B:29:0x00ba, B:30:0x00cf), top: B:48:0x00a2 }] */
    /* JADX WARN: Removed duplicated region for block: B:30:0x00cf A[Catch: all -> 0x00de, TRY_LEAVE, TryCatch #3 {all -> 0x00de, blocks: (B:24:0x00a2, B:26:0x00aa, B:28:0x00b6, B:29:0x00ba, B:30:0x00cf), top: B:48:0x00a2 }] */
    /* JADX WARN: Removed duplicated region for block: B:7:0x0016  */
    /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:23:0x0094 -> B:48:0x00a2). Please report as a decompilation issue!!! */
    @Deprecated(level = DeprecationLevel.HIDDEN, message = "Binary compatibility")
    /*
        Code decompiled incorrectly, please refer to instructions dump.
    */
    public static final /* synthetic */ Object lastIndexOf(ReceiveChannel $this$consumeEach$iv, Object obj, Continuation continuation) throws Throwable {
        C00911 c00911;
        ReceiveChannel $this$consume$iv$iv;
        Object $result;
        Object element;
        Ref.IntRef lastIndex;
        Ref.IntRef lastIndex2;
        ReceiveChannel $this$consume$iv$iv2;
        Throwable cause$iv$iv;
        ChannelIterator channelIterator;
        ReceiveChannel receiveChannel;
        int $i$f$consume;
        int $i$f$consume2;
        if (continuation instanceof C00911) {
            c00911 = (C00911) continuation;
            if ((c00911.label & Integer.MIN_VALUE) != 0) {
                c00911.label -= Integer.MIN_VALUE;
            } else {
                c00911 = new C00911(continuation);
            }
        }
        Object e$iv = c00911.result;
        Object coroutine_suspended = IntrinsicsKt.getCOROUTINE_SUSPENDED();
        int i = 1;
        switch (c00911.label) {
            case 0:
                ResultKt.throwOnFailure(e$iv);
                Object element2 = obj;
                Ref.IntRef index = new Ref.IntRef();
                index.element = -1;
                Ref.IntRef index2 = new Ref.IntRef();
                $this$consume$iv$iv = $this$consumeEach$iv;
                Throwable cause$iv$iv2 = null;
                try {
                    ReceiveChannel $this$consumeEach_u24lambda_u2d1$iv = null;
                    ChannelIterator it = $this$consume$iv$iv.iterator();
                    int $i$f$consumeEach = 0;
                    int $i$f$consume3 = 0;
                    c00911.L$0 = element2;
                    c00911.L$1 = index;
                    c00911.L$2 = index2;
                    c00911.L$3 = $this$consume$iv$iv;
                    c00911.L$4 = it;
                    c00911.label = i;
                    Object objHasNext = it.hasNext(c00911);
                    if (objHasNext != coroutine_suspended) {
                        return coroutine_suspended;
                    }
                    int i2 = $i$f$consume3;
                    $result = e$iv;
                    e$iv = objHasNext;
                    element = element2;
                    lastIndex = index;
                    lastIndex2 = index2;
                    $this$consume$iv$iv2 = $this$consume$iv$iv;
                    cause$iv$iv = cause$iv$iv2;
                    channelIterator = it;
                    receiveChannel = $this$consumeEach_u24lambda_u2d1$iv;
                    $i$f$consume = $i$f$consumeEach;
                    $i$f$consume2 = i2;
                    try {
                        if (!((Boolean) e$iv).booleanValue()) {
                            Unit unit = Unit.INSTANCE;
                            ChannelsKt.cancelConsumed($this$consume$iv$iv2, cause$iv$iv);
                            return Boxing.boxInt(lastIndex.element);
                        }
                        if (Intrinsics.areEqual(element, channelIterator.next())) {
                            lastIndex.element = lastIndex2.element;
                        }
                        lastIndex2.element++;
                        e$iv = $result;
                        $i$f$consume3 = $i$f$consume2;
                        $i$f$consumeEach = $i$f$consume;
                        $this$consumeEach_u24lambda_u2d1$iv = receiveChannel;
                        it = channelIterator;
                        cause$iv$iv2 = cause$iv$iv;
                        $this$consume$iv$iv = $this$consume$iv$iv2;
                        index2 = lastIndex2;
                        index = lastIndex;
                        element2 = element;
                        i = 1;
                        c00911.L$0 = element2;
                        c00911.L$1 = index;
                        c00911.L$2 = index2;
                        c00911.L$3 = $this$consume$iv$iv;
                        c00911.L$4 = it;
                        c00911.label = i;
                        Object objHasNext2 = it.hasNext(c00911);
                        if (objHasNext2 != coroutine_suspended) {
                        }
                    } catch (Throwable th) {
                        e$iv$iv = th;
                        $this$consume$iv$iv = $this$consume$iv$iv2;
                        Throwable cause$iv$iv3 = e$iv$iv;
                        try {
                            throw e$iv$iv;
                        } catch (Throwable e$iv$iv) {
                            ChannelsKt.cancelConsumed($this$consume$iv$iv, cause$iv$iv3);
                            throw e$iv$iv;
                        }
                    }
                } catch (Throwable th2) {
                    e$iv$iv = th2;
                    Throwable cause$iv$iv32 = e$iv$iv;
                    throw e$iv$iv;
                }
                break;
            case 1:
                ChannelIterator channelIterator2 = (ChannelIterator) c00911.L$4;
                $this$consume$iv$iv = (ReceiveChannel) c00911.L$3;
                Ref.IntRef index3 = (Ref.IntRef) c00911.L$2;
                Ref.IntRef lastIndex3 = (Ref.IntRef) c00911.L$1;
                Object element3 = c00911.L$0;
                try {
                    ResultKt.throwOnFailure(e$iv);
                    element = element3;
                    lastIndex = lastIndex3;
                    lastIndex2 = index3;
                    $this$consume$iv$iv2 = $this$consume$iv$iv;
                    cause$iv$iv = null;
                    channelIterator = channelIterator2;
                    receiveChannel = null;
                    $i$f$consume = 0;
                    $i$f$consume2 = 0;
                    $result = e$iv;
                    if (!((Boolean) e$iv).booleanValue()) {
                    }
                } catch (Throwable th3) {
                    e$iv$iv = th3;
                    Throwable cause$iv$iv322 = e$iv$iv;
                    throw e$iv$iv;
                }
                break;
            default:
                throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
        }
    }

    /* JADX WARN: Removed duplicated region for block: B:30:0x0088  */
    /* JADX WARN: Removed duplicated region for block: B:32:0x008c  */
    /* JADX WARN: Removed duplicated region for block: B:37:0x00a4 A[RETURN] */
    /* JADX WARN: Removed duplicated region for block: B:38:0x00a5  */
    /* JADX WARN: Removed duplicated region for block: B:41:0x00b6 A[Catch: all -> 0x00ca, TRY_LEAVE, TryCatch #5 {all -> 0x00ca, blocks: (B:39:0x00ae, B:41:0x00b6), top: B:70:0x00ae }] */
    /* JADX WARN: Removed duplicated region for block: B:43:0x00c5  */
    /* JADX WARN: Removed duplicated region for block: B:7:0x0014  */
    /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:38:0x00a5 -> B:70:0x00ae). Please report as a decompilation issue!!! */
    @Deprecated(level = DeprecationLevel.HIDDEN, message = "Binary compatibility")
    /*
        Code decompiled incorrectly, please refer to instructions dump.
    */
    public static final /* synthetic */ Object lastOrNull(ReceiveChannel $this$consume$iv, Continuation continuation) throws Throwable {
        C00921 c00921;
        int $i$f$consume;
        Throwable cause$iv;
        ReceiveChannel $this$consume$iv2;
        Throwable e$iv;
        ChannelIterator iterator;
        Object objHasNext;
        ReceiveChannel receiveChannel;
        Object $result;
        ReceiveChannel $this$consume$iv3;
        ChannelIterator iterator2;
        Object obj;
        Throwable cause$iv2;
        int $i$f$consume2;
        Object obj2;
        if (continuation instanceof C00921) {
            c00921 = (C00921) continuation;
            if ((c00921.label & Integer.MIN_VALUE) != 0) {
                c00921.label -= Integer.MIN_VALUE;
            } else {
                c00921 = new C00921(continuation);
            }
        }
        C00921 c009212 = c00921;
        Object $result2 = c009212.result;
        Object $result3 = IntrinsicsKt.getCOROUTINE_SUSPENDED();
        switch (c009212.label) {
            case 0:
                ResultKt.throwOnFailure($result2);
                $i$f$consume = 0;
                cause$iv = null;
                try {
                    iterator = $this$consume$iv.iterator();
                    c009212.L$0 = $this$consume$iv;
                    c009212.L$1 = iterator;
                    c009212.label = 1;
                    objHasNext = iterator.hasNext(c009212);
                    if (objHasNext == $result3) {
                        return $result3;
                    }
                    receiveChannel = $this$consume$iv;
                    try {
                        if (((Boolean) objHasNext).booleanValue()) {
                            ChannelsKt.cancelConsumed(receiveChannel, cause$iv);
                            return null;
                        }
                        ReceiveChannel $this$consume$iv4 = receiveChannel;
                        try {
                            ReceiveChannel $this$consume$iv5 = $this$consume$iv4;
                            Throwable cause$iv3 = cause$iv;
                            Object last = iterator.next();
                            try {
                                c009212.L$0 = $this$consume$iv5;
                                c009212.L$1 = iterator;
                                c009212.L$2 = last;
                                c009212.label = 2;
                                Object objHasNext2 = iterator.hasNext(c009212);
                                if (objHasNext2 != $result3) {
                                    return $result3;
                                }
                                Object obj3 = $result3;
                                $result = $result2;
                                $result2 = objHasNext2;
                                $this$consume$iv3 = $this$consume$iv5;
                                iterator2 = iterator;
                                obj = last;
                                cause$iv2 = cause$iv3;
                                $i$f$consume2 = $i$f$consume;
                                obj2 = obj3;
                                try {
                                    if (((Boolean) $result2).booleanValue()) {
                                        ChannelsKt.cancelConsumed($this$consume$iv3, cause$iv2);
                                        return obj;
                                    }
                                    Throwable th = cause$iv2;
                                    last = iterator2.next();
                                    $result2 = $result;
                                    $result3 = obj2;
                                    $i$f$consume = $i$f$consume2;
                                    cause$iv3 = th;
                                    ChannelIterator channelIterator = iterator2;
                                    $this$consume$iv5 = $this$consume$iv3;
                                    iterator = channelIterator;
                                    c009212.L$0 = $this$consume$iv5;
                                    c009212.L$1 = iterator;
                                    c009212.L$2 = last;
                                    c009212.label = 2;
                                    Object objHasNext22 = iterator.hasNext(c009212);
                                    if (objHasNext22 != $result3) {
                                    }
                                } catch (Throwable th2) {
                                    e$iv = th2;
                                    $this$consume$iv2 = $this$consume$iv3;
                                    Throwable cause$iv4 = e$iv;
                                    try {
                                        throw e$iv;
                                    } catch (Throwable e$iv2) {
                                        ChannelsKt.cancelConsumed($this$consume$iv2, cause$iv4);
                                        throw e$iv2;
                                    }
                                }
                            } catch (Throwable th3) {
                                e$iv = th3;
                                $this$consume$iv2 = $this$consume$iv5;
                                Throwable cause$iv42 = e$iv;
                                throw e$iv;
                            }
                        } catch (Throwable th4) {
                            e$iv = th4;
                            $this$consume$iv2 = $this$consume$iv4;
                            Throwable cause$iv422 = e$iv;
                            throw e$iv;
                        }
                    } catch (Throwable th5) {
                        e$iv = th5;
                        $this$consume$iv2 = receiveChannel;
                    }
                } catch (Throwable th6) {
                    $this$consume$iv2 = $this$consume$iv;
                    e$iv = th6;
                    Throwable cause$iv4222 = e$iv;
                    throw e$iv;
                }
                break;
            case 1:
                $i$f$consume = 0;
                ChannelIterator iterator3 = (ChannelIterator) c009212.L$1;
                ReceiveChannel receiveChannel2 = (ReceiveChannel) c009212.L$0;
                try {
                    ResultKt.throwOnFailure($result2);
                    objHasNext = $result2;
                    receiveChannel = receiveChannel2;
                    iterator = iterator3;
                    cause$iv = null;
                    if (((Boolean) objHasNext).booleanValue()) {
                    }
                } catch (Throwable th7) {
                    e$iv = th7;
                    $this$consume$iv2 = receiveChannel2;
                    Throwable cause$iv42222 = e$iv;
                    throw e$iv;
                }
                break;
            case 2:
                Object last2 = c009212.L$2;
                ChannelIterator iterator4 = (ChannelIterator) c009212.L$1;
                ReceiveChannel receiveChannel3 = (ReceiveChannel) c009212.L$0;
                try {
                    ResultKt.throwOnFailure($result2);
                    $this$consume$iv3 = receiveChannel3;
                    iterator2 = iterator4;
                    obj = last2;
                    cause$iv2 = null;
                    $i$f$consume2 = 0;
                    obj2 = $result3;
                    $result = $result2;
                    if (((Boolean) $result2).booleanValue()) {
                    }
                } catch (Throwable th8) {
                    e$iv = th8;
                    $this$consume$iv2 = receiveChannel3;
                    Throwable cause$iv422222 = e$iv;
                    throw e$iv;
                }
                break;
            default:
                throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
        }
    }

    /* JADX WARN: Removed duplicated region for block: B:29:0x0080 A[Catch: all -> 0x0051, TRY_LEAVE, TryCatch #0 {all -> 0x0051, blocks: (B:18:0x004c, B:27:0x0078, B:29:0x0080, B:39:0x00aa, B:40:0x00b1), top: B:47:0x004c }] */
    /* JADX WARN: Removed duplicated region for block: B:35:0x009e  */
    /* JADX WARN: Removed duplicated region for block: B:37:0x00a2 A[Catch: all -> 0x003c, TRY_ENTER, TryCatch #3 {all -> 0x003c, blocks: (B:13:0x0036, B:33:0x0095, B:37:0x00a2, B:38:0x00a9), top: B:52:0x0036 }] */
    /* JADX WARN: Removed duplicated region for block: B:39:0x00aa A[Catch: all -> 0x0051, TRY_ENTER, TryCatch #0 {all -> 0x0051, blocks: (B:18:0x004c, B:27:0x0078, B:29:0x0080, B:39:0x00aa, B:40:0x00b1), top: B:47:0x004c }] */
    /* JADX WARN: Removed duplicated region for block: B:7:0x0014  */
    @Deprecated(level = DeprecationLevel.HIDDEN, message = "Binary compatibility")
    /*
        Code decompiled incorrectly, please refer to instructions dump.
    */
    public static final /* synthetic */ Object single(ReceiveChannel $this$single, Continuation continuation) throws Throwable {
        C00991 c00991;
        ReceiveChannel $this$consume$iv;
        Object objHasNext;
        ReceiveChannel $this$consume$iv2;
        ReceiveChannel $this$consume$iv3;
        Throwable cause$iv;
        ChannelIterator iterator;
        Object objHasNext2;
        ReceiveChannel receiveChannel;
        Object obj;
        if (continuation instanceof C00991) {
            c00991 = (C00991) continuation;
            if ((c00991.label & Integer.MIN_VALUE) != 0) {
                c00991.label -= Integer.MIN_VALUE;
            } else {
                c00991 = new C00991(continuation);
            }
        }
        C00991 c009912 = c00991;
        Object $result = c009912.result;
        Object coroutine_suspended = IntrinsicsKt.getCOROUTINE_SUSPENDED();
        switch (c009912.label) {
            case 0:
                ResultKt.throwOnFailure($result);
                $this$consume$iv = $this$single;
                try {
                    ChannelIterator iterator2 = $this$consume$iv.iterator();
                    c009912.L$0 = $this$consume$iv;
                    c009912.L$1 = iterator2;
                    c009912.label = 1;
                    objHasNext = iterator2.hasNext(c009912);
                    if (objHasNext == coroutine_suspended) {
                        return coroutine_suspended;
                    }
                    $this$consume$iv2 = $this$consume$iv;
                    $this$consume$iv3 = null;
                    cause$iv = null;
                    iterator = iterator2;
                    if (((Boolean) objHasNext).booleanValue()) {
                        throw new NoSuchElementException("ReceiveChannel is empty.");
                    }
                    Object single = iterator.next();
                    c009912.L$0 = $this$consume$iv2;
                    c009912.L$1 = single;
                    c009912.label = 2;
                    objHasNext2 = iterator.hasNext(c009912);
                    if (objHasNext2 == coroutine_suspended) {
                        return coroutine_suspended;
                    }
                    receiveChannel = $this$consume$iv2;
                    obj = single;
                    if (!((Boolean) objHasNext2).booleanValue()) {
                        throw new IllegalArgumentException("ReceiveChannel has more than one element.");
                    }
                    ChannelsKt.cancelConsumed(receiveChannel, cause$iv);
                    return obj;
                } catch (Throwable th) {
                    e$iv = th;
                    Throwable cause$iv2 = e$iv;
                    try {
                        throw e$iv;
                    } catch (Throwable e$iv) {
                        ChannelsKt.cancelConsumed($this$consume$iv, cause$iv2);
                        throw e$iv;
                    }
                }
            case 1:
                $this$consume$iv3 = null;
                iterator = (ChannelIterator) c009912.L$1;
                cause$iv = null;
                $this$consume$iv2 = (ReceiveChannel) c009912.L$0;
                try {
                    ResultKt.throwOnFailure($result);
                    objHasNext = $result;
                    if (((Boolean) objHasNext).booleanValue()) {
                    }
                } catch (Throwable th2) {
                    e$iv = th2;
                    $this$consume$iv = $this$consume$iv2;
                    Throwable cause$iv22 = e$iv;
                    throw e$iv;
                }
                break;
            case 2:
                obj = c009912.L$1;
                receiveChannel = (ReceiveChannel) c009912.L$0;
                cause$iv = null;
                try {
                    ResultKt.throwOnFailure($result);
                    objHasNext2 = $result;
                    if (!((Boolean) objHasNext2).booleanValue()) {
                    }
                } catch (Throwable th3) {
                    e$iv = th3;
                    $this$consume$iv = receiveChannel;
                    Throwable cause$iv222 = e$iv;
                    throw e$iv;
                }
                break;
            default:
                throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
        }
    }

    /* JADX WARN: Removed duplicated region for block: B:29:0x0087  */
    /* JADX WARN: Removed duplicated region for block: B:37:0x00aa  */
    /* JADX WARN: Removed duplicated region for block: B:39:0x00ae  */
    /* JADX WARN: Removed duplicated region for block: B:60:0x008b A[EXC_TOP_SPLITTER, SYNTHETIC] */
    /* JADX WARN: Removed duplicated region for block: B:7:0x0014  */
    @Deprecated(level = DeprecationLevel.HIDDEN, message = "Binary compatibility")
    /*
        Code decompiled incorrectly, please refer to instructions dump.
    */
    public static final /* synthetic */ Object singleOrNull(ReceiveChannel $this$singleOrNull, Continuation continuation) throws Throwable {
        C01001 c01001;
        ReceiveChannel $this$consume$iv;
        Throwable cause$iv;
        Throwable e$iv;
        ChannelIterator iterator;
        Object objHasNext;
        ReceiveChannel $this$consume$iv2;
        ReceiveChannel $this$consume$iv3;
        Object objHasNext2;
        Throwable th;
        ReceiveChannel receiveChannel;
        Object obj;
        if (continuation instanceof C01001) {
            c01001 = (C01001) continuation;
            if ((c01001.label & Integer.MIN_VALUE) != 0) {
                c01001.label -= Integer.MIN_VALUE;
            } else {
                c01001 = new C01001(continuation);
            }
        }
        C01001 c010012 = c01001;
        Object $result = c010012.result;
        Object coroutine_suspended = IntrinsicsKt.getCOROUTINE_SUSPENDED();
        switch (c010012.label) {
            case 0:
                ResultKt.throwOnFailure($result);
                $this$consume$iv = $this$singleOrNull;
                cause$iv = null;
                try {
                    iterator = $this$consume$iv.iterator();
                    c010012.L$0 = $this$consume$iv;
                    c010012.L$1 = iterator;
                    c010012.label = 1;
                    objHasNext = iterator.hasNext(c010012);
                    if (objHasNext == coroutine_suspended) {
                        return coroutine_suspended;
                    }
                    $this$consume$iv2 = null;
                    $this$consume$iv3 = $this$consume$iv;
                    try {
                        if (((Boolean) objHasNext).booleanValue()) {
                            ChannelsKt.cancelConsumed($this$consume$iv3, cause$iv);
                            return null;
                        }
                        try {
                            Object single = iterator.next();
                            c010012.L$0 = $this$consume$iv3;
                            c010012.L$1 = single;
                            c010012.label = 2;
                            objHasNext2 = iterator.hasNext(c010012);
                            if (objHasNext2 == coroutine_suspended) {
                                return coroutine_suspended;
                            }
                            th = cause$iv;
                            receiveChannel = $this$consume$iv3;
                            obj = single;
                            try {
                                if (((Boolean) objHasNext2).booleanValue()) {
                                    ChannelsKt.cancelConsumed(receiveChannel, th);
                                    return obj;
                                }
                                ChannelsKt.cancelConsumed(receiveChannel, th);
                                return null;
                            } catch (Throwable th2) {
                                e$iv = th2;
                                $this$consume$iv = receiveChannel;
                                Throwable cause$iv2 = e$iv;
                                try {
                                    throw e$iv;
                                } catch (Throwable e$iv2) {
                                    ChannelsKt.cancelConsumed($this$consume$iv, cause$iv2);
                                    throw e$iv2;
                                }
                            }
                        } catch (Throwable th3) {
                            e$iv = th3;
                            $this$consume$iv = $this$consume$iv3;
                            Throwable cause$iv22 = e$iv;
                            throw e$iv;
                        }
                    } catch (Throwable th4) {
                        e$iv = th4;
                        $this$consume$iv = $this$consume$iv3;
                    }
                } catch (Throwable th5) {
                    e$iv = th5;
                    Throwable cause$iv222 = e$iv;
                    throw e$iv;
                }
                break;
            case 1:
                ChannelIterator iterator2 = (ChannelIterator) c010012.L$1;
                ReceiveChannel receiveChannel2 = (ReceiveChannel) c010012.L$0;
                try {
                    ResultKt.throwOnFailure($result);
                    objHasNext = $result;
                    $this$consume$iv3 = receiveChannel2;
                    iterator = iterator2;
                    cause$iv = null;
                    $this$consume$iv2 = null;
                    if (((Boolean) objHasNext).booleanValue()) {
                    }
                } catch (Throwable th6) {
                    $this$consume$iv = receiveChannel2;
                    e$iv = th6;
                    Throwable cause$iv2222 = e$iv;
                    throw e$iv;
                }
                break;
            case 2:
                obj = c010012.L$1;
                receiveChannel = (ReceiveChannel) c010012.L$0;
                try {
                    ResultKt.throwOnFailure($result);
                    objHasNext2 = $result;
                    th = null;
                    if (((Boolean) objHasNext2).booleanValue()) {
                    }
                } catch (Throwable th7) {
                    e$iv = th7;
                    $this$consume$iv = receiveChannel;
                    Throwable cause$iv22222 = e$iv;
                    throw e$iv;
                }
                break;
            default:
                throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
        }
    }

    /* JADX INFO: Add missing generic type declarations: [E] */
    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$drop$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(d1 = {"\u0000\f\n\u0000\n\u0002\u0010\u0002\n\u0000\n\u0002\u0018\u0002\u0010\u0000\u001a\u00020\u0001\"\u0004\b\u0000\u0010\u0002*\b\u0012\u0004\u0012\u0002H\u00020\u0003H\u008a@"}, d2 = {"<anonymous>", "", "E", "Lkotlinx/coroutines/channels/ProducerScope;"}, k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$drop$1", f = "Deprecated.kt", i = {0, 0, 1, 2}, l = {164, 169, 170}, m = "invokeSuspend", n = {"$this$produce", "remaining", "$this$produce", "$this$produce"}, s = {"L$0", "I$0", "L$0", "L$0"})
    static final class C00771<E> extends SuspendLambda implements Function2<ProducerScope<? super E>, Continuation<? super Unit>, Object> {
        final /* synthetic */ int $n;
        final /* synthetic */ ReceiveChannel<E> $this_drop;
        int I$0;
        private /* synthetic */ Object L$0;
        Object L$1;
        int label;

        /* JADX WARN: 'super' call moved to the top of the method (can break code semantics) */
        /* JADX WARN: Multi-variable type inference failed */
        C00771(int i, ReceiveChannel<? extends E> receiveChannel, Continuation<? super C00771> continuation) {
            super(2, continuation);
            this.$n = i;
            this.$this_drop = receiveChannel;
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Continuation<Unit> create(Object obj, Continuation<?> continuation) {
            C00771 c00771 = new C00771(this.$n, this.$this_drop, continuation);
            c00771.L$0 = obj;
            return c00771;
        }

        @Override // kotlin.jvm.functions.Function2
        public final Object invoke(ProducerScope<? super E> producerScope, Continuation<? super Unit> continuation) {
            return ((C00771) create(producerScope, continuation)).invokeSuspend(Unit.INSTANCE);
        }

        /* JADX WARN: Removed duplicated region for block: B:19:0x0078 A[RETURN] */
        /* JADX WARN: Removed duplicated region for block: B:20:0x0079  */
        /* JADX WARN: Removed duplicated region for block: B:23:0x0089  */
        /* JADX WARN: Removed duplicated region for block: B:31:0x00b4 A[RETURN] */
        /* JADX WARN: Removed duplicated region for block: B:32:0x00b5  */
        /* JADX WARN: Removed duplicated region for block: B:35:0x00c4  */
        /* JADX WARN: Removed duplicated region for block: B:40:0x00df  */
        /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:20:0x0079 -> B:21:0x0081). Please report as a decompilation issue!!! */
        /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:38:0x00d9 -> B:29:0x00a4). Please report as a decompilation issue!!! */
        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        /*
            Code decompiled incorrectly, please refer to instructions dump.
        */
        public final Object invokeSuspend(Object $result) throws Throwable {
            C00771<E> c00771;
            ProducerScope $this$produce;
            ChannelIterator<E> it;
            Object $result2;
            ProducerScope $this$produce2;
            ChannelIterator<E> channelIterator;
            int remaining;
            C00771<E> c007712;
            Object obj;
            ProducerScope $this$produce3;
            ChannelIterator<E> channelIterator2;
            C00771<E> c007713;
            Object obj2;
            Object $result3;
            Object objHasNext;
            Object $result4 = IntrinsicsKt.getCOROUTINE_SUSPENDED();
            switch (this.label) {
                case 0:
                    ResultKt.throwOnFailure($result);
                    c00771 = this;
                    $this$produce = (ProducerScope) c00771.L$0;
                    int i = c00771.$n;
                    if (!(i >= 0)) {
                        throw new IllegalArgumentException(("Requested element count " + i + " is less than zero.").toString());
                    }
                    int remaining2 = c00771.$n;
                    if (remaining2 > 0) {
                        ProducerScope $this$produce4 = $this$produce;
                        int remaining3 = remaining2;
                        ChannelIterator<E> it2 = c00771.$this_drop.iterator();
                        c00771.L$0 = $this$produce4;
                        c00771.L$1 = it2;
                        c00771.I$0 = remaining3;
                        c00771.label = 1;
                        Object objHasNext2 = it2.hasNext(c00771);
                        if (objHasNext2 != $result4) {
                            return $result4;
                        }
                        Object obj3 = $result4;
                        $result2 = $result;
                        $result = objHasNext2;
                        $this$produce2 = $this$produce4;
                        channelIterator = it2;
                        remaining = remaining3;
                        c007712 = c00771;
                        obj = obj3;
                        if (((Boolean) $result).booleanValue()) {
                            channelIterator.next();
                            int remaining4 = remaining - 1;
                            if (remaining4 != 0) {
                                it2 = channelIterator;
                                $this$produce4 = $this$produce2;
                                C00771<E> c007714 = c007712;
                                remaining3 = remaining4;
                                $result = $result2;
                                $result4 = obj;
                                c00771 = c007714;
                                c00771.L$0 = $this$produce4;
                                c00771.L$1 = it2;
                                c00771.I$0 = remaining3;
                                c00771.label = 1;
                                Object objHasNext22 = it2.hasNext(c00771);
                                if (objHasNext22 != $result4) {
                                }
                            }
                        }
                        $result = $result2;
                        $result4 = obj;
                        c00771 = c007712;
                        $this$produce = $this$produce2;
                    }
                    it = c00771.$this_drop.iterator();
                    c00771.L$0 = $this$produce;
                    c00771.L$1 = it;
                    c00771.label = 2;
                    objHasNext = it.hasNext(c00771);
                    if (objHasNext != $result4) {
                        return $result4;
                    }
                    Object obj4 = $result4;
                    $result3 = $result;
                    $result = objHasNext;
                    $this$produce3 = $this$produce;
                    channelIterator2 = it;
                    c007713 = c00771;
                    obj2 = obj4;
                    if (((Boolean) $result).booleanValue()) {
                        return Unit.INSTANCE;
                    }
                    c007713.L$0 = $this$produce3;
                    c007713.L$1 = channelIterator2;
                    c007713.label = 3;
                    Object e = $this$produce3.send(channelIterator2.next(), c007713);
                    if (e == obj2) {
                        return obj2;
                    }
                    $result = $result3;
                    $result4 = obj2;
                    c00771 = c007713;
                    it = channelIterator2;
                    $this$produce = $this$produce3;
                    c00771.L$0 = $this$produce;
                    c00771.L$1 = it;
                    c00771.label = 2;
                    objHasNext = it.hasNext(c00771);
                    if (objHasNext != $result4) {
                    }
                    break;
                case 1:
                    int remaining5 = this.I$0;
                    ChannelIterator<E> channelIterator3 = (ChannelIterator) this.L$1;
                    ProducerScope $this$produce5 = (ProducerScope) this.L$0;
                    ResultKt.throwOnFailure($result);
                    $this$produce2 = $this$produce5;
                    channelIterator = channelIterator3;
                    remaining = remaining5;
                    c007712 = this;
                    obj = $result4;
                    $result2 = $result;
                    if (((Boolean) $result).booleanValue()) {
                    }
                    $result = $result2;
                    $result4 = obj;
                    c00771 = c007712;
                    $this$produce = $this$produce2;
                    it = c00771.$this_drop.iterator();
                    c00771.L$0 = $this$produce;
                    c00771.L$1 = it;
                    c00771.label = 2;
                    objHasNext = it.hasNext(c00771);
                    if (objHasNext != $result4) {
                    }
                    break;
                case 2:
                    ChannelIterator<E> channelIterator4 = (ChannelIterator) this.L$1;
                    ProducerScope $this$produce6 = (ProducerScope) this.L$0;
                    ResultKt.throwOnFailure($result);
                    $this$produce3 = $this$produce6;
                    channelIterator2 = channelIterator4;
                    c007713 = this;
                    obj2 = $result4;
                    $result3 = $result;
                    if (((Boolean) $result).booleanValue()) {
                    }
                    break;
                case 3:
                    c00771 = this;
                    it = (ChannelIterator) c00771.L$1;
                    $this$produce = (ProducerScope) c00771.L$0;
                    ResultKt.throwOnFailure($result);
                    c00771.L$0 = $this$produce;
                    c00771.L$1 = it;
                    c00771.label = 2;
                    objHasNext = it.hasNext(c00771);
                    if (objHasNext != $result4) {
                    }
                    break;
                default:
                    throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
            }
        }
    }

    public static /* synthetic */ ReceiveChannel drop$default(ReceiveChannel receiveChannel, int i, CoroutineContext coroutineContext, int i2, Object obj) {
        if ((i2 & 2) != 0) {
            coroutineContext = Dispatchers.getUnconfined();
        }
        return drop(receiveChannel, i, coroutineContext);
    }

    @Deprecated(level = DeprecationLevel.HIDDEN, message = "Binary compatibility")
    public static final /* synthetic */ ReceiveChannel drop(ReceiveChannel $this$drop, int n, CoroutineContext context) {
        return ProduceKt.produce(GlobalScope.INSTANCE, (6 & 1) != 0 ? EmptyCoroutineContext.INSTANCE : context, (6 & 2) != 0 ? 0 : 0, (6 & 4) != 0 ? CoroutineStart.DEFAULT : null, (6 & 8) != 0 ? null : ChannelsKt.consumes($this$drop), new C00771(n, $this$drop, null));
    }

    public static /* synthetic */ ReceiveChannel dropWhile$default(ReceiveChannel receiveChannel, CoroutineContext coroutineContext, Function2 function2, int i, Object obj) {
        if ((i & 1) != 0) {
            coroutineContext = Dispatchers.getUnconfined();
        }
        return dropWhile(receiveChannel, coroutineContext, function2);
    }

    /* JADX INFO: Add missing generic type declarations: [E] */
    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$dropWhile$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(d1 = {"\u0000\f\n\u0000\n\u0002\u0010\u0002\n\u0000\n\u0002\u0018\u0002\u0010\u0000\u001a\u00020\u0001\"\u0004\b\u0000\u0010\u0002*\b\u0012\u0004\u0012\u0002H\u00020\u0003H\u008a@"}, d2 = {"<anonymous>", "", "E", "Lkotlinx/coroutines/channels/ProducerScope;"}, k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$dropWhile$1", f = "Deprecated.kt", i = {0, 1, 1, 2, 3, 4}, l = {181, 182, 183, 187, 188}, m = "invokeSuspend", n = {"$this$produce", "$this$produce", "e", "$this$produce", "$this$produce", "$this$produce"}, s = {"L$0", "L$0", "L$2", "L$0", "L$0", "L$0"})
    static final class C00781<E> extends SuspendLambda implements Function2<ProducerScope<? super E>, Continuation<? super Unit>, Object> {
        final /* synthetic */ Function2<E, Continuation<? super Boolean>, Object> $predicate;
        final /* synthetic */ ReceiveChannel<E> $this_dropWhile;
        private /* synthetic */ Object L$0;
        Object L$1;
        Object L$2;
        int label;

        /* JADX WARN: 'super' call moved to the top of the method (can break code semantics) */
        /* JADX WARN: Multi-variable type inference failed */
        C00781(ReceiveChannel<? extends E> receiveChannel, Function2<? super E, ? super Continuation<? super Boolean>, ? extends Object> function2, Continuation<? super C00781> continuation) {
            super(2, continuation);
            this.$this_dropWhile = receiveChannel;
            this.$predicate = function2;
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Continuation<Unit> create(Object obj, Continuation<?> continuation) {
            C00781 c00781 = new C00781(this.$this_dropWhile, this.$predicate, continuation);
            c00781.L$0 = obj;
            return c00781;
        }

        @Override // kotlin.jvm.functions.Function2
        public final Object invoke(ProducerScope<? super E> producerScope, Continuation<? super Unit> continuation) {
            return ((C00781) create(producerScope, continuation)).invokeSuspend(Unit.INSTANCE);
        }

        /* JADX WARN: Multi-variable type inference failed */
        /* JADX WARN: Removed duplicated region for block: B:14:0x0088 A[RETURN] */
        /* JADX WARN: Removed duplicated region for block: B:15:0x0089  */
        /* JADX WARN: Removed duplicated region for block: B:18:0x0097  */
        /* JADX WARN: Removed duplicated region for block: B:24:0x00b9  */
        /* JADX WARN: Removed duplicated region for block: B:29:0x00d5  */
        /* JADX WARN: Removed duplicated region for block: B:33:0x00f1 A[RETURN] */
        /* JADX WARN: Removed duplicated region for block: B:34:0x00f2  */
        /* JADX WARN: Removed duplicated region for block: B:37:0x00fd  */
        /* JADX WARN: Removed duplicated region for block: B:42:0x0114  */
        /* JADX WARN: Type inference failed for: r2v15 */
        /* JADX WARN: Type inference failed for: r2v2 */
        /* JADX WARN: Type inference failed for: r2v3 */
        /* JADX WARN: Type inference failed for: r3v22 */
        /* JADX WARN: Type inference failed for: r3v6, types: [java.lang.Object] */
        /* JADX WARN: Type inference failed for: r3v9 */
        /* JADX WARN: Type inference failed for: r5v0 */
        /* JADX WARN: Type inference failed for: r5v1, types: [java.lang.Object] */
        /* JADX WARN: Type inference failed for: r5v10 */
        /* JADX WARN: Type inference failed for: r5v11 */
        /* JADX WARN: Type inference failed for: r5v12, types: [java.lang.Object, kotlinx.coroutines.channels.ProducerScope] */
        /* JADX WARN: Type inference failed for: r5v13, types: [java.lang.Object] */
        /* JADX WARN: Type inference failed for: r5v14 */
        /* JADX WARN: Type inference failed for: r5v15 */
        /* JADX WARN: Type inference failed for: r5v16 */
        /* JADX WARN: Type inference failed for: r5v17 */
        /* JADX WARN: Type inference failed for: r5v18 */
        /* JADX WARN: Type inference failed for: r5v19 */
        /* JADX WARN: Type inference failed for: r5v20 */
        /* JADX WARN: Type inference failed for: r5v5 */
        /* JADX WARN: Type inference failed for: r5v6, types: [java.lang.Object, kotlinx.coroutines.channels.ProducerScope] */
        /* JADX WARN: Type inference failed for: r5v9 */
        /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:21:0x00ad -> B:22:0x00b1). Please report as a decompilation issue!!! */
        /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:40:0x0112 -> B:31:0x00e1). Please report as a decompilation issue!!! */
        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        /*
            Code decompiled incorrectly, please refer to instructions dump.
        */
        public final Object invokeSuspend(Object obj) throws Throwable {
            ChannelIterator<E> it;
            ?? r5;
            C00781<E> c00781;
            Object obj2;
            Object obj3;
            Object obj4;
            ChannelIterator<E> channelIterator;
            ?? r52;
            C00781<E> c007812;
            ?? r2;
            ?? r53;
            ChannelIterator<E> it2;
            ChannelIterator<E> channelIterator2;
            ?? r54;
            ?? r55;
            Object objHasNext;
            Object coroutine_suspended = IntrinsicsKt.getCOROUTINE_SUSPENDED();
            switch (this.label) {
                case 0:
                    ResultKt.throwOnFailure(obj);
                    C00781<E> c007813 = this;
                    ProducerScope producerScope = (ProducerScope) c007813.L$0;
                    it = c007813.$this_dropWhile.iterator();
                    ?? r3 = producerScope;
                    c007813.L$0 = r3;
                    c007813.L$1 = it;
                    c007813.L$2 = null;
                    c007813.label = 1;
                    Object objHasNext2 = it.hasNext(c007813);
                    if (objHasNext2 != coroutine_suspended) {
                        return coroutine_suspended;
                    }
                    Object obj5 = coroutine_suspended;
                    obj3 = obj;
                    obj = objHasNext2;
                    r5 = r3;
                    c00781 = c007813;
                    obj2 = obj5;
                    r53 = r5;
                    if (((Boolean) obj).booleanValue()) {
                        E next = it.next();
                        Function2<E, Continuation<? super Boolean>, Object> function2 = c00781.$predicate;
                        c00781.L$0 = r5;
                        c00781.L$1 = it;
                        c00781.L$2 = next;
                        c00781.label = 2;
                        Object objInvoke = function2.invoke(next, c00781);
                        if (objInvoke == obj2) {
                            return obj2;
                        }
                        ChannelIterator<E> channelIterator3 = it;
                        obj4 = next;
                        obj = objInvoke;
                        channelIterator = channelIterator3;
                        r52 = r5;
                        if (((Boolean) obj).booleanValue()) {
                            c00781.L$0 = r52;
                            c00781.L$1 = null;
                            c00781.L$2 = null;
                            c00781.label = 3;
                            if (r52.send(obj4, c00781) == obj2) {
                                return obj2;
                            }
                            obj = obj3;
                            coroutine_suspended = obj2;
                            c007812 = c00781;
                            r2 = r52;
                            c00781 = c007812;
                            r53 = r2;
                            obj2 = coroutine_suspended;
                        } else {
                            obj = obj3;
                            coroutine_suspended = obj2;
                            c007813 = c00781;
                            r3 = r52;
                            it = channelIterator;
                            c007813.L$0 = r3;
                            c007813.L$1 = it;
                            c007813.L$2 = null;
                            c007813.label = 1;
                            Object objHasNext22 = it.hasNext(c007813);
                            if (objHasNext22 != coroutine_suspended) {
                            }
                        }
                    }
                    it2 = c00781.$this_dropWhile.iterator();
                    r55 = r53;
                    c00781.L$0 = r55;
                    c00781.L$1 = it2;
                    c00781.label = 4;
                    objHasNext = it2.hasNext(c00781);
                    if (objHasNext == obj2) {
                        return obj2;
                    }
                    channelIterator2 = it2;
                    obj = objHasNext;
                    r54 = r55;
                    if (((Boolean) obj).booleanValue()) {
                        return Unit.INSTANCE;
                    }
                    c00781.L$0 = r54;
                    c00781.L$1 = channelIterator2;
                    c00781.label = 5;
                    if (r54.send(channelIterator2.next(), c00781) == obj2) {
                        return obj2;
                    }
                    it2 = channelIterator2;
                    r55 = r54;
                    c00781.L$0 = r55;
                    c00781.L$1 = it2;
                    c00781.label = 4;
                    objHasNext = it2.hasNext(c00781);
                    if (objHasNext == obj2) {
                    }
                    break;
                case 1:
                    ChannelIterator<E> channelIterator4 = (ChannelIterator) this.L$1;
                    ProducerScope producerScope2 = (ProducerScope) this.L$0;
                    ResultKt.throwOnFailure(obj);
                    r5 = producerScope2;
                    it = channelIterator4;
                    c00781 = this;
                    obj2 = coroutine_suspended;
                    obj3 = obj;
                    r53 = r5;
                    if (((Boolean) obj).booleanValue()) {
                    }
                    it2 = c00781.$this_dropWhile.iterator();
                    r55 = r53;
                    c00781.L$0 = r55;
                    c00781.L$1 = it2;
                    c00781.label = 4;
                    objHasNext = it2.hasNext(c00781);
                    if (objHasNext == obj2) {
                    }
                    break;
                case 2:
                    Object obj6 = this.L$2;
                    ChannelIterator<E> channelIterator5 = (ChannelIterator) this.L$1;
                    ProducerScope producerScope3 = (ProducerScope) this.L$0;
                    ResultKt.throwOnFailure(obj);
                    channelIterator = channelIterator5;
                    obj4 = obj6;
                    c00781 = this;
                    obj2 = coroutine_suspended;
                    obj3 = obj;
                    r52 = producerScope3;
                    if (((Boolean) obj).booleanValue()) {
                    }
                    break;
                case 3:
                    c007812 = this;
                    ProducerScope producerScope4 = (ProducerScope) c007812.L$0;
                    ResultKt.throwOnFailure(obj);
                    r2 = producerScope4;
                    c00781 = c007812;
                    r53 = r2;
                    obj2 = coroutine_suspended;
                    it2 = c00781.$this_dropWhile.iterator();
                    r55 = r53;
                    c00781.L$0 = r55;
                    c00781.L$1 = it2;
                    c00781.label = 4;
                    objHasNext = it2.hasNext(c00781);
                    if (objHasNext == obj2) {
                    }
                    break;
                case 4:
                    channelIterator2 = (ChannelIterator) this.L$1;
                    ProducerScope producerScope5 = (ProducerScope) this.L$0;
                    ResultKt.throwOnFailure(obj);
                    r54 = producerScope5;
                    c00781 = this;
                    obj2 = coroutine_suspended;
                    if (((Boolean) obj).booleanValue()) {
                    }
                    break;
                case 5:
                    ChannelIterator<E> channelIterator6 = (ChannelIterator) this.L$1;
                    ProducerScope producerScope6 = (ProducerScope) this.L$0;
                    ResultKt.throwOnFailure(obj);
                    r55 = producerScope6;
                    c00781 = this;
                    obj2 = coroutine_suspended;
                    it2 = channelIterator6;
                    c00781.L$0 = r55;
                    c00781.L$1 = it2;
                    c00781.label = 4;
                    objHasNext = it2.hasNext(c00781);
                    if (objHasNext == obj2) {
                    }
                    break;
                default:
                    throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
            }
        }
    }

    @Deprecated(level = DeprecationLevel.HIDDEN, message = "Binary compatibility")
    public static final /* synthetic */ ReceiveChannel dropWhile(ReceiveChannel $this$dropWhile, CoroutineContext context, Function2 predicate) {
        return ProduceKt.produce(GlobalScope.INSTANCE, (6 & 1) != 0 ? EmptyCoroutineContext.INSTANCE : context, (6 & 2) != 0 ? 0 : 0, (6 & 4) != 0 ? CoroutineStart.DEFAULT : null, (6 & 8) != 0 ? null : ChannelsKt.consumes($this$dropWhile), new C00781($this$dropWhile, predicate, null));
    }

    public static /* synthetic */ ReceiveChannel filter$default(ReceiveChannel receiveChannel, CoroutineContext coroutineContext, Function2 function2, int i, Object obj) {
        if ((i & 1) != 0) {
            coroutineContext = Dispatchers.getUnconfined();
        }
        return ChannelsKt.filter(receiveChannel, coroutineContext, function2);
    }

    /* JADX INFO: Add missing generic type declarations: [E] */
    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$filter$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(d1 = {"\u0000\f\n\u0000\n\u0002\u0010\u0002\n\u0000\n\u0002\u0018\u0002\u0010\u0000\u001a\u00020\u0001\"\u0004\b\u0000\u0010\u0002*\b\u0012\u0004\u0012\u0002H\u00020\u0003H\u008a@"}, d2 = {"<anonymous>", "", "E", "Lkotlinx/coroutines/channels/ProducerScope;"}, k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$filter$1", f = "Deprecated.kt", i = {0, 1, 1, 2}, l = {198, 199, 199}, m = "invokeSuspend", n = {"$this$produce", "$this$produce", "e", "$this$produce"}, s = {"L$0", "L$0", "L$2", "L$0"})
    static final class C00811<E> extends SuspendLambda implements Function2<ProducerScope<? super E>, Continuation<? super Unit>, Object> {
        final /* synthetic */ Function2<E, Continuation<? super Boolean>, Object> $predicate;
        final /* synthetic */ ReceiveChannel<E> $this_filter;
        private /* synthetic */ Object L$0;
        Object L$1;
        Object L$2;
        int label;

        /* JADX WARN: 'super' call moved to the top of the method (can break code semantics) */
        /* JADX WARN: Multi-variable type inference failed */
        C00811(ReceiveChannel<? extends E> receiveChannel, Function2<? super E, ? super Continuation<? super Boolean>, ? extends Object> function2, Continuation<? super C00811> continuation) {
            super(2, continuation);
            this.$this_filter = receiveChannel;
            this.$predicate = function2;
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Continuation<Unit> create(Object obj, Continuation<?> continuation) {
            C00811 c00811 = new C00811(this.$this_filter, this.$predicate, continuation);
            c00811.L$0 = obj;
            return c00811;
        }

        @Override // kotlin.jvm.functions.Function2
        public final Object invoke(ProducerScope<? super E> producerScope, Continuation<? super Unit> continuation) {
            return ((C00811) create(producerScope, continuation)).invokeSuspend(Unit.INSTANCE);
        }

        /* JADX WARN: Multi-variable type inference failed */
        /* JADX WARN: Removed duplicated region for block: B:12:0x006b A[RETURN] */
        /* JADX WARN: Removed duplicated region for block: B:13:0x006c  */
        /* JADX WARN: Removed duplicated region for block: B:16:0x007b  */
        /* JADX WARN: Removed duplicated region for block: B:22:0x009e  */
        /* JADX WARN: Removed duplicated region for block: B:27:0x00b7  */
        /* JADX WARN: Removed duplicated region for block: B:28:0x00bd  */
        /* JADX WARN: Type inference failed for: r4v10 */
        /* JADX WARN: Type inference failed for: r4v11 */
        /* JADX WARN: Type inference failed for: r4v14, types: [java.lang.Object] */
        /* JADX WARN: Type inference failed for: r4v18 */
        /* JADX WARN: Type inference failed for: r4v2 */
        /* JADX WARN: Type inference failed for: r5v0 */
        /* JADX WARN: Type inference failed for: r5v10 */
        /* JADX WARN: Type inference failed for: r5v5, types: [java.lang.Object] */
        /* JADX WARN: Type inference failed for: r6v0 */
        /* JADX WARN: Type inference failed for: r6v1, types: [java.lang.Object, kotlinx.coroutines.channels.ProducerScope] */
        /* JADX WARN: Type inference failed for: r6v4 */
        /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:25:0x00b1 -> B:10:0x0059). Please report as a decompilation issue!!! */
        /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:27:0x00b7 -> B:10:0x0059). Please report as a decompilation issue!!! */
        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        /*
            Code decompiled incorrectly, please refer to instructions dump.
        */
        public final Object invokeSuspend(Object obj) throws Throwable {
            C00811<E> c00811;
            ?? r4;
            ChannelIterator<E> it;
            ?? r5;
            ChannelIterator<E> channelIterator;
            C00811<E> c008112;
            Object obj2;
            Object obj3;
            ?? r6;
            ChannelIterator<E> channelIterator2;
            Object obj4;
            Object objHasNext;
            Object coroutine_suspended = IntrinsicsKt.getCOROUTINE_SUSPENDED();
            switch (this.label) {
                case 0:
                    ResultKt.throwOnFailure(obj);
                    c00811 = this;
                    ProducerScope producerScope = (ProducerScope) c00811.L$0;
                    r4 = producerScope;
                    it = c00811.$this_filter.iterator();
                    c00811.L$0 = r4;
                    c00811.L$1 = it;
                    c00811.L$2 = null;
                    c00811.label = 1;
                    objHasNext = it.hasNext(c00811);
                    if (objHasNext != coroutine_suspended) {
                        return coroutine_suspended;
                    }
                    Object obj5 = coroutine_suspended;
                    obj3 = obj;
                    obj = objHasNext;
                    r5 = r4;
                    channelIterator = it;
                    c008112 = c00811;
                    obj2 = obj5;
                    if (((Boolean) obj).booleanValue()) {
                        return Unit.INSTANCE;
                    }
                    E next = channelIterator.next();
                    Function2<E, Continuation<? super Boolean>, Object> function2 = c008112.$predicate;
                    c008112.L$0 = r5;
                    c008112.L$1 = channelIterator;
                    c008112.L$2 = next;
                    c008112.label = 2;
                    Object objInvoke = function2.invoke(next, c008112);
                    if (objInvoke == obj2) {
                        return obj2;
                    }
                    ChannelIterator<E> channelIterator3 = channelIterator;
                    obj4 = next;
                    obj = objInvoke;
                    r6 = r5;
                    channelIterator2 = channelIterator3;
                    if (((Boolean) obj).booleanValue()) {
                        obj = obj3;
                        coroutine_suspended = obj2;
                        c00811 = c008112;
                        it = channelIterator2;
                        r4 = r6;
                    } else {
                        c008112.L$0 = r6;
                        c008112.L$1 = channelIterator2;
                        c008112.L$2 = null;
                        c008112.label = 3;
                        if (r6.send(obj4, c008112) == obj2) {
                            return obj2;
                        }
                        obj = obj3;
                        coroutine_suspended = obj2;
                        c00811 = c008112;
                        it = channelIterator2;
                        r4 = r6;
                    }
                    c00811.L$0 = r4;
                    c00811.L$1 = it;
                    c00811.L$2 = null;
                    c00811.label = 1;
                    objHasNext = it.hasNext(c00811);
                    if (objHasNext != coroutine_suspended) {
                    }
                    break;
                case 1:
                    ChannelIterator<E> channelIterator4 = (ChannelIterator) this.L$1;
                    ProducerScope producerScope2 = (ProducerScope) this.L$0;
                    ResultKt.throwOnFailure(obj);
                    r5 = producerScope2;
                    channelIterator = channelIterator4;
                    c008112 = this;
                    obj2 = coroutine_suspended;
                    obj3 = obj;
                    if (((Boolean) obj).booleanValue()) {
                    }
                    break;
                case 2:
                    Object obj6 = this.L$2;
                    ChannelIterator<E> channelIterator5 = (ChannelIterator) this.L$1;
                    ProducerScope producerScope3 = (ProducerScope) this.L$0;
                    ResultKt.throwOnFailure(obj);
                    r6 = producerScope3;
                    channelIterator2 = channelIterator5;
                    obj4 = obj6;
                    c008112 = this;
                    obj2 = coroutine_suspended;
                    obj3 = obj;
                    if (((Boolean) obj).booleanValue()) {
                    }
                    c00811.L$0 = r4;
                    c00811.L$1 = it;
                    c00811.L$2 = null;
                    c00811.label = 1;
                    objHasNext = it.hasNext(c00811);
                    if (objHasNext != coroutine_suspended) {
                    }
                    break;
                case 3:
                    c00811 = this;
                    it = (ChannelIterator) c00811.L$1;
                    ProducerScope producerScope4 = (ProducerScope) c00811.L$0;
                    ResultKt.throwOnFailure(obj);
                    r4 = producerScope4;
                    c00811.L$0 = r4;
                    c00811.L$1 = it;
                    c00811.L$2 = null;
                    c00811.label = 1;
                    objHasNext = it.hasNext(c00811);
                    if (objHasNext != coroutine_suspended) {
                    }
                    break;
                default:
                    throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
            }
        }
    }

    public static final <E> ReceiveChannel<E> filter(ReceiveChannel<? extends E> receiveChannel, CoroutineContext context, Function2<? super E, ? super Continuation<? super Boolean>, ? extends Object> function2) {
        return ProduceKt.produce(GlobalScope.INSTANCE, (6 & 1) != 0 ? EmptyCoroutineContext.INSTANCE : context, (6 & 2) != 0 ? 0 : 0, (6 & 4) != 0 ? CoroutineStart.DEFAULT : null, (6 & 8) != 0 ? null : ChannelsKt.consumes(receiveChannel), new C00811(receiveChannel, function2, null));
    }

    public static /* synthetic */ ReceiveChannel filterIndexed$default(ReceiveChannel receiveChannel, CoroutineContext coroutineContext, Function3 function3, int i, Object obj) {
        if ((i & 1) != 0) {
            coroutineContext = Dispatchers.getUnconfined();
        }
        return filterIndexed(receiveChannel, coroutineContext, function3);
    }

    /* JADX INFO: Add missing generic type declarations: [E] */
    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$filterIndexed$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(d1 = {"\u0000\f\n\u0000\n\u0002\u0010\u0002\n\u0000\n\u0002\u0018\u0002\u0010\u0000\u001a\u00020\u0001\"\u0004\b\u0000\u0010\u0002*\b\u0012\u0004\u0012\u0002H\u00020\u0003H\u008a@"}, d2 = {"<anonymous>", "", "E", "Lkotlinx/coroutines/channels/ProducerScope;"}, k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$filterIndexed$1", f = "Deprecated.kt", i = {0, 0, 1, 1, 1, 2, 2}, l = {211, 212, 212}, m = "invokeSuspend", n = {"$this$produce", "index", "$this$produce", "e", "index", "$this$produce", "index"}, s = {"L$0", "I$0", "L$0", "L$2", "I$0", "L$0", "I$0"})
    static final class C00821<E> extends SuspendLambda implements Function2<ProducerScope<? super E>, Continuation<? super Unit>, Object> {
        final /* synthetic */ Function3<Integer, E, Continuation<? super Boolean>, Object> $predicate;
        final /* synthetic */ ReceiveChannel<E> $this_filterIndexed;
        int I$0;
        private /* synthetic */ Object L$0;
        Object L$1;
        Object L$2;
        int label;

        /* JADX WARN: 'super' call moved to the top of the method (can break code semantics) */
        /* JADX WARN: Multi-variable type inference failed */
        C00821(ReceiveChannel<? extends E> receiveChannel, Function3<? super Integer, ? super E, ? super Continuation<? super Boolean>, ? extends Object> function3, Continuation<? super C00821> continuation) {
            super(2, continuation);
            this.$this_filterIndexed = receiveChannel;
            this.$predicate = function3;
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Continuation<Unit> create(Object obj, Continuation<?> continuation) {
            C00821 c00821 = new C00821(this.$this_filterIndexed, this.$predicate, continuation);
            c00821.L$0 = obj;
            return c00821;
        }

        @Override // kotlin.jvm.functions.Function2
        public final Object invoke(ProducerScope<? super E> producerScope, Continuation<? super Unit> continuation) {
            return ((C00821) create(producerScope, continuation)).invokeSuspend(Unit.INSTANCE);
        }

        /* JADX WARN: Multi-variable type inference failed */
        /* JADX WARN: Removed duplicated region for block: B:12:0x0074 A[RETURN] */
        /* JADX WARN: Removed duplicated region for block: B:13:0x0075  */
        /* JADX WARN: Removed duplicated region for block: B:16:0x0085  */
        /* JADX WARN: Removed duplicated region for block: B:22:0x00ae  */
        /* JADX WARN: Removed duplicated region for block: B:27:0x00ca  */
        /* JADX WARN: Removed duplicated region for block: B:28:0x00d1  */
        /* JADX WARN: Type inference failed for: r5v10 */
        /* JADX WARN: Type inference failed for: r5v12, types: [java.lang.Object] */
        /* JADX WARN: Type inference failed for: r5v16 */
        /* JADX WARN: Type inference failed for: r5v2 */
        /* JADX WARN: Type inference failed for: r5v9 */
        /* JADX WARN: Type inference failed for: r6v0 */
        /* JADX WARN: Type inference failed for: r6v10 */
        /* JADX WARN: Type inference failed for: r6v3, types: [java.lang.Object, kotlinx.coroutines.channels.ProducerScope] */
        /* JADX WARN: Type inference failed for: r6v4, types: [java.lang.Object] */
        /* JADX WARN: Type inference failed for: r6v8 */
        /* JADX WARN: Type inference failed for: r6v9 */
        /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:25:0x00c3 -> B:10:0x0060). Please report as a decompilation issue!!! */
        /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:27:0x00ca -> B:10:0x0060). Please report as a decompilation issue!!! */
        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        /*
            Code decompiled incorrectly, please refer to instructions dump.
        */
        public final Object invokeSuspend(Object obj) throws Throwable {
            C00821<E> c00821;
            ?? r5;
            int i;
            ChannelIterator<E> it;
            ?? r6;
            ChannelIterator<E> channelIterator;
            int i2;
            C00821<E> c008212;
            Object obj2;
            Object obj3;
            Object obj4;
            int i3;
            ?? r62;
            Object objHasNext;
            Object coroutine_suspended = IntrinsicsKt.getCOROUTINE_SUSPENDED();
            switch (this.label) {
                case 0:
                    ResultKt.throwOnFailure(obj);
                    c00821 = this;
                    r5 = (ProducerScope) c00821.L$0;
                    i = 0;
                    it = c00821.$this_filterIndexed.iterator();
                    c00821.L$0 = r5;
                    c00821.L$1 = it;
                    c00821.L$2 = null;
                    c00821.I$0 = i;
                    c00821.label = 1;
                    objHasNext = it.hasNext(c00821);
                    if (objHasNext != coroutine_suspended) {
                        return coroutine_suspended;
                    }
                    Object obj5 = coroutine_suspended;
                    obj3 = obj;
                    obj = objHasNext;
                    r6 = r5;
                    channelIterator = it;
                    i2 = i;
                    c008212 = c00821;
                    obj2 = obj5;
                    if (((Boolean) obj).booleanValue()) {
                        return Unit.INSTANCE;
                    }
                    E next = channelIterator.next();
                    Function3<Integer, E, Continuation<? super Boolean>, Object> function3 = c008212.$predicate;
                    i3 = i2 + 1;
                    Integer numBoxInt = Boxing.boxInt(i2);
                    c008212.L$0 = r6;
                    c008212.L$1 = channelIterator;
                    c008212.L$2 = next;
                    c008212.I$0 = i3;
                    c008212.label = 2;
                    Object objInvoke = function3.invoke(numBoxInt, next, c008212);
                    if (objInvoke == obj2) {
                        return obj2;
                    }
                    obj4 = next;
                    obj = objInvoke;
                    r62 = r6;
                    if (((Boolean) obj).booleanValue()) {
                        obj = obj3;
                        coroutine_suspended = obj2;
                        c00821 = c008212;
                        it = channelIterator;
                        r5 = r62;
                        i = i3;
                    } else {
                        c008212.L$0 = r62;
                        c008212.L$1 = channelIterator;
                        c008212.L$2 = null;
                        c008212.I$0 = i3;
                        c008212.label = 3;
                        if (r62.send(obj4, c008212) == obj2) {
                            return obj2;
                        }
                        obj = obj3;
                        coroutine_suspended = obj2;
                        c00821 = c008212;
                        it = channelIterator;
                        r5 = r62;
                        i = i3;
                    }
                    c00821.L$0 = r5;
                    c00821.L$1 = it;
                    c00821.L$2 = null;
                    c00821.I$0 = i;
                    c00821.label = 1;
                    objHasNext = it.hasNext(c00821);
                    if (objHasNext != coroutine_suspended) {
                    }
                    break;
                case 1:
                    int i4 = this.I$0;
                    ChannelIterator<E> channelIterator2 = (ChannelIterator) this.L$1;
                    ProducerScope producerScope = (ProducerScope) this.L$0;
                    ResultKt.throwOnFailure(obj);
                    r6 = producerScope;
                    channelIterator = channelIterator2;
                    i2 = i4;
                    c008212 = this;
                    obj2 = coroutine_suspended;
                    obj3 = obj;
                    if (((Boolean) obj).booleanValue()) {
                    }
                    break;
                case 2:
                    int i5 = this.I$0;
                    obj4 = this.L$2;
                    channelIterator = (ChannelIterator) this.L$1;
                    ProducerScope producerScope2 = (ProducerScope) this.L$0;
                    ResultKt.throwOnFailure(obj);
                    i3 = i5;
                    c008212 = this;
                    obj2 = coroutine_suspended;
                    obj3 = obj;
                    r62 = producerScope2;
                    if (((Boolean) obj).booleanValue()) {
                    }
                    c00821.L$0 = r5;
                    c00821.L$1 = it;
                    c00821.L$2 = null;
                    c00821.I$0 = i;
                    c00821.label = 1;
                    objHasNext = it.hasNext(c00821);
                    if (objHasNext != coroutine_suspended) {
                    }
                    break;
                case 3:
                    c00821 = this;
                    i = c00821.I$0;
                    it = (ChannelIterator) c00821.L$1;
                    ProducerScope producerScope3 = (ProducerScope) c00821.L$0;
                    ResultKt.throwOnFailure(obj);
                    r5 = producerScope3;
                    c00821.L$0 = r5;
                    c00821.L$1 = it;
                    c00821.L$2 = null;
                    c00821.I$0 = i;
                    c00821.label = 1;
                    objHasNext = it.hasNext(c00821);
                    if (objHasNext != coroutine_suspended) {
                    }
                    break;
                default:
                    throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
            }
        }
    }

    @Deprecated(level = DeprecationLevel.HIDDEN, message = "Binary compatibility")
    public static final /* synthetic */ ReceiveChannel filterIndexed(ReceiveChannel $this$filterIndexed, CoroutineContext context, Function3 predicate) {
        return ProduceKt.produce(GlobalScope.INSTANCE, (6 & 1) != 0 ? EmptyCoroutineContext.INSTANCE : context, (6 & 2) != 0 ? 0 : 0, (6 & 4) != 0 ? CoroutineStart.DEFAULT : null, (6 & 8) != 0 ? null : ChannelsKt.consumes($this$filterIndexed), new C00821($this$filterIndexed, predicate, null));
    }

    public static /* synthetic */ ReceiveChannel filterNot$default(ReceiveChannel receiveChannel, CoroutineContext coroutineContext, Function2 function2, int i, Object obj) {
        if ((i & 1) != 0) {
            coroutineContext = Dispatchers.getUnconfined();
        }
        return filterNot(receiveChannel, coroutineContext, function2);
    }

    /* JADX INFO: Add missing generic type declarations: [E] */
    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$filterNot$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(d1 = {"\u0000\n\n\u0000\n\u0002\u0010\u000b\n\u0002\b\u0002\u0010\u0000\u001a\u00020\u0001\"\u0004\b\u0000\u0010\u00022\u0006\u0010\u0003\u001a\u0002H\u0002H\u008a@"}, d2 = {"<anonymous>", "", "E", "it"}, k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$filterNot$1", f = "Deprecated.kt", i = {}, l = {222}, m = "invokeSuspend", n = {}, s = {})
    static final class C00831<E> extends SuspendLambda implements Function2<E, Continuation<? super Boolean>, Object> {
        final /* synthetic */ Function2<E, Continuation<? super Boolean>, Object> $predicate;
        /* synthetic */ Object L$0;
        int label;

        /* JADX WARN: 'super' call moved to the top of the method (can break code semantics) */
        /* JADX WARN: Multi-variable type inference failed */
        C00831(Function2<? super E, ? super Continuation<? super Boolean>, ? extends Object> function2, Continuation<? super C00831> continuation) {
            super(2, continuation);
            this.$predicate = function2;
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Continuation<Unit> create(Object obj, Continuation<?> continuation) {
            C00831 c00831 = new C00831(this.$predicate, continuation);
            c00831.L$0 = obj;
            return c00831;
        }

        @Override // kotlin.jvm.functions.Function2
        public final Object invoke(E e, Continuation<? super Boolean> continuation) {
            return ((C00831) create(e, continuation)).invokeSuspend(Unit.INSTANCE);
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Object invokeSuspend(Object obj) throws Throwable {
            Object coroutine_suspended = IntrinsicsKt.getCOROUTINE_SUSPENDED();
            switch (this.label) {
                case 0:
                    ResultKt.throwOnFailure(obj);
                    Object obj2 = this.L$0;
                    Function2<E, Continuation<? super Boolean>, Object> function2 = this.$predicate;
                    this.label = 1;
                    Object objInvoke = function2.invoke(obj2, this);
                    if (objInvoke == coroutine_suspended) {
                        return coroutine_suspended;
                    }
                    obj = objInvoke;
                    break;
                case 1:
                    ResultKt.throwOnFailure(obj);
                    break;
                default:
                    throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
            }
            return Boxing.boxBoolean(!((Boolean) obj).booleanValue());
        }
    }

    @Deprecated(level = DeprecationLevel.HIDDEN, message = "Binary compatibility")
    public static final /* synthetic */ ReceiveChannel filterNot(ReceiveChannel $this$filterNot, CoroutineContext context, Function2 predicate) {
        return ChannelsKt.filter($this$filterNot, context, new C00831(predicate, null));
    }

    /* JADX INFO: Add missing generic type declarations: [E] */
    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$filterNotNull$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(d1 = {"\u0000\u000e\n\u0000\n\u0002\u0010\u000b\n\u0000\n\u0002\u0010\u0000\n\u0000\u0010\u0000\u001a\u00020\u0001\"\b\b\u0000\u0010\u0002*\u00020\u00032\b\u0010\u0004\u001a\u0004\u0018\u0001H\u0002H\u008a@"}, d2 = {"<anonymous>", "", "E", "", "it"}, k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$filterNotNull$1", f = "Deprecated.kt", i = {}, l = {}, m = "invokeSuspend", n = {}, s = {})
    static final class C00841<E> extends SuspendLambda implements Function2<E, Continuation<? super Boolean>, Object> {
        /* synthetic */ Object L$0;
        int label;

        C00841(Continuation<? super C00841> continuation) {
            super(2, continuation);
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Continuation<Unit> create(Object obj, Continuation<?> continuation) {
            C00841 c00841 = new C00841(continuation);
            c00841.L$0 = obj;
            return c00841;
        }

        @Override // kotlin.jvm.functions.Function2
        public final Object invoke(E e, Continuation<? super Boolean> continuation) {
            return ((C00841) create(e, continuation)).invokeSuspend(Unit.INSTANCE);
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Object invokeSuspend(Object obj) throws Throwable {
            IntrinsicsKt.getCOROUTINE_SUSPENDED();
            switch (this.label) {
                case 0:
                    ResultKt.throwOnFailure(obj);
                    Object it = this.L$0;
                    return Boxing.boxBoolean(it != null);
                default:
                    throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
            }
        }
    }

    public static final <E> ReceiveChannel<E> filterNotNull(ReceiveChannel<? extends E> receiveChannel) {
        return filter$default(receiveChannel, null, new C00841(null), 1, null);
    }

    /* JADX WARN: Removed duplicated region for block: B:22:0x0070 A[RETURN] */
    /* JADX WARN: Removed duplicated region for block: B:23:0x0071  */
    /* JADX WARN: Removed duplicated region for block: B:26:0x0082 A[Catch: all -> 0x009e, TryCatch #3 {all -> 0x009e, blocks: (B:24:0x007a, B:26:0x0082, B:28:0x008a, B:30:0x0095), top: B:48:0x007a }] */
    /* JADX WARN: Removed duplicated region for block: B:30:0x0095 A[Catch: all -> 0x009e, TRY_LEAVE, TryCatch #3 {all -> 0x009e, blocks: (B:24:0x007a, B:26:0x0082, B:28:0x008a, B:30:0x0095), top: B:48:0x007a }] */
    /* JADX WARN: Removed duplicated region for block: B:7:0x0014  */
    /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:23:0x0071 -> B:48:0x007a). Please report as a decompilation issue!!! */
    @Deprecated(level = DeprecationLevel.HIDDEN, message = "Binary compatibility")
    /*
        Code decompiled incorrectly, please refer to instructions dump.
    */
    public static final /* synthetic */ Object filterNotNullTo(ReceiveChannel $this$consumeEach$iv, Collection collection, Continuation continuation) throws Throwable {
        C00851 c00851;
        ReceiveChannel $this$consume$iv$iv;
        Throwable cause$iv$iv;
        Object $result;
        Collection destination;
        ReceiveChannel $this$consume$iv$iv2;
        Throwable cause$iv$iv2;
        ChannelIterator channelIterator;
        ReceiveChannel receiveChannel;
        Object obj;
        if (continuation instanceof C00851) {
            c00851 = (C00851) continuation;
            if ((c00851.label & Integer.MIN_VALUE) != 0) {
                c00851.label -= Integer.MIN_VALUE;
            } else {
                c00851 = new C00851(continuation);
            }
        }
        C00851 c008512 = c00851;
        Object e$iv = c008512.result;
        Object $result2 = IntrinsicsKt.getCOROUTINE_SUSPENDED();
        switch (c008512.label) {
            case 0:
                ResultKt.throwOnFailure(e$iv);
                Collection destination2 = collection;
                $this$consume$iv$iv = $this$consumeEach$iv;
                Throwable cause$iv$iv3 = null;
                try {
                    ReceiveChannel $this$consumeEach_u24lambda_u2d1$iv = null;
                    ChannelIterator it = $this$consume$iv$iv.iterator();
                    c008512.L$0 = destination2;
                    c008512.L$1 = $this$consume$iv$iv;
                    c008512.L$2 = it;
                    c008512.label = 1;
                    Object objHasNext = it.hasNext(c008512);
                    if (objHasNext != $result2) {
                        return $result2;
                    }
                    Object obj2 = $result2;
                    $result = e$iv;
                    e$iv = objHasNext;
                    destination = destination2;
                    $this$consume$iv$iv2 = $this$consume$iv$iv;
                    cause$iv$iv2 = cause$iv$iv3;
                    channelIterator = it;
                    receiveChannel = $this$consumeEach_u24lambda_u2d1$iv;
                    obj = obj2;
                    try {
                        if (!((Boolean) e$iv).booleanValue()) {
                            Unit unit = Unit.INSTANCE;
                            return destination;
                        }
                        Object e$iv2 = channelIterator.next();
                        if (e$iv2 != null) {
                            destination.add(e$iv2);
                        }
                        e$iv = $result;
                        $result2 = obj;
                        $this$consumeEach_u24lambda_u2d1$iv = receiveChannel;
                        it = channelIterator;
                        cause$iv$iv3 = cause$iv$iv;
                        $this$consume$iv$iv = $this$consume$iv$iv;
                        destination2 = destination;
                        c008512.L$0 = destination2;
                        c008512.L$1 = $this$consume$iv$iv;
                        c008512.L$2 = it;
                        c008512.label = 1;
                        Object objHasNext2 = it.hasNext(c008512);
                        if (objHasNext2 != $result2) {
                        }
                    } catch (Throwable th) {
                        $this$consume$iv$iv = $this$consume$iv$iv;
                        e$iv$iv = th;
                        cause$iv$iv = e$iv$iv;
                        try {
                            throw e$iv$iv;
                        } finally {
                            ChannelsKt.cancelConsumed($this$consume$iv$iv, cause$iv$iv);
                        }
                    }
                } catch (Throwable th2) {
                    e$iv$iv = th2;
                    cause$iv$iv = e$iv$iv;
                    throw e$iv$iv;
                }
                break;
            case 1:
                ChannelIterator channelIterator2 = (ChannelIterator) c008512.L$2;
                $this$consume$iv$iv = (ReceiveChannel) c008512.L$1;
                Collection destination3 = (Collection) c008512.L$0;
                try {
                    ResultKt.throwOnFailure(e$iv);
                    destination = destination3;
                    $this$consume$iv$iv2 = $this$consume$iv$iv;
                    cause$iv$iv2 = null;
                    channelIterator = channelIterator2;
                    receiveChannel = null;
                    obj = $result2;
                    $result = e$iv;
                    if (!((Boolean) e$iv).booleanValue()) {
                    }
                } catch (Throwable th3) {
                    e$iv$iv = th3;
                    cause$iv$iv = e$iv$iv;
                    throw e$iv$iv;
                }
                break;
            default:
                throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
        }
    }

    /* JADX WARN: Removed duplicated region for block: B:25:0x0083 A[RETURN] */
    /* JADX WARN: Removed duplicated region for block: B:26:0x0084  */
    /* JADX WARN: Removed duplicated region for block: B:29:0x0090 A[Catch: all -> 0x00b9, TryCatch #1 {all -> 0x00b9, blocks: (B:27:0x0088, B:29:0x0090, B:31:0x0097, B:37:0x00b0), top: B:51:0x0088 }] */
    /* JADX WARN: Removed duplicated region for block: B:37:0x00b0 A[Catch: all -> 0x00b9, TRY_LEAVE, TryCatch #1 {all -> 0x00b9, blocks: (B:27:0x0088, B:29:0x0090, B:31:0x0097, B:37:0x00b0), top: B:51:0x0088 }] */
    /* JADX WARN: Removed duplicated region for block: B:7:0x0014  */
    /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:34:0x00a7 -> B:35:0x00ab). Please report as a decompilation issue!!! */
    /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:36:0x00ad -> B:23:0x0074). Please report as a decompilation issue!!! */
    @Deprecated(level = DeprecationLevel.HIDDEN, message = "Binary compatibility")
    /*
        Code decompiled incorrectly, please refer to instructions dump.
    */
    public static final /* synthetic */ Object filterNotNullTo(ReceiveChannel $this$consumeEach$iv, SendChannel sendChannel, Continuation continuation) throws Throwable {
        AnonymousClass3 anonymousClass3;
        ReceiveChannel $this$consume$iv$iv;
        SendChannel destination;
        Throwable cause$iv$iv;
        int i;
        ChannelIterator it;
        Object obj;
        Object $result;
        int i2;
        Object objHasNext;
        if (continuation instanceof AnonymousClass3) {
            anonymousClass3 = (AnonymousClass3) continuation;
            if ((anonymousClass3.label & Integer.MIN_VALUE) != 0) {
                anonymousClass3.label -= Integer.MIN_VALUE;
            } else {
                anonymousClass3 = new AnonymousClass3(continuation);
            }
        }
        AnonymousClass3 anonymousClass32 = anonymousClass3;
        Object $result2 = anonymousClass32.result;
        Object $result3 = IntrinsicsKt.getCOROUTINE_SUSPENDED();
        try {
            switch (anonymousClass32.label) {
                case 0:
                    ResultKt.throwOnFailure($result2);
                    destination = sendChannel;
                    $this$consume$iv$iv = $this$consumeEach$iv;
                    cause$iv$iv = null;
                    i = 0;
                    try {
                        it = $this$consume$iv$iv.iterator();
                        anonymousClass32.L$0 = destination;
                        anonymousClass32.L$1 = $this$consume$iv$iv;
                        anonymousClass32.L$2 = it;
                        anonymousClass32.label = 1;
                        objHasNext = it.hasNext(anonymousClass32);
                        if (objHasNext != $result3) {
                            return $result3;
                        }
                        Object obj2 = $result3;
                        $result = $result2;
                        $result2 = objHasNext;
                        obj = obj2;
                        try {
                            if (!((Boolean) $result2).booleanValue()) {
                                Unit unit = Unit.INSTANCE;
                                return destination;
                            }
                            Object it2 = it.next();
                            if (it2 != null) {
                                anonymousClass32.L$0 = destination;
                                anonymousClass32.L$1 = $this$consume$iv$iv;
                                anonymousClass32.L$2 = it;
                                anonymousClass32.label = 2;
                                if (destination.send(it2, anonymousClass32) == obj) {
                                    return obj;
                                }
                                $result2 = $result;
                                $result3 = obj;
                                i2 = i;
                                i = i2;
                                anonymousClass32.L$0 = destination;
                                anonymousClass32.L$1 = $this$consume$iv$iv;
                                anonymousClass32.L$2 = it;
                                anonymousClass32.label = 1;
                                objHasNext = it.hasNext(anonymousClass32);
                                if (objHasNext != $result3) {
                                }
                            } else {
                                $result2 = $result;
                                $result3 = obj;
                                anonymousClass32.L$0 = destination;
                                anonymousClass32.L$1 = $this$consume$iv$iv;
                                anonymousClass32.L$2 = it;
                                anonymousClass32.label = 1;
                                objHasNext = it.hasNext(anonymousClass32);
                                if (objHasNext != $result3) {
                                }
                            }
                        } catch (Throwable th) {
                            e$iv$iv = th;
                            Throwable cause$iv$iv2 = e$iv$iv;
                            try {
                                throw e$iv$iv;
                            } finally {
                                ChannelsKt.cancelConsumed($this$consume$iv$iv, cause$iv$iv2);
                            }
                        }
                    } catch (Throwable th2) {
                        e$iv$iv = th2;
                        Throwable cause$iv$iv22 = e$iv$iv;
                        throw e$iv$iv;
                    }
                    break;
                case 1:
                    ChannelIterator channelIterator = (ChannelIterator) anonymousClass32.L$2;
                    cause$iv$iv = null;
                    ReceiveChannel $this$consume$iv$iv2 = (ReceiveChannel) anonymousClass32.L$1;
                    $this$consume$iv$iv = $this$consume$iv$iv2;
                    destination = (SendChannel) anonymousClass32.L$0;
                    ResultKt.throwOnFailure($result2);
                    it = channelIterator;
                    i = 0;
                    obj = $result3;
                    $result = $result2;
                    if (!((Boolean) $result2).booleanValue()) {
                    }
                    break;
                case 2:
                    i2 = 0;
                    it = (ChannelIterator) anonymousClass32.L$2;
                    cause$iv$iv = null;
                    $this$consume$iv$iv = (ReceiveChannel) anonymousClass32.L$1;
                    destination = (SendChannel) anonymousClass32.L$0;
                    ResultKt.throwOnFailure($result2);
                    i = i2;
                    anonymousClass32.L$0 = destination;
                    anonymousClass32.L$1 = $this$consume$iv$iv;
                    anonymousClass32.L$2 = it;
                    anonymousClass32.label = 1;
                    objHasNext = it.hasNext(anonymousClass32);
                    if (objHasNext != $result3) {
                    }
                    break;
                default:
                    throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
            }
        } catch (Throwable th3) {
            e$iv$iv = th3;
        }
    }

    /* JADX INFO: Add missing generic type declarations: [E] */
    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$take$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(d1 = {"\u0000\f\n\u0000\n\u0002\u0010\u0002\n\u0000\n\u0002\u0018\u0002\u0010\u0000\u001a\u00020\u0001\"\u0004\b\u0000\u0010\u0002*\b\u0012\u0004\u0012\u0002H\u00020\u0003H\u008a@"}, d2 = {"<anonymous>", "", "E", "Lkotlinx/coroutines/channels/ProducerScope;"}, k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$take$1", f = "Deprecated.kt", i = {0, 0, 1, 1}, l = {254, 255}, m = "invokeSuspend", n = {"$this$produce", "remaining", "$this$produce", "remaining"}, s = {"L$0", "I$0", "L$0", "I$0"})
    static final class C01011<E> extends SuspendLambda implements Function2<ProducerScope<? super E>, Continuation<? super Unit>, Object> {
        final /* synthetic */ int $n;
        final /* synthetic */ ReceiveChannel<E> $this_take;
        int I$0;
        private /* synthetic */ Object L$0;
        Object L$1;
        int label;

        /* JADX WARN: 'super' call moved to the top of the method (can break code semantics) */
        /* JADX WARN: Multi-variable type inference failed */
        C01011(int i, ReceiveChannel<? extends E> receiveChannel, Continuation<? super C01011> continuation) {
            super(2, continuation);
            this.$n = i;
            this.$this_take = receiveChannel;
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Continuation<Unit> create(Object obj, Continuation<?> continuation) {
            C01011 c01011 = new C01011(this.$n, this.$this_take, continuation);
            c01011.L$0 = obj;
            return c01011;
        }

        @Override // kotlin.jvm.functions.Function2
        public final Object invoke(ProducerScope<? super E> producerScope, Continuation<? super Unit> continuation) {
            return ((C01011) create(producerScope, continuation)).invokeSuspend(Unit.INSTANCE);
        }

        /* JADX WARN: Removed duplicated region for block: B:19:0x0066 A[RETURN] */
        /* JADX WARN: Removed duplicated region for block: B:20:0x0067  */
        /* JADX WARN: Removed duplicated region for block: B:23:0x0075  */
        /* JADX WARN: Removed duplicated region for block: B:29:0x0097  */
        /* JADX WARN: Removed duplicated region for block: B:31:0x009a  */
        /* JADX WARN: Removed duplicated region for block: B:32:0x009f  */
        /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:26:0x008c -> B:27:0x0092). Please report as a decompilation issue!!! */
        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        /*
            Code decompiled incorrectly, please refer to instructions dump.
        */
        public final Object invokeSuspend(Object $result) throws Throwable {
            C01011<E> c01011;
            int remaining;
            ChannelIterator<E> it;
            ProducerScope $this$produce;
            C01011<E> c010112;
            Object obj;
            Object $result2;
            int remaining2;
            ChannelIterator<E> channelIterator;
            ProducerScope $this$produce2;
            int remaining3;
            Object $result3 = IntrinsicsKt.getCOROUTINE_SUSPENDED();
            switch (this.label) {
                case 0:
                    ResultKt.throwOnFailure($result);
                    c01011 = this;
                    ProducerScope $this$produce3 = (ProducerScope) c01011.L$0;
                    int i = c01011.$n;
                    if (i == 0) {
                        return Unit.INSTANCE;
                    }
                    if (!(i >= 0)) {
                        throw new IllegalArgumentException(("Requested element count " + i + " is less than zero.").toString());
                    }
                    remaining = c01011.$n;
                    it = c01011.$this_take.iterator();
                    c01011.L$0 = $this$produce3;
                    c01011.L$1 = it;
                    c01011.I$0 = remaining;
                    c01011.label = 1;
                    Object objHasNext = it.hasNext(c01011);
                    if (objHasNext != $result3) {
                        return $result3;
                    }
                    Object obj2 = $result3;
                    $result2 = $result;
                    $result = objHasNext;
                    $this$produce = $this$produce3;
                    c010112 = c01011;
                    obj = obj2;
                    if (((Boolean) $result).booleanValue()) {
                        return Unit.INSTANCE;
                    }
                    c010112.L$0 = $this$produce;
                    c010112.L$1 = it;
                    c010112.I$0 = remaining;
                    c010112.label = 2;
                    Object e = $this$produce.send(it.next(), c010112);
                    if (e == obj) {
                        return obj;
                    }
                    $result = $result2;
                    $result3 = obj;
                    c01011 = c010112;
                    remaining2 = remaining;
                    channelIterator = it;
                    $this$produce2 = $this$produce;
                    remaining3 = remaining2 - 1;
                    if (remaining3 != 0) {
                        return Unit.INSTANCE;
                    }
                    ChannelIterator<E> channelIterator2 = channelIterator;
                    remaining = remaining3;
                    $this$produce3 = $this$produce2;
                    it = channelIterator2;
                    c01011.L$0 = $this$produce3;
                    c01011.L$1 = it;
                    c01011.I$0 = remaining;
                    c01011.label = 1;
                    Object objHasNext2 = it.hasNext(c01011);
                    if (objHasNext2 != $result3) {
                    }
                    break;
                case 1:
                    int remaining4 = this.I$0;
                    ChannelIterator<E> channelIterator3 = (ChannelIterator) this.L$1;
                    ProducerScope $this$produce4 = (ProducerScope) this.L$0;
                    ResultKt.throwOnFailure($result);
                    $this$produce = $this$produce4;
                    it = channelIterator3;
                    remaining = remaining4;
                    c010112 = this;
                    obj = $result3;
                    $result2 = $result;
                    if (((Boolean) $result).booleanValue()) {
                    }
                    break;
                case 2:
                    c01011 = this;
                    remaining2 = c01011.I$0;
                    channelIterator = (ChannelIterator) c01011.L$1;
                    $this$produce2 = (ProducerScope) c01011.L$0;
                    ResultKt.throwOnFailure($result);
                    remaining3 = remaining2 - 1;
                    if (remaining3 != 0) {
                    }
                    break;
                default:
                    throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
            }
        }
    }

    public static /* synthetic */ ReceiveChannel take$default(ReceiveChannel receiveChannel, int i, CoroutineContext coroutineContext, int i2, Object obj) {
        if ((i2 & 2) != 0) {
            coroutineContext = Dispatchers.getUnconfined();
        }
        return take(receiveChannel, i, coroutineContext);
    }

    @Deprecated(level = DeprecationLevel.HIDDEN, message = "Binary compatibility")
    public static final /* synthetic */ ReceiveChannel take(ReceiveChannel $this$take, int n, CoroutineContext context) {
        return ProduceKt.produce(GlobalScope.INSTANCE, (6 & 1) != 0 ? EmptyCoroutineContext.INSTANCE : context, (6 & 2) != 0 ? 0 : 0, (6 & 4) != 0 ? CoroutineStart.DEFAULT : null, (6 & 8) != 0 ? null : ChannelsKt.consumes($this$take), new C01011(n, $this$take, null));
    }

    public static /* synthetic */ ReceiveChannel takeWhile$default(ReceiveChannel receiveChannel, CoroutineContext coroutineContext, Function2 function2, int i, Object obj) {
        if ((i & 1) != 0) {
            coroutineContext = Dispatchers.getUnconfined();
        }
        return takeWhile(receiveChannel, coroutineContext, function2);
    }

    /* JADX INFO: Add missing generic type declarations: [E] */
    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$takeWhile$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(d1 = {"\u0000\f\n\u0000\n\u0002\u0010\u0002\n\u0000\n\u0002\u0018\u0002\u0010\u0000\u001a\u00020\u0001\"\u0004\b\u0000\u0010\u0002*\b\u0012\u0004\u0012\u0002H\u00020\u0003H\u008a@"}, d2 = {"<anonymous>", "", "E", "Lkotlinx/coroutines/channels/ProducerScope;"}, k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$takeWhile$1", f = "Deprecated.kt", i = {0, 1, 1, 2}, l = {269, 270, 271}, m = "invokeSuspend", n = {"$this$produce", "$this$produce", "e", "$this$produce"}, s = {"L$0", "L$0", "L$2", "L$0"})
    static final class C01021<E> extends SuspendLambda implements Function2<ProducerScope<? super E>, Continuation<? super Unit>, Object> {
        final /* synthetic */ Function2<E, Continuation<? super Boolean>, Object> $predicate;
        final /* synthetic */ ReceiveChannel<E> $this_takeWhile;
        private /* synthetic */ Object L$0;
        Object L$1;
        Object L$2;
        int label;

        /* JADX WARN: 'super' call moved to the top of the method (can break code semantics) */
        /* JADX WARN: Multi-variable type inference failed */
        C01021(ReceiveChannel<? extends E> receiveChannel, Function2<? super E, ? super Continuation<? super Boolean>, ? extends Object> function2, Continuation<? super C01021> continuation) {
            super(2, continuation);
            this.$this_takeWhile = receiveChannel;
            this.$predicate = function2;
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Continuation<Unit> create(Object obj, Continuation<?> continuation) {
            C01021 c01021 = new C01021(this.$this_takeWhile, this.$predicate, continuation);
            c01021.L$0 = obj;
            return c01021;
        }

        @Override // kotlin.jvm.functions.Function2
        public final Object invoke(ProducerScope<? super E> producerScope, Continuation<? super Unit> continuation) {
            return ((C01021) create(producerScope, continuation)).invokeSuspend(Unit.INSTANCE);
        }

        /* JADX WARN: Multi-variable type inference failed */
        /* JADX WARN: Removed duplicated region for block: B:12:0x0068 A[RETURN] */
        /* JADX WARN: Removed duplicated region for block: B:13:0x0069  */
        /* JADX WARN: Removed duplicated region for block: B:16:0x0078  */
        /* JADX WARN: Removed duplicated region for block: B:22:0x009b  */
        /* JADX WARN: Removed duplicated region for block: B:24:0x009e  */
        /* JADX WARN: Removed duplicated region for block: B:29:0x00b8  */
        /* JADX WARN: Type inference failed for: r3v10 */
        /* JADX WARN: Type inference failed for: r3v13, types: [java.lang.Object] */
        /* JADX WARN: Type inference failed for: r3v17 */
        /* JADX WARN: Type inference failed for: r3v2 */
        /* JADX WARN: Type inference failed for: r4v0 */
        /* JADX WARN: Type inference failed for: r4v10 */
        /* JADX WARN: Type inference failed for: r4v5, types: [java.lang.Object] */
        /* JADX WARN: Type inference failed for: r5v0 */
        /* JADX WARN: Type inference failed for: r5v1, types: [java.lang.Object, kotlinx.coroutines.channels.ProducerScope] */
        /* JADX WARN: Type inference failed for: r5v4 */
        /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:27:0x00b2 -> B:10:0x0058). Please report as a decompilation issue!!! */
        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        /*
            Code decompiled incorrectly, please refer to instructions dump.
        */
        public final Object invokeSuspend(Object obj) throws Throwable {
            C01021<E> c01021;
            ?? r3;
            ChannelIterator<E> it;
            ?? r4;
            ChannelIterator<E> channelIterator;
            C01021<E> c010212;
            Object obj2;
            Object obj3;
            ?? r5;
            ChannelIterator<E> channelIterator2;
            Object obj4;
            Object objHasNext;
            Object coroutine_suspended = IntrinsicsKt.getCOROUTINE_SUSPENDED();
            switch (this.label) {
                case 0:
                    ResultKt.throwOnFailure(obj);
                    c01021 = this;
                    ProducerScope producerScope = (ProducerScope) c01021.L$0;
                    r3 = producerScope;
                    it = c01021.$this_takeWhile.iterator();
                    c01021.L$0 = r3;
                    c01021.L$1 = it;
                    c01021.label = 1;
                    objHasNext = it.hasNext(c01021);
                    if (objHasNext == coroutine_suspended) {
                        return coroutine_suspended;
                    }
                    Object obj5 = coroutine_suspended;
                    obj3 = obj;
                    obj = objHasNext;
                    r4 = r3;
                    channelIterator = it;
                    c010212 = c01021;
                    obj2 = obj5;
                    if (((Boolean) obj).booleanValue()) {
                        return Unit.INSTANCE;
                    }
                    E next = channelIterator.next();
                    Function2<E, Continuation<? super Boolean>, Object> function2 = c010212.$predicate;
                    c010212.L$0 = r4;
                    c010212.L$1 = channelIterator;
                    c010212.L$2 = next;
                    c010212.label = 2;
                    Object objInvoke = function2.invoke(next, c010212);
                    if (objInvoke == obj2) {
                        return obj2;
                    }
                    ChannelIterator<E> channelIterator3 = channelIterator;
                    obj4 = next;
                    obj = objInvoke;
                    r5 = r4;
                    channelIterator2 = channelIterator3;
                    if (((Boolean) obj).booleanValue()) {
                        return Unit.INSTANCE;
                    }
                    c010212.L$0 = r5;
                    c010212.L$1 = channelIterator2;
                    c010212.L$2 = null;
                    c010212.label = 3;
                    if (r5.send(obj4, c010212) == obj2) {
                        return obj2;
                    }
                    obj = obj3;
                    coroutine_suspended = obj2;
                    c01021 = c010212;
                    it = channelIterator2;
                    r3 = r5;
                    c01021.L$0 = r3;
                    c01021.L$1 = it;
                    c01021.label = 1;
                    objHasNext = it.hasNext(c01021);
                    if (objHasNext == coroutine_suspended) {
                    }
                    break;
                case 1:
                    ChannelIterator<E> channelIterator4 = (ChannelIterator) this.L$1;
                    ProducerScope producerScope2 = (ProducerScope) this.L$0;
                    ResultKt.throwOnFailure(obj);
                    r4 = producerScope2;
                    channelIterator = channelIterator4;
                    c010212 = this;
                    obj2 = coroutine_suspended;
                    obj3 = obj;
                    if (((Boolean) obj).booleanValue()) {
                    }
                    break;
                case 2:
                    Object obj6 = this.L$2;
                    ChannelIterator<E> channelIterator5 = (ChannelIterator) this.L$1;
                    ProducerScope producerScope3 = (ProducerScope) this.L$0;
                    ResultKt.throwOnFailure(obj);
                    r5 = producerScope3;
                    channelIterator2 = channelIterator5;
                    obj4 = obj6;
                    c010212 = this;
                    obj2 = coroutine_suspended;
                    obj3 = obj;
                    if (((Boolean) obj).booleanValue()) {
                    }
                    break;
                case 3:
                    c01021 = this;
                    it = (ChannelIterator) c01021.L$1;
                    ProducerScope producerScope4 = (ProducerScope) c01021.L$0;
                    ResultKt.throwOnFailure(obj);
                    r3 = producerScope4;
                    c01021.L$0 = r3;
                    c01021.L$1 = it;
                    c01021.label = 1;
                    objHasNext = it.hasNext(c01021);
                    if (objHasNext == coroutine_suspended) {
                    }
                    break;
                default:
                    throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
            }
        }
    }

    @Deprecated(level = DeprecationLevel.HIDDEN, message = "Binary compatibility")
    public static final /* synthetic */ ReceiveChannel takeWhile(ReceiveChannel $this$takeWhile, CoroutineContext context, Function2 predicate) {
        return ProduceKt.produce(GlobalScope.INSTANCE, (6 & 1) != 0 ? EmptyCoroutineContext.INSTANCE : context, (6 & 2) != 0 ? 0 : 0, (6 & 4) != 0 ? CoroutineStart.DEFAULT : null, (6 & 8) != 0 ? null : ChannelsKt.consumes($this$takeWhile), new C01021($this$takeWhile, predicate, null));
    }

    /* JADX WARN: Removed duplicated region for block: B:25:0x0083 A[RETURN] */
    /* JADX WARN: Removed duplicated region for block: B:26:0x0084  */
    /* JADX WARN: Removed duplicated region for block: B:29:0x0090 A[Catch: all -> 0x00b4, TryCatch #1 {all -> 0x00b4, blocks: (B:27:0x0088, B:29:0x0090, B:34:0x00ab), top: B:48:0x0088 }] */
    /* JADX WARN: Removed duplicated region for block: B:34:0x00ab A[Catch: all -> 0x00b4, TRY_LEAVE, TryCatch #1 {all -> 0x00b4, blocks: (B:27:0x0088, B:29:0x0090, B:34:0x00ab), top: B:48:0x0088 }] */
    /* JADX WARN: Removed duplicated region for block: B:7:0x0014  */
    /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:32:0x00a5 -> B:33:0x00a9). Please report as a decompilation issue!!! */
    /*
        Code decompiled incorrectly, please refer to instructions dump.
    */
    public static final <E, C extends SendChannel<? super E>> Object toChannel(ReceiveChannel<? extends E> receiveChannel, C c, Continuation<? super C> continuation) throws Throwable {
        C01031 c01031;
        ReceiveChannel<? extends E> receiveChannel2;
        SendChannel destination;
        Throwable cause$iv$iv;
        int i;
        ChannelIterator<? extends E> it;
        Object obj;
        Object $result;
        int i2;
        Object objHasNext;
        if (continuation instanceof C01031) {
            c01031 = (C01031) continuation;
            if ((c01031.label & Integer.MIN_VALUE) != 0) {
                c01031.label -= Integer.MIN_VALUE;
            } else {
                c01031 = new C01031(continuation);
            }
        }
        C01031 c010312 = c01031;
        Object $result2 = c010312.result;
        Object $result3 = IntrinsicsKt.getCOROUTINE_SUSPENDED();
        try {
            switch (c010312.label) {
                case 0:
                    ResultKt.throwOnFailure($result2);
                    destination = c;
                    receiveChannel2 = receiveChannel;
                    cause$iv$iv = null;
                    i = 0;
                    try {
                        it = receiveChannel2.iterator();
                        c010312.L$0 = destination;
                        c010312.L$1 = receiveChannel2;
                        c010312.L$2 = it;
                        c010312.label = 1;
                        objHasNext = it.hasNext(c010312);
                        if (objHasNext == $result3) {
                            return $result3;
                        }
                        Object obj2 = $result3;
                        $result = $result2;
                        $result2 = objHasNext;
                        obj = obj2;
                        try {
                            if (((Boolean) $result2).booleanValue()) {
                                Unit unit = Unit.INSTANCE;
                                return destination;
                            }
                            E next = it.next();
                            c010312.L$0 = destination;
                            c010312.L$1 = receiveChannel2;
                            c010312.L$2 = it;
                            c010312.label = 2;
                            if (destination.send(next, c010312) == obj) {
                                return obj;
                            }
                            $result2 = $result;
                            $result3 = obj;
                            i2 = i;
                            i = i2;
                            c010312.L$0 = destination;
                            c010312.L$1 = receiveChannel2;
                            c010312.L$2 = it;
                            c010312.label = 1;
                            objHasNext = it.hasNext(c010312);
                            if (objHasNext == $result3) {
                            }
                        } catch (Throwable th) {
                            e$iv$iv = th;
                            Throwable cause$iv$iv2 = e$iv$iv;
                            try {
                                throw e$iv$iv;
                            } finally {
                                ChannelsKt.cancelConsumed(receiveChannel2, cause$iv$iv2);
                            }
                        }
                    } catch (Throwable th2) {
                        e$iv$iv = th2;
                        Throwable cause$iv$iv22 = e$iv$iv;
                        throw e$iv$iv;
                    }
                    break;
                case 1:
                    ChannelIterator<? extends E> channelIterator = (ChannelIterator) c010312.L$2;
                    cause$iv$iv = null;
                    receiveChannel2 = (ReceiveChannel) c010312.L$1;
                    destination = (SendChannel) c010312.L$0;
                    ResultKt.throwOnFailure($result2);
                    it = channelIterator;
                    i = 0;
                    obj = $result3;
                    $result = $result2;
                    if (((Boolean) $result2).booleanValue()) {
                    }
                    break;
                case 2:
                    i2 = 0;
                    it = (ChannelIterator) c010312.L$2;
                    cause$iv$iv = null;
                    receiveChannel2 = (ReceiveChannel) c010312.L$1;
                    destination = (SendChannel) c010312.L$0;
                    ResultKt.throwOnFailure($result2);
                    i = i2;
                    c010312.L$0 = destination;
                    c010312.L$1 = receiveChannel2;
                    c010312.L$2 = it;
                    c010312.label = 1;
                    objHasNext = it.hasNext(c010312);
                    if (objHasNext == $result3) {
                    }
                    break;
                default:
                    throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
            }
        } catch (Throwable th3) {
            e$iv$iv = th3;
        }
    }

    /* JADX WARN: Multi-variable type inference failed */
    /* JADX WARN: Removed duplicated region for block: B:22:0x0070 A[RETURN] */
    /* JADX WARN: Removed duplicated region for block: B:23:0x0071  */
    /* JADX WARN: Removed duplicated region for block: B:26:0x0082 A[Catch: all -> 0x009c, TryCatch #3 {all -> 0x009c, blocks: (B:24:0x007a, B:26:0x0082, B:27:0x0093), top: B:45:0x007a }] */
    /* JADX WARN: Removed duplicated region for block: B:27:0x0093 A[Catch: all -> 0x009c, TRY_LEAVE, TryCatch #3 {all -> 0x009c, blocks: (B:24:0x007a, B:26:0x0082, B:27:0x0093), top: B:45:0x007a }] */
    /* JADX WARN: Removed duplicated region for block: B:7:0x0014  */
    /* JADX WARN: Type inference failed for: r6v0 */
    /* JADX WARN: Type inference failed for: r6v1, types: [java.lang.Object] */
    /* JADX WARN: Type inference failed for: r6v4 */
    /* JADX WARN: Type inference failed for: r6v5 */
    /* JADX WARN: Type inference failed for: r7v3 */
    /* JADX WARN: Type inference failed for: r7v4, types: [java.lang.Object, java.util.Collection] */
    /* JADX WARN: Type inference failed for: r7v5 */
    /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:23:0x0071 -> B:45:0x007a). Please report as a decompilation issue!!! */
    /*
        Code decompiled incorrectly, please refer to instructions dump.
    */
    public static final <E, C extends Collection<? super E>> Object toCollection(ReceiveChannel<? extends E> receiveChannel, C c, Continuation<? super C> continuation) throws Throwable {
        C01041 c01041;
        ReceiveChannel<? extends E> receiveChannel2;
        Throwable cause$iv$iv;
        Object $result;
        ?? r7;
        ReceiveChannel<? extends E> receiveChannel3;
        Throwable cause$iv$iv2;
        ChannelIterator channelIterator;
        ReceiveChannel receiveChannel4;
        Object obj;
        if (continuation instanceof C01041) {
            c01041 = (C01041) continuation;
            if ((c01041.label & Integer.MIN_VALUE) != 0) {
                c01041.label -= Integer.MIN_VALUE;
            } else {
                c01041 = new C01041(continuation);
            }
        }
        C01041 c010412 = c01041;
        Object e$iv = c010412.result;
        Object $result2 = IntrinsicsKt.getCOROUTINE_SUSPENDED();
        switch (c010412.label) {
            case 0:
                ResultKt.throwOnFailure(e$iv);
                ?? r6 = c;
                receiveChannel2 = receiveChannel;
                Throwable cause$iv$iv3 = null;
                try {
                    ReceiveChannel $this$consumeEach_u24lambda_u2d1$iv = null;
                    ChannelIterator it = receiveChannel2.iterator();
                    c010412.L$0 = r6;
                    c010412.L$1 = receiveChannel2;
                    c010412.L$2 = it;
                    c010412.label = 1;
                    Object objHasNext = it.hasNext(c010412);
                    if (objHasNext != $result2) {
                        return $result2;
                    }
                    Object obj2 = $result2;
                    $result = e$iv;
                    e$iv = objHasNext;
                    r7 = r6;
                    receiveChannel3 = receiveChannel2;
                    cause$iv$iv2 = cause$iv$iv3;
                    channelIterator = it;
                    receiveChannel4 = $this$consumeEach_u24lambda_u2d1$iv;
                    obj = obj2;
                    try {
                        if (((Boolean) e$iv).booleanValue()) {
                            Unit unit = Unit.INSTANCE;
                            return r7;
                        }
                        r7.add(channelIterator.next());
                        e$iv = $result;
                        $result2 = obj;
                        $this$consumeEach_u24lambda_u2d1$iv = receiveChannel4;
                        it = channelIterator;
                        cause$iv$iv3 = cause$iv$iv;
                        receiveChannel2 = receiveChannel2;
                        r6 = r7;
                        c010412.L$0 = r6;
                        c010412.L$1 = receiveChannel2;
                        c010412.L$2 = it;
                        c010412.label = 1;
                        Object objHasNext2 = it.hasNext(c010412);
                        if (objHasNext2 != $result2) {
                        }
                    } catch (Throwable th) {
                        receiveChannel2 = receiveChannel2;
                        e$iv$iv = th;
                        cause$iv$iv = e$iv$iv;
                        try {
                            throw e$iv$iv;
                        } finally {
                            ChannelsKt.cancelConsumed(receiveChannel2, cause$iv$iv);
                        }
                    }
                } catch (Throwable th2) {
                    e$iv$iv = th2;
                    cause$iv$iv = e$iv$iv;
                    throw e$iv$iv;
                }
                break;
            case 1:
                ChannelIterator channelIterator2 = (ChannelIterator) c010412.L$2;
                receiveChannel2 = (ReceiveChannel) c010412.L$1;
                Collection destination = (Collection) c010412.L$0;
                try {
                    ResultKt.throwOnFailure(e$iv);
                    r7 = destination;
                    receiveChannel3 = receiveChannel2;
                    cause$iv$iv2 = null;
                    channelIterator = channelIterator2;
                    receiveChannel4 = null;
                    obj = $result2;
                    $result = e$iv;
                    if (((Boolean) e$iv).booleanValue()) {
                    }
                } catch (Throwable th3) {
                    e$iv$iv = th3;
                    cause$iv$iv = e$iv$iv;
                    throw e$iv$iv;
                }
                break;
            default:
                throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
        }
    }

    /* JADX WARN: Multi-variable type inference failed */
    /* JADX WARN: Removed duplicated region for block: B:22:0x0070 A[RETURN] */
    /* JADX WARN: Removed duplicated region for block: B:23:0x0071  */
    /* JADX WARN: Removed duplicated region for block: B:26:0x0082 A[Catch: all -> 0x00a6, TryCatch #1 {all -> 0x00a6, blocks: (B:24:0x007a, B:26:0x0082, B:27:0x009d), top: B:41:0x007a }] */
    /* JADX WARN: Removed duplicated region for block: B:27:0x009d A[Catch: all -> 0x00a6, TRY_LEAVE, TryCatch #1 {all -> 0x00a6, blocks: (B:24:0x007a, B:26:0x0082, B:27:0x009d), top: B:41:0x007a }] */
    /* JADX WARN: Removed duplicated region for block: B:7:0x0014  */
    /* JADX WARN: Type inference failed for: r6v0 */
    /* JADX WARN: Type inference failed for: r6v1, types: [java.lang.Object] */
    /* JADX WARN: Type inference failed for: r6v4 */
    /* JADX WARN: Type inference failed for: r6v5 */
    /* JADX WARN: Type inference failed for: r7v3 */
    /* JADX WARN: Type inference failed for: r7v4, types: [java.lang.Object, java.util.Map] */
    /* JADX WARN: Type inference failed for: r7v5 */
    /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:23:0x0071 -> B:41:0x007a). Please report as a decompilation issue!!! */
    /*
        Code decompiled incorrectly, please refer to instructions dump.
    */
    public static final <K, V, M extends Map<? super K, ? super V>> Object toMap(ReceiveChannel<? extends Pair<? extends K, ? extends V>> receiveChannel, M m, Continuation<? super M> continuation) throws Throwable {
        AnonymousClass2 anonymousClass2;
        ReceiveChannel<? extends Pair<? extends K, ? extends V>> receiveChannel2;
        Throwable cause$iv$iv;
        Object $result;
        ?? r7;
        ReceiveChannel<? extends Pair<? extends K, ? extends V>> receiveChannel3;
        Throwable cause$iv$iv2;
        ChannelIterator channelIterator;
        ReceiveChannel receiveChannel4;
        Object obj;
        if (continuation instanceof AnonymousClass2) {
            anonymousClass2 = (AnonymousClass2) continuation;
            if ((anonymousClass2.label & Integer.MIN_VALUE) != 0) {
                anonymousClass2.label -= Integer.MIN_VALUE;
            } else {
                anonymousClass2 = new AnonymousClass2(continuation);
            }
        }
        AnonymousClass2 anonymousClass22 = anonymousClass2;
        Object e$iv = anonymousClass22.result;
        Object $result2 = IntrinsicsKt.getCOROUTINE_SUSPENDED();
        switch (anonymousClass22.label) {
            case 0:
                ResultKt.throwOnFailure(e$iv);
                ?? r6 = m;
                receiveChannel2 = receiveChannel;
                Throwable cause$iv$iv3 = null;
                try {
                    ReceiveChannel $this$consumeEach_u24lambda_u2d1$iv = null;
                    ChannelIterator it = receiveChannel2.iterator();
                    anonymousClass22.L$0 = r6;
                    anonymousClass22.L$1 = receiveChannel2;
                    anonymousClass22.L$2 = it;
                    anonymousClass22.label = 1;
                    Object objHasNext = it.hasNext(anonymousClass22);
                    if (objHasNext != $result2) {
                        return $result2;
                    }
                    Object obj2 = $result2;
                    $result = e$iv;
                    e$iv = objHasNext;
                    r7 = r6;
                    receiveChannel3 = receiveChannel2;
                    cause$iv$iv2 = cause$iv$iv3;
                    channelIterator = it;
                    receiveChannel4 = $this$consumeEach_u24lambda_u2d1$iv;
                    obj = obj2;
                    try {
                        if (((Boolean) e$iv).booleanValue()) {
                            Unit unit = Unit.INSTANCE;
                            return r7;
                        }
                        Pair it2 = (Pair) channelIterator.next();
                        r7.put(it2.getFirst(), it2.getSecond());
                        e$iv = $result;
                        $result2 = obj;
                        $this$consumeEach_u24lambda_u2d1$iv = receiveChannel4;
                        it = channelIterator;
                        cause$iv$iv3 = cause$iv$iv;
                        receiveChannel2 = receiveChannel2;
                        r6 = r7;
                        anonymousClass22.L$0 = r6;
                        anonymousClass22.L$1 = receiveChannel2;
                        anonymousClass22.L$2 = it;
                        anonymousClass22.label = 1;
                        Object objHasNext2 = it.hasNext(anonymousClass22);
                        if (objHasNext2 != $result2) {
                        }
                    } catch (Throwable th) {
                        receiveChannel2 = receiveChannel2;
                        e$iv$iv = th;
                        cause$iv$iv = e$iv$iv;
                        try {
                            throw e$iv$iv;
                        } finally {
                            ChannelsKt.cancelConsumed(receiveChannel2, cause$iv$iv);
                        }
                    }
                } catch (Throwable th2) {
                    e$iv$iv = th2;
                    cause$iv$iv = e$iv$iv;
                    throw e$iv$iv;
                }
                break;
            case 1:
                ChannelIterator channelIterator2 = (ChannelIterator) anonymousClass22.L$2;
                receiveChannel2 = (ReceiveChannel) anonymousClass22.L$1;
                Map destination = (Map) anonymousClass22.L$0;
                try {
                    ResultKt.throwOnFailure(e$iv);
                    r7 = destination;
                    receiveChannel3 = receiveChannel2;
                    cause$iv$iv2 = null;
                    channelIterator = channelIterator2;
                    receiveChannel4 = null;
                    obj = $result2;
                    $result = e$iv;
                    if (((Boolean) e$iv).booleanValue()) {
                    }
                } catch (Throwable th3) {
                    e$iv$iv = th3;
                    cause$iv$iv = e$iv$iv;
                    throw e$iv$iv;
                }
                break;
            default:
                throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
        }
    }

    public static /* synthetic */ ReceiveChannel flatMap$default(ReceiveChannel receiveChannel, CoroutineContext coroutineContext, Function2 function2, int i, Object obj) {
        if ((i & 1) != 0) {
            coroutineContext = Dispatchers.getUnconfined();
        }
        return flatMap(receiveChannel, coroutineContext, function2);
    }

    /* JADX INFO: Add missing generic type declarations: [R] */
    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$flatMap$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(d1 = {"\u0000\u000e\n\u0000\n\u0002\u0010\u0002\n\u0002\b\u0002\n\u0002\u0018\u0002\u0010\u0000\u001a\u00020\u0001\"\u0004\b\u0000\u0010\u0002\"\u0004\b\u0001\u0010\u0003*\b\u0012\u0004\u0012\u0002H\u00030\u0004H\u008a@"}, d2 = {"<anonymous>", "", "E", "R", "Lkotlinx/coroutines/channels/ProducerScope;"}, k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$flatMap$1", f = "Deprecated.kt", i = {0, 1, 2}, l = {321, 322, 322}, m = "invokeSuspend", n = {"$this$produce", "$this$produce", "$this$produce"}, s = {"L$0", "L$0", "L$0"})
    static final class C00881<R> extends SuspendLambda implements Function2<ProducerScope<? super R>, Continuation<? super Unit>, Object> {
        final /* synthetic */ ReceiveChannel<E> $this_flatMap;
        final /* synthetic */ Function2<E, Continuation<? super ReceiveChannel<? extends R>>, Object> $transform;
        private /* synthetic */ Object L$0;
        Object L$1;
        int label;

        /* JADX WARN: 'super' call moved to the top of the method (can break code semantics) */
        /* JADX WARN: Multi-variable type inference failed */
        C00881(ReceiveChannel<? extends E> receiveChannel, Function2<? super E, ? super Continuation<? super ReceiveChannel<? extends R>>, ? extends Object> function2, Continuation<? super C00881> continuation) {
            super(2, continuation);
            this.$this_flatMap = receiveChannel;
            this.$transform = function2;
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Continuation<Unit> create(Object obj, Continuation<?> continuation) {
            C00881 c00881 = new C00881(this.$this_flatMap, this.$transform, continuation);
            c00881.L$0 = obj;
            return c00881;
        }

        @Override // kotlin.jvm.functions.Function2
        public final Object invoke(ProducerScope<? super R> producerScope, Continuation<? super Unit> continuation) {
            return ((C00881) create(producerScope, continuation)).invokeSuspend(Unit.INSTANCE);
        }

        /* JADX WARN: Removed duplicated region for block: B:12:0x0065 A[RETURN] */
        /* JADX WARN: Removed duplicated region for block: B:13:0x0066  */
        /* JADX WARN: Removed duplicated region for block: B:16:0x0075  */
        /* JADX WARN: Removed duplicated region for block: B:21:0x009e A[RETURN] */
        /* JADX WARN: Removed duplicated region for block: B:22:0x009f  */
        /* JADX WARN: Removed duplicated region for block: B:24:0x00a5  */
        /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:22:0x009f -> B:10:0x0055). Please report as a decompilation issue!!! */
        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        /*
            Code decompiled incorrectly, please refer to instructions dump.
        */
        public final Object invokeSuspend(Object $result) throws Throwable {
            C00881<R> c00881;
            ProducerScope $this$produce;
            ChannelIterator it;
            ProducerScope $this$produce2;
            ChannelIterator channelIterator;
            C00881<R> c008812;
            Object obj;
            Object $result2;
            Object objHasNext;
            Object $result3 = IntrinsicsKt.getCOROUTINE_SUSPENDED();
            switch (this.label) {
                case 0:
                    ResultKt.throwOnFailure($result);
                    c00881 = this;
                    ProducerScope $this$produce3 = (ProducerScope) c00881.L$0;
                    $this$produce = $this$produce3;
                    it = c00881.$this_flatMap.iterator();
                    c00881.L$0 = $this$produce;
                    c00881.L$1 = it;
                    c00881.label = 1;
                    objHasNext = it.hasNext(c00881);
                    if (objHasNext == $result3) {
                        return $result3;
                    }
                    Object obj2 = $result3;
                    $result2 = $result;
                    $result = objHasNext;
                    $this$produce2 = $this$produce;
                    channelIterator = it;
                    c008812 = c00881;
                    obj = obj2;
                    if (((Boolean) $result).booleanValue()) {
                        return Unit.INSTANCE;
                    }
                    Object e = channelIterator.next();
                    Function2<E, Continuation<? super ReceiveChannel<? extends R>>, Object> function2 = c008812.$transform;
                    c008812.L$0 = $this$produce2;
                    c008812.L$1 = channelIterator;
                    c008812.label = 2;
                    $result = function2.invoke(e, c008812);
                    if ($result == obj) {
                        return obj;
                    }
                    c008812.L$0 = $this$produce2;
                    c008812.L$1 = channelIterator;
                    c008812.label = 3;
                    if (ChannelsKt.toChannel((ReceiveChannel) $result, $this$produce2, c008812) != obj) {
                        return obj;
                    }
                    $result = $result2;
                    $result3 = obj;
                    c00881 = c008812;
                    it = channelIterator;
                    $this$produce = $this$produce2;
                    c00881.L$0 = $this$produce;
                    c00881.L$1 = it;
                    c00881.label = 1;
                    objHasNext = it.hasNext(c00881);
                    if (objHasNext == $result3) {
                    }
                    break;
                case 1:
                    ChannelIterator channelIterator2 = (ChannelIterator) this.L$1;
                    ProducerScope $this$produce4 = (ProducerScope) this.L$0;
                    ResultKt.throwOnFailure($result);
                    $this$produce2 = $this$produce4;
                    channelIterator = channelIterator2;
                    c008812 = this;
                    obj = $result3;
                    $result2 = $result;
                    if (((Boolean) $result).booleanValue()) {
                    }
                    break;
                case 2:
                    ChannelIterator channelIterator3 = (ChannelIterator) this.L$1;
                    ProducerScope $this$produce5 = (ProducerScope) this.L$0;
                    ResultKt.throwOnFailure($result);
                    $this$produce2 = $this$produce5;
                    channelIterator = channelIterator3;
                    c008812 = this;
                    obj = $result3;
                    $result2 = $result;
                    c008812.L$0 = $this$produce2;
                    c008812.L$1 = channelIterator;
                    c008812.label = 3;
                    if (ChannelsKt.toChannel((ReceiveChannel) $result, $this$produce2, c008812) != obj) {
                    }
                    break;
                case 3:
                    c00881 = this;
                    it = (ChannelIterator) c00881.L$1;
                    $this$produce = (ProducerScope) c00881.L$0;
                    ResultKt.throwOnFailure($result);
                    c00881.L$0 = $this$produce;
                    c00881.L$1 = it;
                    c00881.label = 1;
                    objHasNext = it.hasNext(c00881);
                    if (objHasNext == $result3) {
                    }
                    break;
                default:
                    throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
            }
        }
    }

    @Deprecated(level = DeprecationLevel.HIDDEN, message = "Binary compatibility")
    public static final /* synthetic */ ReceiveChannel flatMap(ReceiveChannel $this$flatMap, CoroutineContext context, Function2 transform) {
        return ProduceKt.produce(GlobalScope.INSTANCE, (6 & 1) != 0 ? EmptyCoroutineContext.INSTANCE : context, (6 & 2) != 0 ? 0 : 0, (6 & 4) != 0 ? CoroutineStart.DEFAULT : null, (6 & 8) != 0 ? null : ChannelsKt.consumes($this$flatMap), new C00881($this$flatMap, transform, null));
    }

    public static /* synthetic */ ReceiveChannel map$default(ReceiveChannel receiveChannel, CoroutineContext coroutineContext, Function2 function2, int i, Object obj) {
        if ((i & 1) != 0) {
            coroutineContext = Dispatchers.getUnconfined();
        }
        return ChannelsKt.map(receiveChannel, coroutineContext, function2);
    }

    /* JADX INFO: Add missing generic type declarations: [R] */
    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$map$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(d1 = {"\u0000\u000e\n\u0000\n\u0002\u0010\u0002\n\u0002\b\u0002\n\u0002\u0018\u0002\u0010\u0000\u001a\u00020\u0001\"\u0004\b\u0000\u0010\u0002\"\u0004\b\u0001\u0010\u0003*\b\u0012\u0004\u0012\u0002H\u00030\u0004H\u008a@"}, d2 = {"<anonymous>", "", "E", "R", "Lkotlinx/coroutines/channels/ProducerScope;"}, k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$map$1", f = "Deprecated.kt", i = {0, 0, 1, 1, 2, 2}, l = {487, 333, 333}, m = "invokeSuspend", n = {"$this$produce", "$this$consume$iv$iv", "$this$produce", "$this$consume$iv$iv", "$this$produce", "$this$consume$iv$iv"}, s = {"L$0", "L$2", "L$0", "L$2", "L$0", "L$2"})
    static final class C00931<R> extends SuspendLambda implements Function2<ProducerScope<? super R>, Continuation<? super Unit>, Object> {
        final /* synthetic */ ReceiveChannel<E> $this_map;
        final /* synthetic */ Function2<E, Continuation<? super R>, Object> $transform;
        private /* synthetic */ Object L$0;
        Object L$1;
        Object L$2;
        Object L$3;
        Object L$4;
        int label;

        /* JADX WARN: 'super' call moved to the top of the method (can break code semantics) */
        /* JADX WARN: Multi-variable type inference failed */
        C00931(ReceiveChannel<? extends E> receiveChannel, Function2<? super E, ? super Continuation<? super R>, ? extends Object> function2, Continuation<? super C00931> continuation) {
            super(2, continuation);
            this.$this_map = receiveChannel;
            this.$transform = function2;
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Continuation<Unit> create(Object obj, Continuation<?> continuation) {
            C00931 c00931 = new C00931(this.$this_map, this.$transform, continuation);
            c00931.L$0 = obj;
            return c00931;
        }

        @Override // kotlin.jvm.functions.Function2
        public final Object invoke(ProducerScope<? super R> producerScope, Continuation<? super Unit> continuation) {
            return ((C00931) create(producerScope, continuation)).invokeSuspend(Unit.INSTANCE);
        }

        /* JADX WARN: Removed duplicated region for block: B:24:0x00bd A[RETURN] */
        /* JADX WARN: Removed duplicated region for block: B:25:0x00be  */
        /* JADX WARN: Removed duplicated region for block: B:28:0x00d2 A[Catch: all -> 0x0125, TRY_LEAVE, TryCatch #2 {all -> 0x0125, blocks: (B:26:0x00ca, B:28:0x00d2, B:40:0x011b), top: B:56:0x00ca }] */
        /* JADX WARN: Removed duplicated region for block: B:34:0x0104 A[RETURN] */
        /* JADX WARN: Removed duplicated region for block: B:35:0x0105  */
        /* JADX WARN: Removed duplicated region for block: B:40:0x011b A[Catch: all -> 0x0125, TRY_ENTER, TRY_LEAVE, TryCatch #2 {all -> 0x0125, blocks: (B:26:0x00ca, B:28:0x00d2, B:40:0x011b), top: B:56:0x00ca }] */
        /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:35:0x0105 -> B:36:0x010f). Please report as a decompilation issue!!! */
        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        /*
            Code decompiled incorrectly, please refer to instructions dump.
        */
        public final Object invokeSuspend(Object obj) throws Throwable {
            ReceiveChannel $this$consume$iv$iv;
            C00931<R> c00931;
            Object $result;
            ProducerScope $this$produce;
            Throwable cause$iv$iv;
            ChannelIterator it;
            int $i$f$consumeEach;
            int $i$f$consume;
            int $i$f$consume2;
            Function2 function2;
            Object $result2;
            ReceiveChannel $this$consume$iv$iv2;
            Throwable cause$iv$iv2;
            Function2 function22;
            ChannelIterator channelIterator;
            int i;
            int $i$f$consume3;
            int $i$f$consume4;
            Object $result3;
            Object $result4;
            ProducerScope producerScope;
            ChannelIterator channelIterator2;
            Function2 function23;
            ProducerScope $this$produce2;
            ProducerScope $this$produce3;
            ChannelIterator channelIterator3;
            Function2 function24;
            Object objHasNext;
            Object coroutine_suspended = IntrinsicsKt.getCOROUTINE_SUSPENDED();
            try {
            } catch (Throwable th) {
                e$iv$iv = th;
            }
            switch (this.label) {
                case 0:
                    ResultKt.throwOnFailure(obj);
                    c00931 = this;
                    $result = obj;
                    $this$produce = (ProducerScope) c00931.L$0;
                    ReceiveChannel $this$consumeEach$iv = c00931.$this_map;
                    Function2 function25 = c00931.$transform;
                    $this$consume$iv$iv = $this$consumeEach$iv;
                    cause$iv$iv = null;
                    try {
                        it = $this$consume$iv$iv.iterator();
                        $i$f$consumeEach = 0;
                        $i$f$consume = 0;
                        $i$f$consume2 = 0;
                        function2 = function25;
                        c00931.L$0 = $this$produce;
                        c00931.L$1 = function2;
                        c00931.L$2 = $this$consume$iv$iv;
                        c00931.L$3 = it;
                        c00931.label = 1;
                        objHasNext = it.hasNext(c00931);
                    } catch (Throwable th2) {
                        e$iv$iv = th2;
                    }
                    if (objHasNext == coroutine_suspended) {
                        return coroutine_suspended;
                    }
                    int i2 = $i$f$consume;
                    $result3 = $result;
                    $result2 = objHasNext;
                    $this$consume$iv$iv2 = $this$consume$iv$iv;
                    cause$iv$iv2 = cause$iv$iv;
                    function22 = function2;
                    channelIterator = it;
                    i = $i$f$consume2;
                    $i$f$consume3 = $i$f$consumeEach;
                    $i$f$consume4 = i2;
                    try {
                        if (((Boolean) $result2).booleanValue()) {
                            Unit unit = Unit.INSTANCE;
                            ChannelsKt.cancelConsumed($this$consume$iv$iv2, cause$iv$iv2);
                            return Unit.INSTANCE;
                        }
                        Object it2 = channelIterator.next();
                        $this$produce3 = null;
                        c00931.L$0 = $this$produce;
                        c00931.L$1 = function22;
                        c00931.L$2 = $this$consume$iv$iv2;
                        c00931.L$3 = channelIterator;
                        c00931.L$4 = $this$produce;
                        c00931.label = 2;
                        Object objInvoke = function22.invoke(it2, c00931);
                        if (objInvoke == coroutine_suspended) {
                            return coroutine_suspended;
                        }
                        $result4 = objInvoke;
                        $this$produce2 = $this$produce;
                        function23 = function22;
                        channelIterator2 = channelIterator;
                        producerScope = $this$produce2;
                        try {
                            c00931.L$0 = $this$produce2;
                            c00931.L$1 = function23;
                            c00931.L$2 = $this$consume$iv$iv2;
                            c00931.L$3 = channelIterator2;
                            c00931.L$4 = null;
                            c00931.label = 3;
                            if (producerScope.send($result4, c00931) != coroutine_suspended) {
                                return coroutine_suspended;
                            }
                            $result = $result3;
                            $i$f$consume = $i$f$consume4;
                            $i$f$consumeEach = $i$f$consume3;
                            $i$f$consume2 = i;
                            channelIterator3 = channelIterator2;
                            cause$iv$iv = cause$iv$iv2;
                            $this$consume$iv$iv = $this$consume$iv$iv2;
                            function24 = function23;
                            $this$produce = $this$produce2;
                            it = channelIterator3;
                            function2 = function24;
                            c00931.L$0 = $this$produce;
                            c00931.L$1 = function2;
                            c00931.L$2 = $this$consume$iv$iv;
                            c00931.L$3 = it;
                            c00931.label = 1;
                            objHasNext = it.hasNext(c00931);
                            if (objHasNext == coroutine_suspended) {
                            }
                        } catch (Throwable th3) {
                            e$iv$iv = th3;
                            $this$consume$iv$iv = $this$consume$iv$iv2;
                        }
                    } catch (Throwable th4) {
                        e$iv$iv = th4;
                        $this$consume$iv$iv = $this$consume$iv$iv2;
                    }
                    Throwable cause$iv$iv3 = e$iv$iv;
                    try {
                        throw e$iv$iv;
                    } catch (Throwable e$iv$iv) {
                        ChannelsKt.cancelConsumed($this$consume$iv$iv, cause$iv$iv3);
                        throw e$iv$iv;
                    }
                    break;
                case 1:
                    c00931 = this;
                    $result2 = obj;
                    ChannelIterator channelIterator4 = (ChannelIterator) c00931.L$3;
                    ReceiveChannel $this$consume$iv$iv3 = (ReceiveChannel) c00931.L$2;
                    Function2 function26 = (Function2) c00931.L$1;
                    $this$produce = (ProducerScope) c00931.L$0;
                    ResultKt.throwOnFailure($result2);
                    $this$consume$iv$iv2 = $this$consume$iv$iv3;
                    cause$iv$iv2 = null;
                    function22 = function26;
                    channelIterator = channelIterator4;
                    i = 0;
                    $i$f$consume3 = 0;
                    $i$f$consume4 = 0;
                    $result3 = $result2;
                    if (((Boolean) $result2).booleanValue()) {
                    }
                    Throwable cause$iv$iv32 = e$iv$iv;
                    throw e$iv$iv;
                case 2:
                    c00931 = this;
                    $result4 = obj;
                    producerScope = (ProducerScope) c00931.L$4;
                    channelIterator2 = (ChannelIterator) c00931.L$3;
                    cause$iv$iv2 = null;
                    $this$consume$iv$iv2 = (ReceiveChannel) c00931.L$2;
                    function23 = (Function2) c00931.L$1;
                    ProducerScope $this$produce4 = (ProducerScope) c00931.L$0;
                    try {
                        ResultKt.throwOnFailure($result4);
                        $this$produce2 = $this$produce4;
                        $this$produce3 = null;
                        i = 0;
                        $i$f$consume3 = 0;
                        $i$f$consume4 = 0;
                        $result3 = $result4;
                        c00931.L$0 = $this$produce2;
                        c00931.L$1 = function23;
                        c00931.L$2 = $this$consume$iv$iv2;
                        c00931.L$3 = channelIterator2;
                        c00931.L$4 = null;
                        c00931.label = 3;
                        if (producerScope.send($result4, c00931) != coroutine_suspended) {
                        }
                    } catch (Throwable th5) {
                        e$iv$iv = th5;
                        $this$consume$iv$iv = $this$consume$iv$iv2;
                        break;
                    }
                    break;
                case 3:
                    c00931 = this;
                    $result = obj;
                    $i$f$consume = 0;
                    $i$f$consumeEach = 0;
                    $i$f$consume2 = 0;
                    channelIterator3 = (ChannelIterator) c00931.L$3;
                    cause$iv$iv = null;
                    $this$consume$iv$iv = (ReceiveChannel) c00931.L$2;
                    function24 = (Function2) c00931.L$1;
                    $this$produce = (ProducerScope) c00931.L$0;
                    ResultKt.throwOnFailure($result);
                    it = channelIterator3;
                    function2 = function24;
                    c00931.L$0 = $this$produce;
                    c00931.L$1 = function2;
                    c00931.L$2 = $this$consume$iv$iv;
                    c00931.L$3 = it;
                    c00931.label = 1;
                    objHasNext = it.hasNext(c00931);
                    if (objHasNext == coroutine_suspended) {
                    }
                    break;
                default:
                    throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
            }
        }
    }

    public static final <E, R> ReceiveChannel<R> map(ReceiveChannel<? extends E> receiveChannel, CoroutineContext context, Function2<? super E, ? super Continuation<? super R>, ? extends Object> function2) {
        return ProduceKt.produce(GlobalScope.INSTANCE, (6 & 1) != 0 ? EmptyCoroutineContext.INSTANCE : context, (6 & 2) != 0 ? 0 : 0, (6 & 4) != 0 ? CoroutineStart.DEFAULT : null, (6 & 8) != 0 ? null : ChannelsKt.consumes(receiveChannel), new C00931(receiveChannel, function2, null));
    }

    public static /* synthetic */ ReceiveChannel mapIndexed$default(ReceiveChannel receiveChannel, CoroutineContext coroutineContext, Function3 function3, int i, Object obj) {
        if ((i & 1) != 0) {
            coroutineContext = Dispatchers.getUnconfined();
        }
        return ChannelsKt.mapIndexed(receiveChannel, coroutineContext, function3);
    }

    /* JADX INFO: Add missing generic type declarations: [R] */
    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$mapIndexed$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(d1 = {"\u0000\u000e\n\u0000\n\u0002\u0010\u0002\n\u0002\b\u0002\n\u0002\u0018\u0002\u0010\u0000\u001a\u00020\u0001\"\u0004\b\u0000\u0010\u0002\"\u0004\b\u0001\u0010\u0003*\b\u0012\u0004\u0012\u0002H\u00030\u0004H\u008a@"}, d2 = {"<anonymous>", "", "E", "R", "Lkotlinx/coroutines/channels/ProducerScope;"}, k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$mapIndexed$1", f = "Deprecated.kt", i = {0, 0, 1, 1, 2, 2}, l = {344, 345, 345}, m = "invokeSuspend", n = {"$this$produce", "index", "$this$produce", "index", "$this$produce", "index"}, s = {"L$0", "I$0", "L$0", "I$0", "L$0", "I$0"})
    static final class C00941<R> extends SuspendLambda implements Function2<ProducerScope<? super R>, Continuation<? super Unit>, Object> {
        final /* synthetic */ ReceiveChannel<E> $this_mapIndexed;
        final /* synthetic */ Function3<Integer, E, Continuation<? super R>, Object> $transform;
        int I$0;
        private /* synthetic */ Object L$0;
        Object L$1;
        Object L$2;
        int label;

        /* JADX WARN: 'super' call moved to the top of the method (can break code semantics) */
        /* JADX WARN: Multi-variable type inference failed */
        C00941(ReceiveChannel<? extends E> receiveChannel, Function3<? super Integer, ? super E, ? super Continuation<? super R>, ? extends Object> function3, Continuation<? super C00941> continuation) {
            super(2, continuation);
            this.$this_mapIndexed = receiveChannel;
            this.$transform = function3;
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Continuation<Unit> create(Object obj, Continuation<?> continuation) {
            C00941 c00941 = new C00941(this.$this_mapIndexed, this.$transform, continuation);
            c00941.L$0 = obj;
            return c00941;
        }

        @Override // kotlin.jvm.functions.Function2
        public final Object invoke(ProducerScope<? super R> producerScope, Continuation<? super Unit> continuation) {
            return ((C00941) create(producerScope, continuation)).invokeSuspend(Unit.INSTANCE);
        }

        /* JADX WARN: Removed duplicated region for block: B:12:0x0072 A[RETURN] */
        /* JADX WARN: Removed duplicated region for block: B:13:0x0073  */
        /* JADX WARN: Removed duplicated region for block: B:16:0x0082  */
        /* JADX WARN: Removed duplicated region for block: B:22:0x00b8 A[RETURN] */
        /* JADX WARN: Removed duplicated region for block: B:23:0x00b9  */
        /* JADX WARN: Removed duplicated region for block: B:25:0x00bf  */
        /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:23:0x00b9 -> B:10:0x0060). Please report as a decompilation issue!!! */
        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        /*
            Code decompiled incorrectly, please refer to instructions dump.
        */
        public final Object invokeSuspend(Object $result) throws Throwable {
            C00941<R> c00941;
            ProducerScope $this$produce;
            int index;
            ChannelIterator it;
            ProducerScope $this$produce2;
            ChannelIterator channelIterator;
            C00941<R> c009412;
            Object obj;
            Object $result2;
            ProducerScope producerScope;
            ProducerScope $this$produce3;
            int index2;
            Object objHasNext;
            Object $result3 = IntrinsicsKt.getCOROUTINE_SUSPENDED();
            switch (this.label) {
                case 0:
                    ResultKt.throwOnFailure($result);
                    c00941 = this;
                    $this$produce = (ProducerScope) c00941.L$0;
                    index = 0;
                    it = c00941.$this_mapIndexed.iterator();
                    c00941.L$0 = $this$produce;
                    c00941.L$1 = it;
                    c00941.I$0 = index;
                    c00941.label = 1;
                    objHasNext = it.hasNext(c00941);
                    if (objHasNext == $result3) {
                        return $result3;
                    }
                    Object obj2 = $result3;
                    $result2 = $result;
                    $result = objHasNext;
                    channelIterator = it;
                    $this$produce2 = $this$produce;
                    c009412 = c00941;
                    obj = obj2;
                    if (((Boolean) $result).booleanValue()) {
                        return Unit.INSTANCE;
                    }
                    Object e = channelIterator.next();
                    Function3<Integer, E, Continuation<? super R>, Object> function3 = c009412.$transform;
                    index2 = index + 1;
                    Integer numBoxInt = Boxing.boxInt(index);
                    c009412.L$0 = $this$produce2;
                    c009412.L$1 = channelIterator;
                    c009412.L$2 = $this$produce2;
                    c009412.I$0 = index2;
                    c009412.label = 2;
                    $result = function3.invoke(numBoxInt, e, c009412);
                    if ($result == obj) {
                        return obj;
                    }
                    producerScope = $this$produce2;
                    it = channelIterator;
                    $this$produce3 = producerScope;
                    c009412.L$0 = $this$produce3;
                    c009412.L$1 = it;
                    c009412.L$2 = null;
                    c009412.I$0 = index2;
                    c009412.label = 3;
                    if (producerScope.send($result, c009412) != obj) {
                        return obj;
                    }
                    $result = $result2;
                    $result3 = obj;
                    c00941 = c009412;
                    $this$produce = $this$produce3;
                    index = index2;
                    c00941.L$0 = $this$produce;
                    c00941.L$1 = it;
                    c00941.I$0 = index;
                    c00941.label = 1;
                    objHasNext = it.hasNext(c00941);
                    if (objHasNext == $result3) {
                    }
                    break;
                case 1:
                    int index3 = this.I$0;
                    ChannelIterator channelIterator2 = (ChannelIterator) this.L$1;
                    $this$produce2 = (ProducerScope) this.L$0;
                    ResultKt.throwOnFailure($result);
                    channelIterator = channelIterator2;
                    index = index3;
                    c009412 = this;
                    obj = $result3;
                    $result2 = $result;
                    if (((Boolean) $result).booleanValue()) {
                    }
                    break;
                case 2:
                    int index4 = this.I$0;
                    producerScope = (ProducerScope) this.L$2;
                    it = (ChannelIterator) this.L$1;
                    $this$produce3 = (ProducerScope) this.L$0;
                    ResultKt.throwOnFailure($result);
                    index2 = index4;
                    c009412 = this;
                    obj = $result3;
                    $result2 = $result;
                    c009412.L$0 = $this$produce3;
                    c009412.L$1 = it;
                    c009412.L$2 = null;
                    c009412.I$0 = index2;
                    c009412.label = 3;
                    if (producerScope.send($result, c009412) != obj) {
                    }
                    break;
                case 3:
                    c00941 = this;
                    int index5 = c00941.I$0;
                    ChannelIterator channelIterator3 = (ChannelIterator) c00941.L$1;
                    ProducerScope $this$produce4 = (ProducerScope) c00941.L$0;
                    ResultKt.throwOnFailure($result);
                    index = index5;
                    $this$produce = $this$produce4;
                    it = channelIterator3;
                    c00941.L$0 = $this$produce;
                    c00941.L$1 = it;
                    c00941.I$0 = index;
                    c00941.label = 1;
                    objHasNext = it.hasNext(c00941);
                    if (objHasNext == $result3) {
                    }
                    break;
                default:
                    throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
            }
        }
    }

    public static final <E, R> ReceiveChannel<R> mapIndexed(ReceiveChannel<? extends E> receiveChannel, CoroutineContext context, Function3<? super Integer, ? super E, ? super Continuation<? super R>, ? extends Object> function3) {
        return ProduceKt.produce(GlobalScope.INSTANCE, (6 & 1) != 0 ? EmptyCoroutineContext.INSTANCE : context, (6 & 2) != 0 ? 0 : 0, (6 & 4) != 0 ? CoroutineStart.DEFAULT : null, (6 & 8) != 0 ? null : ChannelsKt.consumes(receiveChannel), new C00941(receiveChannel, function3, null));
    }

    public static /* synthetic */ ReceiveChannel mapIndexedNotNull$default(ReceiveChannel receiveChannel, CoroutineContext coroutineContext, Function3 function3, int i, Object obj) {
        if ((i & 1) != 0) {
            coroutineContext = Dispatchers.getUnconfined();
        }
        return mapIndexedNotNull(receiveChannel, coroutineContext, function3);
    }

    @Deprecated(level = DeprecationLevel.HIDDEN, message = "Binary compatibility")
    public static final /* synthetic */ ReceiveChannel mapIndexedNotNull(ReceiveChannel $this$mapIndexedNotNull, CoroutineContext context, Function3 transform) {
        return ChannelsKt.filterNotNull(ChannelsKt.mapIndexed($this$mapIndexedNotNull, context, transform));
    }

    public static /* synthetic */ ReceiveChannel mapNotNull$default(ReceiveChannel receiveChannel, CoroutineContext coroutineContext, Function2 function2, int i, Object obj) {
        if ((i & 1) != 0) {
            coroutineContext = Dispatchers.getUnconfined();
        }
        return mapNotNull(receiveChannel, coroutineContext, function2);
    }

    @Deprecated(level = DeprecationLevel.HIDDEN, message = "Binary compatibility")
    public static final /* synthetic */ ReceiveChannel mapNotNull(ReceiveChannel $this$mapNotNull, CoroutineContext context, Function2 transform) {
        return ChannelsKt.filterNotNull(ChannelsKt.map($this$mapNotNull, context, transform));
    }

    /* JADX INFO: Add missing generic type declarations: [E] */
    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$withIndex$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(d1 = {"\u0000\u0010\n\u0000\n\u0002\u0010\u0002\n\u0000\n\u0002\u0018\u0002\n\u0002\u0018\u0002\u0010\u0000\u001a\u00020\u0001\"\u0004\b\u0000\u0010\u0002*\u000e\u0012\n\u0012\b\u0012\u0004\u0012\u0002H\u00020\u00040\u0003H\u008a@"}, d2 = {"<anonymous>", "", "E", "Lkotlinx/coroutines/channels/ProducerScope;", "Lkotlin/collections/IndexedValue;"}, k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$withIndex$1", f = "Deprecated.kt", i = {0, 0, 1, 1}, l = {370, 371}, m = "invokeSuspend", n = {"$this$produce", "index", "$this$produce", "index"}, s = {"L$0", "I$0", "L$0", "I$0"})
    static final class C01051<E> extends SuspendLambda implements Function2<ProducerScope<? super IndexedValue<? extends E>>, Continuation<? super Unit>, Object> {
        final /* synthetic */ ReceiveChannel<E> $this_withIndex;
        int I$0;
        private /* synthetic */ Object L$0;
        Object L$1;
        int label;

        /* JADX WARN: 'super' call moved to the top of the method (can break code semantics) */
        /* JADX WARN: Multi-variable type inference failed */
        C01051(ReceiveChannel<? extends E> receiveChannel, Continuation<? super C01051> continuation) {
            super(2, continuation);
            this.$this_withIndex = receiveChannel;
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Continuation<Unit> create(Object obj, Continuation<?> continuation) {
            C01051 c01051 = new C01051(this.$this_withIndex, continuation);
            c01051.L$0 = obj;
            return c01051;
        }

        @Override // kotlin.jvm.functions.Function2
        public final Object invoke(ProducerScope<? super IndexedValue<? extends E>> producerScope, Continuation<? super Unit> continuation) {
            return ((C01051) create(producerScope, continuation)).invokeSuspend(Unit.INSTANCE);
        }

        /* JADX WARN: Removed duplicated region for block: B:11:0x005b A[RETURN] */
        /* JADX WARN: Removed duplicated region for block: B:12:0x005c  */
        /* JADX WARN: Removed duplicated region for block: B:15:0x006a  */
        /* JADX WARN: Removed duplicated region for block: B:20:0x008e  */
        /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:18:0x0088 -> B:9:0x0049). Please report as a decompilation issue!!! */
        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        /*
            Code decompiled incorrectly, please refer to instructions dump.
        */
        public final Object invokeSuspend(Object $result) throws Throwable {
            C01051<E> c01051;
            ProducerScope $this$produce;
            int index;
            ChannelIterator<E> it;
            ProducerScope $this$produce2;
            C01051<E> c010512;
            Object obj;
            Object $result2;
            Object objHasNext;
            Object $result3 = IntrinsicsKt.getCOROUTINE_SUSPENDED();
            switch (this.label) {
                case 0:
                    ResultKt.throwOnFailure($result);
                    c01051 = this;
                    $this$produce = (ProducerScope) c01051.L$0;
                    index = 0;
                    it = c01051.$this_withIndex.iterator();
                    c01051.L$0 = $this$produce;
                    c01051.L$1 = it;
                    c01051.I$0 = index;
                    c01051.label = 1;
                    objHasNext = it.hasNext(c01051);
                    if (objHasNext == $result3) {
                        return $result3;
                    }
                    Object obj2 = $result3;
                    $result2 = $result;
                    $result = objHasNext;
                    $this$produce2 = $this$produce;
                    c010512 = c01051;
                    obj = obj2;
                    if (((Boolean) $result).booleanValue()) {
                        return Unit.INSTANCE;
                    }
                    Object e = it.next();
                    int index2 = index + 1;
                    c010512.L$0 = $this$produce2;
                    c010512.L$1 = it;
                    c010512.I$0 = index2;
                    c010512.label = 2;
                    if ($this$produce2.send(new IndexedValue(index, e), c010512) == obj) {
                        return obj;
                    }
                    $result = $result2;
                    $result3 = obj;
                    c01051 = c010512;
                    $this$produce = $this$produce2;
                    index = index2;
                    c01051.L$0 = $this$produce;
                    c01051.L$1 = it;
                    c01051.I$0 = index;
                    c01051.label = 1;
                    objHasNext = it.hasNext(c01051);
                    if (objHasNext == $result3) {
                    }
                    break;
                case 1:
                    int index3 = this.I$0;
                    ChannelIterator<E> channelIterator = (ChannelIterator) this.L$1;
                    ProducerScope $this$produce3 = (ProducerScope) this.L$0;
                    ResultKt.throwOnFailure($result);
                    $this$produce2 = $this$produce3;
                    it = channelIterator;
                    index = index3;
                    c010512 = this;
                    obj = $result3;
                    $result2 = $result;
                    if (((Boolean) $result).booleanValue()) {
                    }
                    break;
                case 2:
                    c01051 = this;
                    int index4 = c01051.I$0;
                    ChannelIterator<E> channelIterator2 = (ChannelIterator) c01051.L$1;
                    ProducerScope $this$produce4 = (ProducerScope) c01051.L$0;
                    ResultKt.throwOnFailure($result);
                    index = index4;
                    $this$produce = $this$produce4;
                    it = channelIterator2;
                    c01051.L$0 = $this$produce;
                    c01051.L$1 = it;
                    c01051.I$0 = index;
                    c01051.label = 1;
                    objHasNext = it.hasNext(c01051);
                    if (objHasNext == $result3) {
                    }
                    break;
                default:
                    throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
            }
        }
    }

    public static /* synthetic */ ReceiveChannel withIndex$default(ReceiveChannel receiveChannel, CoroutineContext coroutineContext, int i, Object obj) {
        if ((i & 1) != 0) {
            coroutineContext = Dispatchers.getUnconfined();
        }
        return withIndex(receiveChannel, coroutineContext);
    }

    @Deprecated(level = DeprecationLevel.HIDDEN, message = "Binary compatibility")
    public static final /* synthetic */ ReceiveChannel withIndex(ReceiveChannel $this$withIndex, CoroutineContext context) {
        return ProduceKt.produce(GlobalScope.INSTANCE, (6 & 1) != 0 ? EmptyCoroutineContext.INSTANCE : context, (6 & 2) != 0 ? 0 : 0, (6 & 4) != 0 ? CoroutineStart.DEFAULT : null, (6 & 8) != 0 ? null : ChannelsKt.consumes($this$withIndex), new C01051($this$withIndex, null));
    }

    /* JADX INFO: Add missing generic type declarations: [E] */
    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$distinct$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(d1 = {"\u0000\u0004\n\u0002\b\u0003\u0010\u0000\u001a\u0002H\u0001\"\u0004\b\u0000\u0010\u00012\u0006\u0010\u0002\u001a\u0002H\u0001H\u008a@"}, d2 = {"<anonymous>", "E", "it"}, k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$distinct$1", f = "Deprecated.kt", i = {}, l = {}, m = "invokeSuspend", n = {}, s = {})
    static final class C00751<E> extends SuspendLambda implements Function2<E, Continuation<? super E>, Object> {
        /* synthetic */ Object L$0;
        int label;

        C00751(Continuation<? super C00751> continuation) {
            super(2, continuation);
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Continuation<Unit> create(Object obj, Continuation<?> continuation) {
            C00751 c00751 = new C00751(continuation);
            c00751.L$0 = obj;
            return c00751;
        }

        @Override // kotlin.jvm.functions.Function2
        public final Object invoke(E e, Continuation<? super E> continuation) {
            return ((C00751) create(e, continuation)).invokeSuspend(Unit.INSTANCE);
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Object invokeSuspend(Object obj) throws Throwable {
            IntrinsicsKt.getCOROUTINE_SUSPENDED();
            switch (this.label) {
                case 0:
                    ResultKt.throwOnFailure(obj);
                    Object it = this.L$0;
                    return it;
                default:
                    throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
            }
        }
    }

    public static /* synthetic */ ReceiveChannel distinctBy$default(ReceiveChannel receiveChannel, CoroutineContext coroutineContext, Function2 function2, int i, Object obj) {
        if ((i & 1) != 0) {
            coroutineContext = Dispatchers.getUnconfined();
        }
        return ChannelsKt.distinctBy(receiveChannel, coroutineContext, function2);
    }

    /* JADX INFO: Add missing generic type declarations: [E] */
    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$distinctBy$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(d1 = {"\u0000\u000e\n\u0000\n\u0002\u0010\u0002\n\u0002\b\u0002\n\u0002\u0018\u0002\u0010\u0000\u001a\u00020\u0001\"\u0004\b\u0000\u0010\u0002\"\u0004\b\u0001\u0010\u0003*\b\u0012\u0004\u0012\u0002H\u00020\u0004H\u008a@"}, d2 = {"<anonymous>", "", "E", "K", "Lkotlinx/coroutines/channels/ProducerScope;"}, k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$distinctBy$1", f = "Deprecated.kt", i = {0, 0, 1, 1, 1, 2, 2, 2}, l = {387, 388, 390}, m = "invokeSuspend", n = {"$this$produce", "keys", "$this$produce", "keys", "e", "$this$produce", "keys", "k"}, s = {"L$0", "L$1", "L$0", "L$1", "L$3", "L$0", "L$1", "L$3"})
    static final class C00761<E> extends SuspendLambda implements Function2<ProducerScope<? super E>, Continuation<? super Unit>, Object> {
        final /* synthetic */ Function2<E, Continuation<? super K>, Object> $selector;
        final /* synthetic */ ReceiveChannel<E> $this_distinctBy;
        private /* synthetic */ Object L$0;
        Object L$1;
        Object L$2;
        Object L$3;
        int label;

        /* JADX WARN: 'super' call moved to the top of the method (can break code semantics) */
        /* JADX WARN: Multi-variable type inference failed */
        C00761(ReceiveChannel<? extends E> receiveChannel, Function2<? super E, ? super Continuation<? super K>, ? extends Object> function2, Continuation<? super C00761> continuation) {
            super(2, continuation);
            this.$this_distinctBy = receiveChannel;
            this.$selector = function2;
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Continuation<Unit> create(Object obj, Continuation<?> continuation) {
            C00761 c00761 = new C00761(this.$this_distinctBy, this.$selector, continuation);
            c00761.L$0 = obj;
            return c00761;
        }

        @Override // kotlin.jvm.functions.Function2
        public final Object invoke(ProducerScope<? super E> producerScope, Continuation<? super Unit> continuation) {
            return ((C00761) create(producerScope, continuation)).invokeSuspend(Unit.INSTANCE);
        }

        /* JADX WARN: Multi-variable type inference failed */
        /* JADX WARN: Removed duplicated region for block: B:12:0x0082 A[RETURN] */
        /* JADX WARN: Removed duplicated region for block: B:13:0x0083  */
        /* JADX WARN: Removed duplicated region for block: B:16:0x0093  */
        /* JADX WARN: Removed duplicated region for block: B:22:0x00b8  */
        /* JADX WARN: Removed duplicated region for block: B:27:0x00e0  */
        /* JADX WARN: Removed duplicated region for block: B:28:0x00e7  */
        /* JADX WARN: Type inference failed for: r4v10 */
        /* JADX WARN: Type inference failed for: r4v14, types: [java.lang.Object] */
        /* JADX WARN: Type inference failed for: r4v17 */
        /* JADX WARN: Type inference failed for: r4v2 */
        /* JADX WARN: Type inference failed for: r5v0 */
        /* JADX WARN: Type inference failed for: r5v11 */
        /* JADX WARN: Type inference failed for: r5v12 */
        /* JADX WARN: Type inference failed for: r5v15 */
        /* JADX WARN: Type inference failed for: r5v5 */
        /* JADX WARN: Type inference failed for: r5v6, types: [java.lang.Object] */
        /* JADX WARN: Type inference failed for: r6v0 */
        /* JADX WARN: Type inference failed for: r6v1, types: [java.lang.Object, kotlinx.coroutines.channels.ProducerScope] */
        /* JADX WARN: Type inference failed for: r6v4 */
        /* JADX WARN: Type inference failed for: r6v8, types: [java.util.Collection] */
        /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:25:0x00cd -> B:26:0x00d5). Please report as a decompilation issue!!! */
        /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:27:0x00e0 -> B:10:0x006d). Please report as a decompilation issue!!! */
        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        /*
            Code decompiled incorrectly, please refer to instructions dump.
        */
        public final Object invokeSuspend(Object obj) throws Throwable {
            C00761<E> c00761;
            HashSet hashSet;
            ?? r4;
            ChannelIterator<E> it;
            ?? r5;
            HashSet hashSet2;
            ChannelIterator<E> channelIterator;
            C00761<E> c007612;
            Object obj2;
            Object obj3;
            ?? r6;
            HashSet hashSet3;
            ChannelIterator<E> channelIterator2;
            Object obj4;
            ChannelIterator<E> channelIterator3;
            HashSet hashSet4;
            ?? r52;
            Object obj5;
            Object objHasNext;
            Object coroutine_suspended = IntrinsicsKt.getCOROUTINE_SUSPENDED();
            switch (this.label) {
                case 0:
                    ResultKt.throwOnFailure(obj);
                    c00761 = this;
                    ProducerScope producerScope = (ProducerScope) c00761.L$0;
                    hashSet = new HashSet();
                    r4 = producerScope;
                    it = c00761.$this_distinctBy.iterator();
                    c00761.L$0 = r4;
                    c00761.L$1 = hashSet;
                    c00761.L$2 = it;
                    c00761.L$3 = null;
                    c00761.label = 1;
                    objHasNext = it.hasNext(c00761);
                    if (objHasNext != coroutine_suspended) {
                        return coroutine_suspended;
                    }
                    Object obj6 = coroutine_suspended;
                    obj3 = obj;
                    obj = objHasNext;
                    r5 = r4;
                    hashSet2 = hashSet;
                    channelIterator = it;
                    c007612 = c00761;
                    obj2 = obj6;
                    if (((Boolean) obj).booleanValue()) {
                        return Unit.INSTANCE;
                    }
                    E next = channelIterator.next();
                    Function2 function2 = c007612.$selector;
                    c007612.L$0 = r5;
                    c007612.L$1 = hashSet2;
                    c007612.L$2 = channelIterator;
                    c007612.L$3 = next;
                    c007612.label = 2;
                    Object objInvoke = function2.invoke(next, c007612);
                    if (objInvoke == obj2) {
                        return obj2;
                    }
                    ChannelIterator<E> channelIterator4 = channelIterator;
                    obj4 = next;
                    obj = objInvoke;
                    r6 = r5;
                    hashSet3 = hashSet2;
                    channelIterator2 = channelIterator4;
                    if (hashSet3.contains(obj)) {
                        c007612.L$0 = r6;
                        c007612.L$1 = hashSet3;
                        c007612.L$2 = channelIterator2;
                        c007612.L$3 = obj;
                        c007612.label = 3;
                        if (r6.send(obj4, c007612) == obj2) {
                            return obj2;
                        }
                        channelIterator3 = channelIterator2;
                        hashSet4 = hashSet3;
                        r52 = r6;
                        C00761<E> c007613 = c007612;
                        obj5 = obj;
                        obj = obj3;
                        coroutine_suspended = obj2;
                        c00761 = c007613;
                        hashSet4.add(obj5);
                        it = channelIterator3;
                        hashSet = hashSet4;
                        r4 = r52;
                        c00761.L$0 = r4;
                        c00761.L$1 = hashSet;
                        c00761.L$2 = it;
                        c00761.L$3 = null;
                        c00761.label = 1;
                        objHasNext = it.hasNext(c00761);
                        if (objHasNext != coroutine_suspended) {
                        }
                    } else {
                        obj = obj3;
                        coroutine_suspended = obj2;
                        c00761 = c007612;
                        it = channelIterator2;
                        hashSet = hashSet3;
                        r4 = r6;
                        c00761.L$0 = r4;
                        c00761.L$1 = hashSet;
                        c00761.L$2 = it;
                        c00761.L$3 = null;
                        c00761.label = 1;
                        objHasNext = it.hasNext(c00761);
                        if (objHasNext != coroutine_suspended) {
                        }
                    }
                    break;
                case 1:
                    ChannelIterator<E> channelIterator5 = (ChannelIterator) this.L$2;
                    HashSet hashSet5 = (HashSet) this.L$1;
                    ProducerScope producerScope2 = (ProducerScope) this.L$0;
                    ResultKt.throwOnFailure(obj);
                    r5 = producerScope2;
                    hashSet2 = hashSet5;
                    channelIterator = channelIterator5;
                    c007612 = this;
                    obj2 = coroutine_suspended;
                    obj3 = obj;
                    if (((Boolean) obj).booleanValue()) {
                    }
                    break;
                case 2:
                    Object obj7 = this.L$3;
                    ChannelIterator<E> channelIterator6 = (ChannelIterator) this.L$2;
                    HashSet hashSet6 = (HashSet) this.L$1;
                    ProducerScope producerScope3 = (ProducerScope) this.L$0;
                    ResultKt.throwOnFailure(obj);
                    r6 = producerScope3;
                    hashSet3 = hashSet6;
                    channelIterator2 = channelIterator6;
                    obj4 = obj7;
                    c007612 = this;
                    obj2 = coroutine_suspended;
                    obj3 = obj;
                    if (hashSet3.contains(obj)) {
                    }
                    break;
                case 3:
                    c00761 = this;
                    obj5 = c00761.L$3;
                    channelIterator3 = (ChannelIterator) c00761.L$2;
                    hashSet4 = (HashSet) c00761.L$1;
                    ProducerScope producerScope4 = (ProducerScope) c00761.L$0;
                    ResultKt.throwOnFailure(obj);
                    r52 = producerScope4;
                    hashSet4.add(obj5);
                    it = channelIterator3;
                    hashSet = hashSet4;
                    r4 = r52;
                    c00761.L$0 = r4;
                    c00761.L$1 = hashSet;
                    c00761.L$2 = it;
                    c00761.L$3 = null;
                    c00761.label = 1;
                    objHasNext = it.hasNext(c00761);
                    if (objHasNext != coroutine_suspended) {
                    }
                    break;
                default:
                    throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
            }
        }
    }

    public static final <E, K> ReceiveChannel<E> distinctBy(ReceiveChannel<? extends E> receiveChannel, CoroutineContext context, Function2<? super E, ? super Continuation<? super K>, ? extends Object> function2) {
        return ProduceKt.produce(GlobalScope.INSTANCE, (6 & 1) != 0 ? EmptyCoroutineContext.INSTANCE : context, (6 & 2) != 0 ? 0 : 0, (6 & 4) != 0 ? CoroutineStart.DEFAULT : null, (6 & 8) != 0 ? null : ChannelsKt.consumes(receiveChannel), new C00761(receiveChannel, function2, null));
    }

    public static final <E> Object toMutableSet(ReceiveChannel<? extends E> receiveChannel, Continuation<? super Set<E>> continuation) {
        return ChannelsKt.toCollection(receiveChannel, new LinkedHashSet(), continuation);
    }

    /* JADX WARN: Removed duplicated region for block: B:7:0x0014  */
    @Deprecated(level = DeprecationLevel.HIDDEN, message = "Binary compatibility")
    /*
        Code decompiled incorrectly, please refer to instructions dump.
    */
    public static final /* synthetic */ Object any(ReceiveChannel $this$any, Continuation continuation) throws Throwable {
        AnonymousClass1 anonymousClass1;
        ReceiveChannel $this$consume$iv;
        Throwable cause$iv;
        Object objHasNext;
        if (continuation instanceof AnonymousClass1) {
            anonymousClass1 = (AnonymousClass1) continuation;
            if ((anonymousClass1.label & Integer.MIN_VALUE) != 0) {
                anonymousClass1.label -= Integer.MIN_VALUE;
            } else {
                anonymousClass1 = new AnonymousClass1(continuation);
            }
        }
        AnonymousClass1 anonymousClass12 = anonymousClass1;
        Object $result = anonymousClass12.result;
        Object coroutine_suspended = IntrinsicsKt.getCOROUTINE_SUSPENDED();
        switch (anonymousClass12.label) {
            case 0:
                ResultKt.throwOnFailure($result);
                $this$consume$iv = $this$any;
                cause$iv = null;
                try {
                    ChannelIterator it = $this$consume$iv.iterator();
                    anonymousClass12.L$0 = $this$consume$iv;
                    anonymousClass12.label = 1;
                    objHasNext = it.hasNext(anonymousClass12);
                    if (objHasNext == coroutine_suspended) {
                        return coroutine_suspended;
                    }
                    return objHasNext;
                } catch (Throwable th) {
                    e$iv = th;
                    Throwable cause$iv2 = e$iv;
                    try {
                        throw e$iv;
                    } finally {
                        ChannelsKt.cancelConsumed($this$consume$iv, cause$iv2);
                    }
                }
            case 1:
                $this$consume$iv = (ReceiveChannel) anonymousClass12.L$0;
                cause$iv = null;
                try {
                    ResultKt.throwOnFailure($result);
                    objHasNext = $result;
                    return objHasNext;
                } catch (Throwable th2) {
                    e$iv = th2;
                    Throwable cause$iv22 = e$iv;
                    throw e$iv;
                }
            default:
                throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
        }
    }

    /* JADX WARN: Removed duplicated region for block: B:22:0x0076 A[RETURN] */
    /* JADX WARN: Removed duplicated region for block: B:23:0x0077  */
    /* JADX WARN: Removed duplicated region for block: B:26:0x0089 A[Catch: all -> 0x00ab, TryCatch #3 {all -> 0x00ab, blocks: (B:24:0x0081, B:26:0x0089, B:27:0x009c), top: B:45:0x0081 }] */
    /* JADX WARN: Removed duplicated region for block: B:27:0x009c A[Catch: all -> 0x00ab, TRY_LEAVE, TryCatch #3 {all -> 0x00ab, blocks: (B:24:0x0081, B:26:0x0089, B:27:0x009c), top: B:45:0x0081 }] */
    /* JADX WARN: Removed duplicated region for block: B:7:0x0014  */
    /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:23:0x0077 -> B:45:0x0081). Please report as a decompilation issue!!! */
    @Deprecated(level = DeprecationLevel.HIDDEN, message = "Binary compatibility")
    /*
        Code decompiled incorrectly, please refer to instructions dump.
    */
    public static final /* synthetic */ Object count(ReceiveChannel $this$consumeEach$iv, Continuation continuation) throws Throwable {
        C00741 c00741;
        ReceiveChannel $this$consume$iv$iv;
        Object $result;
        Ref.IntRef count;
        ReceiveChannel $this$consume$iv$iv2;
        Throwable cause$iv$iv;
        ChannelIterator channelIterator;
        ReceiveChannel receiveChannel;
        int $i$f$consume;
        Object obj;
        if (continuation instanceof C00741) {
            c00741 = (C00741) continuation;
            if ((c00741.label & Integer.MIN_VALUE) != 0) {
                c00741.label -= Integer.MIN_VALUE;
            } else {
                c00741 = new C00741(continuation);
            }
        }
        C00741 c007412 = c00741;
        Object e$iv = c007412.result;
        Object $result2 = IntrinsicsKt.getCOROUTINE_SUSPENDED();
        switch (c007412.label) {
            case 0:
                ResultKt.throwOnFailure(e$iv);
                Ref.IntRef count2 = new Ref.IntRef();
                $this$consume$iv$iv = $this$consumeEach$iv;
                Throwable cause$iv$iv2 = null;
                try {
                    ReceiveChannel $this$consumeEach_u24lambda_u2d1$iv = null;
                    ChannelIterator it = $this$consume$iv$iv.iterator();
                    int $i$f$consumeEach = 0;
                    c007412.L$0 = count2;
                    c007412.L$1 = $this$consume$iv$iv;
                    c007412.L$2 = it;
                    c007412.label = 1;
                    Object objHasNext = it.hasNext(c007412);
                    if (objHasNext != $result2) {
                        return $result2;
                    }
                    Object obj2 = $result2;
                    $result = e$iv;
                    e$iv = objHasNext;
                    count = count2;
                    $this$consume$iv$iv2 = $this$consume$iv$iv;
                    cause$iv$iv = cause$iv$iv2;
                    channelIterator = it;
                    receiveChannel = $this$consumeEach_u24lambda_u2d1$iv;
                    $i$f$consume = $i$f$consumeEach;
                    obj = obj2;
                    try {
                        if (((Boolean) e$iv).booleanValue()) {
                            Unit unit = Unit.INSTANCE;
                            ChannelsKt.cancelConsumed($this$consume$iv$iv2, cause$iv$iv);
                            int $i$f$consumeEach2 = count.element;
                            return Boxing.boxInt($i$f$consumeEach2);
                        }
                        channelIterator.next();
                        count.element++;
                        e$iv = $result;
                        $result2 = obj;
                        $i$f$consumeEach = $i$f$consume;
                        $this$consumeEach_u24lambda_u2d1$iv = receiveChannel;
                        it = channelIterator;
                        cause$iv$iv2 = cause$iv$iv;
                        $this$consume$iv$iv = $this$consume$iv$iv2;
                        count2 = count;
                        c007412.L$0 = count2;
                        c007412.L$1 = $this$consume$iv$iv;
                        c007412.L$2 = it;
                        c007412.label = 1;
                        Object objHasNext2 = it.hasNext(c007412);
                        if (objHasNext2 != $result2) {
                        }
                    } catch (Throwable th) {
                        $this$consume$iv$iv = $this$consume$iv$iv2;
                        e$iv$iv = th;
                        Throwable cause$iv$iv3 = e$iv$iv;
                        try {
                            throw e$iv$iv;
                        } catch (Throwable e$iv$iv) {
                            ChannelsKt.cancelConsumed($this$consume$iv$iv, cause$iv$iv3);
                            throw e$iv$iv;
                        }
                    }
                } catch (Throwable th2) {
                    e$iv$iv = th2;
                    Throwable cause$iv$iv32 = e$iv$iv;
                    throw e$iv$iv;
                }
                break;
            case 1:
                ChannelIterator channelIterator2 = (ChannelIterator) c007412.L$2;
                $this$consume$iv$iv = (ReceiveChannel) c007412.L$1;
                Ref.IntRef count3 = (Ref.IntRef) c007412.L$0;
                try {
                    ResultKt.throwOnFailure(e$iv);
                    count = count3;
                    $this$consume$iv$iv2 = $this$consume$iv$iv;
                    cause$iv$iv = null;
                    channelIterator = channelIterator2;
                    receiveChannel = null;
                    $i$f$consume = 0;
                    obj = $result2;
                    $result = e$iv;
                    if (((Boolean) e$iv).booleanValue()) {
                    }
                } catch (Throwable th3) {
                    e$iv$iv = th3;
                    Throwable cause$iv$iv322 = e$iv$iv;
                    throw e$iv$iv;
                }
                break;
            default:
                throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
        }
    }

    /* JADX WARN: Removed duplicated region for block: B:29:0x0096  */
    /* JADX WARN: Removed duplicated region for block: B:31:0x009a  */
    /* JADX WARN: Removed duplicated region for block: B:36:0x00b4 A[RETURN] */
    /* JADX WARN: Removed duplicated region for block: B:37:0x00b5  */
    /* JADX WARN: Removed duplicated region for block: B:40:0x00c6 A[Catch: all -> 0x00ea, TRY_LEAVE, TryCatch #5 {all -> 0x00ea, blocks: (B:38:0x00be, B:40:0x00c6), top: B:71:0x00be }] */
    /* JADX WARN: Removed duplicated region for block: B:44:0x00e5  */
    /* JADX WARN: Removed duplicated region for block: B:7:0x0014  */
    /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:37:0x00b5 -> B:71:0x00be). Please report as a decompilation issue!!! */
    @Deprecated(level = DeprecationLevel.HIDDEN, message = "Binary compatibility")
    /*
        Code decompiled incorrectly, please refer to instructions dump.
    */
    public static final /* synthetic */ Object maxWith(ReceiveChannel $this$consume$iv, Comparator comparator, Continuation continuation) throws Throwable {
        C00951 c00951;
        Comparator comparator2;
        Throwable cause$iv;
        ReceiveChannel $this$consume$iv2;
        Throwable e$iv;
        ChannelIterator iterator;
        Object objHasNext;
        ReceiveChannel receiveChannel;
        Object $result;
        Comparator comparator3;
        ReceiveChannel $this$consume$iv3;
        ChannelIterator iterator2;
        Throwable th;
        Object max;
        Object max2;
        if (continuation instanceof C00951) {
            c00951 = (C00951) continuation;
            if ((c00951.label & Integer.MIN_VALUE) != 0) {
                c00951.label -= Integer.MIN_VALUE;
            } else {
                c00951 = new C00951(continuation);
            }
        }
        C00951 c009512 = c00951;
        Object max3 = c009512.result;
        Object $result2 = IntrinsicsKt.getCOROUTINE_SUSPENDED();
        switch (c009512.label) {
            case 0:
                ResultKt.throwOnFailure(max3);
                comparator2 = comparator;
                cause$iv = null;
                try {
                    iterator = $this$consume$iv.iterator();
                    c009512.L$0 = comparator2;
                    c009512.L$1 = $this$consume$iv;
                    c009512.L$2 = iterator;
                    c009512.label = 1;
                    objHasNext = iterator.hasNext(c009512);
                    if (objHasNext == $result2) {
                        return $result2;
                    }
                    receiveChannel = $this$consume$iv;
                    try {
                        if (((Boolean) objHasNext).booleanValue()) {
                            ChannelsKt.cancelConsumed(receiveChannel, cause$iv);
                            return null;
                        }
                        ReceiveChannel $this$consume$iv4 = receiveChannel;
                        try {
                            Throwable cause$iv2 = cause$iv;
                            Object max4 = iterator.next();
                            ReceiveChannel $this$consume$iv5 = $this$consume$iv4;
                            try {
                                c009512.L$0 = comparator2;
                                c009512.L$1 = $this$consume$iv5;
                                c009512.L$2 = iterator;
                                c009512.L$3 = max4;
                                c009512.label = 2;
                                Object objHasNext2 = iterator.hasNext(c009512);
                                if (objHasNext2 != $result2) {
                                    return $result2;
                                }
                                Object obj = $result2;
                                $result = max3;
                                max3 = objHasNext2;
                                comparator3 = comparator2;
                                $this$consume$iv3 = $this$consume$iv5;
                                iterator2 = iterator;
                                th = cause$iv2;
                                max = max4;
                                max2 = obj;
                                try {
                                    if (((Boolean) max3).booleanValue()) {
                                        ChannelsKt.cancelConsumed($this$consume$iv3, th);
                                        return max;
                                    }
                                    Object e = iterator2.next();
                                    if (comparator3.compare(max, e) < 0) {
                                        ReceiveChannel $this$consume$iv6 = $this$consume$iv3;
                                        comparator2 = comparator3;
                                        Object obj2 = max2;
                                        max4 = e;
                                        max3 = $result;
                                        $result2 = obj2;
                                        ChannelIterator channelIterator = iterator2;
                                        $this$consume$iv5 = $this$consume$iv6;
                                        cause$iv2 = th;
                                        iterator = channelIterator;
                                    } else {
                                        ReceiveChannel $this$consume$iv7 = $this$consume$iv3;
                                        comparator2 = comparator3;
                                        ChannelIterator channelIterator2 = iterator2;
                                        $this$consume$iv5 = $this$consume$iv7;
                                        max3 = $result;
                                        $result2 = max2;
                                        max4 = max;
                                        cause$iv2 = th;
                                        iterator = channelIterator2;
                                    }
                                    c009512.L$0 = comparator2;
                                    c009512.L$1 = $this$consume$iv5;
                                    c009512.L$2 = iterator;
                                    c009512.L$3 = max4;
                                    c009512.label = 2;
                                    Object objHasNext22 = iterator.hasNext(c009512);
                                    if (objHasNext22 != $result2) {
                                    }
                                } catch (Throwable th2) {
                                    e$iv = th2;
                                    $this$consume$iv2 = $this$consume$iv3;
                                    Throwable cause$iv3 = e$iv;
                                    try {
                                        throw e$iv;
                                    } catch (Throwable e$iv2) {
                                        ChannelsKt.cancelConsumed($this$consume$iv2, cause$iv3);
                                        throw e$iv2;
                                    }
                                }
                            } catch (Throwable th3) {
                                e$iv = th3;
                                $this$consume$iv2 = $this$consume$iv5;
                                Throwable cause$iv32 = e$iv;
                                throw e$iv;
                            }
                        } catch (Throwable th4) {
                            e$iv = th4;
                            $this$consume$iv2 = $this$consume$iv4;
                            Throwable cause$iv322 = e$iv;
                            throw e$iv;
                        }
                    } catch (Throwable th5) {
                        e$iv = th5;
                        $this$consume$iv2 = receiveChannel;
                    }
                } catch (Throwable th6) {
                    $this$consume$iv2 = $this$consume$iv;
                    e$iv = th6;
                    Throwable cause$iv3222 = e$iv;
                    throw e$iv;
                }
                break;
            case 1:
                ChannelIterator iterator3 = (ChannelIterator) c009512.L$2;
                ReceiveChannel receiveChannel2 = (ReceiveChannel) c009512.L$1;
                comparator2 = (Comparator) c009512.L$0;
                try {
                    ResultKt.throwOnFailure(max3);
                    objHasNext = max3;
                    receiveChannel = receiveChannel2;
                    iterator = iterator3;
                    cause$iv = null;
                    if (((Boolean) objHasNext).booleanValue()) {
                    }
                } catch (Throwable th7) {
                    e$iv = th7;
                    $this$consume$iv2 = receiveChannel2;
                    Throwable cause$iv32222 = e$iv;
                    throw e$iv;
                }
                break;
            case 2:
                Object max5 = c009512.L$3;
                ChannelIterator iterator4 = (ChannelIterator) c009512.L$2;
                ReceiveChannel receiveChannel3 = (ReceiveChannel) c009512.L$1;
                Comparator comparator4 = (Comparator) c009512.L$0;
                try {
                    ResultKt.throwOnFailure(max3);
                    comparator3 = comparator4;
                    $this$consume$iv3 = receiveChannel3;
                    iterator2 = iterator4;
                    th = null;
                    max = max5;
                    max2 = $result2;
                    $result = max3;
                    if (((Boolean) max3).booleanValue()) {
                    }
                } catch (Throwable th8) {
                    e$iv = th8;
                    $this$consume$iv2 = receiveChannel3;
                    Throwable cause$iv322222 = e$iv;
                    throw e$iv;
                }
                break;
            default:
                throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
        }
    }

    /* JADX WARN: Removed duplicated region for block: B:29:0x0096  */
    /* JADX WARN: Removed duplicated region for block: B:31:0x009a  */
    /* JADX WARN: Removed duplicated region for block: B:36:0x00b4 A[RETURN] */
    /* JADX WARN: Removed duplicated region for block: B:37:0x00b5  */
    /* JADX WARN: Removed duplicated region for block: B:40:0x00c6 A[Catch: all -> 0x00ea, TRY_LEAVE, TryCatch #5 {all -> 0x00ea, blocks: (B:38:0x00be, B:40:0x00c6), top: B:71:0x00be }] */
    /* JADX WARN: Removed duplicated region for block: B:44:0x00e5  */
    /* JADX WARN: Removed duplicated region for block: B:7:0x0014  */
    /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:37:0x00b5 -> B:71:0x00be). Please report as a decompilation issue!!! */
    @Deprecated(level = DeprecationLevel.HIDDEN, message = "Binary compatibility")
    /*
        Code decompiled incorrectly, please refer to instructions dump.
    */
    public static final /* synthetic */ Object minWith(ReceiveChannel $this$consume$iv, Comparator comparator, Continuation continuation) throws Throwable {
        C00961 c00961;
        Comparator comparator2;
        Throwable cause$iv;
        ReceiveChannel $this$consume$iv2;
        Throwable e$iv;
        ChannelIterator iterator;
        Object objHasNext;
        ReceiveChannel receiveChannel;
        Object $result;
        Comparator comparator3;
        ReceiveChannel $this$consume$iv3;
        ChannelIterator iterator2;
        Throwable th;
        Object min;
        Object min2;
        if (continuation instanceof C00961) {
            c00961 = (C00961) continuation;
            if ((c00961.label & Integer.MIN_VALUE) != 0) {
                c00961.label -= Integer.MIN_VALUE;
            } else {
                c00961 = new C00961(continuation);
            }
        }
        C00961 c009612 = c00961;
        Object min3 = c009612.result;
        Object $result2 = IntrinsicsKt.getCOROUTINE_SUSPENDED();
        switch (c009612.label) {
            case 0:
                ResultKt.throwOnFailure(min3);
                comparator2 = comparator;
                cause$iv = null;
                try {
                    iterator = $this$consume$iv.iterator();
                    c009612.L$0 = comparator2;
                    c009612.L$1 = $this$consume$iv;
                    c009612.L$2 = iterator;
                    c009612.label = 1;
                    objHasNext = iterator.hasNext(c009612);
                    if (objHasNext == $result2) {
                        return $result2;
                    }
                    receiveChannel = $this$consume$iv;
                    try {
                        if (((Boolean) objHasNext).booleanValue()) {
                            ChannelsKt.cancelConsumed(receiveChannel, cause$iv);
                            return null;
                        }
                        ReceiveChannel $this$consume$iv4 = receiveChannel;
                        try {
                            Throwable cause$iv2 = cause$iv;
                            Object min4 = iterator.next();
                            ReceiveChannel $this$consume$iv5 = $this$consume$iv4;
                            try {
                                c009612.L$0 = comparator2;
                                c009612.L$1 = $this$consume$iv5;
                                c009612.L$2 = iterator;
                                c009612.L$3 = min4;
                                c009612.label = 2;
                                Object objHasNext2 = iterator.hasNext(c009612);
                                if (objHasNext2 != $result2) {
                                    return $result2;
                                }
                                Object obj = $result2;
                                $result = min3;
                                min3 = objHasNext2;
                                comparator3 = comparator2;
                                $this$consume$iv3 = $this$consume$iv5;
                                iterator2 = iterator;
                                th = cause$iv2;
                                min = min4;
                                min2 = obj;
                                try {
                                    if (((Boolean) min3).booleanValue()) {
                                        ChannelsKt.cancelConsumed($this$consume$iv3, th);
                                        return min;
                                    }
                                    Object e = iterator2.next();
                                    if (comparator3.compare(min, e) > 0) {
                                        ReceiveChannel $this$consume$iv6 = $this$consume$iv3;
                                        comparator2 = comparator3;
                                        Object obj2 = min2;
                                        min4 = e;
                                        min3 = $result;
                                        $result2 = obj2;
                                        ChannelIterator channelIterator = iterator2;
                                        $this$consume$iv5 = $this$consume$iv6;
                                        cause$iv2 = th;
                                        iterator = channelIterator;
                                    } else {
                                        ReceiveChannel $this$consume$iv7 = $this$consume$iv3;
                                        comparator2 = comparator3;
                                        ChannelIterator channelIterator2 = iterator2;
                                        $this$consume$iv5 = $this$consume$iv7;
                                        min3 = $result;
                                        $result2 = min2;
                                        min4 = min;
                                        cause$iv2 = th;
                                        iterator = channelIterator2;
                                    }
                                    c009612.L$0 = comparator2;
                                    c009612.L$1 = $this$consume$iv5;
                                    c009612.L$2 = iterator;
                                    c009612.L$3 = min4;
                                    c009612.label = 2;
                                    Object objHasNext22 = iterator.hasNext(c009612);
                                    if (objHasNext22 != $result2) {
                                    }
                                } catch (Throwable th2) {
                                    e$iv = th2;
                                    $this$consume$iv2 = $this$consume$iv3;
                                    Throwable cause$iv3 = e$iv;
                                    try {
                                        throw e$iv;
                                    } catch (Throwable e$iv2) {
                                        ChannelsKt.cancelConsumed($this$consume$iv2, cause$iv3);
                                        throw e$iv2;
                                    }
                                }
                            } catch (Throwable th3) {
                                e$iv = th3;
                                $this$consume$iv2 = $this$consume$iv5;
                                Throwable cause$iv32 = e$iv;
                                throw e$iv;
                            }
                        } catch (Throwable th4) {
                            e$iv = th4;
                            $this$consume$iv2 = $this$consume$iv4;
                            Throwable cause$iv322 = e$iv;
                            throw e$iv;
                        }
                    } catch (Throwable th5) {
                        e$iv = th5;
                        $this$consume$iv2 = receiveChannel;
                    }
                } catch (Throwable th6) {
                    $this$consume$iv2 = $this$consume$iv;
                    e$iv = th6;
                    Throwable cause$iv3222 = e$iv;
                    throw e$iv;
                }
                break;
            case 1:
                ChannelIterator iterator3 = (ChannelIterator) c009612.L$2;
                ReceiveChannel receiveChannel2 = (ReceiveChannel) c009612.L$1;
                comparator2 = (Comparator) c009612.L$0;
                try {
                    ResultKt.throwOnFailure(min3);
                    objHasNext = min3;
                    receiveChannel = receiveChannel2;
                    iterator = iterator3;
                    cause$iv = null;
                    if (((Boolean) objHasNext).booleanValue()) {
                    }
                } catch (Throwable th7) {
                    e$iv = th7;
                    $this$consume$iv2 = receiveChannel2;
                    Throwable cause$iv32222 = e$iv;
                    throw e$iv;
                }
                break;
            case 2:
                Object min5 = c009612.L$3;
                ChannelIterator iterator4 = (ChannelIterator) c009612.L$2;
                ReceiveChannel receiveChannel3 = (ReceiveChannel) c009612.L$1;
                Comparator comparator4 = (Comparator) c009612.L$0;
                try {
                    ResultKt.throwOnFailure(min3);
                    comparator3 = comparator4;
                    $this$consume$iv3 = receiveChannel3;
                    iterator2 = iterator4;
                    th = null;
                    min = min5;
                    min2 = $result2;
                    $result = min3;
                    if (((Boolean) min3).booleanValue()) {
                    }
                } catch (Throwable th8) {
                    e$iv = th8;
                    $this$consume$iv2 = receiveChannel3;
                    Throwable cause$iv322222 = e$iv;
                    throw e$iv;
                }
                break;
            default:
                throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
        }
    }

    /* JADX WARN: Removed duplicated region for block: B:24:0x005f  */
    /* JADX WARN: Removed duplicated region for block: B:25:0x0060  */
    /* JADX WARN: Removed duplicated region for block: B:7:0x0014  */
    @Deprecated(level = DeprecationLevel.HIDDEN, message = "Binary compatibility")
    /*
        Code decompiled incorrectly, please refer to instructions dump.
    */
    public static final /* synthetic */ Object none(ReceiveChannel $this$none, Continuation continuation) throws Throwable {
        C00971 c00971;
        ReceiveChannel $this$consume$iv;
        Throwable cause$iv;
        Object objHasNext;
        if (continuation instanceof C00971) {
            c00971 = (C00971) continuation;
            if ((c00971.label & Integer.MIN_VALUE) != 0) {
                c00971.label -= Integer.MIN_VALUE;
            } else {
                c00971 = new C00971(continuation);
            }
        }
        C00971 c009712 = c00971;
        Object $result = c009712.result;
        Object coroutine_suspended = IntrinsicsKt.getCOROUTINE_SUSPENDED();
        boolean z = true;
        switch (c009712.label) {
            case 0:
                ResultKt.throwOnFailure($result);
                $this$consume$iv = $this$none;
                cause$iv = null;
                try {
                    ChannelIterator it = $this$consume$iv.iterator();
                    c009712.L$0 = $this$consume$iv;
                    c009712.label = 1;
                    objHasNext = it.hasNext(c009712);
                    if (objHasNext == coroutine_suspended) {
                        return coroutine_suspended;
                    }
                    if (!((Boolean) objHasNext).booleanValue()) {
                        z = false;
                    }
                    Boolean boolBoxBoolean = Boxing.boxBoolean(z);
                    ChannelsKt.cancelConsumed($this$consume$iv, cause$iv);
                    return boolBoxBoolean;
                } catch (Throwable th) {
                    e$iv = th;
                    Throwable cause$iv2 = e$iv;
                    try {
                        throw e$iv;
                    } catch (Throwable e$iv) {
                        ChannelsKt.cancelConsumed($this$consume$iv, cause$iv2);
                        throw e$iv;
                    }
                }
            case 1:
                $this$consume$iv = (ReceiveChannel) c009712.L$0;
                cause$iv = null;
                try {
                    ResultKt.throwOnFailure($result);
                    objHasNext = $result;
                    if (!((Boolean) objHasNext).booleanValue()) {
                    }
                    Boolean boolBoxBoolean2 = Boxing.boxBoolean(z);
                    ChannelsKt.cancelConsumed($this$consume$iv, cause$iv);
                    return boolBoxBoolean2;
                } catch (Throwable th2) {
                    e$iv = th2;
                    Throwable cause$iv22 = e$iv;
                    throw e$iv;
                }
            default:
                throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
        }
    }

    /* JADX INFO: Add missing generic type declarations: [E] */
    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$requireNoNulls$1, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(d1 = {"\u0000\n\n\u0002\b\u0002\n\u0002\u0010\u0000\n\u0000\u0010\u0000\u001a\u0002H\u0001\"\b\b\u0000\u0010\u0001*\u00020\u00022\b\u0010\u0003\u001a\u0004\u0018\u0001H\u0001H\u008a@"}, d2 = {"<anonymous>", "E", "", "it"}, k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$requireNoNulls$1", f = "Deprecated.kt", i = {}, l = {}, m = "invokeSuspend", n = {}, s = {})
    static final class C00981<E> extends SuspendLambda implements Function2<E, Continuation<? super E>, Object> {
        final /* synthetic */ ReceiveChannel<E> $this_requireNoNulls;
        /* synthetic */ Object L$0;
        int label;

        /* JADX WARN: 'super' call moved to the top of the method (can break code semantics) */
        /* JADX WARN: Multi-variable type inference failed */
        C00981(ReceiveChannel<? extends E> receiveChannel, Continuation<? super C00981> continuation) {
            super(2, continuation);
            this.$this_requireNoNulls = receiveChannel;
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Continuation<Unit> create(Object obj, Continuation<?> continuation) {
            C00981 c00981 = new C00981(this.$this_requireNoNulls, continuation);
            c00981.L$0 = obj;
            return c00981;
        }

        @Override // kotlin.jvm.functions.Function2
        public final Object invoke(E e, Continuation<? super E> continuation) {
            return ((C00981) create(e, continuation)).invokeSuspend(Unit.INSTANCE);
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Object invokeSuspend(Object obj) throws Throwable {
            IntrinsicsKt.getCOROUTINE_SUSPENDED();
            switch (this.label) {
                case 0:
                    ResultKt.throwOnFailure(obj);
                    Object it = this.L$0;
                    if (it != null) {
                        return it;
                    }
                    throw new IllegalArgumentException("null element found in " + this.$this_requireNoNulls + '.');
                default:
                    throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
            }
        }
    }

    public static /* synthetic */ ReceiveChannel zip$default(ReceiveChannel receiveChannel, ReceiveChannel receiveChannel2, CoroutineContext coroutineContext, Function2 function2, int i, Object obj) {
        if ((i & 2) != 0) {
            coroutineContext = Dispatchers.getUnconfined();
        }
        return ChannelsKt.zip(receiveChannel, receiveChannel2, coroutineContext, function2);
    }

    /* JADX INFO: Add missing generic type declarations: [V] */
    /* JADX INFO: renamed from: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$zip$2, reason: invalid class name and case insensitive filesystem */
    /* JADX INFO: compiled from: Deprecated.kt */
    @Metadata(d1 = {"\u0000\u000e\n\u0000\n\u0002\u0010\u0002\n\u0002\b\u0003\n\u0002\u0018\u0002\u0010\u0000\u001a\u00020\u0001\"\u0004\b\u0000\u0010\u0002\"\u0004\b\u0001\u0010\u0003\"\u0004\b\u0002\u0010\u0004*\b\u0012\u0004\u0012\u0002H\u00040\u0005H\u008a@"}, d2 = {"<anonymous>", "", "E", "R", "V", "Lkotlinx/coroutines/channels/ProducerScope;"}, k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt$zip$2", f = "Deprecated.kt", i = {0, 0, 0, 1, 1, 1, 1, 2, 2, 2}, l = {487, 469, 471}, m = "invokeSuspend", n = {"$this$produce", "otherIterator", "$this$consume$iv$iv", "$this$produce", "otherIterator", "$this$consume$iv$iv", "element1", "$this$produce", "otherIterator", "$this$consume$iv$iv"}, s = {"L$0", "L$1", "L$3", "L$0", "L$1", "L$3", "L$5", "L$0", "L$1", "L$3"})
    static final class C01072<V> extends SuspendLambda implements Function2<ProducerScope<? super V>, Continuation<? super Unit>, Object> {
        final /* synthetic */ ReceiveChannel<R> $other;
        final /* synthetic */ ReceiveChannel<E> $this_zip;
        final /* synthetic */ Function2<E, R, V> $transform;
        private /* synthetic */ Object L$0;
        Object L$1;
        Object L$2;
        Object L$3;
        Object L$4;
        Object L$5;
        int label;

        /* JADX WARN: 'super' call moved to the top of the method (can break code semantics) */
        /* JADX WARN: Multi-variable type inference failed */
        C01072(ReceiveChannel<? extends R> receiveChannel, ReceiveChannel<? extends E> receiveChannel2, Function2<? super E, ? super R, ? extends V> function2, Continuation<? super C01072> continuation) {
            super(2, continuation);
            this.$other = receiveChannel;
            this.$this_zip = receiveChannel2;
            this.$transform = function2;
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Continuation<Unit> create(Object obj, Continuation<?> continuation) {
            C01072 c01072 = new C01072(this.$other, this.$this_zip, this.$transform, continuation);
            c01072.L$0 = obj;
            return c01072;
        }

        @Override // kotlin.jvm.functions.Function2
        public final Object invoke(ProducerScope<? super V> producerScope, Continuation<? super Unit> continuation) {
            return ((C01072) create(producerScope, continuation)).invokeSuspend(Unit.INSTANCE);
        }

        /* JADX WARN: Removed duplicated region for block: B:24:0x00d9 A[RETURN] */
        /* JADX WARN: Removed duplicated region for block: B:25:0x00da  */
        /* JADX WARN: Removed duplicated region for block: B:28:0x00ee A[Catch: all -> 0x016b, TRY_LEAVE, TryCatch #6 {all -> 0x016b, blocks: (B:26:0x00e6, B:28:0x00ee, B:46:0x0161), top: B:70:0x00e6 }] */
        /* JADX WARN: Removed duplicated region for block: B:34:0x0116  */
        /* JADX WARN: Removed duplicated region for block: B:35:0x0121  */
        /* JADX WARN: Removed duplicated region for block: B:46:0x0161 A[Catch: all -> 0x016b, TRY_ENTER, TRY_LEAVE, TryCatch #6 {all -> 0x016b, blocks: (B:26:0x00e6, B:28:0x00ee, B:46:0x0161), top: B:70:0x00e6 }] */
        /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:34:0x0116 -> B:22:0x00c4). Please report as a decompilation issue!!! */
        /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:39:0x0143 -> B:40:0x014a). Please report as a decompilation issue!!! */
        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        /*
            Code decompiled incorrectly, please refer to instructions dump.
        */
        public final Object invokeSuspend(Object obj) throws Throwable {
            ReceiveChannel $this$consume$iv$iv;
            C01072<V> c01072;
            Object $result;
            ProducerScope $this$produce;
            ChannelIterator otherIterator;
            Throwable cause$iv$iv;
            ChannelIterator it;
            int $i$f$consumeEach;
            int $i$f$consume;
            int $i$f$consume2;
            Function2 function2;
            Object $result2;
            ReceiveChannel $this$consume$iv$iv2;
            Throwable cause$iv$iv2;
            Function2 function22;
            ChannelIterator channelIterator;
            int i;
            int $i$f$consume3;
            int $i$f$consume4;
            Object $result3;
            Object element1;
            int i2;
            Object element12;
            Throwable cause$iv$iv3;
            Object objHasNext;
            Object coroutine_suspended = IntrinsicsKt.getCOROUTINE_SUSPENDED();
            Object element13 = null;
            try {
            } catch (Throwable th) {
                e$iv$iv = th;
            }
            switch (this.label) {
                case 0:
                    ResultKt.throwOnFailure(obj);
                    c01072 = this;
                    $result = obj;
                    $this$produce = (ProducerScope) c01072.L$0;
                    otherIterator = c01072.$other.iterator();
                    ReceiveChannel $this$consumeEach$iv = c01072.$this_zip;
                    Function2 function23 = c01072.$transform;
                    $this$consume$iv$iv = $this$consumeEach$iv;
                    cause$iv$iv = null;
                    try {
                        it = $this$consume$iv$iv.iterator();
                        $i$f$consumeEach = 0;
                        $i$f$consume = 0;
                        $i$f$consume2 = 0;
                        function2 = function23;
                        c01072.L$0 = $this$produce;
                        c01072.L$1 = otherIterator;
                        c01072.L$2 = function2;
                        c01072.L$3 = $this$consume$iv$iv;
                        c01072.L$4 = it;
                        c01072.L$5 = element13;
                        c01072.label = 1;
                        objHasNext = it.hasNext(c01072);
                    } catch (Throwable th2) {
                        e$iv$iv = th2;
                    }
                    if (objHasNext != coroutine_suspended) {
                        return coroutine_suspended;
                    }
                    int i3 = $i$f$consume;
                    $result3 = $result;
                    $result2 = objHasNext;
                    $this$consume$iv$iv2 = $this$consume$iv$iv;
                    cause$iv$iv2 = cause$iv$iv;
                    function22 = function2;
                    channelIterator = it;
                    i = $i$f$consume2;
                    $i$f$consume3 = $i$f$consumeEach;
                    $i$f$consume4 = i3;
                    try {
                        if (((Boolean) $result2).booleanValue()) {
                            Unit unit = Unit.INSTANCE;
                            ChannelsKt.cancelConsumed($this$consume$iv$iv2, cause$iv$iv2);
                            return Unit.INSTANCE;
                        }
                        Object element14 = channelIterator.next();
                        i2 = 0;
                        c01072.L$0 = $this$produce;
                        c01072.L$1 = otherIterator;
                        c01072.L$2 = function22;
                        c01072.L$3 = $this$consume$iv$iv2;
                        c01072.L$4 = channelIterator;
                        c01072.L$5 = element14;
                        c01072.label = 2;
                        Object objHasNext2 = otherIterator.hasNext(c01072);
                        if (objHasNext2 == coroutine_suspended) {
                            return coroutine_suspended;
                        }
                        element1 = objHasNext2;
                        element12 = element14;
                        try {
                            if (((Boolean) element1).booleanValue()) {
                                Throwable th3 = cause$iv$iv2;
                                $result = $result3;
                                $i$f$consume = $i$f$consume4;
                                $i$f$consumeEach = $i$f$consume3;
                                $i$f$consume2 = i;
                                it = channelIterator;
                                function2 = function22;
                                $this$consume$iv$iv = $this$consume$iv$iv2;
                                cause$iv$iv = th3;
                                element13 = null;
                                c01072.L$0 = $this$produce;
                                c01072.L$1 = otherIterator;
                                c01072.L$2 = function2;
                                c01072.L$3 = $this$consume$iv$iv;
                                c01072.L$4 = it;
                                c01072.L$5 = element13;
                                c01072.label = 1;
                                objHasNext = it.hasNext(c01072);
                                if (objHasNext != coroutine_suspended) {
                                }
                            } else {
                                Throwable cause$iv$iv4 = cause$iv$iv2;
                                try {
                                    Object element2 = otherIterator.next();
                                    Object objInvoke = function22.invoke(element12, element2);
                                    c01072.L$0 = $this$produce;
                                    c01072.L$1 = otherIterator;
                                    c01072.L$2 = function22;
                                    c01072.L$3 = $this$consume$iv$iv2;
                                    c01072.L$4 = channelIterator;
                                    c01072.L$5 = null;
                                    c01072.label = 3;
                                    if ($this$produce.send(objInvoke, c01072) == coroutine_suspended) {
                                        return coroutine_suspended;
                                    }
                                    cause$iv$iv3 = cause$iv$iv4;
                                    $result = $result3;
                                    $i$f$consume = $i$f$consume4;
                                    $i$f$consumeEach = $i$f$consume3;
                                    $i$f$consume2 = i;
                                    $this$consume$iv$iv = $this$consume$iv$iv2;
                                    it = channelIterator;
                                    function2 = function22;
                                    element13 = null;
                                    cause$iv$iv = cause$iv$iv3;
                                    c01072.L$0 = $this$produce;
                                    c01072.L$1 = otherIterator;
                                    c01072.L$2 = function2;
                                    c01072.L$3 = $this$consume$iv$iv;
                                    c01072.L$4 = it;
                                    c01072.L$5 = element13;
                                    c01072.label = 1;
                                    objHasNext = it.hasNext(c01072);
                                    if (objHasNext != coroutine_suspended) {
                                    }
                                } catch (Throwable th4) {
                                    e$iv$iv = th4;
                                    $this$consume$iv$iv = $this$consume$iv$iv2;
                                }
                            }
                        } catch (Throwable th5) {
                            e$iv$iv = th5;
                            $this$consume$iv$iv = $this$consume$iv$iv2;
                        }
                    } catch (Throwable th6) {
                        e$iv$iv = th6;
                        $this$consume$iv$iv = $this$consume$iv$iv2;
                    }
                    Throwable cause$iv$iv5 = e$iv$iv;
                    try {
                        throw e$iv$iv;
                    } catch (Throwable e$iv$iv) {
                        ChannelsKt.cancelConsumed($this$consume$iv$iv, cause$iv$iv5);
                        throw e$iv$iv;
                    }
                    break;
                case 1:
                    c01072 = this;
                    $result2 = obj;
                    ChannelIterator channelIterator2 = (ChannelIterator) c01072.L$4;
                    ReceiveChannel $this$consume$iv$iv3 = (ReceiveChannel) c01072.L$3;
                    Function2 function24 = (Function2) c01072.L$2;
                    otherIterator = (ChannelIterator) c01072.L$1;
                    $this$produce = (ProducerScope) c01072.L$0;
                    ResultKt.throwOnFailure($result2);
                    $this$consume$iv$iv2 = $this$consume$iv$iv3;
                    cause$iv$iv2 = null;
                    function22 = function24;
                    channelIterator = channelIterator2;
                    i = 0;
                    $i$f$consume3 = 0;
                    $i$f$consume4 = 0;
                    $result3 = $result2;
                    if (((Boolean) $result2).booleanValue()) {
                    }
                    Throwable cause$iv$iv52 = e$iv$iv;
                    throw e$iv$iv;
                case 2:
                    c01072 = this;
                    element1 = obj;
                    Object element15 = c01072.L$5;
                    ChannelIterator channelIterator3 = (ChannelIterator) c01072.L$4;
                    $this$consume$iv$iv = (ReceiveChannel) c01072.L$3;
                    Function2 function25 = (Function2) c01072.L$2;
                    otherIterator = (ChannelIterator) c01072.L$1;
                    $this$produce = (ProducerScope) c01072.L$0;
                    try {
                        ResultKt.throwOnFailure(element1);
                        i2 = 0;
                        i = 0;
                        $i$f$consume3 = 0;
                        $i$f$consume4 = 0;
                        $result3 = element1;
                        cause$iv$iv2 = null;
                        element12 = element15;
                        channelIterator = channelIterator3;
                        function22 = function25;
                        $this$consume$iv$iv2 = $this$consume$iv$iv;
                        if (((Boolean) element1).booleanValue()) {
                        }
                    } catch (Throwable th7) {
                        e$iv$iv = th7;
                        break;
                    }
                    break;
                case 3:
                    c01072 = this;
                    $result = obj;
                    $i$f$consume = 0;
                    $i$f$consumeEach = 0;
                    $i$f$consume2 = 0;
                    channelIterator = (ChannelIterator) c01072.L$4;
                    $this$consume$iv$iv = (ReceiveChannel) c01072.L$3;
                    Function2 function26 = (Function2) c01072.L$2;
                    otherIterator = (ChannelIterator) c01072.L$1;
                    $this$produce = (ProducerScope) c01072.L$0;
                    ResultKt.throwOnFailure($result);
                    cause$iv$iv3 = null;
                    function22 = function26;
                    it = channelIterator;
                    function2 = function22;
                    element13 = null;
                    cause$iv$iv = cause$iv$iv3;
                    c01072.L$0 = $this$produce;
                    c01072.L$1 = otherIterator;
                    c01072.L$2 = function2;
                    c01072.L$3 = $this$consume$iv$iv;
                    c01072.L$4 = it;
                    c01072.L$5 = element13;
                    c01072.label = 1;
                    objHasNext = it.hasNext(c01072);
                    if (objHasNext != coroutine_suspended) {
                    }
                    break;
                default:
                    throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
            }
        }
    }

    public static final <E, R, V> ReceiveChannel<V> zip(ReceiveChannel<? extends E> receiveChannel, ReceiveChannel<? extends R> receiveChannel2, CoroutineContext context, Function2<? super E, ? super R, ? extends V> function2) {
        return ProduceKt.produce(GlobalScope.INSTANCE, (6 & 1) != 0 ? EmptyCoroutineContext.INSTANCE : context, (6 & 2) != 0 ? 0 : 0, (6 & 4) != 0 ? CoroutineStart.DEFAULT : null, (6 & 8) != 0 ? null : ChannelsKt.consumesAll(receiveChannel, receiveChannel2), new C01072(receiveChannel2, receiveChannel, function2, null));
    }

    public static final Function1<Throwable, Unit> consumes(final ReceiveChannel<?> receiveChannel) {
        return new Function1<Throwable, Unit>() { // from class: kotlinx.coroutines.channels.ChannelsKt__DeprecatedKt.consumes.1
            /* JADX WARN: 'super' call moved to the top of the method (can break code semantics) */
            {
                super(1);
            }

            @Override // kotlin.jvm.functions.Function1
            public /* bridge */ /* synthetic */ Unit invoke(Throwable th) {
                invoke2(th);
                return Unit.INSTANCE;
            }

            /* JADX INFO: renamed from: invoke, reason: avoid collision after fix types in other method */
            public final void invoke2(Throwable cause) {
                ChannelsKt.cancelConsumed(receiveChannel, cause);
            }
        };
    }
}
