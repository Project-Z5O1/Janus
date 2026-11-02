package androidx.core.text;

import android.text.TextUtils;
import androidx.constraintlayout.widget.ConstraintLayout;
import kotlin.Metadata;
import kotlin.jvm.internal.Intrinsics;

/* JADX INFO: compiled from: String.kt */
/* JADX INFO: loaded from: classes.dex */
@Metadata(d1 = {"\u0000\b\n\u0000\n\u0002\u0010\u000e\n\u0000\u001a\r\u0010\u0000\u001a\u00020\u0001*\u00020\u0001H\u0086\b¨\u0006\u0002"}, d2 = {"htmlEncode", "", "core-ktx_release"}, k = 2, mv = {1, 7, 1}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
public final class StringKt {
    public static final String htmlEncode(String $this$htmlEncode) {
        Intrinsics.checkNotNullParameter($this$htmlEncode, "<this>");
        String strHtmlEncode = TextUtils.htmlEncode($this$htmlEncode);
        Intrinsics.checkNotNullExpressionValue(strHtmlEncode, "htmlEncode(this)");
        return strHtmlEncode;
    }
}
