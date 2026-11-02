package com.google.android.material.color.utilities;

/* JADX INFO: loaded from: classes.dex */
public class SchemeExpressive extends DynamicScheme {
    private static final double[] HUES = {0.0d, 21.0d, 51.0d, 121.0d, 151.0d, 191.0d, 271.0d, 321.0d, 360.0d};
    private static final double[] SECONDARY_ROTATIONS = {45.0d, 95.0d, 45.0d, 20.0d, 45.0d, 90.0d, 45.0d, 45.0d, 45.0d};
    private static final double[] TERTIARY_ROTATIONS = {120.0d, 120.0d, 20.0d, 45.0d, 20.0d, 15.0d, 20.0d, 120.0d, 120.0d};

    /* JADX WARN: Illegal instructions before constructor call */
    public SchemeExpressive(Hct sourceColorHct, boolean isDark, double contrastLevel) {
        Variant variant = Variant.EXPRESSIVE;
        TonalPalette tonalPaletteFromHueAndChroma = TonalPalette.fromHueAndChroma(MathUtils.sanitizeDegreesDouble(sourceColorHct.getHue() + 120.0d), 40.0d);
        double[] dArr = HUES;
        super(sourceColorHct, variant, isDark, contrastLevel, tonalPaletteFromHueAndChroma, TonalPalette.fromHueAndChroma(DynamicScheme.getRotatedHue(sourceColorHct, dArr, SECONDARY_ROTATIONS), 24.0d), TonalPalette.fromHueAndChroma(DynamicScheme.getRotatedHue(sourceColorHct, dArr, TERTIARY_ROTATIONS), 32.0d), TonalPalette.fromHueAndChroma(sourceColorHct.getHue(), 8.0d), TonalPalette.fromHueAndChroma(sourceColorHct.getHue(), 12.0d));
    }
}
