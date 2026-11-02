package qb.blogfuzz;

import android.os.Bundle;
import android.util.Log;
import androidx.appcompat.app.AppCompatActivity;
import java.nio.charset.StandardCharsets;

/* JADX INFO: loaded from: classes3.dex */
public class MainActivity extends AppCompatActivity {
    private static final String TAG = "blogfuzz";

    @Override // androidx.fragment.app.FragmentActivity, androidx.activity.ComponentActivity, androidx.core.app.ComponentActivity, android.app.Activity
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        new Thread(new Runnable() { // from class: qb.blogfuzz.MainActivity.1
            @Override // java.lang.Runnable
            public void run() {
                MainActivity.this.lambda$onCreate$0();
            }
        }).start();
    }

    /* JADX INFO: Access modifiers changed from: private */
    public void lambda$onCreate$0() {
        byte[] buffer = "AAAAAAAAAAAAAAAA".getBytes(StandardCharsets.US_ASCII);
        Log.i(TAG, "length:" + buffer.length);
        NativeHelper nativeHelper = new NativeHelper();
        try {
            Wrapper wrapper = new Wrapper(buffer);
            nativeHelper.fuzzMeWrapper(wrapper);
            nativeHelper.fuzzMeArray(wrapper.getBuffer());
        } catch (InterruptedException e) {
            Log.e(TAG, "Thread has been interrupted!");
        }
    }
}
