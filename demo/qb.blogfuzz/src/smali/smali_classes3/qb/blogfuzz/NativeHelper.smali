.class public Lqb/blogfuzz/NativeHelper;
.super Ljava/lang/Object;
.source "NativeHelper.java"


# direct methods
.method static constructor <clinit>()V
    .locals 1

    .line 9
    const-string v0, "blogfuzz"

    invoke-static {v0}, Ljava/lang/System;->loadLibrary(Ljava/lang/String;)V

    .line 10
    return-void
.end method

.method public constructor <init>()V
    .locals 0

    .line 3
    invoke-direct {p0}, Ljava/lang/Object;-><init>()V

    return-void
.end method


# virtual methods
.method public native fuzzMeArray([B)V
.end method

.method public native fuzzMeWrapper(Lqb/blogfuzz/Wrapper;)V
.end method
