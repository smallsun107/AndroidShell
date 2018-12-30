#define  byte   unsigned char
#define  UINT32 unsigned int

#define ACC_PUBLIC		0x1    // class, field, method, ic
#define ACC_PRIVATE		0x2   // field, method, ic
#define ACC_PROTECTED	0x4  // field, method, ic
#define ACC_STATIC		0x8 // field, method, ic






struct DexHeader
{
    byte magic[8];
    UINT32 checksum;
    byte signature[20];
    UINT32 fileSize;
    UINT32 headerSize;
    UINT32 endianTag;
    UINT32 linkSize;
    UINT32 linkOff;
    UINT32 mapOff;
    UINT32 stringIdsSize;
    UINT32 stringIdsOff;
    UINT32 typeIdsSize;
    UINT32 typeIdsOff;
    UINT32 protoIdsSize;
    UINT32 protoIdsOff;
    UINT32 fieldIdsSize;
    UINT32 fieldIdsOff;
    UINT32 methodIdsSize;
    UINT32 methodIdsOff;
    UINT32 classDefsSize;
    UINT32 classDefsOff;
    UINT32 dataSize;
    UINT32 dataOff;
};

struct DexMapItem {
    unsigned short type;              /* type code (see kDexType* above) */
    unsigned short unused;
    UINT32 size;              /* count of items of the indicated type */
    UINT32 offset;            /* file offset to the start of data */
};

struct DexMapList {
    UINT32  size;               /* #of entries in list */
    DexMapItem list[1];     /* entries */
};

struct DexStringId {
    UINT32 stringDataOff;      /* file offset to string_data_item */
};

struct DexProtoId
{
    UINT32 shortyIdx;      //指向DexStringID列表的索引
    UINT32 returnTypeIdx;  //指向DexTypeID列表的索引
    UINT32 parametersOff;  //指向DexTypeList的偏移
};

struct DexFieldId
{
    unsigned short  classIdx;		//类的类型
    unsigned short	typeIdx;		//字段类型
    UINT32			nameIdx;		//字段名
};

struct DexMethodId
{
    unsigned short  classIdx;		//类的类型
    unsigned short  protoIdx;		//声明类型
    UINT32			nameIdx;		//方法名
};

struct DexTypeItem {
    UINT32  typeIdx;            /* index into typeIds */
};

struct DexTypeList {
    UINT32  size;               /* #of entries in list */
    DexTypeItem list[1];    /* entries */
};

struct DexClassDef
{
    UINT32  classIdx;			//描述具体的 class 类型,idx of type_ids
    UINT32  accessFlags;		//描述 class 的访问类型 ，如 public , final , static 等
    UINT32	superclassIdx;		//描述父类的类型,idx of type_ids
    UINT32  interfacesOff;		//指向 class 的 interfaces(接口), 被指向的数据结构为 type_list,若无接口，值为0
    UINT32  sourceFileIdx;	//表示源代码文件的信息,值是string_ids的一个 index.若此项信息缺失,此项值赋值为NO_INDEX=0xffff ffff
    UINT32	annotationsOff;	//值是一个偏移地址 ，指向的内容是该 class 的注释 ，位置在 data 区，格式为 annotations_direcotry_item 。若没有此项内容 ，值为 0 。
    UINT32  classDataOff;		//指向class_data_item[]
    UINT32  staticValuesOff;	//指向 data 区里的一个list,格式为 encoded_array_item
};