#ifndef log_message_h
#define log_message_h

#include <core_export.h>
#include <string>

enum MessageType
{
    ENTER_FUNCTION,
    LEAVE_FUNCTION,
    TRACE,
    CRASH,
};

#define MAX_LOG_MESSAGE_LENGTH 1024

class LOGSYSTEM_EXPORT log_message
{
public:
    MessageType     type;
    size_t          level;

    void set_message( const std::string & Msg );
    void set_message( const char *Msg, size_t Length = 0 );
    const char* get_message() const;
    size_t get_message_length() const;

private:
    static void *operator new (size_t n);

private:
    size_t          message_length;
    char            message[ MAX_LOG_MESSAGE_LENGTH + 1 ];
};

#endif