package com.google.android.material.badge;

import android.content.Context;
import android.content.res.Resources;
import android.content.res.TypedArray;
import android.os.Parcel;
import android.os.Parcelable;
import android.util.AttributeSet;
import com.google.android.material.R;
import com.google.android.material.drawable.DrawableUtils;
import com.google.android.material.internal.ThemeEnforcement;
import com.google.android.material.resources.MaterialResources;
import com.google.android.material.resources.TextAppearance;
import java.util.Locale;

/* JADX INFO: loaded from: classes.dex */
public final class BadgeState {
    private static final String BADGE_RESOURCE_TAG = "badge";
    private static final int DEFAULT_MAX_BADGE_CHARACTER_COUNT = 4;
    final float badgeHeight;
    final float badgeRadius;
    final float badgeWidePadding;
    final float badgeWidth;
    final float badgeWithTextHeight;
    final float badgeWithTextRadius;
    final float badgeWithTextWidth;
    private final State currentState;
    final int horizontalInset;
    final int horizontalInsetWithText;
    int offsetAlignmentMode;
    private final State overridingState;

    BadgeState(Context context, int badgeResId, int defStyleAttr, int defStyleRes, State storedState) {
        CharSequence string;
        int i;
        int i2;
        int i3;
        int iIntValue;
        int iIntValue2;
        int iIntValue3;
        int iIntValue4;
        int iIntValue5;
        int iIntValue6;
        int iIntValue7;
        int iIntValue8;
        int iIntValue9;
        int iIntValue10;
        int iIntValue11;
        State state = new State();
        this.currentState = state;
        storedState = storedState == null ? new State() : storedState;
        if (badgeResId != 0) {
            storedState.badgeResId = badgeResId;
        }
        TypedArray a = generateTypedArray(context, storedState.badgeResId, defStyleAttr, defStyleRes);
        Resources res = context.getResources();
        this.badgeRadius = a.getDimensionPixelSize(R.styleable.Badge_badgeRadius, -1);
        this.badgeWidePadding = a.getDimensionPixelSize(R.styleable.Badge_badgeWidePadding, res.getDimensionPixelSize(R.dimen.mtrl_badge_long_text_horizontal_padding));
        this.horizontalInset = context.getResources().getDimensionPixelSize(R.dimen.mtrl_badge_horizontal_edge_offset);
        this.horizontalInsetWithText = context.getResources().getDimensionPixelSize(R.dimen.mtrl_badge_text_horizontal_edge_offset);
        this.badgeWithTextRadius = a.getDimensionPixelSize(R.styleable.Badge_badgeWithTextRadius, -1);
        this.badgeWidth = a.getDimension(R.styleable.Badge_badgeWidth, res.getDimension(R.dimen.m3_badge_size));
        this.badgeWithTextWidth = a.getDimension(R.styleable.Badge_badgeWithTextWidth, res.getDimension(R.dimen.m3_badge_with_text_size));
        this.badgeHeight = a.getDimension(R.styleable.Badge_badgeHeight, res.getDimension(R.dimen.m3_badge_size));
        this.badgeWithTextHeight = a.getDimension(R.styleable.Badge_badgeWithTextHeight, res.getDimension(R.dimen.m3_badge_with_text_size));
        boolean z = true;
        this.offsetAlignmentMode = a.getInt(R.styleable.Badge_offsetAlignmentMode, 1);
        state.alpha = storedState.alpha == -2 ? 255 : storedState.alpha;
        if (storedState.contentDescriptionNumberless == null) {
            string = context.getString(R.string.mtrl_badge_numberless_content_description);
        } else {
            string = storedState.contentDescriptionNumberless;
        }
        state.contentDescriptionNumberless = string;
        if (storedState.contentDescriptionQuantityStrings == 0) {
            i = R.plurals.mtrl_badge_content_description;
        } else {
            i = storedState.contentDescriptionQuantityStrings;
        }
        state.contentDescriptionQuantityStrings = i;
        if (storedState.contentDescriptionExceedsMaxBadgeNumberRes == 0) {
            i2 = R.string.mtrl_exceed_max_badge_number_content_description;
        } else {
            i2 = storedState.contentDescriptionExceedsMaxBadgeNumberRes;
        }
        state.contentDescriptionExceedsMaxBadgeNumberRes = i2;
        if (storedState.isVisible != null && !storedState.isVisible.booleanValue()) {
            z = false;
        }
        state.isVisible = Boolean.valueOf(z);
        if (storedState.maxCharacterCount == -2) {
            i3 = a.getInt(R.styleable.Badge_maxCharacterCount, 4);
        } else {
            i3 = storedState.maxCharacterCount;
        }
        state.maxCharacterCount = i3;
        if (storedState.number == -2) {
            if (a.hasValue(R.styleable.Badge_number)) {
                state.number = a.getInt(R.styleable.Badge_number, 0);
            } else {
                state.number = -1;
            }
        } else {
            state.number = storedState.number;
        }
        if (storedState.badgeShapeAppearanceResId == null) {
            iIntValue = a.getResourceId(R.styleable.Badge_badgeShapeAppearance, R.style.ShapeAppearance_M3_Sys_Shape_Corner_Full);
        } else {
            iIntValue = storedState.badgeShapeAppearanceResId.intValue();
        }
        state.badgeShapeAppearanceResId = Integer.valueOf(iIntValue);
        if (storedState.badgeShapeAppearanceOverlayResId == null) {
            iIntValue2 = a.getResourceId(R.styleable.Badge_badgeShapeAppearanceOverlay, 0);
        } else {
            iIntValue2 = storedState.badgeShapeAppearanceOverlayResId.intValue();
        }
        state.badgeShapeAppearanceOverlayResId = Integer.valueOf(iIntValue2);
        if (storedState.badgeWithTextShapeAppearanceResId == null) {
            iIntValue3 = a.getResourceId(R.styleable.Badge_badgeWithTextShapeAppearance, R.style.ShapeAppearance_M3_Sys_Shape_Corner_Full);
        } else {
            iIntValue3 = storedState.badgeWithTextShapeAppearanceResId.intValue();
        }
        state.badgeWithTextShapeAppearanceResId = Integer.valueOf(iIntValue3);
        if (storedState.badgeWithTextShapeAppearanceOverlayResId == null) {
            iIntValue4 = a.getResourceId(R.styleable.Badge_badgeWithTextShapeAppearanceOverlay, 0);
        } else {
            iIntValue4 = storedState.badgeWithTextShapeAppearanceOverlayResId.intValue();
        }
        state.badgeWithTextShapeAppearanceOverlayResId = Integer.valueOf(iIntValue4);
        if (storedState.backgroundColor == null) {
            iIntValue5 = readColorFromAttributes(context, a, R.styleable.Badge_backgroundColor);
        } else {
            iIntValue5 = storedState.backgroundColor.intValue();
        }
        state.backgroundColor = Integer.valueOf(iIntValue5);
        if (storedState.badgeTextAppearanceResId == null) {
            iIntValue6 = a.getResourceId(R.styleable.Badge_badgeTextAppearance, R.style.TextAppearance_MaterialComponents_Badge);
        } else {
            iIntValue6 = storedState.badgeTextAppearanceResId.intValue();
        }
        state.badgeTextAppearanceResId = Integer.valueOf(iIntValue6);
        if (storedState.badgeTextColor == null) {
            if (!a.hasValue(R.styleable.Badge_badgeTextColor)) {
                TextAppearance textAppearance = new TextAppearance(context, state.badgeTextAppearanceResId.intValue());
                state.badgeTextColor = Integer.valueOf(textAppearance.getTextColor().getDefaultColor());
            } else {
                state.badgeTextColor = Integer.valueOf(readColorFromAttributes(context, a, R.styleable.Badge_badgeTextColor));
            }
        } else {
            state.badgeTextColor = storedState.badgeTextColor;
        }
        if (storedState.badgeGravity == null) {
            iIntValue7 = a.getInt(R.styleable.Badge_badgeGravity, 8388661);
        } else {
            iIntValue7 = storedState.badgeGravity.intValue();
        }
        state.badgeGravity = Integer.valueOf(iIntValue7);
        if (storedState.horizontalOffsetWithoutText == null) {
            iIntValue8 = a.getDimensionPixelOffset(R.styleable.Badge_horizontalOffset, 0);
        } else {
            iIntValue8 = storedState.horizontalOffsetWithoutText.intValue();
        }
        state.horizontalOffsetWithoutText = Integer.valueOf(iIntValue8);
        if (storedState.verticalOffsetWithoutText == null) {
            iIntValue9 = a.getDimensionPixelOffset(R.styleable.Badge_verticalOffset, 0);
        } else {
            iIntValue9 = storedState.verticalOffsetWithoutText.intValue();
        }
        state.verticalOffsetWithoutText = Integer.valueOf(iIntValue9);
        if (storedState.horizontalOffsetWithText == null) {
            iIntValue10 = a.getDimensionPixelOffset(R.styleable.Badge_horizontalOffsetWithText, state.horizontalOffsetWithoutText.intValue());
        } else {
            iIntValue10 = storedState.horizontalOffsetWithText.intValue();
        }
        state.horizontalOffsetWithText = Integer.valueOf(iIntValue10);
        if (storedState.verticalOffsetWithText == null) {
            iIntValue11 = a.getDimensionPixelOffset(R.styleable.Badge_verticalOffsetWithText, state.verticalOffsetWithoutText.intValue());
        } else {
            iIntValue11 = storedState.verticalOffsetWithText.intValue();
        }
        state.verticalOffsetWithText = Integer.valueOf(iIntValue11);
        state.additionalHorizontalOffset = Integer.valueOf(storedState.additionalHorizontalOffset == null ? 0 : storedState.additionalHorizontalOffset.intValue());
        state.additionalVerticalOffset = Integer.valueOf(storedState.additionalVerticalOffset != null ? storedState.additionalVerticalOffset.intValue() : 0);
        a.recycle();
        if (storedState.numberLocale == null) {
            state.numberLocale = Locale.getDefault(Locale.Category.FORMAT);
        } else {
            state.numberLocale = storedState.numberLocale;
        }
        this.overridingState = storedState;
    }

