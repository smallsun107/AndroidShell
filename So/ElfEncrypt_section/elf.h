//
// Created by SmallSun on 2018/12/24.
//

#ifndef ELFENCRYPT_ELF_H
#define ELFENCRYPT_ELF_H


//	Name           Size Alignment   Purpose
//	====           ==== =========   =======
//	Elf32_Addr      4       4       Unsigned program address
//	Elf32_Half      2       2       Unsigned medium integer
//	Elf32_Off       4       4       Unsigned file offset
//	Elf32_Sword     4       4       Signed large integer
//	Elf32_Word      4       4       Unsigned large integer
//	unsigned char   1       1       Unsigned small integer
typedef unsigned short      WORD;
//typedef unsigned long       DWORD; windows下4字节 mac 8字节
typedef unsigned int       DWORD;
typedef	DWORD	Elf32_Addr;
typedef	WORD	Elf32_Half;
typedef	DWORD	Elf32_Off;
typedef	DWORD	Elf32_Sword;
typedef	DWORD	Elf32_Word;

#define EI_NIDENT       16

// ELF header
typedef struct {
    unsigned char       e_ident[EI_NIDENT];
    Elf32_Half          e_type;
    //	e_type
    //
    //	该成员确定该object的类型。
    //
    //	Name        Value  Meaning
    //	====        =====  =======
    //	ET_NONE         0  No file type
    //	ET_REL          1  Relocatable file
    //	ET_EXEC         2  Executable file
    //	ET_DYN          3  Shared object file
    //	ET_CORE         4  Core file
    //	ET_LOPROC  0xff00  Processor-specific
    //	ET_HIPROC  0xffff  Processor-specific

    Elf32_Half          e_machine;
    //	e_machine
    //
    //	该成员变量指出了运行该程序需要的体系结构。
    //
    //	Name      Value  Meaning
    //	====      =====  =======
    //	EM_NONE       0  No machine
    //	EM_M32        1  AT&T WE 32100
    //	EM_SPARC      2  SPARC
    //	EM_386        3  Intel 80386
    //	EM_68K        4  Motorola 68000
    //	EM_88K        5  Motorola 88000
    //	EM_860        7  Intel 80860
    //	EM_MIPS       8  MIPS RS3000

    Elf32_Word          e_version;
    //	e_version
    //
    //	这个成员确定object文件的版本。
    //
    //	Name         Value  Meaning
    //	====         =====  =======
    //	EV_NONE          0  Invalid version
    //	EV_CURRENT       1  Current version

    Elf32_Addr          e_entry;
    //	该成员是系统第一个传输控制的虚拟地址，在那启动进程。
    //	假如文件没有如何关联的入口点，该成员就保持为 0。

    Elf32_Off           e_phoff;
    //	该成员保持着程序头表（program header table）在文件中的偏移量(以字节计数)。
    //	假如该文件没有程序头表的的话，该成员就保持为 0。

    Elf32_Off           e_shoff;
    //	该成员保持着section头表（section header table）在文件中的偏移量(以字节计数)。
    //	假如该文件没有section头表的的话，该成员就保持为0。

    Elf32_Word          e_flags;
    //	该成员保存着相关文件的特定处理器标志。
    //	flag的名字来自于EF_<machine>_<flag>。看下机器信息“Machine Information”部分的flag的定义。

    Elf32_Half          e_ehsize;
    //	该成员保存着ELF头大小(以字节计数)。

    Elf32_Half          e_phentsize;
    //	该成员保存着在文件的程序头表（program header table）中一个入口的大小
    //	(以字节计数)。所有的入口都是同样的大小。

    Elf32_Half          e_phnum;
    //	该成员保存着在程序头表中入口的个数。
    //	因此，e_phentsize和e_phnum的乘机就是表的大小(以字节计数).
    //	假如没有程序头表（program header table），e_phnum变量为0。

    Elf32_Half          e_shentsize;
    //	该成员保存着section头的大小(以字节计数)。
    //	一个section头是在section头表(section header table)的一个入口；
    //	所有的入口都是同样的大小。

    Elf32_Half          e_shnum;
    //	该成员保存着在section header table中的入口数目。
    //	因此，e_shentsize和e_shnum的乘积就是section头表的大小(以字节计数)。
    //	假如文件没有section头表，e_shnum值为0。

    Elf32_Half          e_shstrndx;
    //	该成员保存着跟section名字字符表相关入口的section头表(section header table)索引。
    //	假如文件中没有section名字字符表，该变量值为SHN_UNDEF。
    //	更详细的信息 看下面“Sections”和字符串表(“String Table”) 。

} Elf32_Ehdr;

