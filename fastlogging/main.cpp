#include "logroller.h"

inline void enforce_string_litteral(char *)
{
}

template<size_t N>
inline void enforce_string_litteral(char(&Str)[N])
{
}

template<size_t N>
inline const char* enforce_string_litteral(const char(&Str)[N])
{
    return Str;
}


int main()
{
    LOG_TRACE( "Hello!" );
    LOG_ERROR( "Hello!" );

    std::string s = " - tmp";
    log_trace() << "Hello" << " World! - " << 3 << s.c_str() << s.assign( " - tmp2" );
}