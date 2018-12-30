#include <jni.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <android/log.h>
#include <elf.h>

#define LOG_TAG "DEBUG"
#define LOGD(fmt,args...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,fmt,##args)

extern "C" JNIEXPORT
//把函数放置在自定义的节中
//jstring JNICALL Java_home_com_sotest_MainActivity_stringFromJNI(JNIEnv* env,jobject) __attribute__((section(".mytext")));
jstring JNICALL Java_home_com_sotest_MainActivity_stringFromJNI(
       JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";

    return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT
//替换成自定义section的名字
//jint JNICALL Java_home_com_sotest_MainActivity_Add(JNIEnv *env, jobject obj)  __attribute__((section(".mytext")));
jint JNICALL Java_home_com_sotest_MainActivity_Add(JNIEnv *env, jobject obj)
{
    printf("Call Add Func!\r\n");
    return (jint)(1 + 1);
}


unsigned int GetLibAddr()
{
    char *szName = "libnative-lib.so";
    int nPid = getpid();
    unsigned int nBase = 0;
    char buffer[1024] = {0};
    sprintf(buffer,"/proc/%d/maps",nPid);
    FILE *fp = fopen(buffer,"r");
    if (fp != nullptr)
    {
        while(fgets(buffer,sizeof(buffer),fp))
        {
            if(strstr(buffer,szName))
            {
                //分割字符串 返回-之前内容
                char *temp;
                temp = strtok(buffer,"-");
                nBase = strtoul(temp,nullptr,16);
                LOGD("BASE IS 0x%x\r\n",nBase);
                break;
            }
        }
    }
    fclose(fp);
    return nBase;
}

static unsigned elfhash(const char *_name)
{
    const unsigned char *name = (const unsigned char *) _name;
    unsigned h = 0, g;

    while(*name) {
        h = (h << 4) + *name++;
        g = h & 0xf0000000;
        h ^= g;
        h ^= g >> 24;
    }
    return h;
}

typedef struct _funcInfo{
  Elf32_Addr st_value;
  Elf32_Word st_size;
}funcInfo;

void GetTargetFuncInfo(unsigned int nBase,const char *funcName,funcInfo *info)
{
    Elf32_Ehdr *pEhdr = (Elf32_Ehdr *) nBase;
    Elf32_Phdr *pHdr = (Elf32_Phdr *) (nBase + pEhdr->e_phoff);

    LOGD("phdr =  0x%p, size = 0x%x\n", pEhdr, pEhdr->e_phnum);
    for (int i = 0; i < pEhdr->e_phnum; ++i)
    {
        LOGD("phdr =  0x%p\n", pHdr);
    	//获得动态链接节
        if(pHdr->p_type ==  PT_DYNAMIC)
        {
            LOGD("Find .dynamic segment");
            break;
        }
       pHdr ++;
    }

    Elf32_Off dyn_vaddr = pHdr->p_vaddr + nBase;
    Elf32_Word dyn_size = pHdr->p_filesz;

    Elf32_Dyn *dyn;
    Elf32_Addr dyn_symtab, dyn_strtab, dyn_hash;
    Elf32_Word dyn_strsz;
    for (int i = 0; i < dyn_size / sizeof(Elf32_Dyn); i++)
    {
        dyn = (Elf32_Dyn *)(dyn_vaddr + i * sizeof(Elf32_Dyn));

        //符号表位置
        if(dyn->d_tag == DT_SYMTAB)
        {
            dyn_symtab = (dyn->d_un).d_ptr;
            LOGD("Find .dynsym section, addr = 0x%x\n", dyn_symtab);
        }
        //获得hash段
        if(dyn->d_tag == DT_HASH)
        {
            dyn_hash = (dyn->d_un).d_ptr;
            LOGD("Find .hash section, addr = 0x%x\n", dyn_hash);
        }
        //保存函数字符串的位置
        if(dyn->d_tag == DT_STRTAB)
        {
            dyn_strtab = (dyn->d_un).d_ptr;
            LOGD("Find .dynstr section, addr = 0x%x\n", dyn_strtab);
        }
        //字符串长度
        if(dyn->d_tag == DT_STRSZ)
        {
            dyn_strsz = (dyn->d_un).d_val;
            LOGD("Find strsz size = 0x%x\n", dyn_strsz);
        }
    }
    dyn_symtab += nBase;
    dyn_hash += nBase;
    dyn_strtab += nBase;
    dyn_strsz += nBase;
 /*     nbucket
 *-----------------
 *	   nchain
 *------------------
 *	  bucket[0]
 *       ...
 *   bucket[nbucket-1]
 * ------------------
 *     chain[0]
 *       ...
 *   chain[nchain-1]
 */
    unsigned funHash =  elfhash(funcName);//获得函数名称经过hash运行后的值
    Elf32_Sym *funSym = (Elf32_Sym *) dyn_symtab;
    char *dynstr = (char*) dyn_strtab;
    unsigned nbucket = *((int *) dyn_hash);//获得nbucket的值
    int *bucket = (int *)(dyn_hash + 8);//bucket链
    unsigned int *chain = (unsigned int *)(dyn_hash + 4 * (2 + nbucket));//越过bucket链，到达chain链
    LOGD("hash = 0x%x, nbucket = 0x%x\n", funHash, nbucket);
    //bucket[X%nbucket]给出了一个索引y，该索引可用于符号表，也可用于chain表
    int mod = (funHash % nbucket);
    LOGD("mod = %d\n", mod);
    LOGD("i = 0x%d\n", bucket[mod]);

    //i = mod = bucket[funHash%nbucket]，通过遍历i = chain[i]表，找到funSym对应的符号表
    int i;
    for(i = bucket[funHash % nbucket]; i != 0; i = chain[i])
    {
        if(strcmp(dynstr + (funSym + i)->st_name, funcName) == 0)
        {
            LOGD("Find %s\n", funcName);
            break;
        }
    }
    info->st_value = (funSym + i)->st_value;//函数对应符号表中保存函数的地址
    info->st_size = (funSym + i)->st_size;//函数符号表中保存函数的大小

    LOGD("st_value = %d,st_size = %d",info->st_value,info->st_size);
}


void Init() __attribute__((constructor)); //使用”attribute((constructor))”将函数放到”.init_array”段

void Init()
{
     LOGD("Call Init");
     unsigned int nBase = GetLibAddr();
     Elf32_Ehdr *pEhdr = (Elf32_Ehdr *)nBase;
     funcInfo info;
     const char funcName[] = "Java_home_com_sotest_MainActivity_Add";
     GetTargetFuncInfo(nBase,funcName,&info);

     unsigned int nPage = info.st_size / PAGE_SIZE + ((info.st_size % PAGE_SIZE == 0) ? 0 : 1);
     LOGD("nPage =  %02x", nPage);
     LOGD("nPage =  %02x", PAGE_SIZE);

     if(mprotect((void *) ((nBase + info.st_value) / PAGE_SIZE * PAGE_SIZE), 4096 * nPage, PROT_READ | PROT_EXEC | PROT_WRITE) != 0)
     {
        LOGD("mem privilege change failed");
     }

      for(int i = 0;i < info.st_size - 1; i++)
      {
        char *addr = (char*)(nBase + info.st_value - 1 + i);
        *addr = ~(*addr);
      }

      if(mprotect((void *) ((nBase + info.st_value) / PAGE_SIZE * PAGE_SIZE), 4096 * nPage, PROT_READ | PROT_EXEC) != 0)
      {
        LOGD("mem privilege change failed");
      }
    /*节解密

     //获取加密节地址
     unsigned int mytextBase = pEhdr->e_shoff + nBase;
     unsigned int nBlock = (pEhdr->e_entry) >> 16;//加密节的大小
     unsigned int nSize = (pEhdr->e_entry) & 0xffff;//加密节的大小
     LOGD("nBlock = %d,nSize = %d", nBlock,nSize);
     LOGD("mytextBase = 0x%x", mytextBase);

     //修改内存属性
     if(mprotect((void*)(mytextBase / PAGE_SIZE * PAGE_SIZE),4096 * nSize,PROT_READ | PROT_EXEC | PROT_WRITE) != 0)
     {
        LOGD("mem privilege change failed");
     }

     //解密
     for(int i = 0;i < nBlock; i++)
     {
        unsigned char *addr = (unsigned char*)(mytextBase + i);
        *addr = ~(*addr);
     }

      if(mprotect((void *) (mytextBase / PAGE_SIZE * PAGE_SIZE), 4096 * nSize, PROT_READ | PROT_EXEC) != 0)
      {
         LOGD("mem privilege change failed");
      }

      LOGD("Decrypt success!!!");
      */

}


