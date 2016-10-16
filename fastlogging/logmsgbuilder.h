#ifndef LOGMSGBUILDER_H
#define LOGMSGBUILDER_H

#include "logmsg.h"
#include <sstream>

namespace logroller
{
class logmsgbuilder : public std::ostringstream
{
public:
    logmsgbuilder( logmsg *Msg ) : msg( Msg )
    {
    }

    ~logmsgbuilder();

    template<size_t N>
    logmsgbuilder & operator<<( const char(&Text)[N] )
    {
        this->msg->staticStrings[ this->msg->nbStrings ] = Text;
        *static_cast<std::ostringstream*>( this )
            << '%' << this->msg->nbStrings + 1 << '%';
        ++this->msg->nbStrings;
        return *this;
    }

    template<typename T>
    logmsgbuilder & operator<<( T Text )
    {
        *static_cast<std::ostringstream*>( this ) << Text;
        return *this;
    }

private:
    logmsg * msg;
};
}

#endif