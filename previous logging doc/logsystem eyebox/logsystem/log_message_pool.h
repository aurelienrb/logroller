#ifndef LOG_MESSAGE_POOL_H
#define LOG_MESSAGE_POOL_H

#include "log_message.h"

#include <vector>

class log_message_pool
{
public:
    log_message_pool();

    void set_pool_size_in_kb( size_t );
    void set_pool_size_in_number_of_msg( size_t );

    log_message* alloc();
    size_t get_number_of_msg() const;
    const log_message* get_first() const;
    const log_message* get_last() const;
    const log_message* get_next(const log_message*) const;
    const log_message* get_previous(const log_message*) const;

private:
    std::vector<log_message>    pool;
    size_t                      first_msg_num;
    size_t                      msg_nb;
};

#endif