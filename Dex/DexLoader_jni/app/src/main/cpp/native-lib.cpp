#include <jni.h>
#include <string>
#include <android/log.h>
#define LOG_TAG "smallsun"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

extern "C" JNIEXPORT jstring JNICALL
Java_home_com_dexloader_1jni_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}


extern "C"
JNIEXPORT void JNICALL
Java_home_com_dexloader_1jni_MyApplication_replaceDefaultClassLoader(JNIEnv *env, jobject obj,
                                                                     jstring dexPath_) {
    const char *dexPath = env->GetStringUTFChars(dexPath_, 0);

    // TODO
    jclass ActivityThreadClass = env->FindClass("android/app/ActivityThread");

    jmethodID currentActivityThreadMethodId = env->GetStaticMethodID(ActivityThreadClass, "currentActivityThread", "()Landroid/app/ActivityThread;");

    jobject activityThread = env->CallStaticObjectMethod(ActivityThreadClass, currentActivityThreadMethodId);

    jmethodID getPackageName = env->GetMethodID(env->GetObjectClass(obj), "getPackageName", "()Ljava/lang/String;");
    jstring packageName = (jstring) env->CallObjectMethod(obj, getPackageName);

    jfieldID mPackagesField;

    mPackagesField = env->GetFieldID(ActivityThreadClass, "mPackages", "Landroid/util/ArrayMap;");


    jobject mPackages = env->GetObjectField(activityThread, mPackagesField);

    jmethodID arrayMapGet = env->GetMethodID(env->GetObjectClass(mPackages), "get", "(Ljava/lang/Object;)Ljava/lang/Object;");
    jobject wr = env->CallObjectMethod(mPackages, arrayMapGet, packageName);


    jmethodID getDir = env->GetMethodID(env->GetObjectClass(obj), "getDir", "(Ljava/lang/String;I)Ljava/io/File;");
    jstring optimizedDirName = env->NewStringUTF("optimizedDirectory");
    jobject optimizedDirFolder = env->CallObjectMethod(obj, getDir, optimizedDirName, 0);
    jmethodID getAbsolutePath = env->GetMethodID(env->GetObjectClass(optimizedDirFolder), "getAbsolutePath", "()Ljava/lang/String;");
    jstring optimizedDirFolderPath = (jstring) env->CallObjectMethod(optimizedDirFolder,
                                                                     getAbsolutePath);

    jmethodID getApplicationInfo = env->GetMethodID(env->GetObjectClass(obj), "getApplicationInfo", "()Landroid/content/pm/ApplicationInfo;");
    jobject appInfo = env->CallObjectMethod(obj, getApplicationInfo);

    jfieldID nativeLibraryDirField = env->GetFieldID(env->GetObjectClass(appInfo), "nativeLibraryDir", "Ljava/lang/String;");
    jstring nativeLibraryDir = (jstring) env->GetObjectField(appInfo, nativeLibraryDirField);


    jmethodID weakReferenceGet = env->GetMethodID(env->GetObjectClass(wr), "get", "()Ljava/lang/Object;");
    jobject loadedApk = env->CallObjectMethod(wr, weakReferenceGet);
    jfieldID mClassLoaderField = env->GetFieldID(env->GetObjectClass(loadedApk), "mClassLoader", "Ljava/lang/ClassLoader;");
    jobject mClassLoader = env->GetObjectField(loadedApk, mClassLoaderField);

    jclass DexClassLoaderClass = env->FindClass("dalvik/system/DexClassLoader");
    jmethodID initDexClassLoader = env->GetMethodID(DexClassLoaderClass, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;)V");

    jobject dexClassLoader = env->NewObject(DexClassLoaderClass, initDexClassLoader, dexPath, optimizedDirFolderPath, nativeLibraryDir, mClassLoader);

    env->SetObjectField(loadedApk, mClassLoaderField, dexClassLoader);
    env->ReleaseStringUTFChars(dexPath_, dexPath);
}

