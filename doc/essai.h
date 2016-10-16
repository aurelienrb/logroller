#define TYPE_MASK_LITERAL
#define TYPE_MASK_ARRAY
#define TYPE_MASK_LITERAL

#define TYPE_STR_ANSI_LITERAL\
    (TYPE_MASK_LITERAL | TYPE_MASK_ARRAY + TYPE_ANSI_CHAR)

#define TYPE_STR_ANSI_LITERAL
#define TYPE_STR_ANSI_BUFFER
#define TYPE_UINT32
#define TYPE_INT32
#define TYPE_UINT16
#define TYPE_INT16
#define TYPE_UCHAR
#define TYPE_CHAR

#define TYPE_IOMANIP



struct SubEntry
{
    unsigned char Type;
    unsigned long Size: 24;
    union
    {
      unsigned long ulong;
      void *ptr;
    };
};

struct StaticEntryInfos;

struct Entry
{
    unsigned short TotalSize;
    unsigned char Type;
    unsigned char Flags;
    unsigned long ThreadId;
    StaticEntryInfos *StaticInfos;
    
    SubEntry* GetFirstEntry();
};

 
