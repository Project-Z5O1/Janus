package androidx.constraintlayout.motion.widget;

import android.content.Context;
import android.content.res.TypedArray;
import android.util.AttributeSet;
import android.util.Log;
import android.util.SparseIntArray;
import android.util.TypedValue;
import androidx.constraintlayout.motion.utils.ViewSpline;
import androidx.constraintlayout.motion.utils.ViewTimeCycle;
import androidx.constraintlayout.widget.ConstraintAttribute;
import androidx.constraintlayout.widget.R;
import java.util.HashMap;
import java.util.HashSet;

/* JADX INFO: loaded from: classes.dex */
public class KeyTimeCycle extends Key {
    public static final int KEY_TYPE = 3;
    static final String NAME = "KeyTimeCycle";
    public static final int SHAPE_BOUNCE = 6;
    public static final int SHAPE_COS_WAVE = 5;
    public static final int SHAPE_REVERSE_SAW_WAVE = 4;
    public static final int SHAPE_SAW_WAVE = 3;
    public static final int SHAPE_SIN_WAVE = 0;
    public static final int SHAPE_SQUARE_WAVE = 1;
    public static final int SHAPE_TRIANGLE_WAVE = 2;
    private static final String TAG = "KeyTimeCycle";
    public static final String WAVE_OFFSET = "waveOffset";
    public static final String WAVE_PERIOD = "wavePeriod";
    public static final String WAVE_SHAPE = "waveShape";
    private String mTransitionEasing;
    private int mCurveFit = -1;
    private float mAlpha = Float.NaN;
    private float mElevation = Float.NaN;
    private float mRotation = Float.NaN;
    private float mRotationX = Float.NaN;
    private float mRotationY = Float.NaN;
    private float mTransitionPathRotate = Float.NaN;
    private float mScaleX = Float.NaN;
    private float mScaleY = Float.NaN;
    private float mTranslationX = Float.NaN;
    private float mTranslationY = Float.NaN;
    private float mTranslationZ = Float.NaN;
    private float mProgress = Float.NaN;
    private int mWaveShape = 0;
    private String mCustomWaveShape = null;
    private float mWavePeriod = Float.NaN;
    private float mWaveOffset = 0.0f;

    public KeyTimeCycle() {
        this.mType = 3;
        this.mCustomConstraints = new HashMap<>();
    }

    @Override // androidx.constraintlayout.motion.widget.Key
    public void load(Context context, AttributeSet attrs) {
        TypedArray a = context.obtainStyledAttributes(attrs, R.styleable.KeyTimeCycle);
        Loader.read(this, a);
    }

    @Override // androidx.constraintlayout.motion.widget.Key
    public void getAttributeNames(HashSet<String> attributes) {
        if (!Float.isNaN(this.mAlpha)) {
            attributes.add("alpha");
        }
        if (!Float.isNaN(this.mElevation)) {
            attributes.add("elevation");
        }
        if (!Float.isNaN(this.mRotation)) {
            attributes.add(Key.ROTATION);
        }
        if (!Float.isNaN(this.mRotationX)) {
            attributes.add("rotationX");
        }
        if (!Float.isNaN(this.mRotationY)) {
            attributes.add("rotationY");
        }
        if (!Float.isNaN(this.mTranslationX)) {
            attributes.add("translationX");
        }
        if (!Float.isNaN(this.mTranslationY)) {
            attributes.add("translationY");
        }
        if (!Float.isNaN(this.mTranslationZ)) {
            attributes.add("translationZ");
        }
        if (!Float.isNaN(this.mTransitionPathRotate)) {
            attributes.add("transitionPathRotate");
        }
        if (!Float.isNaN(this.mScaleX)) {
            attributes.add("scaleX");
        }
        if (!Float.isNaN(this.mScaleY)) {
            attributes.add("scaleY");
        }
        if (!Float.isNaN(this.mProgress)) {
            attributes.add("progress");
        }
        if (this.mCustomConstraints.size() > 0) {
            for (String s : this.mCustomConstraints.keySet()) {
                attributes.add("CUSTOM," + s);
            }
        }
    }

