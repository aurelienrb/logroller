#ifndef LOG_MESSAGE_BUILDER_H
#define LOG_MESSAGE_BUILDER_H

#include <sstream>

class log_message_builder
{
public:
    template<size_t N>
    inline log_message_builder & operator << ( const char (&Str)[N] )
    {
        this->oss << Str;
        return *this;
    }

    std::string get_message() const
    {
        return oss.str();
    }

private:
    std::ostringstream oss;
};

#endif