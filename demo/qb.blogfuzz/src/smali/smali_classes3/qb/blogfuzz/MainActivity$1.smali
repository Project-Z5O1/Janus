.class Lqb/blogfuzz/MainActivity$1;
.super Ljava/lang/Object;
.source "MainActivity.java"

# interfaces
.implements Ljava/lang/Runnable;


# annotations
.annotation system Ldalvik/annotation/EnclosingMethod;
    value = Lqb/blogfuzz/MainActivity;->onCreate(Landroid/os/Bundle;)V
.end annotation

.annotation system Ldalvik/annotation/InnerClass;
    accessFlags = 0x0
    name = null
.end annotation


# instance fields
.field final synthetic this$0:Lqb/blogfuzz/MainActivity;


# direct methods
.method constructor <init>(Lqb/blogfuzz/MainActivity;)V
    .locals 0
    .param p1, "this$0"    # Lqb/blogfuzz/MainActivity;
    .annotation system Ldalvik/annotation/MethodParameters;
        accessFlags = {
            0x8010
        }
        names = {
            null
        }
    .end annotation

    .line 15
    iput-object p1, p0, Lqb/blogfuzz/MainActivity$1;->this$0:Lqb/blogfuzz/MainActivity;

    invoke-direct {p0}, Ljava/lang/Object;-><init>()V

    return-void
.end method


# virtual methods
.method public run()V
    .locals 1

    .line 18
    iget-object v0, p0, Lqb/blogfuzz/MainActivity$1;->this$0:Lqb/blogfuzz/MainActivity;

    invoke-static {v0}, Lqb/blogfuzz/MainActivity;->access$000(Lqb/blogfuzz/MainActivity;)V

    .line 19
    return-void
.end method
