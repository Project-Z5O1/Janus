package androidx.constraintlayout.core.motion.key;

import androidx.constraintlayout.core.motion.CustomVariable;
import androidx.constraintlayout.core.motion.utils.SplineSet;
import androidx.constraintlayout.core.motion.utils.TypedValues;
import java.util.HashMap;
import java.util.HashSet;

/* JADX INFO: loaded from: classes.dex */
public class MotionKeyAttributes extends MotionKey {
    private static final boolean DEBUG = false;
    public static final int KEY_TYPE = 1;
    static final String NAME = "KeyAttribute";
    private static final String TAG = "KeyAttributes";
    private String mTransitionEasing;
    private int mCurveFit = -1;
    private int mVisibility = 0;
    private float mAlpha = Float.NaN;
    private float mElevation = Float.NaN;
    private float mRotation = Float.NaN;
    private float mRotationX = Float.NaN;
    private float mRotationY = Float.NaN;
    private float mPivotX = Float.NaN;
    private float mPivotY = Float.NaN;
    private float mTransitionPathRotate = Float.NaN;
    private float mScaleX = Float.NaN;
    private float mScaleY = Float.NaN;
    private float mTranslationX = Float.NaN;
    private float mTranslationY = Float.NaN;
    private float mTranslationZ = Float.NaN;
    private float mProgress = Float.NaN;

    public MotionKeyAttributes() {
        this.mType = 1;
        this.mCustom = new HashMap<>();
    }

