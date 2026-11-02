package androidx.appcompat.app;

import android.content.ComponentName;
import android.content.Context;
import android.os.Build;
import android.util.Log;
import android.util.Xml;
import androidx.appcompat.app.AppCompatDelegate;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayDeque;
import java.util.Queue;
import java.util.concurrent.Executor;
import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlSerializer;

/* JADX INFO: loaded from: classes.dex */
class AppLocalesStorageHelper {
    static final String APPLICATION_LOCALES_RECORD_FILE = "androidx.appcompat.app.AppCompatDelegate.application_locales_record_file";
    static final String APP_LOCALES_META_DATA_HOLDER_SERVICE_NAME = "androidx.appcompat.app.AppLocalesMetadataHolderService";
    static final String LOCALE_RECORD_ATTRIBUTE_TAG = "application_locales";
    static final String LOCALE_RECORD_FILE_TAG = "locales";
    static final String TAG = "AppLocalesStorageHelper";

    private AppLocalesStorageHelper() {
    }

    /* JADX WARN: Code restructure failed: missing block: B:18:0x0042, code lost:
    
        r2 = r4.getAttributeValue(null, androidx.appcompat.app.AppLocalesStorageHelper.LOCALE_RECORD_ATTRIBUTE_TAG);
     */
    /* JADX WARN: Not initialized variable reg: 3, insn: 0x007f: IF  (r3 I:??[int, boolean, OBJECT, ARRAY, byte, short, char] A[D('fis' java.io.FileInputStream)]) == (0 ??[int, boolean, OBJECT, ARRAY, byte, short, char])  -> B:43:0x0086 (LINE:96), block:B:39:0x007f */
    /* JADX WARN: Removed duplicated region for block: B:36:0x0064  */
    /* JADX WARN: Removed duplicated region for block: B:37:0x007b  */
    /*
        Code decompiled incorrectly, please refer to instructions dump.
    */
    static String readLocales(Context context) {
        FileInputStream fis;
        FileInputStream fis2;
        XmlPullParser parser;
        int outerDepth;
        String appLocales = "";
        try {
            try {
                try {
                    fis2 = context.openFileInput(APPLICATION_LOCALES_RECORD_FILE);
                    try {
                        parser = Xml.newPullParser();
                        parser.setInput(fis2, "UTF-8");
                        outerDepth = parser.getDepth();
                    } catch (IOException | XmlPullParserException e) {
                        Log.w(TAG, "Reading app Locales : Unable to parse through file :androidx.appcompat.app.AppCompatDelegate.application_locales_record_file");
                        if (fis2 != null) {
                            fis2.close();
                        }
                    }
                } catch (FileNotFoundException e2) {
                    Log.w(TAG, "Reading app Locales : Locales record file not found: androidx.appcompat.app.AppCompatDelegate.application_locales_record_file");
                    return "";
                }
            } catch (Throwable th) {
                if (fis != null) {
                    try {
                        fis.close();
                    } catch (IOException e3) {
                    }
                }
                throw th;
            }
        } catch (IOException e4) {
        }
        while (true) {
            int type = parser.next();
            if (type == 1 || (type == 3 && parser.getDepth() <= outerDepth)) {
                break;
            }
            if (type != 3 && type != 4) {
                String tagName = parser.getName();
                if (tagName.equals(LOCALE_RECORD_FILE_TAG)) {
                    break;
                }
                if (appLocales.isEmpty()) {
                    Log.d(TAG, "Reading app Locales : Locales read from file: androidx.appcompat.app.AppCompatDelegate.application_locales_record_file , appLocales: " + appLocales);
                } else {
                    context.deleteFile(APPLICATION_LOCALES_RECORD_FILE);
                }
                return appLocales;
            }
        }
        if (fis2 != null) {
            fis2.close();
        }
        if (appLocales.isEmpty()) {
        }
        return appLocales;
    }

