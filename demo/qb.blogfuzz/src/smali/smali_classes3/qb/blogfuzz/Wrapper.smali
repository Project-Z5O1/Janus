.class public Lqb/blogfuzz/Wrapper;
.super Ljava/lang/Object;
.source "Wrapper.java"


# static fields
.field public static final TIME_TO_WAIT:J = 0x7d0L


# instance fields
.field private final mBuffer:[B


# direct methods
.method public constructor <init>([B)V
    .locals 3
    .param p1, "buffer"    # [B
    .annotation system Ldalvik/annotation/Throws;
        value = {
            Ljava/lang/InterruptedException;
        }
    .end annotation

    .line 9
    invoke-direct {p0}, Ljava/lang/Object;-><init>()V

    .line 11
    array-length v0, p1

    new-array v0, v0, [B

    .line 12
    .local v0, "bArr":[B
    iput-object v0, p0, Lqb/blogfuzz/Wrapper;->mBuffer:[B

    .line 13
    const/4 v1, 0x0

    array-length v2, v0

    invoke-static {p1, v1, v0, v1, v2}, Ljava/lang/System;->arraycopy(Ljava/lang/Object;ILjava/lang/Object;II)V

    .line 14
    return-void
.end method


# virtual methods
.method public getBuffer()[B
    .locals 2

    .line 17
    iget-object v0, p0, Lqb/blogfuzz/Wrapper;->mBuffer:[B

    .line 18
    .local v0, "bArr":[B
    array-length v1, v0

    invoke-static {v0, v1}, Ljava/util/Arrays;->copyOf([BI)[B

    move-result-object v1

    return-object v1
.end method
