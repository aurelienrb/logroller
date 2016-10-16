#ifndef LOG_THREAD_MESSAGE_MGR_H
#define LOG_THREAD_MESSAGE_MGR_H

#include "log_message.h"

#include <string>

class log_message_pool;

class LOGSYSTEM_EXPORT log_thread_message_mgr
{
public:
    log_thread_message_mgr();
    ~log_thread_message_mgr();

    void enter_function( const char * function_name, const std::string & Msg );

    void leave_function( const char * function_name );

    std::string dump_html();

    log_message_pool * get_pool();
    const log_message_pool * get_pool() const;

private:
    log_thread_message_mgr( const log_thread_message_mgr & ) {};
    void operator=( const log_thread_message_mgr & ) {};

private:
    class pimpl;
    pimpl *p_;
};

#endif