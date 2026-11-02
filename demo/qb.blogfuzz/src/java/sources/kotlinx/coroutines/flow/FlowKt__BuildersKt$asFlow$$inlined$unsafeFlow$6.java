package kotlinx.coroutines.flow;

import androidx.constraintlayout.widget.ConstraintLayout;
import kotlin.Metadata;
import kotlin.ResultKt;
import kotlin.Unit;
import kotlin.coroutines.Continuation;
import kotlin.coroutines.intrinsics.IntrinsicsKt;
import kotlin.coroutines.jvm.internal.ContinuationImpl;
import kotlin.coroutines.jvm.internal.DebugMetadata;

/* JADX INFO: Add missing generic type declarations: [T] */
/* JADX INFO: compiled from: SafeCollector.common.kt */
/* JADX INFO: loaded from: classes.dex */
@Metadata(d1 = {"\u0000\u0019\n\u0000\n\u0002\u0018\u0002\n\u0000\n\u0002\u0010\u0002\n\u0000\n\u0002\u0018\u0002\n\u0002\b\u0002*\u0001\u0000\b\n\u0018\u00002\b\u0012\u0004\u0012\u00028\u00000\u0001J\u001f\u0010\u0002\u001a\u00020\u00032\f\u0010\u0004\u001a\b\u0012\u0004\u0012\u00028\u00000\u0005H\u0096@ø\u0001\u0000¢\u0006\u0002\u0010\u0006\u0082\u0002\u0004\n\u0002\b\u0019¨\u0006\u0007¸\u0006\u0000"}, d2 = {"kotlinx/coroutines/flow/internal/SafeCollector_commonKt$unsafeFlow$1", "Lkotlinx/coroutines/flow/Flow;", "collect", "", "collector", "Lkotlinx/coroutines/flow/FlowCollector;", "(Lkotlinx/coroutines/flow/FlowCollector;Lkotlin/coroutines/Continuation;)Ljava/lang/Object;", "kotlinx-coroutines-core"}, k = 1, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
public final class FlowKt__BuildersKt$asFlow$$inlined$unsafeFlow$6<T> implements Flow<T> {
    final /* synthetic */ Object[] $this_asFlow$inlined;

    /* JADX INFO: renamed from: kotlinx.coroutines.flow.FlowKt__BuildersKt$asFlow$$inlined$unsafeFlow$6$1, reason: invalid class name */
    /* JADX INFO: compiled from: SafeCollector.common.kt */
    @Metadata(k = 3, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
    @DebugMetadata(c = "kotlinx.coroutines.flow.FlowKt__BuildersKt$asFlow$$inlined$unsafeFlow$6", f = "Builders.kt", i = {0}, l = {115}, m = "collect", n = {"$this$asFlow_u24lambda_u2d11"}, s = {"L$0"})
    public static final class AnonymousClass1 extends ContinuationImpl {
        int I$0;
        int I$1;
        Object L$0;
        Object L$1;
        int label;
        /* synthetic */ Object result;

        public AnonymousClass1(Continuation continuation) {
            super(continuation);
        }

        @Override // kotlin.coroutines.jvm.internal.BaseContinuationImpl
        public final Object invokeSuspend(Object obj) {
            this.result = obj;
            this.label |= Integer.MIN_VALUE;
            return FlowKt__BuildersKt$asFlow$$inlined$unsafeFlow$6.this.collect(null, this);
        }
    }

    /* JADX WARN: Multi-variable type inference failed */
    /* JADX WARN: Removed duplicated region for block: B:15:0x0054  */
    /* JADX WARN: Removed duplicated region for block: B:20:0x0071  */
    /* JADX WARN: Removed duplicated region for block: B:7:0x0014  */
    /* JADX WARN: Type inference failed for: r7v0 */
    /* JADX WARN: Type inference failed for: r7v1, types: [java.lang.Object, kotlinx.coroutines.flow.FlowCollector] */
    /* JADX WARN: Type inference failed for: r7v2 */
    /* JADX WARN: Type inference failed for: r7v5 */
    /* JADX WARN: Type inference failed for: r7v6 */
    /* JADX WARN: Type inference failed for: r7v7 */
    /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:18:0x006b -> B:19:0x006e). Please report as a decompilation issue!!! */
    @Override // kotlinx.coroutines.flow.Flow
    /*
        Code decompiled incorrectly, please refer to instructions dump.
    */
    public Object collect(FlowCollector<? super T> flowCollector, Continuation<? super Unit> continuation) throws Throwable {
        AnonymousClass1 anonymousClass1;
        int i;
        ?? r7;
        Object[] objArr;
        int length;
        int i2;
        Object[] objArr2;
        ?? r72;
        if (continuation instanceof AnonymousClass1) {
            anonymousClass1 = (AnonymousClass1) continuation;
            if ((anonymousClass1.label & Integer.MIN_VALUE) != 0) {
                anonymousClass1.label -= Integer.MIN_VALUE;
            } else {
                anonymousClass1 = new AnonymousClass1(continuation);
            }
        }
        AnonymousClass1 anonymousClass12 = anonymousClass1;
        Object obj = anonymousClass12.result;
        Object coroutine_suspended = IntrinsicsKt.getCOROUTINE_SUSPENDED();
        switch (anonymousClass12.label) {
            case 0:
                ResultKt.throwOnFailure(obj);
                Object[] objArr3 = this.$this_asFlow$inlined;
                i = 0;
                r7 = flowCollector;
                objArr = objArr3;
                length = objArr3.length;
                if (i < length) {
                    Object obj2 = objArr[i];
                    int i3 = i + 1;
                    anonymousClass12.L$0 = r7;
                    anonymousClass12.L$1 = objArr;
                    anonymousClass12.I$0 = i3;
                    anonymousClass12.I$1 = length;
                    anonymousClass12.label = 1;
                    if (r7.emit(obj2, anonymousClass12) == coroutine_suspended) {
                        return coroutine_suspended;
                    }
                    i2 = i3;
                    objArr2 = objArr;
                    r72 = r7;
                    objArr = objArr2;
                    i = i2;
                    r7 = r72;
                    if (i < length) {
                        return Unit.INSTANCE;
                    }
                }
                break;
            case 1:
                length = anonymousClass12.I$1;
                i2 = anonymousClass12.I$0;
                objArr2 = (Object[]) anonymousClass12.L$1;
                FlowCollector flowCollector2 = (FlowCollector) anonymousClass12.L$0;
                ResultKt.throwOnFailure(obj);
                r72 = flowCollector2;
                objArr = objArr2;
                i = i2;
                r7 = r72;
                if (i < length) {
                }
                break;
            default:
                throw new IllegalStateException("call to 'resume' before 'invoke' with coroutine");
        }
    }

    public FlowKt__BuildersKt$asFlow$$inlined$unsafeFlow$6(Object[] objArr) {
        this.$this_asFlow$inlined = objArr;
    }
}
