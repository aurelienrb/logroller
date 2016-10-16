#include <iostream>

using namespace std;

struct log_point
{
    const char * file;
    const char * line;
    const char * msg;
};

void enter( const log_point & P )
{
    std::cout << P.line << ": " << P.msg << '\n';
}

struct log_point2
{
    const char * file;
    const int line;
    const char * msg;
};

void enter( const log_point2 & P )
{
    std::cout << P.line << ": " << P.msg << '\n';
}

#define CONCAT(x, y) x##y

#define _STR(x) #x
#define STR(x) _STR(x)

#define _LOG(msg,line) static const log_point CONCAT(logpt, line) = { __FILE__, STR(__LINE__), msg }; enter(CONCAT(logpt, line))
#define LOG(msg) _LOG(msg, __LINE__)

#define _LOG2(msg,line) static const log_point2 CONCAT(logpt, line) = { __FILE__, line, msg }; enter(CONCAT(logpt, line))
#define LOG2(msg) _LOG2(msg, __LINE__)

int main()
{
    //LOG( "Hello!" );
    //LOG2( "Hello!" );
    std::string s = "msg1";
    cout << s << s.assign( " msg2" );
}
