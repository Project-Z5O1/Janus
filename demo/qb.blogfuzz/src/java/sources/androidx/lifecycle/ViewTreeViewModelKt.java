package androidx.lifecycle;

import android.view.View;
import androidx.constraintlayout.widget.ConstraintLayout;
import kotlin.Metadata;
import kotlin.jvm.internal.Intrinsics;

/* JADX INFO: compiled from: ViewTreeViewModel.kt */
/* JADX INFO: loaded from: classes.dex */
@Metadata(d1 = {"\u0000\f\n\u0000\n\u0002\u0018\u0002\n\u0002\u0018\u0002\n\u0000\u001a\f\u0010\u0000\u001a\u0004\u0018\u00010\u0001*\u00020\u0002¨\u0006\u0003"}, d2 = {"findViewTreeViewModelStoreOwner", "Landroidx/lifecycle/ViewModelStoreOwner;", "Landroid/view/View;", "lifecycle-viewmodel_release"}, k = 2, mv = {1, 6, 0}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
public final class ViewTreeViewModelKt {
    public static final ViewModelStoreOwner findViewTreeViewModelStoreOwner(View $this$findViewTreeViewModelStoreOwner) {
        Intrinsics.checkNotNullParameter($this$findViewTreeViewModelStoreOwner, "<this>");
        return ViewTreeViewModelStoreOwner.get($this$findViewTreeViewModelStoreOwner);
    }
}
