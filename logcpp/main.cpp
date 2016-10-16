#include "logcpp.h"

#include <iostream>
#include <string>
#include <stack>
#include <vector>
using namespace std;

struct AnsiStr
{
  const char *Str;
  unsigned short Length;
}; 

template<typename T>
void TestAnsiStr(unsigned char *RawData, size_t NbStr, const AnsiStr *StrTable, size_t MaxNbStrInBuffer)
{
  assert( NbStr >= 2 );
  assert( MaxNbStrInBuffer >= 1 );
  assert( MaxNbStrInBuffer <= NbStr );
  
  // set buffer size too small to store all the strings
  size_t buff_size = 0;
  for ( size_t i = 0; i < MaxNbStrInBuffer; ++i )
  {
    buff_size += T::GetTotalSizeNeeded( StrTable[i].Length );
  }

  LogBuffer buff;
  buff.Attach( RawData, buff_size );
  assert( buff.GetMoreRecentMsg() == 0 );
  assert( buff.GetOlderMsg() == 0 );
  
  size_t next_msg_to_store = 0;
  while ( next_msg_to_store < MaxNbStrInBuffer )
  {
    const AnsiStr & s = StrTable[next_msg_to_store];
    T *msg = static_cast<T*>( buff.AllocMsg( T::GetTotalSizeNeeded( s.Length ) ) );
    assert( msg );
    msg->Fill( s.Str, s.Length );
    assert( msg->CheckIntegrity() );
    ++next_msg_to_store;
  }
  
  // buffer should be full
  if ( MaxNbStrInBuffer < NbStr )
  {
    const AnsiStr & s = StrTable[next_msg_to_store];
    
    T *msg = static_cast<T *>( 
      buff.AllocMsg( T::GetTotalSizeNeeded( s.Length ) ) );
    assert( msg == 0 );
    
    // ok, we reset it
    buff.Rotate();
    assert( buff.GetActualFreeMemorySize() == 0 );
    assert( buff.GetMaxReleasableMemorySize() == buff.GetCapacity() );
    
    // now, the alloc should be ok
    msg = static_cast<T *>(
      buff.AllocMsg( T::GetTotalSizeNeeded( s.Length )  ) );
    assert( msg );
    // we should reuse the msg 0
    assert( string(msg->AnsiText) == StrTable[0].Str );
    // overwrite
    msg->Fill( s.Str, s.Length );
    assert( msg->CheckIntegrity() );
    ++next_msg_to_store;
    
    // additional checks...
    assert( buff.GetMoreRecentMsg() == msg );
    assert( buff.GetNextMsg(msg) == 0 );
  }
  
  //cout << "filling...\n";
  // we continue to use the buffer to store all the remaining messages
  while ( next_msg_to_store < NbStr )
  {
    //cout << "trying to store msg number " << next_msg_to_store << "\n";
    const AnsiStr & s = StrTable[next_msg_to_store];
    T *msg = static_cast<T *>(
      buff.AllocMsg( T::GetTotalSizeNeeded( s.Length ) ) );    
    if ( msg == 0 )
    {
      size_t needed_size = T::GetTotalSizeNeeded( s.Length );
      //cout << buff.GetActualFreeMemorySize() << "  " << buff.GetMaxReleasableMemorySize() << "\n";
      assert( buff.GetActualFreeMemorySize() + buff.GetMaxReleasableMemorySize() < needed_size );
      // buffer is full, do a rotation to free space
      buff.Rotate();
      assert( buff.GetActualFreeMemorySize() == 0 );
      assert( buff.GetMaxReleasableMemorySize() == buff.GetCapacity() );
      if ( buff.GetCapacity() < needed_size )
      {
        //cout << "buffer is too small\n";
        break;
      }
      continue;
    }
    //cout << "storing " << s.Str << '\n';
    msg->Fill( s.Str, s.Length );
    assert( msg->CheckIntegrity() );
    ++next_msg_to_store;
  }
  
  // now we dump all the stored message
  {
    //cout << "dumping...\n";
    const LogMsgBase * msg = buff.GetOlderMsg();
    const LogMsgBase * last_valid_msg = msg;
    stack<string> msg_dump;
    while ( msg )
    {      
      last_valid_msg = msg;
      const T *ansi_msg = static_cast<const T *>( msg );
      assert( ansi_msg->CheckIntegrity() );
      
      msg_dump.push( string( ansi_msg->AnsiText, ansi_msg->CharCount ) );
      
      msg = buff.GetNextMsg( msg );
      assert( msg_dump.size() <= MaxNbStrInBuffer );
    }
    assert( last_valid_msg == buff.GetMoreRecentMsg() );
    // Note: the number of msg stored may be < to MaxNbStrInBuffer
    // because of their variable length
    assert( msg_dump.size() <= MaxNbStrInBuffer );
    //cout << "dump completed\n";
    // we compare the dumped msg with the expected ones
    size_t expected_num = next_msg_to_store - 1;
    while ( !msg_dump.empty() )
    {
      const AnsiStr & expected_msg = StrTable[expected_num];
      //cout << msg_dump.top() << ", " << expected_msg.Str << '\n';
      assert( msg_dump.top() == string(expected_msg.Str, expected_msg.Length) );      
      msg_dump.pop();
      --expected_num;
    }    
  }
}

