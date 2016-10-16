#include "log_sentence.h"

namespace
{
    void test_compile()
    {
        log_sentence s1( log_sentence::TRACE );
        s1 << "chaine litterale";
        log_sentence s2;
        s2.swap( s1 );
    }
}
