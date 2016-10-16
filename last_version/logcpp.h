#ifndef LOGCPP_H
#define LOGCPP_H

// -------- CORE -------- 
struct LogEntryData
{
    unsigned short TotalSize;
    unsigned short DataType;
};
// -------- DATA ENCODING --------
struct LogEntryData_AnsiStrLiteral : public LogEntryData
{
    unsigned long CharCount;
    const char * AnsiText;
};

struct LogEntryData_AnsiStrBuffer : public LogEntryData
{
    unsigned long CharCount;
    char AnsiText[1];
};

struct LogEntryData_Integer : public LogEntryData
{
    unsigned long Value;
};
// -------- PRESENTATION LAYER --------
operator<<()

struct LogMsgStaticEntryInfos;
typedef unsigned long UINT32_OR_64;

struct LogMsgDynamicEntryInfos
{
  unsigned long MsgFlags;
  unsigned long ThreadId;
};

struct LogMsgEntry
{
    UINT32_OR_64 TotalSize;   
    const LogMsgStaticEntryInfos *StaticInfos;
    LogMsgDynamicEntryInfos DynInfos;    
    
    LogEntryData * GetFirstData();
    LogEntryData * GetNextStr(LogEntryData*);
};









#include <cassert>
#include <iostream>

struct LogMsgData
{
  unsigned short TotalSize;
  unsigned char DataType;
  unsigned char PaddingSize;
};

struct LogMsgDataLiteral : public LogMsgData
{
  union
  {
    const char * AnsiText;
    const wchar_t * WideText;
  } Str;
};

struct LogMsgDataInteger : public LogMsgData
{
  unsigned long Value;
};

struct LogMsgStr : public LogMsgData
{
  unsigned short CharCount; 
  union
  {
    const char * AnsiText;
    const wchar_t * WideText;
  } Str;
};

// sizeof = 64
struct LogMsgData32
{
  unsigned short TotalSize;
  unsigned char DataType;
  unsigned char PaddingSize;
  
  size_t GetCharCount() const;
  const char* GetAnsiStr() const;
  
  int GetInt() const;
};

// sizeof = 128
struct LogMsgData64
{
  unsigned long TotalSize;
  unsigned short DataType;
  unsigned short PaddingSize;
};
  
struct LogMsgString
{
  unsigned long TotalSize;
  bool IsWide : 1;
  unsigned short CharCount : 15;
  union
  {
    const char * AnsiText;
    const wchar_t * WideText;
  } Str;
  
  bool IsLiteral() const;
  size_t GetBufferAnsiCharCapacity();
  size_t GetBufferWideCharCapacity();
  void CopyNonLiteral( const char * AnsiStr, size_t Length );
  void CopyNonLiteral( const wchar_t * WideStr, size_t Length );
  void ConcatNonLiteral( const char * AnsiStr, size_t Length );
  void ConcatNonLiteral( const wchar_t * WideStr, size_t Length );
  bool CheckIntegrity() const;
};

struct LogMsgStaticEntryInfos
{
  unsigned long MsgType;
  const char * SrcFileName;
  unsigned long SrcFileLine;
  const char * FunctionName;
};

// the size of this struct should be aligned on 32/64 bit boundaries
struct LogMsgDynamicEntryInfos
{
  unsigned long MsgFlags; // LOG_ENTRY_CONSTRUCTION_ABORDED, SCOPE_EXIT_IS_TRACKED, EXITING_SCOPE, EXECEPTION_IN_PROGRESS
  unsigned long ThreadId;
};

ostream_get_first_msg & error_x();
ostream_get_error_code & operator << ( const ostream_get_first_msg &, const char * );
std::exception operator << ( const ostream_get_error_code &, int ErrCode );

throw error_x() << "Error " << 10;

=>
std::exception operator << ( const ostream_get_file_name &, const QFile & );
throw file_error() << qfile;

// capacite a detecter une taille statique minimale a partir de son utilisation precedente
// ainsi que le nombre de StrMsg statiques qu'il faut allouer
static LogMsgEntryInfos entry_line38 = {}; logmgr::instance() = newentry(&entry_line38)
static LogMsgEntryInfos entry_line39 = {}; entry_line39.newusage()
// no scoping
static LogMsgEntryInfos entry_line38 = {}; LogScoper scope_line38 = logmgr::newentry(&entry_line38)
// scoping
static LogMsgEntryInfos entry_line38 = {}; logmgr::newentry(&entry_line38)

