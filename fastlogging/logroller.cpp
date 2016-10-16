#include "logroller.h"
#include "logbuffer.h"
#include <iostream>
using namespace std;

logroller::logbuffer buffer;

namespace logroller
{
void pass( const logpoint & P )
{
    cout << P.type << " (" << P.line << "): " << P.msg << '\n';
}

logmsgbuilder log( const logpoint * logpt )
{
    return logmsgbuilder(
        buffer.alloc( logpt ) );
}
}
