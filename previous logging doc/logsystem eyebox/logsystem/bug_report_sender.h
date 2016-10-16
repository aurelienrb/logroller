#ifndef BUG_REPORT_SENDER_H
#define BUG_REPORT_SENDER_H

#include <string>

class log_message_pool;

class bug_report_sender
{
public:
    bool init( const std::string & MailAddress );

    void send_report( const log_message_pool *, const char *report_title, int nb_msg_to_report = -1 );
};

#endif