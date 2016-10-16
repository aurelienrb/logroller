#include "logmsg.h"

namespace logroller
{
logmsg::logmsg( int MsgId, const logpoint * LogPoint ):
    msgId( MsgId ),
    logPoint( LogPoint ),
    nbStrings( 0 )
{
}
}