    private TypedArray generateTypedArray(Context context, int badgeResId, int defStyleAttr, int defStyleRes) {
        AttributeSet attrs = null;
        int style = 0;
        if (badgeResId != 0) {
            attrs = DrawableUtils.parseDrawableXml(context, badgeResId, BADGE_RESOURCE_TAG);
            style = attrs.getStyleAttribute();
        }
        if (style == 0) {
            style = defStyleRes;
        }
        return ThemeEnforcement.obtainStyledAttributes(context, attrs, R.styleable.Badge, defStyleAttr, style, new int[0]);
    }

    State getOverridingState() {
        return this.overridingState;
    }

    boolean isVisible() {
        return this.currentState.isVisible.booleanValue();
    }

    void setVisible(boolean visible) {
        this.overridingState.isVisible = Boolean.valueOf(visible);
        this.currentState.isVisible = Boolean.valueOf(visible);
    }

    boolean hasNumber() {
        return this.currentState.number != -1;
    }

    int getNumber() {
        return this.currentState.number;
    }

    void setNumber(int number) {
        this.overridingState.number = number;
        this.currentState.number = number;
    }

    void clearNumber() {
        setNumber(-1);
    }

    int getAlpha() {
        return this.currentState.alpha;
    }

    void setAlpha(int alpha) {
        this.overridingState.alpha = alpha;
        this.currentState.alpha = alpha;
    }

