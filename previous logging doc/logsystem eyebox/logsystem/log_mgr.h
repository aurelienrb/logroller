#ifndef LOG_MGR_H
#define LOG_MGR_H

// include volontairement dans le .h
#include "log_thread_message_mgr.h"

class LOGSYSTEM_EXPORT log_mgr
{
public:
    static log_thread_message_mgr* get_this_thread_mgr();
    static void destroy_this_thread_mgr();
    static void enable_bug_report();
    static void test();
};

#endif