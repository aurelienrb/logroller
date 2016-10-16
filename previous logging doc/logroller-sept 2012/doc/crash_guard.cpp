#include "common.h"

#include <cstdlib> // atexit

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

namespace
{
    HANDLE hLogFile = NULL;

    // nom du fichier de log
    std::string log_file_name;

    // si vide : ne rien envoyer
    std::string MailAddress;

    bool do_not_delete_file = false;

    void close_log_file()
    {
        if ( hLogFile != NULL )
        {
            ::CloseHandle( hLogFile );
            hLogFile = NULL;
            if ( !do_not_delete_file )
            {
                ::DeleteFile( log_file_name.c_str() );
            }
            log_file_name.clear();
            do_not_delete_file = false;
        }
    }

    HANDLE create_log_file()
    {
        // générer un nom à partir de la date
        SYSTEMTIME time;
        ::GetLocalTime( &time );

        std::ostringstream oss;
        oss << "log_";
        oss << static_cast<int>( time.wDay ) << '-';
        oss << static_cast<int>( time.wMonth ) << '-';
        oss << static_cast<int>( time.wYear ) << ' ';
        oss << static_cast<int>( time.wHour ) << 'h';
        oss << static_cast<int>( time.wMinute ) << 'm';
        oss << static_cast<int>( time.wSecond ) << "s.txt";            
        log_file_name = oss.str();

        // créer le fichier
        HANDLE hFile = ::CreateFileA(
            log_file_name.c_str(),
            GENERIC_READ | GENERIC_WRITE, // permettre la lecture pour l'envoi
            0,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL );
        if ( hFile == INVALID_HANDLE_VALUE )
        {
            log_file_name.clear();
            do_not_delete_file = true;
            return NULL;
        }       
        do_not_delete_file = false;

        // s'assurer de sa fermeture en fin d'exécution
        std::atexit( &close_log_file );
        return hFile;
    }

    void append_log_text( const char *Msg, size_t MsgLength = 0 )
    {
        if ( hLogFile == NULL )
        {
            hLogFile = create_log_file();
            if ( hLogFile == NULL )
            {
                return;
            }
        }
        if ( MsgLength == 0 )
        {
            MsgLength = strlen( Msg );
        }
        DWORD nb_written;
        ::WriteFile(
            hLogFile,
            Msg,
            MsgLength,
            &nb_written,
            NULL );
    }

    #define EXCEPTION_MSC 0xE06d7363 // 11100000 + 'msc'

    HANDLE CreateMiniDump( EXCEPTION_POINTERS *pEP ) 
    {
        const TCHAR minidump_filename[] = TEXT( "MiniDump.dmp" );
        HANDLE hFile = ::CreateFile(
            minidump_filename,
            GENERIC_READ | GENERIC_WRITE, 
            0,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,//FILE_ATTRIBUTE_NORMAL,
            NULL ); 

        if( hFile == INVALID_HANDLE_VALUE )
        {
            return NULL;
        }

        MINIDUMP_EXCEPTION_INFORMATION mdei; 

        mdei.ThreadId           = ::GetCurrentThreadId(); 
        mdei.ExceptionPointers  = pEP; 
        mdei.ClientPointers     = FALSE; 

        MINIDUMP_TYPE mdt       = ::MiniDumpNormal; 

        if ( !::MiniDumpWriteDump(
            ::GetCurrentProcess(), 
            ::GetCurrentProcessId(), 
            hFile, 
            mdt, 
            (pEP != 0) ? &mdei : 0,
            0,
            0 ) )
        {
            ::CloseHandle( hFile ); 
            return NULL;
        }
        // OK
        return hFile; 
    }

