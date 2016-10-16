#include "logcpp.h"

namespace priv // internal usage only
{
char* GetAnsiBuffer( LogMsgString *Msg )
{
  assert( !Msg->IsLiteral() );
  assert( !Msg->IsWide );
  
  unsigned char *addr = reinterpret_cast<unsigned char *>( Msg );
  addr += sizeof *Msg;
  return reinterpret_cast<char *>( addr );
}

const char* GetAnsiBuffer( const LogMsgString *Msg )
{
  return GetAnsiBuffer( const_cast<LogMsgString *>( Msg ) );
}

wchar_t* GetWideBuffer( LogMsgString *Msg )
{
  assert( !Msg->IsLiteral() );
  assert( Msg->IsWide );
  
  unsigned char *addr = reinterpret_cast<unsigned char *>( Msg );
  addr += sizeof *Msg;
  return reinterpret_cast<wchar_t *>( addr );
}

const wchar_t* GetWideBuffer( const LogMsgString *Msg )
{
  return GetWideBuffer( const_cast<LogMsgString *>( Msg ) );  
}
}

size_t LogMsgString::GetBufferAnsiCharCapacity()
{
  assert( !Msg->IsLiteral() );
  assert( !Msg->IsWide );

  size_t buff_size = this->TotalSize - sizeof *this;
  return buff_size / sizeof(char);
}

size_t LogMsgString::GetBufferWideCharCapacity()
{
  assert( !Msg->IsLiteral() );
  assert( Msg->IsWide );
  
  size_t buff_size = this->TotalSize - sizeof *this;
  return buff_size / sizeof(wchar_t);
}

void LogMsgString::CopyNonLiteral( const char * AnsiStr, size_t Length )
{
  assert( !this->IsLiteral() );

  this->IsWide = false;
  assert( Length < this->GetBufferAnsiCharCapacity() );
  
  this->Str.AnsiText = priv::GetAnsiBuffer( this );
  
  // do not copy the trailing terminator, because we don't know if it exists...
  memcpy( this->Str.AnsiText, AnsiStr, Length * sizeof(char) );
  this->CharCount = CharCount;
  this->Str.AnsiText[this->CharCount] = '\0';
}

void LogMsgString::CopyNonLiteral( const wchar_t * WideStr, size_t Length )
{
  assert( !this->IsLiteral() );
  
  this->IsWide = true;
  assert( Length < this->GetBufferWideCharCapacity() );
  
  this->Str.WideText = priv::GetWideBuffer( this );
  
  // do not copy the trailing terminator, because we don't know if it exists...
  memcpy( this->Str.WideText, WideStr, Length * sizeof(wchar_t) );
  this->CharCount = CharCount;
  this->Str.WideText[this->CharCount] = L'\0';
}

void LogMsgString::ConcatNonLiteral( const char * AnsiStr, size_t Length )
{
  assert( !this->IsLiteral() );
  assert( !this->IsWide );
  assert( Length < this->GetBufferAnsiCharCapacity() - this->CharCount );
  assert( this->Str.AnsiText == priv::GetAnsiBuffer( this ) );
  
  void *dest = &this->Str.AnsiText[this->CharCount];
  // do not copy the trailing terminator, because we don't know if it exists...
  memcpy( dest, AnsiStr, Length * sizeof(*AnsiStr) );
  this->CharCount += Length;
  this->Str.AnsiText[this->CharCount] = '\0';  
}

void LogMsgString::ConcatNonLiteral( const wchar_t * WideStr, size_t Length )
{
  assert( !this->IsLiteral() );
  assert( this->IsWide );
  assert( Length < this->GetBufferWideCharCapacity() - this->CharCount );
  assert( this->Str.WideText == priv::GetWideBuffer( this ) );
  
  void *dest = &this->Str.WideText[this->CharCount];
  // do not copy the trailing terminator, because we don't know if it exists...
  memcpy( dest, WideStr, Length * sizeof(*WideStr) );
  this->CharCount += Length;
  this->Str.WideText[this->CharCount] = L'\0';  
}

