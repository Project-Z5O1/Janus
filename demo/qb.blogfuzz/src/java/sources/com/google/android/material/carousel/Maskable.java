package com.google.android.material.carousel;

import android.graphics.RectF;

/* JADX INFO: loaded from: classes.dex */
interface Maskable {
    RectF getMaskRectF();

    float getMaskXPercentage();

    void setMaskXPercentage(float f);

    void setOnMaskChangedListener(OnMaskChangedListener onMaskChangedListener);
}
