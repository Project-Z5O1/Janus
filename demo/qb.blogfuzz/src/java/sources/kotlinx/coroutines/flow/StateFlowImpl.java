package kotlinx.coroutines.flow;

import androidx.constraintlayout.core.motion.utils.TypedValues;
import androidx.constraintlayout.widget.ConstraintLayout;
import java.util.List;
import kotlin.Metadata;
import kotlin.ResultKt;
import kotlin.Unit;
import kotlin.collections.CollectionsKt;
import kotlin.coroutines.Continuation;
import kotlin.coroutines.CoroutineContext;
import kotlin.coroutines.intrinsics.IntrinsicsKt;
import kotlin.coroutines.jvm.internal.ContinuationImpl;
import kotlin.coroutines.jvm.internal.DebugMetadata;
import kotlin.jvm.internal.Intrinsics;
import kotlinx.coroutines.Job;
import kotlinx.coroutines.JobKt;
import kotlinx.coroutines.channels.BufferOverflow;
import kotlinx.coroutines.flow.internal.AbstractSharedFlow;
import kotlinx.coroutines.flow.internal.FusibleFlow;
import kotlinx.coroutines.flow.internal.NullSurrogateKt;
import kotlinx.coroutines.internal.Symbol;

/* JADX INFO: compiled from: StateFlow.kt */
/* JADX INFO: loaded from: classes.dex */
@Metadata(d1 = {"\u0000p\n\u0002\u0018\u0002\n\u0000\n\u0002\u0010\u0000\n\u0002\b\u0003\n\u0002\u0018\u0002\n\u0000\n\u0002\u0010\u0001\n\u0002\b\u0004\n\u0002\u0010\u000b\n\u0002\b\u0002\n\u0002\u0018\u0002\n\u0002\b\u0002\n\u0002\u0010\b\n\u0000\n\u0002\u0010\u0011\n\u0002\b\u0003\n\u0002\u0010\u0002\n\u0002\b\u0002\n\u0002\u0018\u0002\n\u0002\b\u0002\n\u0002\u0018\u0002\n\u0000\n\u0002\u0018\u0002\n\u0002\b\t\n\u0002\u0010 \n\u0002\b\n\n\u0002\u0018\u0002\n\u0002\u0018\u0002\n\u0002\u0018\u0002\n\u0002\u0018\u0002\b\u0002\u0018\u0000*\u0004\b\u0000\u0010\u00012\b\u0012\u0004\u0012\u00020\u0010062\b\u0012\u0004\u0012\u00028\u0000072\b\u0012\u0004\u0012\u00028\u0000082\b\u0012\u0004\u0012\u00028\u000009B\u000f\u0012\u0006\u0010\u0003\u001a\u00020\u0002¢\u0006\u0004\b\u0004\u0010\u0005J!\u0010\t\u001a\u00020\b2\f\u0010\u0007\u001a\b\u0012\u0004\u0012\u00028\u00000\u0006H\u0096@ø\u0001\u0000¢\u0006\u0004\b\t\u0010\nJ\u001f\u0010\u000e\u001a\u00020\r2\u0006\u0010\u000b\u001a\u00028\u00002\u0006\u0010\f\u001a\u00028\u0000H\u0016¢\u0006\u0004\b\u000e\u0010\u000fJ\u000f\u0010\u0011\u001a\u00020\u0010H\u0014¢\u0006\u0004\b\u0011\u0010\u0012J\u001f\u0010\u0016\u001a\n\u0012\u0006\u0012\u0004\u0018\u00010\u00100\u00152\u0006\u0010\u0014\u001a\u00020\u0013H\u0014¢\u0006\u0004\b\u0016\u0010\u0017J\u001b\u0010\u001a\u001a\u00020\u00192\u0006\u0010\u0018\u001a\u00028\u0000H\u0096@ø\u0001\u0000¢\u0006\u0004\b\u001a\u0010\u001bJ-\u0010\"\u001a\b\u0012\u0004\u0012\u00028\u00000!2\u0006\u0010\u001d\u001a\u00020\u001c2\u0006\u0010\u001e\u001a\u00020\u00132\u0006\u0010 \u001a\u00020\u001fH\u0016¢\u0006\u0004\b\"\u0010#J\u000f\u0010$\u001a\u00020\u0019H\u0016¢\u0006\u0004\b$\u0010%J\u0017\u0010&\u001a\u00020\r2\u0006\u0010\u0018\u001a\u00028\u0000H\u0016¢\u0006\u0004\b&\u0010'J!\u0010*\u001a\u00020\r2\b\u0010(\u001a\u0004\u0018\u00010\u00022\u0006\u0010)\u001a\u00020\u0002H\u0002¢\u0006\u0004\b*\u0010\u000fR\u001a\u0010.\u001a\b\u0012\u0004\u0012\u00028\u00000+8VX\u0096\u0004¢\u0006\u0006\u001a\u0004\b,\u0010-R\u0016\u0010/\u001a\u00020\u00138\u0002@\u0002X\u0082\u000e¢\u0006\u0006\n\u0004\b/\u00100R*\u0010\u0018\u001a\u00028\u00002\u0006\u0010\u0018\u001a\u00028\u00008V@VX\u0096\u000e¢\u0006\u0012\u0012\u0004\b4\u0010%\u001a\u0004\b1\u00102\"\u0004\b3\u0010\u0005\u0082\u0002\u0004\n\u0002\b\u0019¨\u00065"}, d2 = {"Lkotlinx/coroutines/flow/StateFlowImpl;", "T", "", "initialState", "<init>", "(Ljava/lang/Object;)V", "Lkotlinx/coroutines/flow/FlowCollector;", "collector", "", "collect", "(Lkotlinx/coroutines/flow/FlowCollector;Lkotlin/coroutines/Continuation;)Ljava/lang/Object;", "expect", "update", "", "compareAndSet", "(Ljava/lang/Object;Ljava/lang/Object;)Z", "Lkotlinx/coroutines/flow/StateFlowSlot;", "createSlot", "()Lkotlinx/coroutines/flow/StateFlowSlot;", "", "size", "", "createSlotArray", "(I)[Lkotlinx/coroutines/flow/StateFlowSlot;", "value", "", "emit", "(Ljava/lang/Object;Lkotlin/coroutines/Continuation;)Ljava/lang/Object;", "Lkotlin/coroutines/CoroutineContext;", "context", "capacity", "Lkotlinx/coroutines/channels/BufferOverflow;", "onBufferOverflow", "Lkotlinx/coroutines/flow/Flow;", "fuse", "(Lkotlin/coroutines/CoroutineContext;ILkotlinx/coroutines/channels/BufferOverflow;)Lkotlinx/coroutines/flow/Flow;", "resetReplayCache", "()V", "tryEmit", "(Ljava/lang/Object;)Z", "expectedState", "newState", "updateState", "", "getReplayCache", "()Ljava/util/List;", "replayCache", "sequence", "I", "getValue", "()Ljava/lang/Object;", "setValue", "getValue$annotations", "kotlinx-coroutines-core", "Lkotlinx/coroutines/flow/internal/AbstractSharedFlow;", "Lkotlinx/coroutines/flow/MutableStateFlow;", "Lkotlinx/coroutines/flow/CancellableFlow;", "Lkotlinx/coroutines/flow/internal/FusibleFlow;"}, k = 1, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
final class StateFlowImpl<T> extends AbstractSharedFlow<StateFlowSlot> implements MutableStateFlow<T>, CancellableFlow<T>, FusibleFlow<T> {
    private volatile /* synthetic */ Object _state;
    private int sequence;

    /* JADX INFO: renamed from: kotlinx.coroutines.flow.StateFlowImpl$collect$1, reason: invalid class name */
    /* JADX INFO: compiled from: StateFlow.kt */
    @Metadata(k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.flow.StateFlowImpl", f = "StateFlow.kt", i = {0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2}, l = {386, 398, TypedValues.CycleType.TYPE_ALPHA}, m = "collect", n = {"this", "collector", "slot", "this", "collector", "slot", "collectorJob", "newState", "this", "collector", "slot", "collectorJob", "oldState"}, s = {"L$0", "L$1", "L$2", "L$0", "L$1", "L$2", "L$3", "L$4", "L$0", "L$1", "L$2", "L$3", "L$4"})
    static final class AnonymousClass1 extends ContinuationImpl {
        Object L$0;
        Object L$1;
        Object L$2;
        Object L$3;
        Object L$4;
        int label;
        /* synthetic */ Object result;
        final /* synthetic */ StateFlowImpl<T> this$0;

        /* JADX WARN: 'super' call moved to the top of the method (can break code semantics) */
        AnonymousClass1(StateFlowImpl<T> stateFlowImpl, Continuation<? super AnonymousClass1> continuation) {
            super(continuation);
            this.this$0 = stateFlowImpl;
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Object invokeSuspend(Object obj) {
            this.result = obj;
            this.label |= Integer.MIN_VALUE;
            return this.this$0.collect(null, this);
        }
    }

    public static /* synthetic */ void getValue$annotations() {
    }

    public StateFlowImpl(Object initialState) {
        this._state = initialState;
    }

    @Override // kotlinx.coroutines.flow.MutableStateFlow, kotlinx.coroutines.flow.StateFlow
    public T getValue() {
        Symbol symbol = NullSurrogateKt.NULL;
        T t = (T) this._state;
        if (t == symbol) {
            return null;
        }
        return t;
    }

    @Override // kotlinx.coroutines.flow.MutableStateFlow
    public void setValue(T t) {
        updateState(null, t == null ? NullSurrogateKt.NULL : t);
    }

    @Override // kotlinx.coroutines.flow.MutableStateFlow
    public boolean compareAndSet(T expect, T update) {
        return updateState(expect == null ? NullSurrogateKt.NULL : expect, update == null ? NullSurrogateKt.NULL : update);
    }

    private final boolean updateState(Object expectedState, Object newState) {
        getSlots();
        synchronized (this) {
            Object oldState = this._state;
            if (expectedState != null && !Intrinsics.areEqual(oldState, expectedState)) {
                return false;
            }
            if (Intrinsics.areEqual(oldState, newState)) {
                return true;
            }
            this._state = newState;
            int curSequence = this.sequence;
            if ((curSequence & 1) == 0) {
                int curSequence2 = curSequence + 1;
                this.sequence = curSequence2;
                Object curSlots = getSlots();
                Unit unit = Unit.INSTANCE;
                while (true) {
                    StateFlowSlot[] stateFlowSlotArr = (StateFlowSlot[]) curSlots;
                    if (stateFlowSlotArr != null) {
                        int length = stateFlowSlotArr.length;
                        int i = 0;
                        while (i < length) {
                            StateFlowSlot stateFlowSlot = stateFlowSlotArr[i];
                            i++;
                            if (stateFlowSlot != null) {
                                stateFlowSlot.makePending();
                            }
                        }
                    }
                    synchronized (this) {
                        int i2 = this.sequence;
                        if (i2 == curSequence2) {
                            this.sequence = curSequence2 + 1;
                            return true;
                        }
                        curSequence2 = i2;
                        curSlots = getSlots();
                        Unit unit2 = Unit.INSTANCE;
                    }
                }
            } else {
                this.sequence = curSequence + 2;
                return true;
            }
        }
    }

    @Override // kotlinx.coroutines.flow.SharedFlow
    public List<T> getReplayCache() {
        return CollectionsKt.listOf(getValue());
    }

    @Override // kotlinx.coroutines.flow.MutableSharedFlow
    public boolean tryEmit(T value) {
        setValue(value);
        return true;
    }

    @Override // kotlinx.coroutines.flow.MutableSharedFlow, kotlinx.coroutines.flow.FlowCollector
    public Object emit(T t, Continuation<? super Unit> continuation) {
        setValue(t);
        return Unit.INSTANCE;
    }

    @Override // kotlinx.coroutines.flow.MutableSharedFlow
    public void resetReplayCache() {
        throw new UnsupportedOperationException("MutableStateFlow.resetReplayCache is not supported");
    }

    /* JADX WARN: Code restructure failed: missing block: B:35:0x00b5, code lost:
    
        if (kotlin.jvm.internal.Intrinsics.areEqual(r11, r6) == false) goto L36;
     */
    /* JADX WARN: Multi-variable type inference failed */
    /* JADX WARN: Removed duplicated region for block: B:31:0x00ab  */
    /* JADX WARN: Removed duplicated region for block: B:32:0x00ac A[Catch: all -> 0x00f1, TryCatch #0 {all -> 0x00f1, blocks: (B:13:0x003f, B:29:0x00a6, B:34:0x00b1, B:45:0x00d6, B:47:0x00dc, B:36:0x00b7, B:40:0x00bf, B:32:0x00ac, B:16:0x0056, B:19:0x006a, B:28:0x0093, B:22:0x007b, B:24:0x007f), top: B:54:0x0022 }] */
    /* JADX WARN: Removed duplicated region for block: B:34:0x00b1 A[Catch: all -> 0x00f1, TryCatch #0 {all -> 0x00f1, blocks: (B:13:0x003f, B:29:0x00a6, B:34:0x00b1, B:45:0x00d6, B:47:0x00dc, B:36:0x00b7, B:40:0x00bf, B:32:0x00ac, B:16:0x0056, B:19:0x006a, B:28:0x0093, B:22:0x007b, B:24:0x007f), top: B:54:0x0022 }] */
    /* JADX WARN: Removed duplicated region for block: B:38:0x00bc  */
    /* JADX WARN: Removed duplicated region for block: B:39:0x00be  */
    /* JADX WARN: Removed duplicated region for block: B:42:0x00d2 A[RETURN] */
    /* JADX WARN: Removed duplicated region for block: B:43:0x00d3  */
    /* JADX WARN: Removed duplicated region for block: B:47:0x00dc A[Catch: all -> 0x00f1, TRY_LEAVE, TryCatch #0 {all -> 0x00f1, blocks: (B:13:0x003f, B:29:0x00a6, B:34:0x00b1, B:45:0x00d6, B:47:0x00dc, B:36:0x00b7, B:40:0x00bf, B:32:0x00ac, B:16:0x0056, B:19:0x006a, B:28:0x0093, B:22:0x007b, B:24:0x007f), top: B:54:0x0022 }] */
    /* JADX WARN: Removed duplicated region for block: B:7:0x0014  */
    /* JADX WARN: Type inference failed for: r4v11 */
    /* JADX WARN: Type inference failed for: r4v12 */
    /* JADX WARN: Type inference failed for: r4v13 */
    /* JADX WARN: Type inference failed for: r4v14 */
    /* JADX WARN: Type inference failed for: r4v15 */
    /* JADX WARN: Type inference failed for: r4v16 */
    /* JADX WARN: Type inference failed for: r4v17 */
    /* JADX WARN: Type inference failed for: r4v18 */
    /* JADX WARN: Type inference failed for: r4v6 */
    /* JADX WARN: Type inference failed for: r4v7, types: [java.lang.Object] */
    /* JADX WARN: Type inference failed for: r4v8, types: [java.lang.Object, kotlinx.coroutines.flow.FlowCollector] */
    /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:46:0x00da -> B:29:0x00a6). Please report as a decompilation issue!!! */
    /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:48:0x00ed -> B:29:0x00a6). Please report as a decompilation issue!!! */
    @Override // kotlinx.coroutines.flow.SharedFlow, kotlinx.coroutines.flow.Flow
    /*
        Code decompiled incorrectly, please refer to instructions dump.
    */
    public Object collect(FlowCollector<? super T> flowCollector, Continuation<?> continuation) throws Throwable {
        AnonymousClass1 anonymousClass1;
        StateFlowImpl<T> stateFlowImpl;
        StateFlowSlot stateFlowSlotAllocateSlot;
        FlowCollector<? super T> flowCollector2;
        Job job;
        Object obj;
        Object obj2;
        ?? r4;
        ?? r42;
        ?? r43;
        Object obj3;
        if (continuation instanceof AnonymousClass1) {
            anonymousClass1 = (AnonymousClass1) continuation;
            if ((anonymousClass1.label & Integer.MIN_VALUE) != 0) {
                anonymousClass1.label -= Integer.MIN_VALUE;
            } else {
                anonymousClass1 = new AnonymousClass1(this, continuation);
            }
        }
        AnonymousClass1 anonymousClass12 = anonymousClass1;
        Object obj4 = anonymousClass12.result;
        Object coroutine_suspended = IntrinsicsKt.getCOROUTINE_SUSPENDED();
        try {
            switch (anonymousClass12.label) {
                case 0:
                    ResultKt.throwOnFailure(obj4);
                    stateFlowImpl = this;
                    FlowCollector<? super T> flowCollector3 = flowCollector;
                    stateFlowSlotAllocateSlot = stateFlowImpl.allocateSlot();
                    boolean z = flowCollector3 instanceof SubscribedFlowCollector;
                    flowCollector2 = flowCollector3;
                    if (z) {
                        anonymousClass12.L$0 = stateFlowImpl;
                        anonymousClass12.L$1 = flowCollector3;
                        anonymousClass12.L$2 = stateFlowSlotAllocateSlot;
                        anonymousClass12.label = 1;
                        Object objOnSubscription = ((SubscribedFlowCollector) flowCollector3).onSubscription(anonymousClass12);
                        flowCollector2 = flowCollector3;
                        if (objOnSubscription == coroutine_suspended) {
                            return coroutine_suspended;
                        }
                    }
                    job = (Job) anonymousClass12.getContext().get(Job.INSTANCE);
                    obj = null;
                    r43 = flowCollector2;
                    Object obj5 = stateFlowImpl._state;
                    if (job != null) {
                        JobKt.ensureActive(job);
                    }
                    if (obj != null) {
                        r42 = r43;
                    }
                    obj3 = obj5 == NullSurrogateKt.NULL ? null : obj5;
                    anonymousClass12.L$0 = stateFlowImpl;
                    anonymousClass12.L$1 = r43;
                    anonymousClass12.L$2 = stateFlowSlotAllocateSlot;
                    anonymousClass12.L$3 = job;
                    anonymousClass12.L$4 = obj5;
                    anonymousClass12.label = 2;
                    if (r43.emit(obj3, anonymousClass12) == coroutine_suspended) {
                        return coroutine_suspended;
                    }
                    obj2 = obj5;
                    r4 = r43;
                    obj = obj2;
                    r42 = r4;
                    r43 = r42;
                    if (!stateFlowSlotAllocateSlot.takePending()) {
                        anonymousClass12.L$0 = stateFlowImpl;
                        anonymousClass12.L$1 = r42;
                        anonymousClass12.L$2 = stateFlowSlotAllocateSlot;
                        anonymousClass12.L$3 = job;
                        anonymousClass12.L$4 = obj;
                        anonymousClass12.label = 3;
                        r43 = r42;
                        if (stateFlowSlotAllocateSlot.awaitPending(anonymousClass12) == coroutine_suspended) {
                            return coroutine_suspended;
                        }
                    }
                    Object obj52 = stateFlowImpl._state;
                    if (job != null) {
                    }
                    if (obj != null) {
                    }
                    if (obj52 == NullSurrogateKt.NULL) {
                    }
                    anonymousClass12.L$0 = stateFlowImpl;
                    anonymousClass12.L$1 = r43;
                    anonymousClass12.L$2 = stateFlowSlotAllocateSlot;
                    anonymousClass12.L$3 = job;
                    anonymousClass12.L$4 = obj52;
                    anonymousClass12.label = 2;
                    if (r43.emit(obj3, anonymousClass12) == coroutine_suspended) {
                    }
                    break;
                case 1:
                    stateFlowSlotAllocateSlot = (StateFlowSlot) anonymousClass12.L$2;
                    FlowCollector<? super T> flowCollector4 = (FlowCollector) anonymousClass12.L$1;
                    stateFlowImpl = (StateFlowImpl) anonymousClass12.L$0;
                    ResultKt.throwOnFailure(obj4);
                    flowCollector2 = flowCollector4;
                    job = (Job) anonymousClass12.getContext().get(Job.INSTANCE);
                    obj = null;
                    r43 = flowCollector2;
                    Object obj522 = stateFlowImpl._state;
                    if (job != null) {
                    }
                    if (obj != null) {
                    }
                    if (obj522 == NullSurrogateKt.NULL) {
                    }
                    anonymousClass12.L$0 = stateFlowImpl;
                    anonymousClass12.L$1 = r43;
                    anonymousClass12.L$2 = stateFlowSlotAllocateSlot;
                    anonymousClass12.L$3 = job;
                    anonymousClass12.L$4 = obj522;
                    anonymousClass12.label = 2;
                    if (r43.emit(obj3, anonymousClass12) == coroutine_suspended) {
                    }
                    break;
                case 2:
                    obj2 = anonymousClass12.L$4;
                    job = (Job) anonymousClass12.L$3;
                    stateFlowSlotAllocateSlot = (StateFlowSlot) anonymousClass12.L$2;
                    FlowCollector flowCollector5 = (FlowCollector) anonymousClass12.L$1;
                    stateFlowImpl = (StateFlowImpl) anonymousClass12.L$0;
                    ResultKt.throwOnFailure(obj4);
                    r4 = flowCollector5;
                    obj = obj2;
                    r42 = r4;
                    r43 = r42;
                    if (!stateFlowSlotAllocateSlot.takePending()) {
                    }
                    Object obj5222 = stateFlowImpl._state;
                    if (job != null) {
                    }
                    if (obj != null) {
                    }
                    if (obj5222 == NullSurrogateKt.NULL) {
                    }
                    anonymousClass12.L$0 = stateFlowImpl;
                    anonymousClass12.L$1 = r43;
                    anonymousClass12.L$2 = stateFlowSlotAllocateSlot;
                    anonymousClass12.L$3 = job;
                    anonymousClass12.L$4 = obj5222;
                    anonymousClass12.label = 2;
                    if (r43.emit(obj3, anonymousClass12) == coroutine_suspended) {
                    }
                    break;
                case 3:
                    obj = anonymousClass12.L$4;
                    job = (Job) anonymousClass12.L$3;
                    stateFlowSlotAllocateSlot = (StateFlowSlot) anonymousClass12.L$2;
                    FlowCollector flowCollector6 = (FlowCollector) anonymousClass12.L$1;
                    stateFlowImpl = (StateFlowImpl) anonymousClass12.L$0;
                    ResultKt.throwOnFailure(obj4);
                    r43 = flowCollector6;
                    Object obj52222 = stateFlowImpl._state;
                    if (job != null) {
                    }
                    if (obj != null) {
                    }
                    if (obj52222 == NullSurrogateKt.NULL) {
                    }
                    anonymousClass12.L$0 = stateFlowImpl;
                    anonymousClass12.L$1 = r43;
                    anonymousClass12.L$2 = stateFlowSlotAllocateSlot;
                    anonymousClass12.L$3 = job;
                    anonymousClass12.L$4 = obj52222;
                    anonymousClass12.label = 2;
                    if (r43.emit(obj3, anonymousClass12) == coroutine_suspended) {
                    }
                    break;
                default:
                    throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
            }
        } catch (Throwable th) {
            stateFlowImpl.freeSlot(stateFlowSlotAllocateSlot);
            throw th;
        }
    }

    /* JADX INFO: Access modifiers changed from: protected */
    @Override // kotlinx.coroutines.flow.internal.AbstractSharedFlow
    public StateFlowSlot createSlot() {
        return new StateFlowSlot();
    }

    /* JADX INFO: Access modifiers changed from: protected */
    @Override // kotlinx.coroutines.flow.internal.AbstractSharedFlow
    public StateFlowSlot[] createSlotArray(int size) {
        return new StateFlowSlot[size];
    }

    @Override // kotlinx.coroutines.flow.internal.FusibleFlow
    public Flow<T> fuse(CoroutineContext context, int capacity, BufferOverflow onBufferOverflow) {
        return StateFlowKt.fuseStateFlow(this, context, capacity, onBufferOverflow);
    }
}