const size_t AnsiStrTableSize = 110;
const AnsiStr AnsiStrTable[AnsiStrTableSize] = {
  { "0", 1 }, { "1", 1 }, { "2", 1 }, { "3", 1 }, { "4", 1 },  { "5", 1 }, { "6", 1 }, { "7", 1 }, { "8", 1 }, { "9", 1 },
  { "10", 2 }, { "11", 2 }, { "12", 2 }, { "13", 2 }, { "14", 2 },  { "15", 2 }, { "16", 2 }, { "17", 2 }, { "18", 2 }, { "19", 2 },
  { "20", 2 }, { "21", 2 }, { "22", 2 }, { "23", 2 }, { "24", 2 },  { "25", 2 }, { "26", 2 }, { "27", 2 }, { "28", 2 }, { "29", 2 },
  { "30", 2 }, { "31", 2 }, { "32", 2 }, { "33", 2 }, { "34", 2 },  { "35", 2 }, { "36", 2 }, { "37", 2 }, { "38", 2 }, { "39", 2 },
  { "40", 2 }, { "41", 2 }, { "42", 2 }, { "43", 2 }, { "44", 2 },  { "45", 2 }, { "46", 2 }, { "47", 2 }, { "48", 2 }, { "49", 2 },
  { "50", 2 }, { "51", 2 }, { "52", 2 }, { "53", 2 }, { "54", 2 },  { "55", 2 }, { "56", 2 }, { "57", 2 }, { "58", 2 }, { "59", 2 },
  { "60", 2 }, { "61", 2 }, { "62", 2 }, { "63", 2 }, { "64", 2 },  { "65", 2 }, { "66", 2 }, { "67", 2 }, { "68", 2 }, { "69", 2 },
  { "70", 2 }, { "71", 2 }, { "72", 2 }, { "73", 2 }, { "74", 2 },  { "75", 2 }, { "76", 2 }, { "77", 2 }, { "78", 2 }, { "79", 2 },
  { "80", 2 }, { "81", 2 }, { "82", 2 }, { "83", 2 }, { "84", 2 },  { "85", 2 }, { "86", 2 }, { "87", 2 }, { "88", 2 }, { "89", 2 },
  { "90", 2 }, { "91", 2 }, { "92", 2 }, { "93", 2 }, { "94", 2 },  { "95", 2 }, { "96", 2 }, { "97", 2 }, { "98", 2 }, { "99", 2 },
  { "100", 3 }, { "101", 3 }, { "102", 3 }, { "103", 3 }, { "104", 3 },  { "105", 3 }, { "106", 3 }, { "107", 3 }, { "108", 3 }, { "109", 3 },
};

