#include <iostream>
#include "elf.h"


#define MAX_PATH 256
typedef struct _funcInfo{
    Elf32_Addr st_value;
    Elf32_Word st_size;
}funcInfo;

int GetFileSize(FILE *fp)
{
    fseek(fp,0,SEEK_END);
    int nSize = ftell(fp);
    rewind(fp);
    return nSize;
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

void GetTargetFuncInfo(unsigned char* buffer,char *funcName,funcInfo *info)
{
    Elf32_Ehdr *pEhdr = (Elf32_Ehdr *) buffer;
    Elf32_Phdr *pHdr = (Elf32_Phdr *) (buffer + pEhdr->e_phoff);
    Elf32_Word DynSize = 0;
    Elf32_Word DynStrsz = 0;
    Elf32_Off DynOffset = 0;
    Elf32_Addr DynSymtab, DynStrtab, DynHash;

    for (int i = 0; i < pEhdr->e_phnum; i++)
    {
        if (pHdr->p_type == PT_DYNAMIC)
        {
            DynSize = pHdr->p_filesz;
            DynOffset = pHdr->p_offset;
            printf("Find Section %s size = %d offser = %02x\n", "PT_DYNAMIC", DynSize, DynOffset);
        }
        pHdr++;
    }

    Elf32_Dyn *pDyn = (Elf32_Dyn *) (buffer + DynOffset);

    for (int j = 0; j < DynSize / sizeof(Elf32_Dyn); j++)
    {
        //符号表位置
        if (pDyn->d_tag == DT_SYMTAB)
        {
            DynSymtab = pDyn->d_un.d_ptr;
            printf("Find .dynsym, addr = 0x%x, val = 0x%x\n", DynSymtab, pDyn->d_un.d_val);
        }
        //hash位置
        if (pDyn->d_tag == DT_HASH)
        {
            DynHash = pDyn->d_un.d_ptr;
            printf("Find .hash, addr = 0x%x\n", DynHash);
        }
        //字符串位置
        if (pDyn->d_tag == DT_STRTAB)
        {
            DynStrtab = pDyn->d_un.d_ptr;
            printf("Find .dynstr, addr = 0x%x\n", DynStrtab);
        }
        //字符串大小
        if (pDyn->d_tag == DT_STRSZ)
        {
            DynStrsz = pDyn->d_un.d_val;
            printf("Find .dynstr size, size = 0x%x\n", DynStrsz);
        }
        pDyn++;

    }

    //定位字符串表
    char *dynStr = (char *) malloc(DynStrsz);
    memcpy(dynStr, buffer + DynStrtab, DynStrsz);


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
    //计算函数名称经过hash运行后的值
    unsigned funHash = elfhash(funcName);
    printf("Function %s hashVal = 0x%x\n", funcName, funHash);

    //获取nbucket的值
     int nNbucket = *(int *) (buffer + DynHash);
    printf("nbucket = %d\n", nNbucket);

    //获取nchain
    int nNchain = *(int *) (buffer + DynHash + 4);
    printf("nchain = %d\n", nNchain);

    funHash = funHash % nNbucket;        //bucket[X%nbucket]给出了一个索引y，该索引可用于符号表，也可用于chain表
    printf("funHash mod nbucket = %d \n", funHash);

    int nFunIndex = *(int *) (buffer + DynHash + 8 + funHash * 4);//y = bucket[X%nbucket]返回的索引y
    printf("funcIndex:%d\n", nFunIndex);

    Elf32_Sym *pSym = (Elf32_Sym *) (buffer + DynSymtab + nFunIndex * sizeof(Elf32_Sym));//该索引对应的符号表

    //如果索引y对应的符号表不是所需要的,那么chain[y]则给出了具有相同哈希值的下一个符号表项
    if (strcmp(dynStr + pSym->st_name, funcName) != 0)
    {
        while (1)
        {
            printf("hash:%x,nbucket:%d,funIndex:%d\n", DynHash, nNbucket, nFunIndex);
            nFunIndex = *(int *) (buffer + DynHash + 4 * (2 + nNbucket + nFunIndex));  //搜索chain链
            printf("funcIndex:%d\n", nFunIndex);

            if (nFunIndex == 0) {
                puts("Cannot find funtion!\n");
                return;
            }
            pSym = (Elf32_Sym *) (buffer + DynSymtab + nFunIndex * sizeof(Elf32_Sym)); //chain[]中对应的符号表
            if (strcmp(dynStr + pSym->st_name, funcName) == 0)
            {
                break;
            }
        }
    }
    printf("Find: %s, offset = 0x%x, size = 0x%x\n", funcName, pSym->st_value, pSym->st_size);
    info->st_value = pSym->st_value;
    info->st_size = pSym->st_size;
    free(dynStr);
    return;
}

int main()
{

    printf("输入SO文件路径:\r\n");
    char filePath[MAX_PATH] = {0};
    // char *test = "/Users/smallsun/Desktop/SoTest/app/build/outputs/apk/debug/app-debug/lib/armeabi-v7a/libnative-lib.so";
    scanf("%s", filePath);

    FILE *fp = fopen(filePath,"rb");
    if (fp != nullptr)
    {
        int nSize = GetFileSize(fp);

        unsigned char *pBuffer = (unsigned char *)malloc(nSize);
        fread(pBuffer,1,nSize,fp);
        funcInfo info;
        //通过hash段中chain链获得的索引，获取在dynsym对应的条目
        GetTargetFuncInfo(pBuffer,"Java_home_com_sotest_MainActivity_Add",&info);
        //获取函数地址
        for (int i = 0; i < info.st_size - 1; i++)
        {
            char *content = (char*)(pBuffer + info.st_value - 1 + i);
            *content = ~(*content);
        }


        FILE *fp = fopen("libnative-lib_new.so","wb");
        fwrite(pBuffer,nSize,1,fp);
    }
    else
    {
        printf("文件打开失败！\r\n");
    }


    return 0;
}