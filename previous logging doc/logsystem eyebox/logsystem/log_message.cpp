#ifndef _SCL_SECURE_NO_DEPRECATE
// warning C4996: 'std::_Copy_opt' a été déclaré désapprouvé
#pragma warning( disable : 4996 )
#endif

#include "log_message.h"

#include <algorithm>

void log_message::set_message( const std::string & Msg )
{
    this->set_message( Msg.c_str(), Msg.length() );
}

void log_message::set_message( const char *Msg, size_t Length )
{
    if ( Length == 0 )
    {
        Length = strlen( Msg );
    }
    size_t len = std::min<size_t>( Length, MAX_LOG_MESSAGE_LENGTH );
    std::copy( Msg, Msg + len, this->message );
    this->message_length = len;
    this->message[ this->message_length ] = '\0';
}

const char* log_message::get_message() const
{
    return this->message;
}

size_t log_message::get_message_length() const
{
    return this->message_length;
}

