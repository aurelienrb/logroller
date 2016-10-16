#include <iostream>
#include <vector>
using namespace std;

#define STR_EXPAND(tok) #tok
#define MAKESTR(tok) STR_EXPAND(tok)

#define CONCAT2(x, y) x##y
#define CONCAT(x,y) CONCAT2(x, y)
#define BUILD_NAME(x) CONCAT(x,__LINE__)

#ifdef _MSC_VER
//#pragma section(".logdata", read) // crash en debug!!
#pragma data_seg(".logdata")
#pragma section(".logidx", read)
#define put_in_section(decl, name)\
    __declspec(allocate(name)) decl
#endif

struct log_data_t
{
    const char * fileName;
    unsigned int lineNumber;
    const char * functionName;
    const char * logMessage;
};

void log_message(const log_data_t *);

#define log_str(msg) \
    static const log_data_t put_in_section(BUILD_NAME(log_data), ".logdata") = { __FILE__, __LINE__, __FUNCTION__, msg };\
    log_message(&BUILD_NAME(log_data));

    //static const log_data_t * put_in_section(BUILD_NAME(log_index), ".logidx") = &BUILD_NAME(log_data);
    //log_message(&BUILD_NAME(data_ref));

#define log_trace(msg) log_str(".trace " msg)

static vector<const log_data_t *> logs;

int main() {
    for (int i = 0; i < 1000 * 1000 * 100; ++i) {
        log_trace("A trace severity message that is longer than the previous one");
    }
    std::cout.write((const char *)logs.data(), sizeof(void*)* logs.size());
}


void log_message(const log_data_t * data) {
    if (logs.empty()) {
        logs.reserve(64 * 1000  / sizeof(void*));
    }
    if (logs.size() == logs.capacity()) {
        std::cout.write((const char *)logs.data(), sizeof(void*)* logs.size());
        logs.clear();
    }
    logs.push_back(data);

    //size_t addr = (size_t)data;
    //std::cout.write((const char *)&addr, sizeof(addr));
}
