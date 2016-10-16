#include "logbuffer.h"

namespace logroller
{
logbuffer::logbuffer() : buffer( 100 ), msgCount( 0 )
{
}

logmsg* logbuffer::alloc( const logpoint * logPoint )
{
    this->buffer.push_back(
        logmsg( this->msgCount, logPoint ) );
    ++this->msgCount;
    return &this->buffer.back();
}
}
