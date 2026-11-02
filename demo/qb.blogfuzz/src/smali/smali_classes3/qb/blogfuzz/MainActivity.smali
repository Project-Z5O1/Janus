.class public Lqb/blogfuzz/MainActivity;
.super Landroidx/appcompat/app/AppCompatActivity;
.source "MainActivity.java"


# static fields
.field private static final TAG:Ljava/lang/String; = "blogfuzz"


# direct methods
.method public constructor <init>()V
    .locals 0

    .line 8
    invoke-direct {p0}, Landroidx/appcompat/app/AppCompatActivity;-><init>()V

    return-void
.end method

.method static synthetic access$000(Lqb/blogfuzz/MainActivity;)V
    .locals 0
    .param p0, "x0"    # Lqb/blogfuzz/MainActivity;

    .line 8
    invoke-direct {p0}, Lqb/blogfuzz/MainActivity;->lambda$onCreate$0()V

    return-void
.end method

.method private lambda$onCreate$0()V
    .locals 5

    .line 24
    const-string v0, "AAAAAAAAAAAAAAAA"

    sget-object v1, Ljava/nio/charset/StandardCharsets;->US_ASCII:Ljava/nio/charset/Charset;

    invoke-virtual {v0, v1}, Ljava/lang/String;->getBytes(Ljava/nio/charset/Charset;)[B

    move-result-object v0

    .line 25
    .local v0, "buffer":[B
    new-instance v1, Ljava/lang/StringBuilder;

    invoke-direct {v1}, Ljava/lang/StringBuilder;-><init>()V

    const-string v2, "length:"

    invoke-virtual {v1, v2}, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;

    move-result-object v1

    array-length v2, v0

    invoke-virtual {v1, v2}, Ljava/lang/StringBuilder;->append(I)Ljava/lang/StringBuilder;

    move-result-object v1

    invoke-virtual {v1}, Ljava/lang/StringBuilder;->toString()Ljava/lang/String;

    move-result-object v1

    const-string v2, "blogfuzz"

    invoke-static {v2, v1}, Landroid/util/Log;->i(Ljava/lang/String;Ljava/lang/String;)I

    .line 26
    new-instance v1, Lqb/blogfuzz/NativeHelper;

    invoke-direct {v1}, Lqb/blogfuzz/NativeHelper;-><init>()V

    .line 28
    .local v1, "nativeHelper":Lqb/blogfuzz/NativeHelper;
    :try_start_0
    new-instance v3, Lqb/blogfuzz/Wrapper;

    invoke-direct {v3, v0}, Lqb/blogfuzz/Wrapper;-><init>([B)V

    .line 29
    .local v3, "wrapper":Lqb/blogfuzz/Wrapper;
    invoke-virtual {v1, v3}, Lqb/blogfuzz/NativeHelper;->fuzzMeWrapper(Lqb/blogfuzz/Wrapper;)V

    .line 30
    invoke-virtual {v3}, Lqb/blogfuzz/Wrapper;->getBuffer()[B

    move-result-object v4

    invoke-virtual {v1, v4}, Lqb/blogfuzz/NativeHelper;->fuzzMeArray([B)V
    :try_end_0
    .catch Ljava/lang/InterruptedException; {:try_start_0 .. :try_end_0} :catch_0

    .line 33
    .end local v3    # "wrapper":Lqb/blogfuzz/Wrapper;
    goto :goto_0

    .line 31
    :catch_0
    move-exception v3

    .line 32
    .local v3, "e":Ljava/lang/InterruptedException;
    const-string v4, "Thread has been interrupted!"

    invoke-static {v2, v4}, Landroid/util/Log;->e(Ljava/lang/String;Ljava/lang/String;)I

    .line 34
    .end local v3    # "e":Ljava/lang/InterruptedException;
    :goto_0
    return-void
.end method


# virtual methods
.method protected onCreate(Landroid/os/Bundle;)V
    .locals 2
    .param p1, "savedInstanceState"    # Landroid/os/Bundle;

    .line 13
    invoke-super {p0, p1}, Landroidx/appcompat/app/AppCompatActivity;->onCreate(Landroid/os/Bundle;)V

    .line 14
    sget v0, Lqb/blogfuzz/R$layout;->activity_main:I

    invoke-virtual {p0, v0}, Lqb/blogfuzz/MainActivity;->setContentView(I)V

    .line 15
    new-instance v0, Ljava/lang/Thread;

    new-instance v1, Lqb/blogfuzz/MainActivity$1;

    invoke-direct {v1, p0}, Lqb/blogfuzz/MainActivity$1;-><init>(Lqb/blogfuzz/MainActivity;)V

    invoke-direct {v0, v1}, Ljava/lang/Thread;-><init>(Ljava/lang/Runnable;)V

    .line 20
    invoke-virtual {v0}, Ljava/lang/Thread;->start()V

    .line 21
    return-void
.end method
