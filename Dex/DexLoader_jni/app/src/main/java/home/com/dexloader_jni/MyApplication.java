package home.com.dexloader_jni;

import android.app.Application;
import android.content.Context;
import android.util.Log;

import java.io.BufferedInputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

public class MyApplication extends Application {
    private String apkPath;
    private String odexPath;
    private String libPath;

    @Override
    protected void attachBaseContext(Context base) {
        super.attachBaseContext(base);
        try {
            File odex = this.getDir("test_odex", MODE_PRIVATE);
            File libs = this.getDir("test_lib", MODE_PRIVATE);
            odexPath = odex.getAbsolutePath();
            libPath = libs.getAbsolutePath();

            apkPath = odex.getAbsolutePath() + "/test.apk";
            File dexFile = new File(apkPath);

            if (!dexFile.exists()) {
                dexFile.createNewFile();
                byte[] dexdata = readDexFromAPK();
                decryptDexFile(dexdata);
            }
            replaceDefaultClassLoader(apkPath);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onCreate() {
        super.onCreate();
        originalAppCreate();
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
        newApk = decrypt1(newApk);
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

    private native byte[] decrypt1(byte[] srcdata);

    private native void replaceDefaultClassLoader(String dexPath);

    private native void originalAppCreate();

}
