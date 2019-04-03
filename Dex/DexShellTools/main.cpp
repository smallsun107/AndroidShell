#include <iostream>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include "/usr/local/opt/openssl/include/openssl/sha.h"
#include "/usr/local/opt/openssl/include/openssl/crypto.h"

#include "Dex.h"

#define MAX_PATH 256

//小尾转大尾模式


typedef unsigned int uint_32;
uint_32 bswap_32(uint_32 x) {
    return (((uint_32) (x) & 0xff000000) >> 24) | \
        (((uint_32) (x) & 0x00ff0000) >> 8) | \
        (((uint_32) (x) & 0x0000ff00) << 8) | \
        (((uint_32) (x) & 0x000000ff) << 24);
}

//是否显示dex文件信息
bool g_bFlag = false;


int GetFileSize(FILE *fp)
{
    fseek(fp,SEEK_SET,SEEK_END);
    int nSize = ftell(fp);
    //把文件指针指向文件头
    rewind(fp);
    return nSize;
}

int FormatDex(char *buffer)
{
    DexHeader *pDexHeader = (DexHeader*)buffer;

    //是否打印DEX文件信息
    if (g_bFlag == true)
    {
        //printf("magic is:%s\r\n",pDexHeader->magic);
        //printf("checksum is:0x%x\r\n",pDexHeader->checksum);
        //printf("Sha - 1 is:");
        //for (int i = 0; i < 20; ++i)
        //{
        //    printf("%x",pDexHeader->signature[i]);
        //}
        printf("\r\n");
        //printf("file size is:0x%x",pDexHeader->fileSize);
        
        printf("Dex文件头:\r\n");

        printf("Magic:%s", pDexHeader->magic);
    
        printf("\n");
        printf("校检码:0x%x\n", pDexHeader->checksum);
    
        printf("Sha - 1签名:");
        for (int i = 0; i < 20;i++)
        {
            printf("%x", pDexHeader->signature[i]);
        }
        printf("\n");
        printf("Dex文件大小:  0x%x\n", pDexHeader->fileSize);
        printf("文件头大小:   0x%x\n", pDexHeader->headerSize);
        printf("连接段大小:   0x%x\n", pDexHeader->linkSize);
        printf("连接段开始位置:0x%x\n", pDexHeader->linkOff);
        printf("Map数据基地址: 0x%x\n", pDexHeader->mapOff);
        printf("字符串个数:   0x%x\n", pDexHeader->stringIdsSize);
        printf("字符串表基地址:0x%x\n", pDexHeader->stringIdsOff);
        printf("类型个数:     0x%x\n",pDexHeader->typeIdsSize);
        printf("类型表基地址: 0x%x\n", pDexHeader->typeIdsOff);
        printf("原型个数:     0x%x\n", pDexHeader->protoIdsSize);
        printf("原型表基地址: 0x%x\n", pDexHeader->protoIdsOff);
        printf("方法个数:     0x%x\n", pDexHeader->methodIdsSize);
        printf("方法表基地址: 0x%x\n", pDexHeader->methodIdsOff);
        printf("类个数:       0x%x\n", pDexHeader->classDefsSize);
        printf("类定义表基地址:0x%x\n", pDexHeader->classDefsOff);
        printf("数据段大小:   0x%x\n", pDexHeader->dataSize);
        printf("数据段基地址: 0x%x\n", pDexHeader->dataOff);
        printf("\r\n");

    
        printf("DexProtoId:\r\n");
        DexProtoId *pProtoId = (DexProtoId*)(buffer + pDexHeader->protoIdsOff);
        for (UINT32 i = 0; i < pDexHeader->protoIdsSize; i++)
        {
            printf("shortyIdx:%x\t", (&pProtoId[i])->shortyIdx);
            printf("returnTypeIdx:%d\t", (&pProtoId[i])->returnTypeIdx);
            printf("parametersOff:%d\t", (&pProtoId[i])->parametersOff);
            printf("\r\n");
        }
        printf("\r\n");
    
        printf("DexFieldId:\r\n");
        DexFieldId *pFieldId = (DexFieldId*)(buffer + pDexHeader->fieldIdsOff);
        for (UINT32 i = 0; i < pDexHeader->fieldIdsSize; i++)
        {
            printf("classIdx:0x%x\t", (&pFieldId[i])->classIdx);
            printf("typeIdx:0x%x\t", (&pFieldId[i])->typeIdx);
            printf("nameIdx:0x%x\t", (&pFieldId[i])->nameIdx);
            printf("\r\n");
        }
    
        printf("\r\n");
    
        printf("DexMethodId:\r\n");
        DexMethodId *pMethodId = (DexMethodId*)(buffer + pDexHeader->methodIdsOff);
        for (UINT32 i = 0; i < pDexHeader->methodIdsSize; i++)
        {
            printf("classIdx:0x%x\t", (&pMethodId[i])->classIdx);
            printf("protoIdx:0x%x\t", (&pMethodId[i])->protoIdx);
            printf("nameIdx:0x%x\t", (&pMethodId[i])->nameIdx);
            printf("\r\n");
        }
        printf("\r\n");
    
        printf("DexClassDef:\r\n");
        DexClassDef *pDexClassDef = (DexClassDef*)(buffer + pDexHeader->classDefsOff);
        for (UINT32 i = 0; i < pDexHeader->classDefsSize; i++)
        {
            printf("classIdx      :0x%x\t", (&pDexClassDef[i])->classIdx);
            printf("accessFlags   :0x%x\t", (&pDexClassDef[i])->accessFlags);
            printf("superclassIdx :0x%x\t", (&pDexClassDef[i])->superclassIdx);
            printf("interfacesOff :0x%x\t", (&pDexClassDef[i])->interfacesOff);
            printf("sourceFileIdx :0x%x\t", (&pDexClassDef[i])->sourceFileIdx);
            printf("annotationsOff:0x%x\t", (&pDexClassDef[i])->annotationsOff);
            printf("classbufferOff  :0x%x\t", (&pDexClassDef[i])->classDataOff);
            printf("staticValuesOff:0x%x\t", (&pDexClassDef[i])->staticValuesOff);
            printf("\r\n");
        }
        printf("\r\n");
    
        printf("DexStringId:\r\n");
        DexStringId *pDexStringId = (DexStringId*)(buffer + pDexHeader->stringIdsOff);
        for (UINT32 i = 0; i < pDexHeader->stringIdsSize; i++)
        {
            printf("stringbufferOff:0x%x %s\t", (&pDexStringId[i])->stringDataOff, (buffer + (&pDexStringId[i])->stringDataOff));
            printf("\r\n");
        }
        printf("\r\n");
    
    
        printf("DexMapList:\r\n");
        DexMapList *pDexMapList = (DexMapList*)(buffer + pDexHeader->mapOff);
        DexMapItem *pDexMapItem = (DexMapItem*)(buffer + pDexHeader->mapOff + 4);
        printf("Size:%d\r\n", pDexMapList->size);
    
        for (UINT32 i = 0; i < pDexMapList->size; i++,pDexMapItem++)
        {
            printf("type:%d\t", pDexMapItem->type);
            printf("unused:%d\t", pDexMapItem->unused);
            printf("size:%d\t", pDexMapItem->size);
            printf("offset:%d\t", pDexMapItem->offset);
            printf("\r\n");
        }
        printf("\r\n");
    
        DexTypeList *pDexTypeList = (DexTypeList*)(buffer + pDexHeader->typeIdsOff);
        DexTypeItem *pDexTypeItem = (DexTypeItem*)pDexTypeList;
        printf("Size:%d\r\n", pDexHeader->typeIdsSize);
        for (UINT32 i = 0; i < 7; i++, pDexTypeItem++)
        {
            printf("typeIdx:%d\t", pDexTypeItem->typeIdx);
            printf("\r\n");
        }
    }


}


