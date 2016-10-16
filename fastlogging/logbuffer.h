#ifndef LOGBUFFER_H
#define LOGBUFFER_H

#include "logmsg.h"
#include <boost/circular_buffer.hpp>

namespace logroller
{
class logbuffer
{
public:
    logbuffer();

    logmsg* alloc( const logpoint * );

private:
    int msgCount;
    boost::circular_buffer<logmsg> buffer;
};
}

#endif