    @Override // androidx.constraintlayout.motion.widget.Key
    public void setInterpolation(HashMap<String, Integer> interpolation) {
        if (this.mCurveFit == -1) {
            return;
        }
        if (!Float.isNaN(this.mAlpha)) {
            interpolation.put("alpha", Integer.valueOf(this.mCurveFit));
        }
        if (!Float.isNaN(this.mElevation)) {
            interpolation.put("elevation", Integer.valueOf(this.mCurveFit));
        }
        if (!Float.isNaN(this.mRotation)) {
            interpolation.put(Key.ROTATION, Integer.valueOf(this.mCurveFit));
        }
        if (!Float.isNaN(this.mRotationX)) {
            interpolation.put("rotationX", Integer.valueOf(this.mCurveFit));
        }
        if (!Float.isNaN(this.mRotationY)) {
            interpolation.put("rotationY", Integer.valueOf(this.mCurveFit));
        }
        if (!Float.isNaN(this.mTranslationX)) {
            interpolation.put("translationX", Integer.valueOf(this.mCurveFit));
        }
        if (!Float.isNaN(this.mTranslationY)) {
            interpolation.put("translationY", Integer.valueOf(this.mCurveFit));
        }
        if (!Float.isNaN(this.mTranslationZ)) {
            interpolation.put("translationZ", Integer.valueOf(this.mCurveFit));
        }
        if (!Float.isNaN(this.mTransitionPathRotate)) {
            interpolation.put("transitionPathRotate", Integer.valueOf(this.mCurveFit));
        }
        if (!Float.isNaN(this.mScaleX)) {
            interpolation.put("scaleX", Integer.valueOf(this.mCurveFit));
        }
        if (!Float.isNaN(this.mScaleX)) {
            interpolation.put("scaleY", Integer.valueOf(this.mCurveFit));
        }
        if (!Float.isNaN(this.mProgress)) {
            interpolation.put("progress", Integer.valueOf(this.mCurveFit));
        }
        if (this.mCustomConstraints.size() > 0) {
            for (String s : this.mCustomConstraints.keySet()) {
                interpolation.put("CUSTOM," + s, Integer.valueOf(this.mCurveFit));
            }
        }
    }

    @Override // androidx.constraintlayout.motion.widget.Key
    public void addValues(HashMap<String, ViewSpline> splines) {
        throw new IllegalArgumentException(" KeyTimeCycles do not support SplineSet");
    }