extern "C"
JNIEXPORT void JNICALL
Java_home_com_dexloader_1jni_MyApplication_originalAppCreate(JNIEnv *env, jobject obj) {

    // TODO
    jclass ProxyApplicationClass = env->GetObjectClass(obj);
    jmethodID getPackageManagerMethodId = env->GetMethodID(ProxyApplicationClass, "getPackageManager","()Landroid/content/pm/PackageManager;");
    jobject packageManager = env->CallObjectMethod(obj, getPackageManagerMethodId);
    jmethodID pmGetApplicationInfoMethodId = env->GetMethodID(env->GetObjectClass(packageManager), "getApplicationInfo", "(Ljava/lang/String;I)Landroid/content/pm/ApplicationInfo;");
    jmethodID getPackageNameMethodId = env->GetMethodID(ProxyApplicationClass, "getPackageName", "()Ljava/lang/String;");
    jobject pmAppInfo = env->CallObjectMethod(packageManager, pmGetApplicationInfoMethodId, env->CallObjectMethod(obj, getPackageNameMethodId), 128);


    jclass PackageItemInfoClass = env->FindClass("android/content/pm/PackageItemInfo");
    jfieldID metaDataField = env->GetFieldID(PackageItemInfoClass, "metaData", "Landroid/os/Bundle;");
    jobject metaData = env->GetObjectField(pmAppInfo, metaDataField);

    if (metaData == NULL) {
        LOGE("未找到Bundle");
        return;
    }

    jmethodID bundleGetStringMethodId = env->GetMethodID(env->GetObjectClass(metaData), "getString", "(Ljava/lang/String;)Ljava/lang/String;");

    jstring originApplicationName = (jstring) env->CallObjectMethod(metaData, bundleGetStringMethodId, env->NewStringUTF("APPLICATION_CLASS_NAME"));
    if (originApplicationName == NULL) {
        LOGE("未找到原始Application Name");
        return;
    }
    LOGE("原始Application Name : %s", env->GetStringUTFChars(originApplicationName, JNI_FALSE));


    jclass ActivityThreadClass = env->FindClass("android/app/ActivityThread");
    jmethodID currentActivityThreadMethodId = env->GetStaticMethodID(ActivityThreadClass, "currentActivityThread", "()Landroid/app/ActivityThread;");
    jobject activityThread = env->CallStaticObjectMethod(ActivityThreadClass, currentActivityThreadMethodId);


    jfieldID mBoundApplicationField = env->GetFieldID(ActivityThreadClass, "mBoundApplication", "Landroid/app/ActivityThread$AppBindData;");
    jobject mBoundApplication = env->GetObjectField(activityThread, mBoundApplicationField);


    jfieldID infoField = env->GetFieldID(env->GetObjectClass(mBoundApplication), "info", "Landroid/app/LoadedApk;");
    jobject info = env->GetObjectField(mBoundApplication, infoField);

    jfieldID mApplicationField = env->GetFieldID(env->GetObjectClass(info), "mApplication", "Landroid/app/Application;");
    env->SetObjectField(info, mApplicationField, NULL);

    jfieldID mInitialApplicationField = env->GetFieldID(ActivityThreadClass, "mInitialApplication", "Landroid/app/Application;");
    jobject mInitialApplication = env->GetObjectField(activityThread, mInitialApplicationField);


    jfieldID mAllApplicationsField = env->GetFieldID(ActivityThreadClass, "mAllApplications", "Ljava/util/ArrayList;");
    jobject mAllApplications = env->GetObjectField(activityThread, mAllApplicationsField);
    jmethodID removeMethodId = env->GetMethodID(env->GetObjectClass(mAllApplications), "remove", "(Ljava/lang/Object;)Z");
    env->CallBooleanMethod(mAllApplications, removeMethodId, mInitialApplication);

    jfieldID appInfoField = env->GetFieldID(env->GetObjectClass(mBoundApplication), "appInfo", "Landroid/content/pm/ApplicationInfo;");
    jobject appInfo = env->GetObjectField(mBoundApplication, appInfoField);

    jfieldID mApplicationInfoField = env->GetFieldID(env->GetObjectClass(info), "mApplicationInfo", "Landroid/content/pm/ApplicationInfo;");
    jobject mApplicationInfo = env->GetObjectField(info, mApplicationInfoField);


    jfieldID classNameField = env->GetFieldID(env->GetObjectClass(appInfo), "className", "Ljava/lang/String;");
    env->SetObjectField(appInfo, classNameField, originApplicationName);
    env->SetObjectField(mApplicationInfo, classNameField, originApplicationName);

    jmethodID makeApplication = env->GetMethodID(env->GetObjectClass(info), "makeApplication", "(ZLandroid/app/Instrumentation;)Landroid/app/Application;");
    jobject originalApp = env->CallObjectMethod(info, makeApplication, JNI_FALSE, NULL);
    env->SetObjectField(activityThread, mInitialApplicationField, originalApp);


    jfieldID mProviderMapField;

    mProviderMapField = env->GetFieldID(ActivityThreadClass, "mProviderMap", "Landroid/util/ArrayMap;");


    if (mProviderMapField == NULL) {
        LOGE("未找到mProviderMapField");
        return;
    }

    jobject mProviderMap = env->GetObjectField(activityThread, mProviderMapField);
    jmethodID valuesMethodId = env->GetMethodID(env->GetObjectClass(mProviderMap), "values", "()Ljava/util/Collection;");
    jobject collections = env->CallObjectMethod(mProviderMap, valuesMethodId);

    jmethodID iterator = env->GetMethodID(env->GetObjectClass(collections), "iterator", "()Ljava/util/Iterator;");

    jobject mIterator = env->CallObjectMethod(collections, iterator);
    jmethodID hasNext = env->GetMethodID(env->GetObjectClass(mIterator), "hasNext", "()Z");
    jmethodID next = env->GetMethodID(env->GetObjectClass(mIterator), "next", "()Ljava/lang/Object;");
    while (env->CallBooleanMethod(mIterator, hasNext)) {
        jobject providerClientRecord = env->CallObjectMethod(mIterator, next);
        if (providerClientRecord == NULL) {
            LOGE("providerClientRecord = NULL");
            continue;
        }
        jclass ProviderClientRecordClass = env->FindClass(
                "android/app/ActivityThread$ProviderClientRecord");
        jfieldID mLocalProviderField = env->GetFieldID(ProviderClientRecordClass, "mLocalProvider", "Landroid/content/ContentProvider;");
        if (mLocalProviderField == NULL) {
            LOGE("mLocalProviderField not found");
            continue;
        }
        jobject mLocalProvider = env->GetObjectField(providerClientRecord, mLocalProviderField);
        if (mLocalProvider == NULL) {
            LOGE("mLocalProvider is NULL");
            continue;
        }

        jfieldID mContextField = env->GetFieldID(env->GetObjectClass(mLocalProvider), "mContext", "Landroid/content/Context;");
        if (mContextField == NULL) {
            LOGE("mContextField not found");
            continue;
        }
        env->SetObjectField(mLocalProvider, mContextField, originalApp);
    }
    LOGE("已完成脱壳");
    jmethodID onCreate = env->GetMethodID(env->GetObjectClass(originalApp), "onCreate", "()V");
    env->CallVoidMethod(originalApp, onCreate);
    LOGE("壳Application执行完毕");

}


extern "C"
JNIEXPORT jbyteArray JNICALL
Java_home_com_dexloader_1jni_MyApplication_decrypt1(JNIEnv *env, jobject instance,
                                                    jbyteArray srcdata_) {
    jbyte *srcdata = env->GetByteArrayElements(srcdata_, NULL);

    // TODO
    int len = env->GetArrayLength(srcdata_);
    signed char buff[len];
    env->GetByteArrayRegion(srcdata_, 0, len, buff);
    for (int i = 0; i < len; i++) {
        buff[i] = 0xFF ^ buff[i];
    }
    jbyteArray result = env->NewByteArray(len);
    env->SetByteArrayRegion(result, 0, len, buff);
    env->ReleaseByteArrayElements(srcdata_, srcdata, 0);
    return result;
}