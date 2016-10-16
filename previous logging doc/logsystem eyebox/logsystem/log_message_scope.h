#ifndef LOG_MESSAGE_SCOPE_H
#define LOG_MESSAGE_SCOPE_H

#include "log_mgr.h"
#include "log_message_builder.h"

#include <cassert>

class log_message_scope
{
public:
    template<size_t N>
    log_message_scope( const char(&Fn)[N] ) : fn( Fn ), msg_logged( false )
    {
    }

    void operator=( const log_message_builder & Msg )
    {
        assert( !msg_logged );
        log_mgr::get_this_thread_mgr()->enter_function( this->fn, Msg.get_message() );
        msg_logged = true;
    }

    ~log_message_scope()
    {
        assert( msg_logged );
        log_mgr::get_this_thread_mgr()->leave_function( this->fn );
    }

private:
    const char *fn;
    bool msg_logged;
};

#endif