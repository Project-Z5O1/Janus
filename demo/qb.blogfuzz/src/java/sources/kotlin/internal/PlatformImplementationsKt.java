package kotlin.internal;

import androidx.constraintlayout.widget.ConstraintLayout;
import kotlin.KotlinVersion;
import kotlin.Metadata;
import kotlin.jvm.internal.Intrinsics;
import kotlin.text.StringsKt;

/* JADX INFO: compiled from: PlatformImplementations.kt */
/* JADX INFO: loaded from: classes.dex */
@Metadata(d1 = {"\u0000\u001e\n\u0000\n\u0002\u0018\u0002\n\u0000\n\u0002\u0010\u000b\n\u0000\n\u0002\u0010\b\n\u0002\b\u0004\n\u0002\u0010\u0000\n\u0002\b\u0004\u001a \u0010\u0002\u001a\u00020\u00032\u0006\u0010\u0004\u001a\u00020\u00052\u0006\u0010\u0006\u001a\u00020\u00052\u0006\u0010\u0007\u001a\u00020\u0005H\u0001\u001a\"\u0010\b\u001a\u0002H\t\"\n\b\u0000\u0010\t\u0018\u0001*\u00020\n2\u0006\u0010\u000b\u001a\u00020\nH\u0083\b¢\u0006\u0002\u0010\f\u001a\b\u0010\r\u001a\u00020\u0005H\u0002\"\u0010\u0010\u0000\u001a\u00020\u00018\u0000X\u0081\u0004¢\u0006\u0002\n\u0000¨\u0006\u000e"}, d2 = {"IMPLEMENTATIONS", "Lkotlin/internal/PlatformImplementations;", "apiVersionIsAtLeast", "", "major", "", "minor", "patch", "castToBaseType", "T", "", "instance", "(Ljava/lang/Object;)Ljava/lang/Object;", "getJavaVersion", "kotlin-stdlib"}, k = 2, mv = {1, 7, 1}, xi = ConstraintLayout.LayoutParams.Table.LAYOUT_CONSTRAINT_VERTICAL_CHAINSTYLE)
public final class PlatformImplementationsKt {
    public static final PlatformImplementations IMPLEMENTATIONS;

