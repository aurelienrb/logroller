#include "log_mgr.h"
#include "tls_singleton.h"
#include "bug_report_sender.h"

#include "log_message_pool.h"

#include <iostream>
#include <typeinfo>
#include <sstream>
#include <cassert>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

namespace
{
    tls_singleton<log_thread_message_mgr> g_tls_mgr;
    bug_report_sender g_sender;

#define EXCEPTION_MSC 0xE06d7363

void CreateMiniDump( EXCEPTION_POINTERS* pep ) 
{
    // Open the file 

    HANDLE hFile = CreateFile( TEXT("MiniDump.dmp"), GENERIC_READ | GENERIC_WRITE, 
        0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ); 

    if( ( hFile != NULL ) && ( hFile != INVALID_HANDLE_VALUE ) ) 
    {
        // Create the minidump 

        MINIDUMP_EXCEPTION_INFORMATION mdei; 

        mdei.ThreadId           = GetCurrentThreadId(); 
        mdei.ExceptionPointers  = pep; 
        mdei.ClientPointers     = FALSE; 

        MINIDUMP_TYPE mdt       = MiniDumpNormal; 

        BOOL rv = MiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(), 
            hFile, mdt, (pep != 0) ? &mdei : 0, 0, 0 ); 

        //if( !rv ) 
            //_tprintf( _T("MiniDumpWriteDump failed. Error: %u \n"), GetLastError() ); 
        //else 
            //_tprintf( _T("Minidump created.\n") ); 

        // Close the file 

        CloseHandle( hFile ); 

    }
    else 
    {
        //_tprintf( _T("CreateFile failed. Error: %u \n"), GetLastError() ); 
    }

}

LONG WINAPI MyUnhandledExceptionFilter( PEXCEPTION_POINTERS ExceptionInfo )
{
/*
if ( ExceptionInfo->ExceptionRecord->ExceptionCode == 0xE06d7363 && // '?msc'
     ExceptionInfo->ExceptionRecord->NumberParameters == 3 )
{
    ULONG_PTR param0 = ExceptionInfo->ExceptionRecord->ExceptionInformation[ 0 ];
    ULONG_PTR param1 = ExceptionInfo->ExceptionRecord->ExceptionInformation[ 1 ];
    ULONG_PTR param2 = ExceptionInfo->ExceptionRecord->ExceptionInformation[ 2 ];

    DWORD magicNumber = param0;
    if ( magicNumber == 0x19930520 ) // 1999/05/20
    {
        void *pExceptionObject = reinterpret_cast<void*>( param1 );
        const _s__ThrowInfo * pThrowInfo = reinterpret_cast<_s__ThrowInfo *>( param2 );
        _TypeDescriptor * pType = pThrowInfo->pCatchableTypeArray->arrayOfCatchableTypes[ 0 ]->pType;

        type_info *info = reinterpret_cast<type_info*>( pType );
        std::cout << info->name() << '\n';
    }
}
*/

    CreateMiniDump( ExceptionInfo );





    static DWORD crash_address;
    static DWORD exception_code;

    crash_address = reinterpret_cast<DWORD>( ExceptionInfo->ExceptionRecord->ExceptionAddress );
    exception_code = ExceptionInfo->ExceptionRecord->ExceptionCode;
    if ( ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW )
    {
        static char MyStack[1024*128];  // be sure that we have enought space...
        // it assumes that DS and SS are the same!!! (this is the case for Win32)
        // change the stack only if the selectors are the same (this is the case for Win32)
        //__asm push offset MyStack[1024*128];
        //__asm pop esp;
        __asm mov eax,offset MyStack[1024*128];
        __asm mov esp,eax;
    }

    std::ostringstream oss;
    switch ( exception_code )
    {
    case EXCEPTION_ACCESS_VIOLATION:
        oss << "Access violation"; break;
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
        oss << "Division par zéro"; break;
    case EXCEPTION_STACK_OVERFLOW:
        oss << "Stack overflow"; break;
    case EXCEPTION_MSC:
        oss << "Microsoft C++ Exception"; break;
    default:
        oss << "exception de type 0x" << std::hex << exception_code;
    }
    oss << " à l'adresse : 0x" << std::hex << crash_address << '\n';

    log_message_pool *pool = g_tls_mgr.get_this_thread_instance()->get_pool();
    log_message *msg = pool->alloc();
    msg->level = 0;
    msg->type = CRASH;
    msg->set_message( oss.str() );

    //g_sender.send_report( pool, "crash" );

    ::MessageBox(
        NULL,         
        TEXT( "Le programme a rencontré une erreur interne critique qui nécessite sa fermeture.\n" )
        TEXT( "Nous vous prions de nous excuser pour le désagrément encouru." ),
        TEXT( "Erreur interne" ),
        MB_ICONHAND | MB_APPLMODAL );

    ::TerminateProcess(
        ::GetCurrentProcess(),
        1 ); // GetExceptionCode()
    return EXCEPTION_EXECUTE_HANDLER;
}
}


log_thread_message_mgr* log_mgr::get_this_thread_mgr()
{   
    log_thread_message_mgr *mgr = g_tls_mgr.get_this_thread_instance();
    if ( !mgr )
    {
        mgr = new log_thread_message_mgr;
        g_tls_mgr.set_this_thread_instance( mgr );
        assert( g_tls_mgr.get_this_thread_instance() == mgr );
    }
    return mgr;
}

void log_mgr::destroy_this_thread_mgr()
{
    g_tls_mgr.delete_this_thread_instance();
}

void log_mgr::enable_bug_report()
{
    g_sender.init( "bugreport@192.168.0.1" );
    ::SetUnhandledExceptionFilter( &MyUnhandledExceptionFilter );
}

void log_mgr::test()
{
    g_sender.send_report( g_tls_mgr.get_this_thread_instance()->get_pool(), "test" );
}