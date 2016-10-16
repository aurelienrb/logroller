#include <iostream>
#include <string>
using namespace std;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

ptrdiff_t vaddr(const void * addr)
{
	static void * base = ::GetModuleHandle(NULL);
	return LPBYTE(addr) - LPBYTE(base);
}

#ifdef _MSC_VER
#pragma section(".logroll", read)
#define PLACE_IN_LOGROLLER_SECTION(DECL) __declspec(allocate(".logroll")) DECL
#elif _GCC
#define PLACE_IN_LOGROLLER_SECTION(DECL) DECL __attribute__((section(".logroll, \"r\"#")))
#else
#error Compiler not supported
#endif

struct log_info_t
{
    const char * fileName;
    unsigned int lineNumber;
    const char * functionName;
    const char * logMessage;
};

struct log_info_data_t
{
    char fileName[256];
    unsigned int lineNumber;
    char functionName[128];
    char logMessage[256];
};

void f(const char *n)
{ 
    cout << n << "\n";
}

std::ostream & print_log_info(const char *log_info)
{
    size_t offset = 0;
    string logMessage = &log_info[offset];
    
    offset += logMessage.size() + 1; 
    string fileName = &log_info[offset];
    
    offset += fileName.size() + 1; 
    string functionName = &log_info[offset];
    
    cout << "(" << fileName << ") ["
        << functionName << "] "
        << logMessage;
    return cout; 
}

#define log_info(msg)\
        if (bool logged = false) {}\
        else for (PLACE_IN_LOGROLLER_SECTION(static const char log_info[]) =\
                      msg "\0" __FILE__ "\0" __FUNCTION__;\
                  logged == false;\
                  logged = true)\
                  print_log_info(log_info)

void test_macro(int n)
{
    print_log_info("test\n" "\0" __FILE__ "\0" __FUNCTION__);

    if (n == 0)
        log_info("n est nul\n");
    else
        log_info("n == ") << n << "\n";

    for (int i = 0; i < n; ++i)
        log_info("* i == ") << i << "\n";
}

int main(int argc, char **argv)
{ 
    test_macro(0);
    test_macro(1);
    test_macro(2);
}
#if 0
int main()
{
    for (static const char log_info[] = "Hello" "\0" __FILE__ "\0" __FUNCTION__;
         ;
         )
         print_log_info(log_info);



    static const log_info_t log_info = {
        __FILE__,
        __LINE__,
        __FUNCTION__,
        "Hello World!"
    };


    PLACE_IN_LOGROLLER_SECTION(static const log_info_t * const log_info_ptr) = &log_info;

	cout << "Virtual addr of log_info_ptr: 0x" << hex << vaddr(&log_info_ptr) << endl;
	cout << "Virtual addr of log_info1: 0x" << vaddr(&log_info) << endl;
	cout << "Virtual addr of log_info1.fileName: 0x" << vaddr(log_info.fileName) << endl;

	PLACE_IN_LOGROLLER_SECTION(static const log_info_data_t log_info2) = {
        __FILE__,
        __LINE__,
        __FUNCTION__,
        "Hello World!"
    };
	cout << "Virtual addr of log_info2: 0x" << vaddr(&log_info2) << endl;
	cout << "Virtual addr of log_info2.fileName: 0x" << vaddr(log_info2.fileName) << endl;
}
#endif