    /* JADX WARN: Can't fix incorrect switch cases order, some code will duplicate */
    /* JADX WARN: Removed duplicated region for block: B:52:0x00cf  */
    /*
        Code decompiled incorrectly, please refer to instructions dump.
    */
    public void addTimeValues(HashMap<String, ViewTimeCycle> splines) {
        for (String s : splines.keySet()) {
            ViewTimeCycle splineSet = splines.get(s);
            if (splineSet != null) {
                if (s.startsWith("CUSTOM")) {
                    String cKey = s.substring("CUSTOM".length() + 1);
                    ConstraintAttribute cValue = this.mCustomConstraints.get(cKey);
                    if (cValue != null) {
                        ((ViewTimeCycle.CustomSet) splineSet).setPoint(this.mFramePosition, cValue, this.mWavePeriod, this.mWaveShape, this.mWaveOffset);
                    }
                } else {
                    switch (s) {
                        case "alpha":
                            if (Float.isNaN(this.mAlpha)) {
                                break;
                            } else {
                                splineSet.setPoint(this.mFramePosition, this.mAlpha, this.mWavePeriod, this.mWaveShape, this.mWaveOffset);
                                break;
                            }
                            break;
                        case "elevation":
                            if (Float.isNaN(this.mElevation)) {
                                break;
                            } else {
                                splineSet.setPoint(this.mFramePosition, this.mElevation, this.mWavePeriod, this.mWaveShape, this.mWaveOffset);
                                break;
                            }
                            break;
                        case "rotation":
                            if (Float.isNaN(this.mRotation)) {
                                break;
                            } else {
                                splineSet.setPoint(this.mFramePosition, this.mRotation, this.mWavePeriod, this.mWaveShape, this.mWaveOffset);
                                break;
                            }
                            break;
                        case "rotationX":
                            if (Float.isNaN(this.mRotationX)) {
                                break;
                            } else {
                                splineSet.setPoint(this.mFramePosition, this.mRotationX, this.mWavePeriod, this.mWaveShape, this.mWaveOffset);
                                break;
                            }
                            break;
                        case "rotationY":
                            if (Float.isNaN(this.mRotationY)) {
                                break;
                            } else {
                                splineSet.setPoint(this.mFramePosition, this.mRotationY, this.mWavePeriod, this.mWaveShape, this.mWaveOffset);
                                break;
                            }
                            break;
                        case "transitionPathRotate":
                            if (Float.isNaN(this.mTransitionPathRotate)) {
                                break;
                            } else {
                                splineSet.setPoint(this.mFramePosition, this.mTransitionPathRotate, this.mWavePeriod, this.mWaveShape, this.mWaveOffset);
                                break;
                            }
                            break;
                        case "scaleX":
                            if (Float.isNaN(this.mScaleX)) {
                                break;
                            } else {
                                splineSet.setPoint(this.mFramePosition, this.mScaleX, this.mWavePeriod, this.mWaveShape, this.mWaveOffset);
                                break;
                            }
                            break;
                        case "scaleY":
                            if (Float.isNaN(this.mScaleY)) {
                                break;
                            } else {
                                splineSet.setPoint(this.mFramePosition, this.mScaleY, this.mWavePeriod, this.mWaveShape, this.mWaveOffset);
                                break;
                            }
                            break;
                        case "translationX":
                            if (Float.isNaN(this.mTranslationX)) {
                                break;
                            } else {
                                splineSet.setPoint(this.mFramePosition, this.mTranslationX, this.mWavePeriod, this.mWaveShape, this.mWaveOffset);
                                break;
                            }
                            break;
                        case "translationY":
                            if (Float.isNaN(this.mTranslationY)) {
                                break;
                            } else {
                                splineSet.setPoint(this.mFramePosition, this.mTranslationY, this.mWavePeriod, this.mWaveShape, this.mWaveOffset);
                                break;
                            }
                            break;
                        case "translationZ":
                            if (Float.isNaN(this.mTranslationZ)) {
                                break;
                            } else {
                                splineSet.setPoint(this.mFramePosition, this.mTranslationZ, this.mWavePeriod, this.mWaveShape, this.mWaveOffset);
                                break;
                            }
                            break;
                        case "progress":
                            if (Float.isNaN(this.mProgress)) {
                                break;
                            } else {
                                splineSet.setPoint(this.mFramePosition, this.mProgress, this.mWavePeriod, this.mWaveShape, this.mWaveOffset);
                                break;
                            }
                            break;
                        default:
                            Log.e("KeyTimeCycles", "UNKNOWN addValues \"" + s + "\"");
                            break;
                    }
                }
            }
        }
    }

    /* JADX WARN: Can't fix incorrect switch cases order, some code will duplicate */
    /* JADX WARN: Removed duplicated region for block: B:56:0x00c8  */
    @Override // androidx.constraintlayout.motion.widget.Key
    /*
        Code decompiled incorrectly, please refer to instructions dump.
    */
    public void setValue(String tag, Object value) {
        switch (tag) {
            case "alpha":
                this.mAlpha = toFloat(value);
                break;
            case "curveFit":
                this.mCurveFit = toInt(value);
                break;
            case "elevation":
                this.mElevation = toFloat(value);
                break;
            case "motionProgress":
                this.mProgress = toFloat(value);
                break;
            case "rotation":
                this.mRotation = toFloat(value);
                break;
            case "rotationX":
                this.mRotationX = toFloat(value);
                break;
            case "rotationY":
                this.mRotationY = toFloat(value);
                break;
            case "scaleX":
                this.mScaleX = toFloat(value);
                break;
            case "scaleY":
                this.mScaleY = toFloat(value);
                break;
            case "transitionEasing":
                this.mTransitionEasing = value.toString();
                break;
            case "transitionPathRotate":
                this.mTransitionPathRotate = toFloat(value);
                break;
            case "translationX":
                this.mTranslationX = toFloat(value);
                break;
            case "translationY":
                this.mTranslationY = toFloat(value);
                break;
            case "translationZ":
                this.mTranslationZ = toFloat(value);
                break;
            case "wavePeriod":
                this.mWavePeriod = toFloat(value);
                break;
            case "waveOffset":
                this.mWaveOffset = toFloat(value);
                break;
            case "waveShape":
                if (value instanceof Integer) {
                    this.mWaveShape = toInt(value);
                    break;
                } else {
                    this.mWaveShape = 7;
                    this.mCustomWaveShape = value.toString();
                    break;
                }
                break;
        }
    }

