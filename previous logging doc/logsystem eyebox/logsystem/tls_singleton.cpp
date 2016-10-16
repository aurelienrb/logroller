#include "tls_singleton.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace
{
    void test_compile()
    {
        tls_singleton<int> s;
        s.set_this_thread_instance(new int);
        s.get_this_thread_instance();
        s.delete_this_thread_instance();
        s.get_undeleted_instances();
    }
}

tls_index::tls_index()
{
    this->zTlsIndex = ::TlsAlloc();
    if ( this->zTlsIndex == TLS_OUT_OF_INDEXES )
    {
        throw std::runtime_error( "TlsAlloc failed" );
    }
}

tls_index::~tls_index()
{
    // détruire les instances allouées
    ::TlsFree( this->zTlsIndex );
}

void tls_index::set( void *Value )
{
    ::TlsSetValue(
        this->zTlsIndex,
        Value );
}

void* tls_index::get() const
{
    return ::TlsGetValue( this->zTlsIndex );
}