#define BASE 65521
unsigned long adler32(unsigned char *buf, int len)
{

    unsigned long adler=1;
    unsigned long s1 = adler & 0xffff;
    unsigned long s2 = (adler >> 16) & 0xffff;

    int i;
    for (i = 0; i < len; i++)
    {
        s1 = (s1 + buf[i]) % BASE;
        s2 = (s2 + s1) % BASE;
    }
    return (s2 << 16) + s1;
}

int FixCheckSum(unsigned char *buffer, size_t nLen)
{
    printf("%d\r\n",nLen - 12);
    long  uCheckSum = adler32(buffer + 12,nLen - 12);
    printf("new CheckSum is %02x\r\n",uCheckSum);
    memcpy(buffer + 8,&uCheckSum,4);
    return 0;
}

int FixSHA1Header(unsigned char *buffer, int nLen)
{
    unsigned char digest[SHA_DIGEST_LENGTH];
    SHA_CTX ctx;
    SHA1_Init(&ctx);
    printf("%d\r\n",nLen - 32);
    SHA1_Update(&ctx,(buffer + 32),nLen - 32);
    SHA1_Final(digest,&ctx);

    char result[SHA_DIGEST_LENGTH * 2 + 1] = {0};
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i)
    {
        sprintf(&result[i * 2],"%02x",digest[i]);
    }

    memcpy(buffer + 12,digest,SHA_DIGEST_LENGTH);

    printf("new SHA1 is %s\r\n",result);
    return 0;
}

