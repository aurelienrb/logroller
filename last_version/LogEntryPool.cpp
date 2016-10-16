#include "LogEntryPool.h"

#define assert_completely_empty( pool )\
assert( this->GetHead() == 0 );\
assert( this->zLastUsedByte == 0 );\
assert( this->zOldDataFirstByte == 0 );\
assert( this->zOldDataLastUsedByte == 0 );
        
#define SHOULD_NEVER_HAPPEN(x) x

LogEntryPool::LogEntryPool()
{
  this->AttachBuffer( 0, 0 );
}

void LogEntryPool::AttachBuffer( unsigned char * Buffer, size_t BufferSize )
{
  assert( Buffer && BufferSize );
  
  // TODO: ensure zBufferFirstByte is correctly aligned
  this->zBufferFirstByte = Buffer;
  this->zBufferSize = BufferSize;
  if ( this->zBufferFirstByte && this->zBufferSize > 0 )
  {
      this->zBufferLastValidByte = this->zBufferFirstByte + this->zBufferSize - 1;
  }
  else
  {
      this->zBufferLastValidByte = 0; 
  }
  this->zNewDataLastUsedByte = 0;
  this->zOldDataFirstByte = 0;
  this->zOldDataLastUsedByte = 0; 
}


const LogEntry * LogEntryPool::GetHead() const
{
  return 0;
}

const LogEntry * LogEntryPool::GetNext( const LogEntry * Entry ) const
{
  if ( !Entry )
  {
    SHOULD_NEVER_HAPPEN(return 0); 
  }
  return 0; 
}

LogEntry* LogEntryPool::GetTail()
{
  return 0;
}

LogEntry* LogEntryPool::AllocNewTail()
{
  LogEntry *tail = this->GetTail();
  size_t size_needed = sizeof(*tail);
  if ( !tail )
  {
    // no tail, which should mean the buffer is completely empty
    assert( this->IsEmpty() );
    // we simply create a new one, which also becomes the new head    
    if ( this->GetFreeMemorySize() >= size_needed )
    {
      this->zLastUsedByte = this->zBufferFirstByte + size_needed - 1;
      return reinterpret_cast<LogEntry*>( this->zBufferFirstByte );
    }
    else
    {
      return 0; // sorry, buffer is too small, no way to solve this 
    }
  }
  // test if the existing tail is in the recent data chunck
  if ( this->zLastUsedByte == 0 )
  {
    // there is no recent data, the actual tail should be in the old data chunck
    assert( IsInOldDataChunck( tail ) );
    
    // so we create the tail at the top of the recent data chunck
    if ( this->GetFreeMemorySize() >= size_needed )
    {
      this->zLastUsedByte = this->zBufferFirstByte + size_needed - 1;
      return reinterpret_cast<LogEntry*>( this->zBufferFirstByte );
    }
    else
    {
      if ( size_needed > this->GetFreeMemorySize() )
      {
        // sorry, buffer is too small, no way to solve this
        return 0;
      }
      assert( size_needed <= this->GetFreeMemorySize() + this->GetMaxReleasableMemorySize() );
      // we need to free some data from the old data chunck
      // that's exactly what the following code does
    }
  }
  // we are here to allocate a tail in the recent data chunck
  // the new tail can or not be the first entry in the recent data chunck,
  // depending on the value of zLastUsedByte
  if ( size_needed > this->GetFreeMemorySize() + this->GetMaxReleasableMemorySize() )
  {
    // not enought memory, try to call RotateBuffer()
    return 0;
  }
  while ( size_needed > this->GetFreeMemorySize() )
  {
    assert( this->AdvanceHeadToNextEntry() );
  }
  // here, we are sure to have enought free memory for our new tail
  if ( this->zLastUsedByte == 0 )
  {
    // the tail is the first entry in the recent data chunck
    this->zLastUsedByte = this->zBufferFirstByte - 1;
  }
  // ok, update our use of the memory in the recent data chunck
  tail = reinterpret_cast<LogEntry*>( this->zLastUsedByte + 1 );
  this->zLastUsedByte += size_needed;
  tail->TotalSize = size_needed;
  return tail; 
}

// BlackBox Test:
// - Attach
// - assert IsEmpty
// - tail = AllocNewTail
// - assert tail == GetTail
// - assert !IsEmpty
// - assert GetHead == GetTail

bool LogEntryPool::ResizeTail( size_t SizeToGrow )
{
  // the tail must be in the recent data chunck because
  // we do not allow the modification of old data
  assert( this->zLastUsedByte != 0 );
  
  LogEntry *tail = this->GetTail();
  if ( !IsInRecentDataChunck( tail ) )
  {
    // this call to ResizeTail() is a logic error
    SHOULD_NEVER_HAPPEN(return false);
  }   
  
  if ( SizeToGrow > this->GetFreeMemorySize() + this->GetMaxReleasableMemorySize() )
  {
    // not enought memory, try to call RotateBuffer()
    return false;
  }
  
  
  
}

// Test:
// - AllocNewTail()
// - RotateBuffer()
// Assertion::ThrowExceptionOnFailure();
// bool ok = false
// try {
//   - ResizeTail( 1 );
// } catch { const AssertionFailure & ) {
//   - ok = true
// }
// assert( ok )

void LogEntryPool::RotateBuffer();

LogEntry* LogEntryPool::MoveTailToTheTopOfTheBuffer();