    LONG WINAPI MyUnhandledExceptionFilter( PEXCEPTION_POINTERS ExceptionInfo )
    {
        // logguer l'erreur
        const char *exception_name = 0;
        switch ( ExceptionInfo->ExceptionRecord->ExceptionCode )
        {
        case EXCEPTION_ACCESS_VIOLATION:
            exception_name = "Access violation"; break;
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
            exception_name = "Division par zéro"; break;
        case EXCEPTION_STACK_OVERFLOW:
            exception_name = "Stack overflow"; break;
        case EXCEPTION_MSC:
            exception_name = "Microsoft C++ Exception"; break;
        }
        append_log_text( "<font color='red'>" );
        append_log_text( exception_name );
        append_log_text( "</font><br />" );


        // tester pour une exception bad_alloc = mémoire pleine
        if ( ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_MSC &&
             ExceptionInfo->ExceptionRecord->NumberParameters == 3 )
        {
            ULONG_PTR param0 = ExceptionInfo->ExceptionRecord->ExceptionInformation[ 0 ];
            ULONG_PTR param1 = ExceptionInfo->ExceptionRecord->ExceptionInformation[ 1 ];
            ULONG_PTR param2 = ExceptionInfo->ExceptionRecord->ExceptionInformation[ 2 ];

            DWORD magicNumber = param0;
            if ( magicNumber == 0x19930520 ) // 1993/05/20
            {
                void *pExceptionObject = reinterpret_cast<void*>( param1 );
                const _s__ThrowInfo * pThrowInfo = reinterpret_cast<_s__ThrowInfo *>( param2 );

                int nb = pThrowInfo->pCatchableTypeArray->nCatchableTypes;
                for ( int i = 0; i < nb; ++i )
                {
                    _TypeDescriptor * pType = pThrowInfo->pCatchableTypeArray->arrayOfCatchableTypes[ i ]->pType;

                    type_info *info = reinterpret_cast<type_info*>( pType );
                    if ( *info == typeid( std::bad_alloc ) )
                    {
                        append_log_text( "<b>std::bad_alloc</b><br />" );
                        // exception bad_alloc : plus de mémoire, on essaye d'en libérer
                        ::HeapDestroy( reinterpret_cast<HANDLE>(
                            _get_heap_handle() ) );
                        break;
                    }
                    else if ( *info == typeid( std::exception ) )
                    {
                        std::exception *e = reinterpret_cast<std::exception*>( pExceptionObject );
                        append_log_text( "<b>std::exception</b> : " );
                        append_log_text( e->what() );
                        append_log_text( "<br />" );
                        break;
                    }
                    else
                    {
                        append_log_text( "<b>" );
                        append_log_text( info->name() );
                        append_log_text( "</b><br />" );
                    }
                }
            }
        }
        else if ( ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW )
        {
            static char MyStack[1024*32];  // be sure that we have enought space...
            // it assumes that DS and SS are the same!!! (this is the case for Win32)
            // change the stack only if the selectors are the same (this is the case for Win32)
            __asm mov eax, offset MyStack[1024*32];
            __asm mov esp,eax;
        }

        // ATTENTION: ici, la mémoire de la CRT peut avoir été supprimée, donc il faut
        // veiller à partir de maintenant à ne pas utiliser des objets alloués par la CRT
        // de même toutes les fonctions de la CRT sont à proscrire

        // En premier lieu, on tente de créer des infos post-mortem sur le plantage

        // créer le minidump
        HANDLE hMiniDump = CreateMiniDump( ExceptionInfo );

        // Pas d'alloc dynamique car pas de CRT + on est en plein crash...
        static char file_name[ MAX_PATH ];
        strcpy_s( file_name, MAX_PATH, log_file_name.c_str() );
        do_not_delete_file = true;
        close_log_file();
        
        if ( hMiniDump )
        {
            // l'efface en même temps
            ::CloseHandle( hMiniDump );
        }
        // message d'erreur
        ::MessageBox(
            NULL,         
            TEXT( "Le programme a rencontré une erreur interne critique qui nécessite sa fermeture.\n" )
            TEXT( "Nous vous prions de nous excuser pour le désagrément encouru." ),
            TEXT( "Erreur interne" ),
            MB_ICONHAND | MB_APPLMODAL );

        // fin
        ::TerminateProcess(
            ::GetCurrentProcess(),
            ExceptionInfo->ExceptionRecord->ExceptionCode );
        return EXCEPTION_EXECUTE_HANDLER;
    }
}

void crash_guard::install_guard()
{
    static bool installed = false;
    if ( !installed )
    {
        ::SetUnhandledExceptionFilter(
            &MyUnhandledExceptionFilter );
        installed = true;
    }
}

void crash_guard::enable_mail_reporting( const std::string & Address )
{
    MailAddress = Address;
}

void crash_guard::append_log_text( const char *Msg )
{
    ::append_log_text( Msg );
}

void crash_guard::append_log_text( const std::string &Msg )
{
    ::append_log_text( Msg.c_str(), Msg.size() );
}