    int getMaxCharacterCount() {
        return this.currentState.maxCharacterCount;
    }

    void setMaxCharacterCount(int maxCharacterCount) {
        this.overridingState.maxCharacterCount = maxCharacterCount;
        this.currentState.maxCharacterCount = maxCharacterCount;
    }

    int getBackgroundColor() {
        return this.currentState.backgroundColor.intValue();
    }

    void setBackgroundColor(int backgroundColor) {
        this.overridingState.backgroundColor = Integer.valueOf(backgroundColor);
        this.currentState.backgroundColor = Integer.valueOf(backgroundColor);
    }

    int getBadgeTextColor() {
        return this.currentState.badgeTextColor.intValue();
    }

    void setBadgeTextColor(int badgeTextColor) {
        this.overridingState.badgeTextColor = Integer.valueOf(badgeTextColor);
        this.currentState.badgeTextColor = Integer.valueOf(badgeTextColor);
    }

    int getTextAppearanceResId() {
        return this.currentState.badgeTextAppearanceResId.intValue();
    }

    void setTextAppearanceResId(int textAppearanceResId) {
        this.overridingState.badgeTextAppearanceResId = Integer.valueOf(textAppearanceResId);
        this.currentState.badgeTextAppearanceResId = Integer.valueOf(textAppearanceResId);
    }

