#include <iostream>
#include <string>
using namespace std;

#define LOGROLLER_ALLOW_LITERAL_STRING_AS_ARG
#define EXPLICIT_STR_COPY


#ifdef LOGROLLER_ALLOW_LITERAL_STRING_AS_ARG
#define LITERAL_STR_IS_ALLOWED 1
#define LITERAL_STR_ACCESS public
#else
#define LITERAL_STR_IS_ALLOWED 0
#define LITERAL_STR_ACCESS private
#endif

#ifdef EXPLICIT_STR_COPY_REQUIRED
#endif

// is_array_of<T>::enabled
template<typename T>
struct is_array_of
{
    enum { enabled = 0 };
};

template<> struct is_array_of<char> { enum { enabled = 0 }; };
template<> struct is_array_of<const char> { enum { enabled = 0 }; };
template<> struct is_array_of<volatile char> { enum { enabled = 0 }; };
template<> struct is_array_of<volatile const char> { enum { enabled = 0 }; };


class A
{
LITERAL_STR_ACCESS:
    template<size_t N>
    A & operator<<(const char (&)[N])
    {
        static_assert(LITERAL_STR_IS_ALLOWED, "Literal string as argument is not allowed."
            "\nRewrite:\n    LOG(\"time=\") << time << \"ms\";\nto become:\n    LOG(\"time=%1ms\") << time;");
        return *this;
    }

public:
    template<typename T>
    A & operator<<(T *&)
    {
        static_assert(is_array_of<T>::enabled, "Logging an array of elements needs an explicit copy. Use log_copy().");
        return *this;
    }

#if 0
    A & operator<<(char *&)
    {
        return *this;
    }

    A & operator<<(const char *&)
    {
        return *this;
    }
#endif
};

class log_param
{
};

#define COPYREF &&

log_param COPYREF log_copy(const char *, size_t = -1)
{
    return log_param();
}

log_param COPYREF log_copy(const volatile char *, size_t = -1)
{
    return log_param();
}

A & operator<<(A & a, const log_param &)
{
    return a;
}

#define log_info(msg) A a; a << msg

int main(int argc, const char **argv)
{
    const volatile char * c = 0;

    log_info("hello!") << log_copy(argv[argc - 1]) << log_copy(c);


    //cout << "hello" << hex << "world";
}