    @Override // androidx.constraintlayout.core.motion.key.MotionKey
    public void getAttributeNames(HashSet<String> attributes) {
        if (!Float.isNaN(this.mAlpha)) {
            attributes.add("alpha");
        }
        if (!Float.isNaN(this.mElevation)) {
            attributes.add("elevation");
        }
        if (!Float.isNaN(this.mRotation)) {
            attributes.add("rotationZ");
        }
        if (!Float.isNaN(this.mRotationX)) {
            attributes.add("rotationX");
        }
        if (!Float.isNaN(this.mRotationY)) {
            attributes.add("rotationY");
        }
        if (!Float.isNaN(this.mPivotX)) {
            attributes.add("pivotX");
        }
        if (!Float.isNaN(this.mPivotY)) {
            attributes.add("pivotY");
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
            attributes.add("pathRotate");
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
        if (this.mCustom.size() > 0) {
            for (String s : this.mCustom.keySet()) {
                attributes.add("CUSTOM," + s);
            }
        }
    }

    /* JADX WARN: Can't fix incorrect switch cases order, some code will duplicate */
    /* JADX WARN: Removed duplicated region for block: B:58:0x00e0  */
    @Override // androidx.constraintlayout.core.motion.key.MotionKey
    /*
        Code decompiled incorrectly, please refer to instructions dump.
    */
    public void addValues(HashMap<String, SplineSet> splines) {
        for (String s : splines.keySet()) {
            SplineSet splineSet = splines.get(s);
            if (splineSet != null) {
                if (s.startsWith("CUSTOM")) {
                    String cKey = s.substring("CUSTOM".length() + 1);
                    CustomVariable cValue = this.mCustom.get(cKey);
                    if (cValue != null) {
                        ((SplineSet.CustomSpline) splineSet).setPoint(this.mFramePosition, cValue);
                    }
                } else {
                    switch (s) {
                        case "alpha":
                            if (Float.isNaN(this.mAlpha)) {
                                break;
                            } else {
                                splineSet.setPoint(this.mFramePosition, this.mAlpha);
                                break;
                            }
                            break;
                        case "elevation":
                            if (Float.isNaN(this.mElevation)) {
                                break;
                            } else {
                                splineSet.setPoint(this.mFramePosition, this.mElevation);
                                break;
                            }
                            break;
                        case "rotationZ":
                            if (Float.isNaN(this.mRotation)) {
                                break;
                            } else {
                                splineSet.setPoint(this.mFramePosition, this.mRotation);
                                break;
                            }
                            break;
                        case "rotationX":
                            if (Float.isNaN(this.mRotationX)) {
                                break;
                            } else {
                                splineSet.setPoint(this.mFramePosition, this.mRotationX);
                                break;
                            }
                            break;
                        case "rotationY":
                            if (Float.isNaN(this.mRotationY)) {
                                break;
                            } else {
                                splineSet.setPoint(this.mFramePosition, this.mRotationY);
                                break;
                            }
                            break;
                        case "pivotX":
                            if (Float.isNaN(this.mRotationX)) {
                                break;
                            } else {
                                splineSet.setPoint(this.mFramePosition, this.mPivotX);
                                break;
                            }
                            break;
                        case "pivotY":
                            if (Float.isNaN(this.mRotationY)) {
                                break;
                            } else {
                                splineSet.setPoint(this.mFramePosition, this.mPivotY);
                                break;
                            }
                            break;
                        case "pathRotate":
                            if (Float.isNaN(this.mTransitionPathRotate)) {
                                break;
                            } else {
                                splineSet.setPoint(this.mFramePosition, this.mTransitionPathRotate);
                                break;
                            }
                            break;
                        case "scaleX":
                            if (Float.isNaN(this.mScaleX)) {
                                break;
                            } else {
                                splineSet.setPoint(this.mFramePosition, this.mScaleX);
                                break;
                            }
                            break;
                        case "scaleY":
                            if (Float.isNaN(this.mScaleY)) {
                                break;
                            } else {
                                splineSet.setPoint(this.mFramePosition, this.mScaleY);
                                break;
                            }
                            break;
                        case "translationX":
                            if (Float.isNaN(this.mTranslationX)) {
                                break;
                            } else {
                                splineSet.setPoint(this.mFramePosition, this.mTranslationX);
                                break;
                            }
                            break;
                        case "translationY":
                            if (Float.isNaN(this.mTranslationY)) {
                                break;
                            } else {
                                splineSet.setPoint(this.mFramePosition, this.mTranslationY);
                                break;
                            }
                            break;
                        case "translationZ":
                            if (Float.isNaN(this.mTranslationZ)) {
                                break;
                            } else {
                                splineSet.setPoint(this.mFramePosition, this.mTranslationZ);
                                break;
                            }
                            break;
                        case "progress":
                            if (Float.isNaN(this.mProgress)) {
                                break;
                            } else {
                                splineSet.setPoint(this.mFramePosition, this.mProgress);
                                break;
                            }
                            break;
                        default:
                            System.err.println("not supported by KeyAttributes " + s);
                            break;
                    }
                }
            }
        }
    }

    @Override // androidx.constraintlayout.core.motion.key.MotionKey
    /* JADX INFO: renamed from: clone */
    public MotionKey mo8clone() {
        return null;
    }

    @Override // androidx.constraintlayout.core.motion.key.MotionKey, androidx.constraintlayout.core.motion.utils.TypedValues
    public boolean setValue(int type, int value) {
        switch (type) {
            case 100:
                this.mFramePosition = value;
                return true;
            case 301:
                this.mCurveFit = value;
                return true;
            case 302:
                this.mVisibility = value;
                return true;
            default:
                if (!setValue(type, value)) {
                    return super.setValue(type, value);
                }
                return true;
        }
    }

    @Override // androidx.constraintlayout.core.motion.key.MotionKey, androidx.constraintlayout.core.motion.utils.TypedValues
    public boolean setValue(int type, float value) {
        switch (type) {
            case 100:
                this.mTransitionPathRotate = value;
                return true;
            case 303:
                this.mAlpha = value;
                return true;
            case 304:
                this.mTranslationX = value;
                return true;
            case 305:
                this.mTranslationY = value;
                return true;
            case 306:
                this.mTranslationZ = value;
                return true;
            case 307:
                this.mElevation = value;
                return true;
            case 308:
                this.mRotationX = value;
                return true;
            case 309:
                this.mRotationY = value;
                return true;
            case 310:
                this.mRotation = value;
                return true;
            case 311:
                this.mScaleX = value;
                return true;
            case 312:
                this.mScaleY = value;
                return true;
            case 313:
                this.mPivotX = value;
                return true;
            case 314:
                this.mPivotY = value;
                return true;
            case 315:
                this.mProgress = value;
                return true;
            case TypedValues.AttributesType.TYPE_PATH_ROTATE /* 316 */:
                this.mTransitionPathRotate = value;
                return true;
            default:
                return super.setValue(type, value);
        }
    }

    @Override // androidx.constraintlayout.core.motion.key.MotionKey
    public void setInterpolation(HashMap<String, Integer> interpolation) {
        if (!Float.isNaN(this.mAlpha)) {
            interpolation.put("alpha", Integer.valueOf(this.mCurveFit));
        }
        if (!Float.isNaN(this.mElevation)) {
            interpolation.put("elevation", Integer.valueOf(this.mCurveFit));
        }
        if (!Float.isNaN(this.mRotation)) {
            interpolation.put("rotationZ", Integer.valueOf(this.mCurveFit));
        }
        if (!Float.isNaN(this.mRotationX)) {
            interpolation.put("rotationX", Integer.valueOf(this.mCurveFit));
        }
        if (!Float.isNaN(this.mRotationY)) {
            interpolation.put("rotationY", Integer.valueOf(this.mCurveFit));
        }
        if (!Float.isNaN(this.mPivotX)) {
            interpolation.put("pivotX", Integer.valueOf(this.mCurveFit));
        }
        if (!Float.isNaN(this.mPivotY)) {
            interpolation.put("pivotY", Integer.valueOf(this.mCurveFit));
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
            interpolation.put("pathRotate", Integer.valueOf(this.mCurveFit));
        }
        if (!Float.isNaN(this.mScaleX)) {
            interpolation.put("scaleX", Integer.valueOf(this.mCurveFit));
        }
        if (!Float.isNaN(this.mScaleY)) {
            interpolation.put("scaleY", Integer.valueOf(this.mCurveFit));
        }
        if (!Float.isNaN(this.mProgress)) {
            interpolation.put("progress", Integer.valueOf(this.mCurveFit));
        }
        if (this.mCustom.size() > 0) {
            for (String s : this.mCustom.keySet()) {
                interpolation.put("CUSTOM," + s, Integer.valueOf(this.mCurveFit));
            }
        }
    }

    @Override // androidx.constraintlayout.core.motion.key.MotionKey, androidx.constraintlayout.core.motion.utils.TypedValues
    public boolean setValue(int type, String value) {
        switch (type) {
            case TypedValues.TYPE_TARGET /* 101 */:
                this.mTargetString = value;
                return true;
            case TypedValues.AttributesType.TYPE_EASING /* 317 */:
                this.mTransitionEasing = value;
                return true;
            default:
                return super.setValue(type, value);
        }
    }

    @Override // androidx.constraintlayout.core.motion.utils.TypedValues
    public int getId(String name) {
        return TypedValues.AttributesType.getId(name);
    }

    public int getCurveFit() {
        return this.mCurveFit;
    }

    public void printAttributes() {
        HashSet<String> nameSet = new HashSet<>();
        getAttributeNames(nameSet);
        System.out.println(" ------------- " + this.mFramePosition + " -------------");
        String[] names = (String[]) nameSet.toArray(new String[0]);
        for (int i = 0; i < names.length; i++) {
            int id = TypedValues.AttributesType.getId(names[i]);
            System.out.println(names[i] + ":" + getFloatValue(id));
        }
    }

    private float getFloatValue(int id) {
        switch (id) {
            case 100:
                return this.mFramePosition;
            case 303:
                return this.mAlpha;
            case 304:
                return this.mTranslationX;
            case 305:
                return this.mTranslationY;
            case 306:
                return this.mTranslationZ;
            case 307:
                return this.mElevation;
            case 308:
                return this.mRotationX;
            case 309:
                return this.mRotationY;
            case 310:
                return this.mRotation;
            case 311:
                return this.mScaleX;
            case 312:
                return this.mScaleY;
            case 313:
                return this.mPivotX;
            case 314:
                return this.mPivotY;
            case 315:
                return this.mProgress;
            case TypedValues.AttributesType.TYPE_PATH_ROTATE /* 316 */:
                return this.mTransitionPathRotate;
            default:
                return Float.NaN;
        }
    }
}
