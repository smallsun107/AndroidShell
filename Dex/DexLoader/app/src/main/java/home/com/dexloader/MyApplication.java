package home.com.dexloader;

import android.app.Application;
import android.app.Instrumentation;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.content.res.Resources;
import android.os.Bundle;
import android.util.ArrayMap;
import android.util.Log;
import dalvik.system.DexClassLoader;

import java.io.*;
import java.lang.ref.WeakReference;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

public class MyApplication extends Application {
    Context mContext;
    private String apkPath;
    private String odexPath;
    private String libPath;

    @Override
    protected void attachBaseContext(Context base) {
        super.attachBaseContext(base);

        File odex = this.getDir("test_odex", MODE_PRIVATE);
        File libs = this.getDir("test_lib", MODE_PRIVATE);
        odexPath = odex.getAbsolutePath();
        libPath = libs.getAbsolutePath();
        apkPath = odexPath + "/test.apk";

        Log.i("smallsun", "odexPath" + odexPath + "\r\n");
        Log.i("smallsun", "libPath" + libPath + "\r\n");
        Log.i("smallsun", "apkPath" + apkPath + "\r\n");

        File apkFile = new File(apkPath);

        if (apkFile.exists()) {
            Log.i("smallsun", "file is exists");
            return;
        }
        try {

            //创建APK
            apkFile.createNewFile();

            byte[] dexData = readDexFromAPK();
            Log.i("smallsun", "decryptDexFile");
            decryptDexFile(dexData);

            //配置动态加载环境
            Object currentActivityThread = RefInvoke.invokeStaticMethod("android.app.ActivityThread","currentActivityThread",new Class[]{},new Object[]{});
            String packageName = this.getPackageName();
            ArrayMap mPackage = (ArrayMap) RefInvoke.getFieldOjbect("android.app.ActivityThread",currentActivityThread,"mPackages");
            WeakReference wr = (WeakReference) mPackage.get(packageName);
            //创建被加壳APK的DexClassLoader对象  加载apk内的类和本地代码（c/c++代码）
            DexClassLoader dexClassLoader = new DexClassLoader(apkPath,odexPath,libPath, (ClassLoader) RefInvoke.getFieldOjbect("android.app.LoadedApk", wr.get(), "mClassLoader"));
         
            //把当前进程的DexClassLoader 设置成了被加壳apk的DexClassLoader
            RefInvoke.setFieldOjbect("android.app.LoadedApk","mClassLoader",wr.get(),dexClassLoader);

            Log.i("smallsun","ClassLoader:" + dexClassLoader);

            try {
                Object object = dexClassLoader.loadClass("com.apktest.MainActivity");
                Log.i("smallsun", "object : " + object);
            }catch (Exception e) {
                Log.i("smallsun", "activity:" + Log.getStackTraceString(e));
            }

        } catch (IOException e) {
            throw new RuntimeException(e);
        }

    }

    @Override
    public void onCreate() {
        super.onCreate();

        //loadResources(apkFileName);

        Log.i("smallsun", "onCreate");

        String appClassName = null;
        try {
            ApplicationInfo ai = this.getPackageManager().getApplicationInfo(this.getPackageName(), PackageManager.GET_META_DATA);
            Bundle bundle = ai.metaData;
            if (bundle != null && bundle.containsKey("APPLICATION_CLASS_NAME")) {
                appClassName = bundle.getString("APPLICATION_CLASS_NAME");
            } else {
                return;
            }
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }
        Object currentActivityThread = RefInvoke.invokeStaticMethod("android.app.ActivityThread", "currentActivityThread", new Class[]{}, new Object[]{});
        Object mBoundApplication = RefInvoke.getFieldOjbect("android.app.ActivityThread", currentActivityThread, "mBoundApplication");
        Object loadedApkInfo = RefInvoke.getFieldOjbect("android.app.ActivityThread$AppBindData", mBoundApplication, "info");
        RefInvoke.setFieldOjbect("android.app.LoadedApk", "mApplication", loadedApkInfo, null);
        Object oldApplication = RefInvoke.getFieldOjbect("android.app.ActivityThread", currentActivityThread, "mInitialApplication");
        ArrayList<Application> mAllApplications = (ArrayList<Application>) RefInvoke.getFieldOjbect("android.app.ActivityThread", currentActivityThread, "mAllApplications");
        mAllApplications.remove(oldApplication);
        ApplicationInfo appinfo_In_LoadedApk = (ApplicationInfo) RefInvoke.getFieldOjbect("android.app.LoadedApk", loadedApkInfo, "mApplicationInfo");
        ApplicationInfo appinfo_In_AppBindData = (ApplicationInfo) RefInvoke.getFieldOjbect("android.app.ActivityThread$AppBindData", mBoundApplication, "appInfo");
        appinfo_In_LoadedApk.className = appClassName;
        appinfo_In_AppBindData.className = appClassName;
        Application app = (Application) RefInvoke.invokeMethod("android.app.LoadedApk", "makeApplication", loadedApkInfo, new Class[]{boolean.class, Instrumentation.class}, new Object[]{false, null});
        RefInvoke.setFieldOjbect("android.app.ActivityThread", "mInitialApplication", currentActivityThread, app);
        ArrayMap mProviderMap = (ArrayMap) RefInvoke.getFieldOjbect("android.app.ActivityThread", currentActivityThread, "mProviderMap");
        Iterator it = mProviderMap.values().iterator();
        while (it.hasNext()) {
            Object providerClientRecord = it.next();
            Object localProvider = RefInvoke.getFieldOjbect("android.app.ActivityThread$ProviderClientRecord", providerClientRecord, "mLocalProvider");
            RefInvoke.setFieldOjbect("android.content.ContentProvider", "mContext", localProvider, app);
        }
        app.onCreate();
    }