char sz_desc_e_type[][128] = {	"No file type",
                                  "Relocatable file",
                                  "Executable file",
                                  "Shared object file",
                                  "Core file"
};

char sz_desc_e_machine[][128] = {"No mach", "AT&T", "SPARC", "80386", "Motorola 68", "Motorola 88", "Unknown", "8086", "MIPS"};

char sz_desc_e_entry[] = "Entry point.";


// Program Header
typedef struct {
    Elf32_Word p_type;
    //	Name             Value
    //	====             =====
    //	PT_NULL              0
    //	PT_LOAD              1
    //	PT_DYNAMIC           2
    //	PT_INTERP            3
    //	PT_NOTE              4
    //	PT_SHLIB             5
    //	PT_PHDR              6
    //	PT_LOPROC   0x70000000
    //	PT_HIPROC   0x7fffffff

    Elf32_Off  p_offset;
    //	该成员给出了该段的驻留位置相对于文件开始处的偏移。

    Elf32_Addr p_vaddr;
    //	该成员给出了该段在内存中的首字节地址。

    Elf32_Addr p_paddr;

    Elf32_Word p_filesz;
    //	文件映像中该段的字节数；它可能是 0 。

    Elf32_Word p_memsz;
    //	内存映像中该段的字节数；它可能是 0 。

    Elf32_Word p_flags;

    Elf32_Word p_align;
    //	该成员给出了该段在内存和文件中排列值。
    //	0 和 1 表示不需要排列。否则， p_align 必须为正的 2 的幂，
    //	并且 p_vaddr 应当等于 p_offset 模 p_align 。


} Elf32_Phdr;


char sz_desc_p_type[][128] = {"PT_NULL", "PT_LOAD", "PT_DYNAMIC", "PT_INTERP", "PT_NOTE", "PT_SHLIB", "PT_PHDR"};

