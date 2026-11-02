package com.google.android.material.progressindicator;

import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.Rect;
import android.graphics.RectF;
import com.google.android.material.color.MaterialColors;

/* JADX INFO: loaded from: classes.dex */
final class LinearDrawingDelegate extends DrawingDelegate<LinearProgressIndicatorSpec> {
    private float displayedCornerRadius;
    private Path displayedTrackPath;
    private float displayedTrackThickness;
    private float trackLength;

    public LinearDrawingDelegate(LinearProgressIndicatorSpec spec) {
        super(spec);
        this.trackLength = 300.0f;
    }

    @Override // com.google.android.material.progressindicator.DrawingDelegate
    public int getPreferredWidth() {
        return -1;
    }

    @Override // com.google.android.material.progressindicator.DrawingDelegate
    public int getPreferredHeight() {
        return ((LinearProgressIndicatorSpec) this.spec).trackThickness;
    }

    @Override // com.google.android.material.progressindicator.DrawingDelegate
    public void adjustCanvas(Canvas canvas, Rect bounds, float trackThicknessFraction) {
        this.trackLength = bounds.width();
        float trackSize = ((LinearProgressIndicatorSpec) this.spec).trackThickness;
        canvas.translate(bounds.left + (bounds.width() / 2.0f), bounds.top + (bounds.height() / 2.0f) + Math.max(0.0f, (bounds.height() - ((LinearProgressIndicatorSpec) this.spec).trackThickness) / 2.0f));
        if (((LinearProgressIndicatorSpec) this.spec).drawHorizontallyInverse) {
            canvas.scale(-1.0f, 1.0f);
        }
        if ((this.drawable.isShowing() && ((LinearProgressIndicatorSpec) this.spec).showAnimationBehavior == 1) || (this.drawable.isHiding() && ((LinearProgressIndicatorSpec) this.spec).hideAnimationBehavior == 2)) {
            canvas.scale(1.0f, -1.0f);
        }
        if (this.drawable.isShowing() || this.drawable.isHiding()) {
            canvas.translate(0.0f, (((LinearProgressIndicatorSpec) this.spec).trackThickness * (trackThicknessFraction - 1.0f)) / 2.0f);
        }
        float f = this.trackLength;
        canvas.clipRect((-f) / 2.0f, (-trackSize) / 2.0f, f / 2.0f, trackSize / 2.0f);
        this.displayedTrackThickness = ((LinearProgressIndicatorSpec) this.spec).trackThickness * trackThicknessFraction;
        this.displayedCornerRadius = ((LinearProgressIndicatorSpec) this.spec).trackCornerRadius * trackThicknessFraction;
    }

    @Override // com.google.android.material.progressindicator.DrawingDelegate
    public void fillIndicator(Canvas canvas, Paint paint, float startFraction, float endFraction, int color) {
        if (startFraction == endFraction) {
            return;
        }
        float f = this.trackLength;
        float originX = (-f) / 2.0f;
        float adjustedStartX = ((startFraction * f) + originX) - (this.displayedCornerRadius * 2.0f);
        float adjustedEndX = (f * endFraction) + originX;
        paint.setStyle(Paint.Style.FILL);
        paint.setAntiAlias(true);
        paint.setColor(color);
        canvas.save();
        canvas.clipPath(this.displayedTrackPath);
        float f2 = this.displayedTrackThickness;
        RectF indicatorBound = new RectF(adjustedStartX, (-f2) / 2.0f, adjustedEndX, f2 / 2.0f);
        float f3 = this.displayedCornerRadius;
        canvas.drawRoundRect(indicatorBound, f3, f3, paint);
        canvas.restore();
    }

    @Override // com.google.android.material.progressindicator.DrawingDelegate
    void fillTrack(Canvas canvas, Paint paint) {
        int trackColor = MaterialColors.compositeARGBWithAlpha(((LinearProgressIndicatorSpec) this.spec).trackColor, this.drawable.getAlpha());
        paint.setStyle(Paint.Style.FILL);
        paint.setAntiAlias(true);
        paint.setColor(trackColor);
        Path path = new Path();
        this.displayedTrackPath = path;
        float f = this.trackLength;
        float f2 = this.displayedTrackThickness;
        RectF rectF = new RectF((-f) / 2.0f, (-f2) / 2.0f, f / 2.0f, f2 / 2.0f);
        float f3 = this.displayedCornerRadius;
        path.addRoundRect(rectF, f3, f3, Path.Direction.CCW);
        canvas.drawPath(this.displayedTrackPath, paint);
    }
}
