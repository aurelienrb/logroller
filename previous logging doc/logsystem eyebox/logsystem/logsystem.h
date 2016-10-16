#ifndef LOGSYSTEM_H
#define LOGSYSTEM_H

#include "log_message_scope.h"
#include "log_mgr.h"

#define zpriv_logsystem_concat(x,y) x##y
#define zpriv_logsystem_concat_declare_scope(line) zpriv_logsystem_concat( zscope, line )
#define log_trace()\
    log_message_scope zpriv_logsystem_concat_declare_scope(__LINE__)( __FUNCTION__ );\
    zpriv_logsystem_concat_declare_scope(__LINE__) = log_message_builder()

#define log_close_this_thread_logging()\
    log_mgr::destroy_this_thread_mgr()

#endif