    private static class Loader {
        private static final int ANDROID_ALPHA = 1;
        private static final int ANDROID_ELEVATION = 2;
        private static final int ANDROID_ROTATION = 4;
        private static final int ANDROID_ROTATION_X = 5;
        private static final int ANDROID_ROTATION_Y = 6;
        private static final int ANDROID_SCALE_X = 7;
        private static final int ANDROID_SCALE_Y = 14;
        private static final int ANDROID_TRANSLATION_X = 15;
        private static final int ANDROID_TRANSLATION_Y = 16;
        private static final int ANDROID_TRANSLATION_Z = 17;
        private static final int CURVE_FIT = 13;
        private static final int FRAME_POSITION = 12;
        private static final int PROGRESS = 18;
        private static final int TARGET_ID = 10;
        private static final int TRANSITION_EASING = 9;
        private static final int TRANSITION_PATH_ROTATE = 8;
        private static final int WAVE_OFFSET = 21;
        private static final int WAVE_PERIOD = 20;
        private static final int WAVE_SHAPE = 19;
        private static SparseIntArray mAttrMap;

        private Loader() {
        }

        static {
            SparseIntArray sparseIntArray = new SparseIntArray();
            mAttrMap = sparseIntArray;
            sparseIntArray.append(R.styleable.KeyTimeCycle_android_alpha, 1);
            mAttrMap.append(R.styleable.KeyTimeCycle_android_elevation, 2);
            mAttrMap.append(R.styleable.KeyTimeCycle_android_rotation, 4);
            mAttrMap.append(R.styleable.KeyTimeCycle_android_rotationX, 5);
            mAttrMap.append(R.styleable.KeyTimeCycle_android_rotationY, 6);
            mAttrMap.append(R.styleable.KeyTimeCycle_android_scaleX, 7);
            mAttrMap.append(R.styleable.KeyTimeCycle_transitionPathRotate, 8);
            mAttrMap.append(R.styleable.KeyTimeCycle_transitionEasing, 9);
            mAttrMap.append(R.styleable.KeyTimeCycle_motionTarget, 10);
            mAttrMap.append(R.styleable.KeyTimeCycle_framePosition, 12);
            mAttrMap.append(R.styleable.KeyTimeCycle_curveFit, 13);
            mAttrMap.append(R.styleable.KeyTimeCycle_android_scaleY, 14);
            mAttrMap.append(R.styleable.KeyTimeCycle_android_translationX, 15);
            mAttrMap.append(R.styleable.KeyTimeCycle_android_translationY, 16);
            mAttrMap.append(R.styleable.KeyTimeCycle_android_translationZ, 17);
            mAttrMap.append(R.styleable.KeyTimeCycle_motionProgress, 18);
            mAttrMap.append(R.styleable.KeyTimeCycle_wavePeriod, 20);
            mAttrMap.append(R.styleable.KeyTimeCycle_waveOffset, 21);
            mAttrMap.append(R.styleable.KeyTimeCycle_waveShape, 19);
        }