    public byte[] readDexFromAPK() throws IOException {

        ByteArrayOutputStream dexOutputStream = new ByteArrayOutputStream();
        //APK 路径只能这么获取
        ZipInputStream zipInputStream = new ZipInputStream(new BufferedInputStream(new FileInputStream(this.getApplicationInfo().sourceDir)));

        while (true){
            ZipEntry zipEntry = zipInputStream.getNextEntry();
            if (zipEntry == null){
                zipInputStream.close();
                break;
            }

            if (zipEntry.getName().equals("classes.dex")){
                byte[] arrayOfByte = new byte[1024];

                while (true) {
                    int i = zipInputStream.read(arrayOfByte);
                    if (i == -1)
                        break;
                    dexOutputStream.write(arrayOfByte, 0, i);
                }
            }
            zipInputStream.closeEntry();
        }
        zipInputStream.close();
       return dexOutputStream.toByteArray();
    }

    private byte[] decrypt(byte[] srcdata) {
        for (int i = 0; i < srcdata.length; i++) {
            srcdata[i] = (byte) (0xFF ^ srcdata[i]);
        }
        return srcdata;
    }

    public void decryptDexFile(byte[] apkData) throws IOException {
        byte xor_key = (byte) 0xff;

        int nTotalLen = apkData.length;

        byte[] encryptLen = new byte[4];
        System.arraycopy(apkData, nTotalLen - 4, encryptLen, 0, 4);



        ByteArrayInputStream byteArrayInputStream = new ByteArrayInputStream(encryptLen);
        DataInputStream dataInputStream = new DataInputStream(byteArrayInputStream);
        int nReadInt = dataInputStream.readInt();
        System.out.println("Apk size is " + Integer.toHexString(nReadInt));
        byte[] newApk = new byte[nReadInt];
        System.arraycopy(apkData, nTotalLen - 4 - nReadInt, newApk, 0, nReadInt);

        //解密数据
        newApk = decrypt(newApk);
        Log.i("smallsun","new File end");
        File file = new File(apkPath);
        try {
            FileOutputStream fileOutputStream = new FileOutputStream(file);
            fileOutputStream.write(newApk);
            fileOutputStream.close();
        } catch (IOException e) {
            throw new RuntimeException(e);
        }

        //处理被加壳的APK
        ZipInputStream zipInputStream = new ZipInputStream(new BufferedInputStream(new FileInputStream(file)));
        while (true) {
            ZipEntry localZipEntry = zipInputStream.getNextEntry();//不了解这个是否也遍历子目录，看样子应该是遍历的
            if (localZipEntry == null) {
                zipInputStream.close();
                break;
            }
            //取出被加壳apk用到的so文件，放到 libPath中（data/data/包名/payload_lib)
            String name = localZipEntry.getName();
            if (name.startsWith("lib/") && name.endsWith(".so")) {
                File storeFile = new File(libPath + "/" + name.substring(name.lastIndexOf('/')));
                storeFile.createNewFile();
                FileOutputStream fos = new FileOutputStream(storeFile);
                byte[] arrayOfByte = new byte[1024];
                while (true) {
                    int i = zipInputStream.read(arrayOfByte);
                    if (i == -1)
                        break;
                    fos.write(arrayOfByte, 0, i);
                }
                fos.flush();
                fos.close();
            }
            zipInputStream.closeEntry();
        }
        zipInputStream.close();

    }



    //以下是加载资源
    protected AssetManager mAssetManager;//资源管理器
    protected Resources mResources;//资源
    protected Resources.Theme mTheme;//主题

    protected void loadResources(String dexPath) {
        try {
            AssetManager assetManager = AssetManager.class.newInstance();
            Method addAssetPath = assetManager.getClass().getMethod("addAssetPath", String.class);
            addAssetPath.invoke(assetManager, dexPath);
            mAssetManager = assetManager;
        } catch (Exception e) {
            Log.i("inject", "loadResource error:"+Log.getStackTraceString(e));
            e.printStackTrace();
        }
        Resources superRes = super.getResources();
        superRes.getDisplayMetrics();
        superRes.getConfiguration();
        mResources = new Resources(mAssetManager, superRes.getDisplayMetrics(),superRes.getConfiguration());
        mTheme = mResources.newTheme();
        mTheme.setTo(super.getTheme());
    }

    @Override
    public AssetManager getAssets() {
        return mAssetManager == null ? super.getAssets() : mAssetManager;
    }

    @Override
    public Resources getResources() {
        return mResources == null ? super.getResources() : mResources;
    }

    @Override
    public Resources.Theme getTheme() {
        return mTheme == null ? super.getTheme() : mTheme;
    }

}



