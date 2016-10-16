#ifndef LOGCPP_H
#define LOGCPP_H

#include <cassert>
#include <iostream>

struct LogMsgBase
{
    unsigned short TotalSize;
    unsigned short CharCount; // without the trailing NULL
    
    inline bool IsOfTypeAnsiStringLiteral() const;
};

struct AnsiStrLiteralMsg : public LogMsgBase
{
public:
  const char *AnsiText;

  static size_t GetTotalSizeNeeded( size_t /*unused StrLength*/ )
  {
    assert( sizeof(AnsiStrLiteralMsg) == 8 ); // 64 bit boundaries
    return sizeof(AnsiStrLiteralMsg);
  }

public:
  void Fill( const char *Str, unsigned short Length )
  {
    assert( this->IsOfTypeAnsiStringLiteral() );
    assert( Str[Length] == '\0' );
    this->CharCount = Length;
    this->AnsiText = Str;
  }
  
  bool CheckIntegrity() const
  {
    assert( this->IsOfTypeAnsiStringLiteral() );
    assert( this->AnsiText[this->CharCount] == '\0' );
    return true;
  }
};

struct AnsiStrBufferMsg : public LogMsgBase
{
public:
  // we make sure sizeof(AnsiStrBufferMsg) > sizeof(AnsiStrLiteralMsg)
  // => sizeof(AnsiStrBufferMsg) == 2 * 64 bits
  char AnsiText[12]; // first bytes of buffer
  
public:
  static size_t GetTotalSizeNeeded( size_t StrLength )
  {
    size_t minimal = sizeof(LogMsgBase) + StrLength + 1;
    if ( minimal < sizeof(AnsiStrBufferMsg) )
    {
      assert( sizeof(AnsiStrBufferMsg) % 8 == 0 ); // 64 bit boundaries
      return sizeof(AnsiStrBufferMsg);
    }
    // else, align size on 64 bit bounds
    size_t mod_res = minimal % 8;
    if ( mod_res != 0 )
    {
      minimal += ( 8 - mod_res );
    }
    assert( minimal % 8 == 0 ); // 64 bit boundaries
    return static_cast<unsigned short>( minimal );
  }
    
public:
  void Fill( const char *Str, unsigned short Length, bool Truncate = false )
  {
      assert( this->TotalSize == GetTotalSizeNeeded(Length) );
      assert( !this->IsOfTypeAnsiStringLiteral() );

      if ( Length > this->TotalSize - sizeof(LogMsgBase) - 1 )
      {
        // we always do truncation - the Truncate param is here
        // to be sure the caller knows we are doing so
        assert( Truncate );
        Length = this->TotalSize - sizeof(LogMsgBase) - 1;
        Truncate = true;
      }
      else
      {
        assert( !Truncate );
        Truncate = false;
      }
      
      this->CharCount = Length;

      for ( size_t i = 0; i < this->CharCount; ++i )
      {
        this->AnsiText[i] = Str[i];
      }
      this->AnsiText[this->CharCount] = '\0';

      if ( Truncate && this->CharCount > 5 )
      {
        // we modify the text to signal truncation
        this->AnsiText[this->CharCount - 5] = '[';
        this->AnsiText[this->CharCount - 4] = '.';
        this->AnsiText[this->CharCount - 3] = '.';
        this->AnsiText[this->CharCount - 2] = '.';
        this->AnsiText[this->CharCount - 1] = ']';
      }
  }
  
  bool TryToConcatString( const char *Str, size_t Length )
  {
    assert( !this->IsOfTypeAnsiStringLiteral() );

    // compute available space to store chars without including the terminator
    size_t available = this->TotalSize - sizeof *this - 1;
    assert( this->CharCount <= available );
    if ( this->CharCount + Length > available )
    {
      // sorry, there is no free space
      return false;
    }
    // ok, there is enought free space
    for ( size_t i = 0; i < Length; ++i )
    {
      this->AnsiText[this->CharCount + i] = Str[i];
    }
    this->CharCount = static_cast<unsigned short>( this->CharCount + Length );
    this->AnsiText[this->CharCount] = '\0';
    return true;
  }
  
  bool CheckIntegrity() const
  {
    assert( !this->IsOfTypeAnsiStringLiteral() );
    assert( this->TotalSize % 8 == 0 ); // 64 bit boundaries
    assert( this->CharCount < this->TotalSize - sizeof(LogMsgBase) - 1 );
    assert( this->AnsiText[this->CharCount] == '\0' );
    return true;
  }
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