#include <iostream>
#include "elf.h"

#define MAX_PATH 256
int GetFileSize(FILE *fp)
{
    fseek(fp,0,SEEK_END);
    int nSize = ftell(fp);
    rewind(fp);
    return nSize;
}

int FormatElf(unsigned char *buffer)
{
    //自定义节名
    char *szSection = ".mytext";

    //ELF头地址
    Elf32_Ehdr *pEhdr = (Elf32_Ehdr*)buffer;
    if (!((pEhdr->e_ident[0] == 0x7F) &&
          (pEhdr->e_ident[1] == 'E') &&
          (pEhdr->e_ident[2] == 'L') &&
          (pEhdr->e_ident[3] == 'F')))
    {
        printf("无效的文件\r\n");
        return 0;
    }

    //定位到节表
    Elf32_Shdr *pShdr = (Elf32_Shdr*)(buffer + pEhdr->e_shoff);

    //定位字符串表
    char *str = nullptr;
    Elf32_Shdr *pShdrstr = (Elf32_Shdr *)(buffer + pEhdr->e_shoff + sizeof(Elf32_Shdr) * pEhdr->e_shstrndx);
    str = (char*)(pShdrstr->sh_offset + buffer);

    int nOffset = 0;
    int nSize = 0;

    for (int i = 0; i < pEhdr->e_shnum; i++)
    {
        if (strcmp(str + pShdr->sh_name,szSection) == 0)
        {
            //获取偏移和大小
            nOffset = pShdr->sh_offset;
            nSize   = pShdr->sh_size;
            printf("find section %s offset is 0x%x size is 0x%x\n",szSection,nOffset,nSize);
            break;
        }
        pShdr++;
    }

    
    unsigned char *sectionBuf = (unsigned char *)(buffer + nOffset);


    //计算section占用内存几页，一个页大小4096
    int nPage = nSize / 4096 + (nSize % 4096 == 0 ? 0 :1);
    printf("nOffset = 0x%x, nSize = 0x%x\n", nOffset, nSize);
    printf("nPage = %d\n",nPage);
    pEhdr->e_entry = (nSize << 16) + nPage;
    pEhdr->e_shoff = nOffset;

    //加密
    printf("sectionBuf is %x",sectionBuf);
    for (int j = 0; j < nSize; j++)
    {
        sectionBuf[j] = ~sectionBuf[j];
    }

    return 1;

}

int main()
{
    /*流程
     *从文件头读取section偏移shoff，shnum和shstrtab
     *读取shstrtab中的字符串
     *从shoff读取section header
     *通过pShdr->sh_name读取节表名比较是否自定义节表名
     *通过pShdr->sh_offset pShdr->sh_size读取节表大小以及内容进行加密
     *修改section字段中的e_shoff为pShdr->sh_addr，修改e_entry为pShdr->sh_size
     * */

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

        FormatElf(pBuffer);
        FILE *fp = fopen("libnative-lib_new.so","wb");
        fwrite(pBuffer,nSize,1,fp);
    }
    else
    {
        printf("文件打开失败！\r\n");
    }

    return 0;
}