#include "logsystem.h"

namespace
{
    void test_compile()
    {
        log_trace() << "Test";
        log_close_this_thread_logging();
    }
}