// Section Header
typedef struct {
    Elf32_Word sh_name;
    //	该成员指定了这个section的名字。
    //	它的值是section报头字符表section的索引。[看以下的“String Table”], 以NULL空字符结束。

    Elf32_Word sh_type;
    //	Section Types, sh_type
    //	---------------------------
    //	Name                 Value    Description
    //	====                 =====    ===========
    //	SHT_NULL				 0    该值表明该section头是无效的；它没有相关的section。
    //	SHT_PROGBITS			 1    该section保存被程序定义了的一些信息，它的格式和意义取决于程序本身。
    //	SHT_SYMTAB				 2    该sections保存着一个符号表（symbol table）。
    //	SHT_STRTAB				 3    该section保存着一个字符串表。
    //	SHT_RELA				 4    该section保存着具有明确加数的重定位入口。
    //	SHT_HASH				 5    该标号保存着一个标号的哈希(hash)表。
    //	SHT_DYNAMIC				 6    该section保存着动态连接的信息。
    //	SHT_NOTE				 7    该section保存着其他的一些标志文件的信息。
    //	SHT_NOBITS				 8    该类型的section在文件中不占空间，但是类似SHT_PROGBITS。
    //	SHT_REL					 9    该section保存着重定位的入口。
    //	SHT_SHLIB				10    该section类型保留但语意没有指明。包含这个类型的section的程序是不符合ABI的。
    //	SHT_DYNSYM				11    该sections保存着一个符号表（symbol table）。
    //	SHT_LOPROC		0x70000000    在这范围之间的值为特定处理器语意保留的。
    //	SHT_HIPROC		0x7fffffff    在这范围之间的值为特定处理器语意保留的。
    //	SHT_LOUSER		0x80000000    该变量为应用程序保留的索引范围的最小边界。
    //	SHT_HIUSER		0xffffffff    该变量为应用程序保留的索引范围的最大边界。


    Elf32_Word sh_flags;
    //	Section Attribute Flags, sh_flags
    //	-----------------------------------
    //	Name                Value    Description
    //	====                =====    ===========
    //	SHF_WRITE             0x1    该section包含了在进程执行过程中可被写的数据。
    //	SHF_ALLOC             0x2    该section在进程执行过程中占据着内存。
    //	SHF_EXECINSTR         0x4    该section包含了可执行的机器指令。
    //	SHF_MASKPROC   0xf0000000    所有的包括在这掩码中的位为特定处理语意保留的。

    Elf32_Addr sh_addr;
    //	假如该section将出现在进程的内存映象空间里，该成员给出了一个该section在内存中的位置。否则，该变量为0。

    Elf32_Off  sh_offset;
    //	该成员变量给出了该section的字节偏移量(从文件开始计数)。

    Elf32_Word sh_size;
    //	该成员给你了section的字节大小。

    Elf32_Word sh_link;
    //	该成员保存了一个section报头表的索引连接，它的解释依靠该section的类型。
    //	更多信息参见表"sh_link and sh_info Interpretation"

    Elf32_Word sh_info;
    //	该成员保存着额外的信息，它的解释依靠该section的类型。

    //	sh_link and sh_info Interpretation

    //	-------------------------------------------------------------------------------
    //	sh_type        sh_link                          sh_info
    //	=======        =======                          =======
    //	SHT_DYNAMIC    The section header index of      0
    //	               the string table used by
    //	               entries in the section.
    //	-------------------------------------------------------------------------------
    //	SHT_HASH       The section header index of      0
    //	               the symbol table to which the
    //	               hash table applies.
    //	-------------------------------------------------------------------------------
    //	SHT_REL        The section header index of      The section header index of
    //	SHT_RELA       the associated symbol table.     the section to which the
    //	                                                relocation applies.
    //	-------------------------------------------------------------------------------
    //	SHT_SYMTAB     The section header index of      One greater than the symbol
    //	-------------------------------------------------------------------------------
    //	SHT_DYNSYM     the associated string table.     table index of the last local
    //	               symbol (binding STB_LOCAL).
    //	-------------------------------------------------------------------------------
    //	other          SHN_UNDEF                        0
    //	-------------------------------------------------------------------------------


    Elf32_Word sh_addralign;
    //	一些sections有地址对齐的约束。

    Elf32_Word sh_entsize;
    //	一些sections保存着一张固定大小入口的表，就象符号表。
    //	对于这样一个section来说，该成员给出了每个入口的字节大小。
    //	如果该section没有保存着一张固定大小入口的表，该成员就为0。
} Elf32_Shdr;


char sz_desc_sh_type[][128] = {	"SHT_NULL", "SHT_PROGBITS", "SHT_SYMTAB", "SHT_STRTAB",
                                   "SHT_RELA", "SHT_HASH", "SHT_DYNAMIC", "SHT_NOTE",
                                   "SHT_NOBITS", "SHT_REL", "SHT_SHLIB", "SHT_DYNSYM"};
char sz_desc_sh_flags[][128] = {"Unknown", "SHF_WRITE", "SHF_ALLOC", "SHF_WRITE & SHF_ALLOC", "SHF_EXECINSTR",
                                "SHF_WRITE & SHF_ALLOC", "SHF_ALLOC & SHF_EXECINSTR",
                                "SHF_WRITE & SHF_ALLOC & SHF_EXECINSTR"};

char sz_desc_sh_addr[] = "Position in Mem.";

char sz_desc_sh_offset[] = "Position in file.";


//重定位表
typedef struct
{
    Elf32_Addr  r_offset; //重定位地址
    Elf32_Word  r_info;  //重定位类型和相关符号
    Elf32_Sword r_addend;//加到符号上的常数
}Elf32_Rel;

//符号表
typedef struct
{
    Elf32_Word st_name;//符号串表索引
    Elf32_Addr st_value;//符号值
    Elf32_Word st_size;//对象大小
    unsigned char st_info;//绑定属性和类型
    unsigned char st_other;//未使用
    Elf32_Half sth_shndx;//所在节索引
}Elf32_Sym;

#endif //ELFENCRYPT_ELF_H