bool LogMsgString::CheckIntegrity() const
{
  assert( this->TotalSize >= sizeof *this );

  if ( this->IsWide )
  {
    if ( !this->IsLiteral() )
    {
      assert( this->Str.WideText == priv::GetWideBuffer( this ) );
      assert( this->CharCount < this->GetBufferWideCharCapacity() );
    }
    assert( this->Str.WideText[ this->CharCount ] == L'\0' );
  }
  else
  {
    if ( !this->IsLiteral() )
    {
      assert( this->Str.AnsiText == priv::GetAnsiBuffer( this ) );
      assert( this->CharCount < this->GetBufferAnsiCharCapacity() );
    }
    assert( this->Str.AnsiText[ this->CharCount ] == '\0' );
  }
  return true;
}

LogMsgEntry* LogMsgBuffer::StartNewActiveEntry(LogMsgEntryInfos *Infos)
{
  LogMsgEntry *entry = this->zBuffer.AllocNewEntry();
  assert( entry->TotalSize = sizeof(*entry) );
  // init
  entry->ThreadId = 0;
  entry->StaticInfos = Infos; // can be NULL
  entry->FirstStr.TotalSize = sizeof(entry->FirstStr);
  
  this->zActiveEntry = entry;
  return this->zActiveEntry;
}

//void EndActiveEntry();

LogMsgString* LogMsgBuffer::AddNewStrToActiveEntry( size_t ExtraSizeInByte )
{
  assert( this->zActiveEntry );
  size_t total_size = sizeof(LogMsgString) + ExtraSizeInByte;
  LogMsgString *msg_str = this->zBuffer.GrowLastEntrySize( total_size );
  if ( msg_str )
  {
      msg_str->TotalSize = total_size;
      this->zLastActiveEntryStr = msg_str;
      return this->zLastActiveEntryStr;
  }
  return 0;
}

LogMsgString* LogMsgBuffer::AddNewLiteralStrToActiveEntry( const char *AnsiStr, size_t Length )
{
  LogMsgString *str = this->AddNewStrToActiveEntry( 0 );
  if ( str )
  {
    str->IsWide = false;
    str->CharCount = Length;
    str->Str.AnsiText = AnsiStr;
    return str;
  }
  return 0;
}

LogMsgString* LogMsgBuffer::AddNewLiteralStrToActiveEntry( const wchar_t *WideStr, size_t Length )
{
  LogMsgString *str = this->AddNewStrToActiveEntry( 0 );
  if ( str )
  {
    str->IsWide = true;
    str->CharCount = Length;
    str->Str.AnsiText = WideStr;
    return str;
  }
  return 0;
}

LogMsgString* LogMsgBuffer::AddNewStrToActiveEntry( const char *AnsiStr, size_t Length )
{
  LogMsgString *str = this->AddNewStrToActiveEntry( (Length + 1) * sizeof(*AnsiStr) );
  if ( str )
  {
    str->IsWide = false;
    str->CopyNonLiteral( AnsiStr, Length );
    return str;
  }
  return 0;
}

LogMsgString* LogMsgBuffer::AddNewStrToActiveEntry( const wchar_t *WideStr, size_t Length )
{
  LogMsgString *str = this->AddNewStrToActiveEntry( (Length + 1) * sizeof(*WideStr) );
  if ( str )
  {
    str->IsWide = true;
    str->CopyNonLiteral( WideStr, Length );
    return str;
  }
  return 0;
}

//LogMsgString* ResizeActiveEntryLastStr( size_t SizeToGrow );

private:
LogMsgEntry *zActiveEntry;
unsigned char * zBufferLastUsedByte;
unsigned char * zBufferLastValidByte;