void DumpLogMgrAndCompareWithExpected( const LogMgr & Log, const std::string & Expected )
{
  size_t fullsize = Log.GetFullTextLength();
  assert( fullsize == Expected.size() );

  std::vector<char> logdata( fullsize + 1 );
  assert( Log.Write( &logdata[0], logdata.size() ) == logdata.size() );
  assert( Expected == &logdata[0] );
}

void TestLogMgr()
{
  std::vector<unsigned char> data( 10000 );
  unsigned char *raw_data = &data[0];

  for ( size_t buff_size = 3; buff_size < 500; buff_size += 3 )
  {
    LogMgr logmgr;
    logmgr.Init( raw_data, buff_size );

    // keep a track of all logged messages
    std::string output;

    output.reserve( 1000 );
    for ( size_t i = 0; i < AnsiStrTableSize; ++i )
    {
      const AnsiStr & s = AnsiStrTable[i];
      logmgr.ForceNewStringBuffer( s.Str, s.Length );
      output += s.Str;
    }
    logmgr.CheckIntegrity();
    DumpLogMgrAndCompareWithExpected( logmgr, output );

    output.clear();
    output.reserve( 1000 );
    for ( size_t i = 0; i < AnsiStrTableSize; ++i )
    {
      const AnsiStr & s = AnsiStrTable[i];
      logmgr.ForceNewStringLiteral( s.Str, s.Length );
      output += s.Str;
    }
    logmgr.CheckIntegrity();
    DumpLogMgrAndCompareWithExpected( logmgr, output );

    output.clear();
    output.reserve( 1000 );
    for ( size_t i = 0; i < AnsiStrTableSize; ++i )
    {
      const AnsiStr & s = AnsiStrTable[i];
      logmgr.AutoAppendText( s.Str, s.Length );
      output += s.Str;
    }
    logmgr.CheckIntegrity();
    DumpLogMgrAndCompareWithExpected( logmgr, output );

    output.clear();
    output.reserve( 1000 );
    for ( size_t i = 0; i < AnsiStrTableSize; ++i )
    {
      const AnsiStr & s = AnsiStrTable[i];

      int pseudo_random = rand() % 10 + 1;
      if ( pseudo_random < 3 )
      {
        logmgr.AutoAppendText( s.Str, s.Length );
        output += s.Str;
      }
      else if ( pseudo_random < 6 )
      {
        logmgr.ForceNewStringLiteral( s.Str, s.Length );
        output += s.Str;
      }
      else
      {
        logmgr.AutoAppendText( "\n", 1 );
        output += "\n";
      }
    }
    logmgr.CheckIntegrity();
    DumpLogMgrAndCompareWithExpected( logmgr, output );
  }
}

void TestLogBuffer()
{
  std::vector<unsigned char> data( 10000 );
  unsigned char *raw_data = &data[0];
  for ( size_t nb_msg = 2; nb_msg <= AnsiStrTableSize; ++nb_msg )
  {
    for ( size_t max_nb = 1; max_nb <= nb_msg; ++max_nb )
    {
      //cout << "testing " << max_nb << " / " << nb_msg << "\n";
      TestAnsiStr<AnsiStrLiteralMsg>(raw_data, nb_msg, AnsiStrTable, max_nb);
      //cout << "Ok\n";
      TestAnsiStr<AnsiStrBufferMsg>(raw_data, nb_msg, AnsiStrTable, max_nb);
    }
  }
  cout << "LogBuffer testing is successful\n";
}

int main()
{
  
  // test adding one char :
  // - after a Rotate
  // - after a Rotate on a full buffer
  // - on a completely full buffer
  // - on a buffer with no old msg
  // test truncate -> specify max msg limit

}