    int getBadgeShapeAppearanceResId() {
        return this.currentState.badgeShapeAppearanceResId.intValue();
    }

    void setBadgeShapeAppearanceResId(int shapeAppearanceResId) {
        this.overridingState.badgeShapeAppearanceResId = Integer.valueOf(shapeAppearanceResId);
        this.currentState.badgeShapeAppearanceResId = Integer.valueOf(shapeAppearanceResId);
    }

    int getBadgeShapeAppearanceOverlayResId() {
        return this.currentState.badgeShapeAppearanceOverlayResId.intValue();
    }

    void setBadgeShapeAppearanceOverlayResId(int shapeAppearanceOverlayResId) {
        this.overridingState.badgeShapeAppearanceOverlayResId = Integer.valueOf(shapeAppearanceOverlayResId);
        this.currentState.badgeShapeAppearanceOverlayResId = Integer.valueOf(shapeAppearanceOverlayResId);
    }

    int getBadgeWithTextShapeAppearanceResId() {
        return this.currentState.badgeWithTextShapeAppearanceResId.intValue();
    }

    void setBadgeWithTextShapeAppearanceResId(int shapeAppearanceResId) {
        this.overridingState.badgeWithTextShapeAppearanceResId = Integer.valueOf(shapeAppearanceResId);
        this.currentState.badgeWithTextShapeAppearanceResId = Integer.valueOf(shapeAppearanceResId);
    }

    int getBadgeWithTextShapeAppearanceOverlayResId() {
        return this.currentState.badgeWithTextShapeAppearanceOverlayResId.intValue();
    }

    void setBadgeWithTextShapeAppearanceOverlayResId(int shapeAppearanceOverlayResId) {
        this.overridingState.badgeWithTextShapeAppearanceOverlayResId = Integer.valueOf(shapeAppearanceOverlayResId);
        this.currentState.badgeWithTextShapeAppearanceOverlayResId = Integer.valueOf(shapeAppearanceOverlayResId);
    }

    int getBadgeGravity() {
        return this.currentState.badgeGravity.intValue();
    }

    void setBadgeGravity(int badgeGravity) {
        this.overridingState.badgeGravity = Integer.valueOf(badgeGravity);
        this.currentState.badgeGravity = Integer.valueOf(badgeGravity);
    }

    int getHorizontalOffsetWithoutText() {
        return this.currentState.horizontalOffsetWithoutText.intValue();
    }

    void setHorizontalOffsetWithoutText(int offset) {
        this.overridingState.horizontalOffsetWithoutText = Integer.valueOf(offset);
        this.currentState.horizontalOffsetWithoutText = Integer.valueOf(offset);
    }

    int getVerticalOffsetWithoutText() {
        return this.currentState.verticalOffsetWithoutText.intValue();
    }

    void setVerticalOffsetWithoutText(int offset) {
        this.overridingState.verticalOffsetWithoutText = Integer.valueOf(offset);
        this.currentState.verticalOffsetWithoutText = Integer.valueOf(offset);
    }