    /* JADX WARN: Unsupported multi-entry loop pattern (BACK_EDGE: B:20:0x007a -> B:31:0x007c). Please report as a decompilation issue!!! */
    static void persistLocales(Context context, String locales) {
        if (locales.equals("")) {
            context.deleteFile(APPLICATION_LOCALES_RECORD_FILE);
            return;
        }
        try {
            FileOutputStream fos = context.openFileOutput(APPLICATION_LOCALES_RECORD_FILE, 0);
            XmlSerializer serializer = Xml.newSerializer();
            try {
                try {
                    try {
                        serializer.setOutput(fos, null);
                        serializer.startDocument("UTF-8", true);
                        serializer.startTag(null, LOCALE_RECORD_FILE_TAG);
                        serializer.attribute(null, LOCALE_RECORD_ATTRIBUTE_TAG, locales);
                        serializer.endTag(null, LOCALE_RECORD_FILE_TAG);
                        serializer.endDocument();
                        Log.d(TAG, "Storing App Locales : app-locales: " + locales + " persisted successfully.");
                        if (fos != null) {
                            fos.close();
                        }
                    } catch (Exception e) {
                        Log.w(TAG, "Storing App Locales : Failed to persist app-locales: " + locales, e);
                        if (fos == null) {
                        } else {
                            fos.close();
                        }
                    }
                } catch (IOException e2) {
                }
            } catch (Throwable th) {
                if (fos != null) {
                    try {
                        fos.close();
                    } catch (IOException e3) {
                    }
                }
                throw th;
            }
        } catch (FileNotFoundException e4) {
            Log.w(TAG, String.format("Storing App Locales : FileNotFoundException: Cannot open file %s for writing ", APPLICATION_LOCALES_RECORD_FILE));
        }
    }

    static void syncLocalesToFramework(Context context) {
        if (Build.VERSION.SDK_INT >= 33) {
            ComponentName app_locales_component = new ComponentName(context, APP_LOCALES_META_DATA_HOLDER_SERVICE_NAME);
            if (context.getPackageManager().getComponentEnabledSetting(app_locales_component) != 1) {
                if (AppCompatDelegate.getApplicationLocales().isEmpty()) {
                    String appLocales = readLocales(context);
                    Object localeManager = context.getSystemService("locale");
                    if (localeManager != null) {
                        AppCompatDelegate.Api33Impl.localeManagerSetApplicationLocales(localeManager, AppCompatDelegate.Api24Impl.localeListForLanguageTags(appLocales));
                    }
                }
                context.getPackageManager().setComponentEnabledSetting(app_locales_component, 1, 1);
            }
        }
    }

    static class ThreadPerTaskExecutor implements Executor {
        ThreadPerTaskExecutor() {
        }

        @Override // java.util.concurrent.Executor
        public void execute(Runnable r) {
            new Thread(r).start();
        }
    }

    static class SerialExecutor implements Executor {
        Runnable mActive;
        final Executor mExecutor;
        private final Object mLock = new Object();
        final Queue<Runnable> mTasks = new ArrayDeque();

        SerialExecutor(Executor executor) {
            this.mExecutor = executor;
        }

        @Override // java.util.concurrent.Executor
        public void execute(final Runnable r) {
            synchronized (this.mLock) {
                this.mTasks.add(new Runnable() { // from class: androidx.appcompat.app.AppLocalesStorageHelper$SerialExecutor$$ExternalSyntheticLambda0
                    @Override // java.lang.Runnable
                    public final void run() {
                        this.f$0.m4xd188c474(r);
                    }
                });
                if (this.mActive == null) {
                    scheduleNext();
                }
            }
        }

        /* JADX INFO: renamed from: lambda$execute$0$androidx-appcompat-app-AppLocalesStorageHelper$SerialExecutor, reason: not valid java name */
        /* synthetic */ void m4xd188c474(Runnable r) {
            try {
                r.run();
            } finally {
                scheduleNext();
            }
        }

        protected void scheduleNext() {
            synchronized (this.mLock) {
                Runnable runnablePoll = this.mTasks.poll();
                this.mActive = runnablePoll;
                if (runnablePoll != null) {
                    this.mExecutor.execute(runnablePoll);
                }
            }
        }
    }
}
