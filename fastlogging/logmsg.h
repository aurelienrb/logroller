#ifndef LOGMSG_H
#define LOGMSG_H

#include <string>

namespace logroller
{
struct logpoint
{
    const char * file;
    const char * line;
    const char * msg;
    const char * type;
};

class logmsg
{
public:
    logmsg( int MsgId, const logpoint * LogPoint );

public:
    int msgId;
    const logpoint * logPoint;
    int nbStrings;
    const char * staticStrings[10];
    std::string dynText;
};
}

#endif