#define log_scope()\
    entry_line38 = {}; log_scoper scoper_38(&entry_line38, LOG_ONLY_EXIT_BY_EXCEPTION) = logmgr::newentry(&entry_line38)

logmgr::log<__FILE__, __LINE__>

typedef unsigned long UINT32_OR_64;

struct LogMsgEntry
{
    UINT32_OR_64 TotalSize;   
    const LogMsgStaticEntryInfos *StaticInfos;
    LogMsgDynamicEntryInfos DynInfos;    
    
    LogMsgString * GetFirstStr();
    LogMsgString * GetNextStr(LogMsgString*);
};

class LogEntryPool
{
public:
  void AttachBuffer( unsigned char * Buffer, size_t BufferSize );
  
  const LogMsgEntry * GetHead() const;
  
  const LogMsgEntry * GetNext( const LogMsgEntry * ) const;
  
  LogMsgEntry* GetTail();
  
  LogMsgEntry* AllocNewTail();
  
  bool ResizeTail( size_t SizeToGrow );
  
  void RotateBuffer();
  
  LogMsgEntry* MoveTailToTheTopOfTheBuffer();
  
private:
  // buffer management
  unsigned char *zBufferFirstByte;
  unsigned char *zBufferLastValidByte;
  size_t zBufferSize;
  // new / old data chuncks management
  unsigned char *zNewDataLastUsedByte;
  unsigned char *zOldDataFirstByte;
  unsigned char *zOldDataLastUsedByte;
};

class LogMgr
{
public:
  void 
};

class LogBuffer
{
public:
  LogBuffer();

  void Attach( unsigned char *RawData, size_t DataSize );

  LogMsgBase* AllocMsg( size_t WantedSize );

  void Rotate();

  LogMsgBase * GetMoreRecentMsg();
  const LogMsgBase * GetMoreRecentMsg() const;
  const LogMsgBase * GetOlderMsg() const;
  const LogMsgBase * GetNextMsg(const LogMsgBase*) const;

  size_t GetCapacity() const
  {
    return this->zBufferSize;
  }

  size_t GetActualFreeMemorySize() const;
  size_t GetMaxReleasableMemorySize() const;
  bool ReleaseOlderChunkMemory( size_t NeededSize );
  
  size_t GetMaxMsgSize() const;
  
  bool ExpandMoreRecentMsgSize( size_t SizeToGrow );

  size_t ReleaseOlderMsgMemory();

private:
  LogMsgBase *zMoreRecentMsg;
  unsigned char *zBufferFirstByte;
  unsigned char *zBufferLastValidByte;
  unsigned char *zLastUsedByte;
  unsigned char *zOlderChunkFirstByte;
  unsigned char *zOlderChunkLastUsedByte;
  size_t zBufferSize;
};

class LogMgr
{
public:
  void ForceNewStringLiteral( const char *Str, size_t Length );
  void ForceNewStringBuffer( const char *Str, size_t Length );
  void AutoAppendText( const char *Str, size_t Length );
  void CheckIntegrity();

  void Init( unsigned char *RawData, size_t DataSize );
  size_t GetFullTextLength() const;
  size_t Write( char *Buffer, size_t BufferSize, size_t Offset = 0 ) const;

private:
  LogBuffer zBuffer;
};

#define log_trace()\
  LogMgr::ThisThreadMgr()->AppendStringLiteral

/*
class LogStream : public std::ostream
{
};

struct LogMsg
{
    size_t Length;
    const char * Text;
};

class LogMgr
{
public:
  uchar* first_byte( const LogMsg *Msg ) const
  {
    return reinterpret_cast<uchar*>( Msg );
  }

  uchar* last_byte( const LogMsg *Msg ) const
  {
    return first_byte( Msg ) + sizeof *Msg + Msg->Length;
  }

  size_t get_active_buffer_free_space() const;

  size_t get_needed_
  void append_msg( const char *, size_t TextLength )
  {
    size_t free_space = this->get_active_buffer_free_space();

  }

  const LogMsg * head() const;
  const LogMsg * tail() const;
  const LogMsg * next_msg(const LogMsg *) const;


private:
  uchar  *zBuffer0FirstByte;
  uchar  *zBuffer0LastByte;
  uchar  *zBuffer1FirstByte;
  uchar  *zBuffer1LastByte;
  int    zActiveBufferNum;
  size_t zBuffer_size;
  LogMsg *zHead;
  LogMsg *zTail;
};
*/

inline bool LogMsgBase::IsOfTypeAnsiStringLiteral() const
{
  return this->TotalSize == sizeof(AnsiStrLiteralMsg);
}

#endif
