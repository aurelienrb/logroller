#ifndef LOGENTRYPOOL_H
#define LOGENTRYPOOL_H

typedef unsigned long UINT32_OR_64;

struct LogEntry
{
    UINT32_OR_64 TotalSize;   
};

class LogEntryPool
{
public:
  LogEntryPool();
  
  void AttachBuffer( unsigned char * Buffer, size_t BufferSize );
  
  const LogEntry * GetHead() const;
  
  const LogEntry * GetNext( const LogEntry * ) const;
  
  LogEntry* GetTail();
  
  LogEntry* AllocNewTail();
  
  bool ResizeTail( size_t SizeToGrow );
  
  void RotateBuffer();
  
  LogEntry* MoveTailToTheTopOfTheBuffer();
  
private:
  // buffer management
  unsigned char *zBufferFirstByte;
  unsigned char *zBufferLastValidByte;
  size_t zBufferSize;
  // new / old data chuncks management
  unsigned char *zLastUsedByte;
  unsigned char *zOldDataFirstByte;
  unsigned char *zOldDataLastUsedByte;
};

#endif