/*
void LogMgr::ForceNewStringLiteral(const char *Str, size_t Length)
{
  size_t needed_size = sizeof(AnsiStrLiteralMsg);
  LogMsgBase *msg = this->zBuffer.AllocMsg( needed_size );
  if ( msg == 0 )
  {
    // rotate the memory to get more free space
    this->zBuffer.Rotate();
    // should never fail
    msg = this->zBuffer.AllocMsg( needed_size );    
  }
  assert( msg != 0 );
  if ( msg )
  {
    static_cast<AnsiStrLiteralMsg *>( msg )->Fill( Str,
      static_cast<unsigned short>( Length ) );
  }
}

void LogMgr::ForceNewStringBuffer(const char *Str, size_t Length)
{
  size_t needed_size = AnsiStrBufferMsg::GetTotalSizeNeeded( Length );
  LogMsgBase *msg = this->zBuffer.AllocMsg( needed_size );
  bool truncate = false;
  if ( msg == 0 )
  {
    // before to try again, make sure we do not exceed the maximum size of a message
    if ( needed_size > this->zBuffer.GetMaxMsgSize() )
    {
      // Fill() will truncate the Str so the msg fits into the buffer limits 
      needed_size = this->zBuffer.GetMaxMsgSize();
      truncate = true;
    }
    // rotate the memory to get more free space
    this->zBuffer.Rotate();
    // should never fail
    msg = this->zBuffer.AllocMsg( needed_size );
  }
  assert( msg != 0 );
  if ( msg )
  {
    static_cast<AnsiStrBufferMsg *>( msg )->Fill( Str, 
      static_cast<unsigned short>( Length ), truncate );
  }
}

void LogMgr::AutoAppendText( const char *Str, size_t Length )
{
  LogMsgBase *msg = this->zBuffer.GetMoreRecentMsg();
  if ( !msg->IsOfTypeAnsiStringLiteral() )
  {
    AnsiStrBufferMsg *ansi_msg = static_cast<AnsiStrBufferMsg *>( msg );
    if ( ansi_msg->TryToConcatString( Str, Length ) )
    {
      // cool, no more thing to do
      return;
    }
    else
    {
      // we try to expand the msg
      if ( this->zBuffer.ExpandMoreRecentMsgSize( Length ) )
      {
        assert( this->zBuffer.GetMoreRecentMsg() == msg );
        // should never fail
        assert( ansi_msg->TryToConcatString( Str, Length ) );
        return;
      }
      else
      {
        // the only reason we fail should be because the buffer is completely full
        assert( this->zBuffer.GetActualFreeMemorySize() == 0 );
        assert( this->zBuffer.GetMaxReleasableMemorySize() == 0 );
      }
    }
  }
  // we are here because we need to allocate a new AnsiStrBufferMsg
  this->ForceNewStringBuffer( Str, Length );
}

size_t LogMgr::GetFullTextLength() const
{
  const LogMsgBase * msg = this->zBuffer.GetOlderMsg();
  size_t total_size = 0;
  while ( msg )
  {
    total_size += msg->CharCount;
    msg = this->zBuffer.GetNextMsg( msg );
  }
  return total_size;
}

size_t LogMgr::Write( char *Buffer, size_t BufferSize, size_t Offset ) const
{
  const LogMsgBase * msg = this->zBuffer.GetOlderMsg();
  
  size_t current_offset = 0;
  size_t total_written = 0;

  while ( msg )
  {
    size_t suboffset = 0;
    size_t size_to_write = 0;

    if ( current_offset >= Offset )
    {
      size_to_write = BufferSize - 1 - total_written;
      suboffset = 0;
    }
    else if ( current_offset + msg->CharCount >= Offset )
    {
      size_to_write = BufferSize - 1 - total_written;
      suboffset = Offset - current_offset;
    }
    size_to_write = std::min( size_to_write, msg->CharCount );

    for ( size_t i = 0; i < size_to_write; ++i )
    {
      assert( suboffset + i < msg->CharCount );
      Buffer[ total_written ] = msg->AnsiText[ suboffset + i ];
      ++total_written;
    }

    current_offset += msg->CharCount;
    msg = this->zBuffer.GetNextMsg( msg );
  }
  Buffer[ total_written ] = '\0';
  return total_written;
}

LogBuffer::LogBuffer():
  zMoreRecentMsg(0),
  zLastUsedByte(0),
  zOlderChunkFirstByte(0),
  zOlderChunkLastUsedByte(0),
  zBufferSize(0),
  zBufferFirstByte(0),
  zBufferLastValidByte(0)
{
}

void LogBuffer::Attach( unsigned char *RawData, size_t DataSize )
{
  this->zMoreRecentMsg = 0;
  this->zLastUsedByte = 0;
  this->zOlderChunkFirstByte = 0;
  this->zOlderChunkLastUsedByte = 0;
  this->zBufferSize = DataSize;
  this->zBufferFirstByte = RawData;
  this->zBufferLastValidByte = this->zBufferFirstByte + this->zBufferSize - 1;
}

LogMsgBase* LogBuffer::AllocMsg( size_t WantedSize )
{
    if ( WantedSize > this->GetMaxMsgSize() ||
         WantedSize > this->GetCapacity() )
    {
      // error : msg size exceed the maximum allowed / available
      return 0;
    }
    if ( WantedSize <= this->GetActualFreeMemorySize() )
    {
      // ok, there is enought free memory
      LogMsgBase *msg;
      if ( this->zLastUsedByte == 0 )
      {
        // we are allocating the first recent msg
        msg = reinterpret_cast<LogMsgBase *>( this->zBufferFirstByte );
        this->zLastUsedByte = this->zBufferFirstByte + WantedSize - 1;
        this->zMoreRecentMsg = msg;
      }
      else
      {
        // we allocate a new msg after the existing one(s)
        msg = reinterpret_cast<LogMsgBase *>( this->zLastUsedByte + 1 );
        this->zLastUsedByte += WantedSize;
        this->zMoreRecentMsg = msg;
      }      
      msg->TotalSize = static_cast<unsigned short>( WantedSize );
      msg->CharCount = 0;

      assert( this->GetMoreRecentMsg() == msg );
      assert( reinterpret_cast<unsigned char *>( msg )
        + msg->TotalSize - 1 == this->zLastUsedByte );
      return msg;
    }
    // we need to release some memory by erasing old messages
    if ( !this->ReleaseOlderChunkMemory( WantedSize ) )
    {
      return 0;
    }
    // now, it should be ok
    // we do a re-entrant call which must succeed
    LogMsgBase *msg = this->AllocMsg( WantedSize );
    assert( msg );
    return msg;
}

bool LogBuffer::ExpandMoreRecentMsgSize( size_t SizeToGrow )
{ 
  if ( this->zMoreRecentMsg == 0 )
  {
    // you should not call this function if the buffer is empty
    assert( false );
    return false; 
  }
  // first, check we are not asked to exceed the max size
  if ( this->zMoreRecentMsg->TotalSize + SizeToGrow > this->GetMaxMsgSize() )
  {
    // we can't do that
    return false; 
  }
  
  unsigned char * addr_fisrt = reinterpret_cast<unsigned char *>( this->zMoreRecentMsg );
  unsigned char * addr_last = addr_fisrt + this->zMoreRecentMsg->TotalSize - 1;
  // is it a msg from the old msg chunck ?
  if ( addr_last == this->zOlderChunkLastUsedByte )
  {
    // yes, it is an old msg
    assert( this->zLastUsedByte == 0 );
    
    size_t free_size = this->zBufferLastValidByte - addr_last;
    if ( free_size > SizeToGrow )
    {
      this->zOlderChunkLastUsedByte += SizeToGrow;
      this->zMoreRecentMsg->TotalSize = static_cast<unsigned short>(
        this->zMoreRecentMsg->TotalSize + SizeToGrow );
      return true; // success!
    }
    // sorry, you need to allocate a new msg in the recent msg chunck
    return false;
  }
  // our msg comes from the recent msg chunk
  assert( addr_last == this->zLastUsedByte );
  if ( this->GetActualFreeMemorySize() > SizeToGrow )
  {
    // cool, we can easily augment the msg size
    this->zLastUsedByte += SizeToGrow;
    this->zMoreRecentMsg->TotalSize = static_cast<unsigned char>(
      this->zMoreRecentMsg->TotalSize + SizeToGrow );
    return true;
  }
  // we need to release some memory by erasing old messages
  if ( !this->ReleaseOlderChunkMemory( SizeToGrow ) )
  {
    return false;
  }
  // now, it should be ok
  // we do a re-entrant call which must succeed
  bool res = this->ExpandMoreRecentMsgSize( SizeToGrow );
  assert( res );
  return res;  
}

bool LogBuffer::ReleaseOlderChunkMemory( size_t NeededSize )
{
  // we need to release some memory by erasing old messages
  if ( NeededSize > this->GetActualFreeMemorySize() + this->GetMaxReleasableMemorySize() )
  {
    // sorry, this buffer is currently too small
    // try again after a call to Rotate()
    return false;
  }
  assert( NeededSize <= this->zBufferSize );
  while ( NeededSize > this->GetActualFreeMemorySize() )
  {
    assert( this->ReleaseOlderMsgMemory() > 0 );
  }
  return true;
}

size_t LogBuffer::GetMaxMsgSize() const
{
  const size_t max_limit = 65535; // see type of 
  return std::min( max_limit, this->GetCapacity() );
}

size_t LogBuffer::GetActualFreeMemorySize() const
{
  if ( this->zOlderChunkFirstByte == 0 )
  {
    // there is no old data in this buffer, i.e all
    // the bytes after zLastUsedByte are free
    assert( this->zOlderChunkLastUsedByte == 0 );    

    if ( this->zLastUsedByte == 0 )
    {
      // buffer is completely empty!
      assert(this->zOlderChunkFirstByte == 0);
      return this->zBufferSize;
    }
    assert( this->zLastUsedByte >= this->zBufferFirstByte );

    size_t recent_msg_chunk_size = 
      this->zLastUsedByte - this->zBufferFirstByte + 1;
    return this->zBufferSize - recent_msg_chunk_size;
  }
  assert( this->zOlderChunkFirstByte >= this->zBufferFirstByte );
  assert( this->zOlderChunkFirstByte <= this->zBufferLastValidByte );

  // we can not freely use all the first bytes of the buffer
  // because they are still used to store old data
  if ( this->zLastUsedByte == 0 )
  {
    return this->zOlderChunkFirstByte - this->zBufferFirstByte;
  }
  else
  {
    assert( this->zOlderChunkFirstByte > this->zLastUsedByte );
    return this->zOlderChunkFirstByte - this->zLastUsedByte - 1;
  }
}

size_t LogBuffer::GetMaxReleasableMemorySize() const
{
  if ( this->zOlderChunkFirstByte == 0 )
  {
    // there is no old data in this buffer, i.e nothing to release...
    return 0;
  }
  assert( this->zOlderChunkFirstByte >= this->zBufferFirstByte );
  assert( this->zOlderChunkFirstByte <= this->zBufferLastValidByte );

  return this->zBufferLastValidByte - this->zOlderChunkFirstByte + 1;
}

size_t LogBuffer::ReleaseOlderMsgMemory()
{
  if ( this->zOlderChunkFirstByte == 0 )
  {
    // there is no old data to release...
    return 0;
  }
  assert( this->zOlderChunkFirstByte >= this->zBufferFirstByte );
  assert( this->zOlderChunkLastUsedByte >= this->zOlderChunkFirstByte );
  assert( this->zOlderChunkLastUsedByte <= this->zBufferLastValidByte );

  LogMsgBase *msg = reinterpret_cast<LogMsgBase *>( this->zOlderChunkFirstByte );
  size_t msg_size = msg->TotalSize;

  unsigned char *new_first_byte = this->zOlderChunkFirstByte + msg_size;

  if ( new_first_byte > this->zOlderChunkLastUsedByte )
  {
    assert( this->zOlderChunkLastUsedByte == new_first_byte - 1 );
    // we released the last old msg!
    new_first_byte = 0;
    this->zOlderChunkLastUsedByte = 0;
  }
  this->zOlderChunkFirstByte = new_first_byte;
  return msg_size;
}

void LogBuffer::Rotate()
{
  if ( this->zLastUsedByte == 0 )
  {
    return;
  }
  this->zOlderChunkFirstByte = this->zBufferFirstByte;
  this->zOlderChunkLastUsedByte = this->zLastUsedByte;
  this->zLastUsedByte = 0;
  // zMoreRecentMsg remains the same
}

LogMsgBase * LogBuffer::GetMoreRecentMsg()
{
  // we do some const casts so we don't duplicate the code
  const LogMsgBase * msg = const_cast<const LogBuffer *>( this )->GetMoreRecentMsg();
  return const_cast<LogMsgBase *>( msg );
}

const LogMsgBase * LogBuffer::GetMoreRecentMsg() const
{
#if 0
  const LogMsgBase * msg = 0;
  // first, we look for a msg in the recent msg chunck
  if ( this->zLastUsedByte != 0 )
  {
    msg = reinterpret_cast<const LogMsgBase *>( this->zBufferFirstByte );
  }  
  // then, we look for a msg in the old msg chunck
  else if ( this->zOlderChunkFirstByte != 0 )
  {
    assert( this->zOlderChunkLastUsedByte != 0 );
    
    msg = reinterpret_cast<const LogMsgBase *>( this->zOlderChunkFirstByte );
  }
  else
  {
    // buffer is completely empty
    return 0;    
  }
  
  // now we get a message, we look for its successors until we get the last one
  const LogMsgBase * next = msg;
  do
  {
    msg = next;
    next = this->GetNextMsg( msg );
  } while ( next != 0 );
  return msg;  
#endif
  if ( this->zMoreRecentMsg == 0 )
  {
    // buffer should be empty
    assert( this->zLastUsedByte == 0 );
    assert( this->zOlderChunkFirstByte == 0 );
    assert( this->zOlderChunkLastUsedByte == 0 ); 
  }
  else
  {
    const unsigned char *last_byte =
      reinterpret_cast<unsigned char *>( this->zMoreRecentMsg ) 
        + this->zMoreRecentMsg->TotalSize - 1;
    assert( last_byte == this->zLastUsedByte
      || last_byte == this->zOlderChunkLastUsedByte );
  }
  return this->zMoreRecentMsg;
}

const LogMsgBase * LogBuffer::GetOlderMsg() const
{
  // first, we look for a msg in the old msg chunck
  if ( this->zOlderChunkFirstByte != 0 )
  {
    return reinterpret_cast<const LogMsgBase *>( this->zOlderChunkFirstByte );
  }
  assert( this->zOlderChunkLastUsedByte == 0 );

  // then we look for a msg in the recent msg chunck
  if ( this->zLastUsedByte != 0 )
  {
    return reinterpret_cast<const LogMsgBase *>( this->zBufferFirstByte );
  }
  // buffer is completely empty
  return 0;
}

const LogMsgBase * LogBuffer::GetNextMsg(const LogMsgBase *Msg) const
{
  assert( Msg );
  if ( Msg == 0 )
  {
    return 0;
  }

  const unsigned char *msg_addr = reinterpret_cast<const unsigned char *>( Msg );
  assert( msg_addr >= this->zBufferFirstByte );
  assert( msg_addr <= this->zBufferLastValidByte );

  // optimization
  if ( Msg == this->zMoreRecentMsg )
  {
    // we know there is no more msg...
    const unsigned char *last_byte = msg_addr + Msg->TotalSize - 1;
    assert( last_byte == this->zLastUsedByte
      || last_byte == this->zOlderChunkLastUsedByte );
    return 0;
  }
  
  // first, we test if it is a msg from the old msg chunck
  if ( this->zOlderChunkFirstByte != 0 &&
       msg_addr >= this->zOlderChunkFirstByte )
  {    
      // yes, it is
      assert( this->zOlderChunkLastUsedByte >= this->zOlderChunkFirstByte );
      assert( this->zOlderChunkLastUsedByte <= this->zBufferLastValidByte );

      const unsigned char *next_old_msg = msg_addr + Msg->TotalSize;

      // test if it is not the last one
      if ( next_old_msg >= this->zOlderChunkLastUsedByte )
      {
        // yes, it is the last one...
        assert( this->zOlderChunkLastUsedByte == next_old_msg - 1 );

        // so we return the first msg of the recent msg chunck, if it exists
        if ( this->zLastUsedByte == 0 )
        {
          // no more msg
          return 0;
        }
        else
        {
          assert( this->zLastUsedByte >= this->zBufferFirstByte );
          return reinterpret_cast<const LogMsgBase *>( this->zBufferFirstByte );
        }
      }
      else
      {
        // ok, we found one more msg in the old msg chunck
        return reinterpret_cast<const LogMsgBase *>( next_old_msg );
      }
  }

  // the given Msg is not a msg from the old msg chunck
  const unsigned char *next_msg = msg_addr + Msg->TotalSize;
  assert( next_msg <= this->zLastUsedByte );
  return reinterpret_cast<const LogMsgBase *>( next_msg );
}
*/