    int getHorizontalOffsetWithText() {
        return this.currentState.horizontalOffsetWithText.intValue();
    }

    void setHorizontalOffsetWithText(int offset) {
        this.overridingState.horizontalOffsetWithText = Integer.valueOf(offset);
        this.currentState.horizontalOffsetWithText = Integer.valueOf(offset);
    }

    int getVerticalOffsetWithText() {
        return this.currentState.verticalOffsetWithText.intValue();
    }

    void setVerticalOffsetWithText(int offset) {
        this.overridingState.verticalOffsetWithText = Integer.valueOf(offset);
        this.currentState.verticalOffsetWithText = Integer.valueOf(offset);
    }

    int getAdditionalHorizontalOffset() {
        return this.currentState.additionalHorizontalOffset.intValue();
    }

    void setAdditionalHorizontalOffset(int offset) {
        this.overridingState.additionalHorizontalOffset = Integer.valueOf(offset);
        this.currentState.additionalHorizontalOffset = Integer.valueOf(offset);
    }

    int getAdditionalVerticalOffset() {
        return this.currentState.additionalVerticalOffset.intValue();
    }

    void setAdditionalVerticalOffset(int offset) {
        this.overridingState.additionalVerticalOffset = Integer.valueOf(offset);
        this.currentState.additionalVerticalOffset = Integer.valueOf(offset);
    }

    CharSequence getContentDescriptionNumberless() {
        return this.currentState.contentDescriptionNumberless;
    }

    void setContentDescriptionNumberless(CharSequence contentDescriptionNumberless) {
        this.overridingState.contentDescriptionNumberless = contentDescriptionNumberless;
        this.currentState.contentDescriptionNumberless = contentDescriptionNumberless;
    }

    int getContentDescriptionQuantityStrings() {
        return this.currentState.contentDescriptionQuantityStrings;
    }

    void setContentDescriptionQuantityStringsResource(int stringsResource) {
        this.overridingState.contentDescriptionQuantityStrings = stringsResource;
        this.currentState.contentDescriptionQuantityStrings = stringsResource;
    }

    int getContentDescriptionExceedsMaxBadgeNumberStringResource() {
        return this.currentState.contentDescriptionExceedsMaxBadgeNumberRes;
    }

    void setContentDescriptionExceedsMaxBadgeNumberStringResource(int stringsResource) {
        this.overridingState.contentDescriptionExceedsMaxBadgeNumberRes = stringsResource;
        this.currentState.contentDescriptionExceedsMaxBadgeNumberRes = stringsResource;
    }

    Locale getNumberLocale() {
        return this.currentState.numberLocale;
    }

    void setNumberLocale(Locale locale) {
        this.overridingState.numberLocale = locale;
        this.currentState.numberLocale = locale;
    }

    private static int readColorFromAttributes(Context context, TypedArray a, int index) {
        return MaterialResources.getColorStateList(context, a, index).getDefaultColor();
    }

    public static final class State implements Parcelable {
        private static final int BADGE_NUMBER_NONE = -1;
        public static final Parcelable.Creator<State> CREATOR = new Parcelable.Creator<State>() { // from class: com.google.android.material.badge.BadgeState.State.1
            /* JADX WARN: Can't rename method to resolve collision */
            @Override // android.os.Parcelable.Creator
            public State createFromParcel(Parcel in) {
                return new State(in);
            }

            /* JADX WARN: Can't rename method to resolve collision */
            @Override // android.os.Parcelable.Creator
            public State[] newArray(int size) {
                return new State[size];
            }
        };
        private static final int NOT_SET = -2;
        private Integer additionalHorizontalOffset;
        private Integer additionalVerticalOffset;
        private int alpha;
        private Integer backgroundColor;
        private Integer badgeGravity;
        private int badgeResId;
        private Integer badgeShapeAppearanceOverlayResId;
        private Integer badgeShapeAppearanceResId;
        private Integer badgeTextAppearanceResId;
        private Integer badgeTextColor;
        private Integer badgeWithTextShapeAppearanceOverlayResId;
        private Integer badgeWithTextShapeAppearanceResId;
        private int contentDescriptionExceedsMaxBadgeNumberRes;
        private CharSequence contentDescriptionNumberless;
        private int contentDescriptionQuantityStrings;
        private Integer horizontalOffsetWithText;
        private Integer horizontalOffsetWithoutText;
        private Boolean isVisible;
        private int maxCharacterCount;
        private int number;
        private Locale numberLocale;
        private Integer verticalOffsetWithText;
        private Integer verticalOffsetWithoutText;