        public static void read(KeyTimeCycle c, TypedArray a) {
            int N = a.getIndexCount();
            for (int i = 0; i < N; i++) {
                int attr = a.getIndex(i);
                switch (mAttrMap.get(attr)) {
                    case 1:
                        c.mAlpha = a.getFloat(attr, c.mAlpha);
                        break;
                    case 2:
                        c.mElevation = a.getDimension(attr, c.mElevation);
                        break;
                    case 3:
                    case 11:
                    default:
                        Log.e("KeyTimeCycle", "unused attribute 0x" + Integer.toHexString(attr) + "   " + mAttrMap.get(attr));
                        break;
                    case 4:
                        c.mRotation = a.getFloat(attr, c.mRotation);
                        break;
                    case 5:
                        c.mRotationX = a.getFloat(attr, c.mRotationX);
                        break;
                    case 6:
                        c.mRotationY = a.getFloat(attr, c.mRotationY);
                        break;
                    case 7:
                        c.mScaleX = a.getFloat(attr, c.mScaleX);
                        break;
                    case 8:
                        c.mTransitionPathRotate = a.getFloat(attr, c.mTransitionPathRotate);
                        break;
                    case 9:
                        c.mTransitionEasing = a.getString(attr);
                        break;
                    case 10:
                        if (MotionLayout.IS_IN_EDIT_MODE) {
                            c.mTargetId = a.getResourceId(attr, c.mTargetId);
                            if (c.mTargetId == -1) {
                                c.mTargetString = a.getString(attr);
                            }
                        } else if (a.peekValue(attr).type == 3) {
                            c.mTargetString = a.getString(attr);
                        } else {
                            c.mTargetId = a.getResourceId(attr, c.mTargetId);
                        }
                        break;
                    case 12:
                        c.mFramePosition = a.getInt(attr, c.mFramePosition);
                        break;
                    case 13:
                        c.mCurveFit = a.getInteger(attr, c.mCurveFit);
                        break;
                    case 14:
                        c.mScaleY = a.getFloat(attr, c.mScaleY);
                        break;
                    case 15:
                        c.mTranslationX = a.getDimension(attr, c.mTranslationX);
                        break;
                    case 16:
                        c.mTranslationY = a.getDimension(attr, c.mTranslationY);
                        break;
                    case 17:
                        c.mTranslationZ = a.getDimension(attr, c.mTranslationZ);
                        break;
                    case 18:
                        c.mProgress = a.getFloat(attr, c.mProgress);
                        break;
                    case 19:
                        if (a.peekValue(attr).type == 3) {
                            c.mCustomWaveShape = a.getString(attr);
                            c.mWaveShape = 7;
                        } else {
                            c.mWaveShape = a.getInt(attr, c.mWaveShape);
                        }
                        break;
                    case 20:
                        c.mWavePeriod = a.getFloat(attr, c.mWavePeriod);
                        break;
                    case 21:
                        TypedValue type = a.peekValue(attr);
                        if (type.type == 5) {
                            c.mWaveOffset = a.getDimension(attr, c.mWaveOffset);
                        } else {
                            c.mWaveOffset = a.getFloat(attr, c.mWaveOffset);
                        }
                        break;
                }
            }
        }
    }

    @Override // androidx.constraintlayout.motion.widget.Key
    public Key copy(Key src) {
        super.copy(src);
        KeyTimeCycle k = (KeyTimeCycle) src;
        this.mTransitionEasing = k.mTransitionEasing;
        this.mCurveFit = k.mCurveFit;
        this.mWaveShape = k.mWaveShape;
        this.mWavePeriod = k.mWavePeriod;
        this.mWaveOffset = k.mWaveOffset;
        this.mProgress = k.mProgress;
        this.mAlpha = k.mAlpha;
        this.mElevation = k.mElevation;
        this.mRotation = k.mRotation;
        this.mTransitionPathRotate = k.mTransitionPathRotate;
        this.mRotationX = k.mRotationX;
        this.mRotationY = k.mRotationY;
        this.mScaleX = k.mScaleX;
        this.mScaleY = k.mScaleY;
        this.mTranslationX = k.mTranslationX;
        this.mTranslationY = k.mTranslationY;
        this.mTranslationZ = k.mTranslationZ;
        return this;
    }

    @Override // androidx.constraintlayout.motion.widget.Key
    /* JADX INFO: renamed from: clone */
    public Key mo10clone() {
        return new KeyTimeCycle().copy(this);
    }
}