int FixDexHeader(unsigned char *buffer, int nLen)
{
    int nDexSize = nLen;
    memcpy(buffer + 32,&nDexSize,4);
}

int Encrypt(char *in,int nSize)
{
    for (int i = 0; i < nSize; ++i)
    {
        //异或
        in[i] =  in[i] ^ 0xff;
    }
    return 0;
}






int main() {

    /*
     * 加壳程序流程
     * 加密源程序APK
     * 把加密数值写入壳APK的DEX文件末尾，在文件尾部添加加密数据的大小
     * 修改壳DEX头文件中的checksum，signature，file_size
     * 使用源程序的AndroidMainfest.xml覆盖壳程序的AndroidMainfest.xml
     */

    /*
     * 脱壳程序流程
     * 读取DEX文件尾部的加密数据大小
     * 从DEX文件中读取数据并解密，把解密后数据保存为newApk.apk文件
     * 通过DexClassLoader动态加载APK
     */


    //sha1 测试


    printf("----------------------\r\n");
    printf("|   APK 简单加固实现  |\r\n");
    printf("----------------------\r\n");

    //壳APK
    printf("输入Dex文件路径:\r\n");
    char filePathDex[MAX_PATH] = {0};
    scanf("%s", filePathDex);

    //源APK
    printf("输入APK文件路径:\r\n");
    char filePathApk[MAX_PATH] = {0};
    scanf("%s", filePathApk);

    printf("是否显示Dex文件信息? 1 / 0\r\n");
    scanf("%d",&g_bFlag);


    //读取APK文件并加密
    FILE *fileApk = fopen(filePathApk, "rb");
    int nApkSize = 0;
    char *apkBuffer = nullptr;

    if (fileApk != nullptr)
    {
        nApkSize = GetFileSize(fileApk);
        apkBuffer = (char *) malloc(nApkSize + 1);
        fread(apkBuffer, 1, nApkSize, fileApk);
        //加密源APK
        Encrypt(apkBuffer, nApkSize);
    }
    else
    {
        printf("APK文件打开失败!\r\n");
    }


    FILE *fileDex = fopen(filePathDex,"rb");
    int nDexSize = 0;
    char *dexBuffer = nullptr;

    if (fileDex != nullptr)
    {
        nDexSize = GetFileSize(fileDex);
        dexBuffer = (char*)malloc(nDexSize + 1);
        fread(dexBuffer,1,nDexSize,fileDex);

        //解压文件,偷懒不解析文件结构取解压后大小
        //Bytef *uDestBuffer= (Bytef*)malloc(1024 * 1024);
        //uLongf uDestBufferLen = 1024 * 1024;
        //int nRet = uncompress(uDestBuffer,&uDestBufferLen,(Bytef*)dexBuffer,nDexSize);

        //解析dex
        FormatDex(dexBuffer);

    }
    else
    {
        printf("Dex文件打开失败!\r\n");
    }

    //合并文件
    uint32_t  nTotalLen = nApkSize + nDexSize + 4;//多的四字节保存apk长度
    printf("TotalLen is %d\r\n",nTotalLen);

    unsigned char *newDexBuffer = (unsigned char*)malloc(nTotalLen);
    //拷贝解壳dex
    memcpy(newDexBuffer,dexBuffer,nDexSize);
    //拷贝加密后APK
    memcpy(newDexBuffer + nDexSize,apkBuffer,nApkSize);
    //拷贝解壳数据长度 长度要注意有符号无符号转换
    //大尾存放长度
    printf("APK size is %d\r\n",nApkSize);
    unsigned int nn = bswap_32(nApkSize);
    memcpy(newDexBuffer + nDexSize + nApkSize,&nn,4);
    //test crc sha1

    //先修改文件长度，才能计算signature checksum
    FixDexHeader(newDexBuffer,nTotalLen);
    //需要先写sinature,才能计算checksum
    FixSHA1Header(newDexBuffer,nTotalLen);

    FixCheckSum(newDexBuffer,nTotalLen);



    //生成新的Dex
    FILE *newFile = fopen("classes.dex","wb");
    if (newFile != nullptr)
    {
        fwrite(newDexBuffer,nTotalLen,1,newFile);
        printf("Success!!!\r\n");
    }


    //收尾
    delete(apkBuffer);
    delete(dexBuffer);
    delete(newDexBuffer);
    return 0;
}