    /* JADX WARN: Removed duplicated region for block: B:38:0x00d3 A[Catch: ClassCastException -> 0x00d7, ClassNotFoundException -> 0x0115, TRY_ENTER, TryCatch #2 {ClassCastException -> 0x00d7, blocks: (B:38:0x00d3, B:41:0x00d9, B:42:0x00de), top: B:67:0x00d1, outer: #4 }] */
    /* JADX WARN: Removed duplicated region for block: B:41:0x00d9 A[Catch: ClassCastException -> 0x00d7, ClassNotFoundException -> 0x0115, TryCatch #2 {ClassCastException -> 0x00d7, blocks: (B:38:0x00d3, B:41:0x00d9, B:42:0x00de), top: B:67:0x00d1, outer: #4 }] */
    /* JADX WARN: Removed duplicated region for block: B:69:0x00c4 A[EXC_TOP_SPLITTER, SYNTHETIC] */
    static {
        PlatformImplementations platformImplementations;
        Object objNewInstance;
        int version = getJavaVersion();
        if (version >= 65544 || version < 65536) {
            try {
                Object objNewInstance2 = Class.forName("kotlin.internal.jdk8.JDK8PlatformImplementations").newInstance();
                Intrinsics.checkNotNullExpressionValue(objNewInstance2, "forName(\"kotlin.internal…entations\").newInstance()");
                try {
                    if (objNewInstance2 == null) {
                        throw new NullPointerException("null cannot be cast to non-null type kotlin.internal.PlatformImplementations");
                    }
                    platformImplementations = (PlatformImplementations) objNewInstance2;
                } catch (ClassCastException e) {
                    ClassLoader classLoader = objNewInstance2.getClass().getClassLoader();
                    ClassLoader classLoader2 = PlatformImplementations.class.getClassLoader();
                    if (!Intrinsics.areEqual(classLoader, classLoader2)) {
                        throw new ClassNotFoundException("Instance class was loaded from a different classloader: " + classLoader + ", base type classloader: " + classLoader2, e);
                    }
                    throw e;
                }
            } catch (ClassNotFoundException e2) {
                try {
                    Object objNewInstance3 = Class.forName("kotlin.internal.JRE8PlatformImplementations").newInstance();
                    Intrinsics.checkNotNullExpressionValue(objNewInstance3, "forName(\"kotlin.internal…entations\").newInstance()");
                    try {
                        if (objNewInstance3 == null) {
                            throw new NullPointerException("null cannot be cast to non-null type kotlin.internal.PlatformImplementations");
                        }
                        platformImplementations = (PlatformImplementations) objNewInstance3;
                    } catch (ClassCastException e3) {
                        ClassLoader classLoader3 = objNewInstance3.getClass().getClassLoader();
                        ClassLoader classLoader4 = PlatformImplementations.class.getClassLoader();
                        if (!Intrinsics.areEqual(classLoader3, classLoader4)) {
                            throw new ClassNotFoundException("Instance class was loaded from a different classloader: " + classLoader3 + ", base type classloader: " + classLoader4, e3);
                        }
                        throw e3;
                    }
                } catch (ClassNotFoundException e4) {
                    if (version < 65543) {
                        try {
                            objNewInstance = Class.forName("kotlin.internal.jdk7.JDK7PlatformImplementations").newInstance();
                            Intrinsics.checkNotNullExpressionValue(objNewInstance, "forName(\"kotlin.internal…entations\").newInstance()");
                            try {
                                if (objNewInstance != null) {
                                }
                            } catch (ClassCastException e5) {
                                ClassLoader classLoader5 = objNewInstance.getClass().getClassLoader();
                                ClassLoader classLoader6 = PlatformImplementations.class.getClassLoader();
                                if (!Intrinsics.areEqual(classLoader5, classLoader6)) {
                                    throw new ClassNotFoundException("Instance class was loaded from a different classloader: " + classLoader5 + ", base type classloader: " + classLoader6, e5);
                                }
                                throw e5;
                            }
                        } catch (ClassNotFoundException e6) {
                            try {
                                Object objNewInstance4 = Class.forName("kotlin.internal.JRE7PlatformImplementations").newInstance();
                                Intrinsics.checkNotNullExpressionValue(objNewInstance4, "forName(\"kotlin.internal…entations\").newInstance()");
                                try {
                                    if (objNewInstance4 == null) {
                                        throw new NullPointerException("null cannot be cast to non-null type kotlin.internal.PlatformImplementations");
                                    }
                                    platformImplementations = (PlatformImplementations) objNewInstance4;
                                } catch (ClassCastException e7) {
                                    ClassLoader classLoader7 = objNewInstance4.getClass().getClassLoader();
                                    ClassLoader classLoader8 = PlatformImplementations.class.getClassLoader();
                                    if (!Intrinsics.areEqual(classLoader7, classLoader8)) {
                                        throw new ClassNotFoundException("Instance class was loaded from a different classloader: " + classLoader7 + ", base type classloader: " + classLoader8, e7);
                                    }
                                    throw e7;
                                }
                            } catch (ClassNotFoundException e8) {
                                platformImplementations = new PlatformImplementations();
                            }
                        }
                    }
                }
            }
        } else if (version < 65543 || version < 65536) {
            objNewInstance = Class.forName("kotlin.internal.jdk7.JDK7PlatformImplementations").newInstance();
            Intrinsics.checkNotNullExpressionValue(objNewInstance, "forName(\"kotlin.internal…entations\").newInstance()");
            if (objNewInstance != null) {
                throw new NullPointerException("null cannot be cast to non-null type kotlin.internal.PlatformImplementations");
            }
            platformImplementations = (PlatformImplementations) objNewInstance;
        } else {
            platformImplementations = new PlatformImplementations();
        }
        IMPLEMENTATIONS = platformImplementations;
    }

    private static final /* synthetic */ <T> T castToBaseType(Object obj) throws ClassNotFoundException {
        try {
            Intrinsics.reifiedOperationMarker(1, "T");
            return (T) obj;
        } catch (ClassCastException e) {
            ClassLoader classLoader = obj.getClass().getClassLoader();
            Intrinsics.reifiedOperationMarker(4, "T");
            ClassLoader classLoader2 = Object.class.getClassLoader();
            if (!Intrinsics.areEqual(classLoader, classLoader2)) {
                throw new ClassNotFoundException("Instance class was loaded from a different classloader: " + classLoader + ", base type classloader: " + classLoader2, e);
            }
            throw e;
        }
    }

    private static final int getJavaVersion() {
        String version = System.getProperty("java.specification.version");
        if (version == null) {
            return 65542;
        }
        int firstDot = StringsKt.indexOf$default((CharSequence) version, '.', 0, false, 6, (Object) null);
        if (firstDot < 0) {
            try {
                return Integer.parseInt(version) * 65536;
            } catch (NumberFormatException e) {
                return 65542;
            }
        }
        int secondDot = StringsKt.indexOf$default((CharSequence) version, '.', firstDot + 1, false, 4, (Object) null);
        if (secondDot < 0) {
            secondDot = version.length();
        }
        String firstPart = version.substring(0, firstDot);
        Intrinsics.checkNotNullExpressionValue(firstPart, "this as java.lang.String…ing(startIndex, endIndex)");
        String secondPart = version.substring(firstDot + 1, secondDot);
        Intrinsics.checkNotNullExpressionValue(secondPart, "this as java.lang.String…ing(startIndex, endIndex)");
        try {
            return (Integer.parseInt(firstPart) * 65536) + Integer.parseInt(secondPart);
        } catch (NumberFormatException e2) {
            return 65542;
        }
    }

    public static final boolean apiVersionIsAtLeast(int major, int minor, int patch) {
        return KotlinVersion.CURRENT.isAtLeast(major, minor, patch);
    }
}