        public State() {
            this.alpha = 255;
            this.number = -2;
            this.maxCharacterCount = -2;
            this.isVisible = true;
        }

        State(Parcel in) {
            this.alpha = 255;
            this.number = -2;
            this.maxCharacterCount = -2;
            this.isVisible = true;
            this.badgeResId = in.readInt();
            this.backgroundColor = (Integer) in.readSerializable();
            this.badgeTextColor = (Integer) in.readSerializable();
            this.badgeTextAppearanceResId = (Integer) in.readSerializable();
            this.badgeShapeAppearanceResId = (Integer) in.readSerializable();
            this.badgeShapeAppearanceOverlayResId = (Integer) in.readSerializable();
            this.badgeWithTextShapeAppearanceResId = (Integer) in.readSerializable();
            this.badgeWithTextShapeAppearanceOverlayResId = (Integer) in.readSerializable();
            this.alpha = in.readInt();
            this.number = in.readInt();
            this.maxCharacterCount = in.readInt();
            this.contentDescriptionNumberless = in.readString();
            this.contentDescriptionQuantityStrings = in.readInt();
            this.badgeGravity = (Integer) in.readSerializable();
            this.horizontalOffsetWithoutText = (Integer) in.readSerializable();
            this.verticalOffsetWithoutText = (Integer) in.readSerializable();
            this.horizontalOffsetWithText = (Integer) in.readSerializable();
            this.verticalOffsetWithText = (Integer) in.readSerializable();
            this.additionalHorizontalOffset = (Integer) in.readSerializable();
            this.additionalVerticalOffset = (Integer) in.readSerializable();
            this.isVisible = (Boolean) in.readSerializable();
            this.numberLocale = (Locale) in.readSerializable();
        }

        @Override // android.os.Parcelable
        public int describeContents() {
            return 0;
        }

        @Override // android.os.Parcelable
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeInt(this.badgeResId);
            dest.writeSerializable(this.backgroundColor);
            dest.writeSerializable(this.badgeTextColor);
            dest.writeSerializable(this.badgeTextAppearanceResId);
            dest.writeSerializable(this.badgeShapeAppearanceResId);
            dest.writeSerializable(this.badgeShapeAppearanceOverlayResId);
            dest.writeSerializable(this.badgeWithTextShapeAppearanceResId);
            dest.writeSerializable(this.badgeWithTextShapeAppearanceOverlayResId);
            dest.writeInt(this.alpha);
            dest.writeInt(this.number);
            dest.writeInt(this.maxCharacterCount);
            CharSequence charSequence = this.contentDescriptionNumberless;
            dest.writeString(charSequence == null ? null : charSequence.toString());
            dest.writeInt(this.contentDescriptionQuantityStrings);
            dest.writeSerializable(this.badgeGravity);
            dest.writeSerializable(this.horizontalOffsetWithoutText);
            dest.writeSerializable(this.verticalOffsetWithoutText);
            dest.writeSerializable(this.horizontalOffsetWithText);
            dest.writeSerializable(this.verticalOffsetWithText);
            dest.writeSerializable(this.additionalHorizontalOffset);
            dest.writeSerializable(this.additionalVerticalOffset);
            dest.writeSerializable(this.isVisible);
            dest.writeSerializable(this.numberLocale);
        }
    }
}
