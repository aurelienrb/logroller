#include <logsystem.h>

#include <fstream>
#include <iostream>
#include <vector>
#include <cstdio>
using namespace std;

#include <smtp_mail_sender.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

DWORD WINAPI ThreadProc( LPVOID )
{
    //log_thread_entry_point();

    for ( int i = 0; i < 40; ++i )
    {
        ::Sleep( 25 );
        log_trace() << "message depuis un thread";
    }
    std::cout << log_mgr::get_this_thread_mgr()->dump_html();
    log_close_this_thread_logging();
    return 0;
}

void toto(int)
{
    log_trace() << "dans toto(int)";
}

void toto()
{
    log_trace() << "dans toto()";
    toto(10);
}

void overflow(int*)
{
    int a[ 1000 ] = {};
    std::cout << a[ 0 ];
    overflow(a);
}

/*
bool start_watchdog( bool display_error_msg_on_failure )
{
    // analyser la ligne de commande

    // processus fils ?
    {
        // oui : exécuter en tant que watchdog

        // fin : renvoyer false
    }
    else
    {
        // lancer le watchdog

        // 
    }
}

class bug_reporter
{
public:
    void send_application_report();
    void send_crash_report();
    void send_user_comment_report();
};

class win32_crash_guard
{
private:
    void generate_minidump();

};
*/

int main()
{
    assert( 0 );

    SMTPMailSender sender;
    if ( !sender.open_smtp_connection( "bugreport@192.168.0.1", "Test" ) )
    {
        cout << "Echec open_smtp_connection\n";
        return 1;
    }

    sender.send_text_body( "hello" );
    sender.send_text_body( "<html><body><h3>Hello!</h3></body></html>", "text/html" );
    char data[] = "A!";
    sender.send_attached_rawdata( "test.txt", data, sizeof data - 1 );
    sender.send_attached_file( "image.bmp", "C:\\Documents and Settings\\Aurelien\\Bureau\\user.bmp", "image/bmp" );
    sender.terminate_and_close();
/*
    bug_reporter reporter( "bugreport@192.168.0.1" );

    log_system log( reporter );

    crash_guard guard( reporter );

    if ( !guard.start_watchdog() )
    {
        return 0;
    }

    log_mgr::enable_bug_report();
    //CloseHandle( CreateThread( NULL, 0, &ThreadProc, NULL, 0, NULL ) );
    for ( int i = 0; i < 10; ++i )
    {
        //::Sleep( 1000 );
        log_trace() << "Hello";
        toto();
        toto();
        log_trace() << "World";
    }
    
    //std::cout << log_mgr::get_this_thread_mgr()->dump_html();
    //log_mgr::test();
    //throw int(0);

    std::ofstream file( "test.txt" );
    //fopen( "toto.txt", "w" );
    fcloseall();

    //new int[ 30000000 ];
    //::Sleep( 10000 );
    //HeapDestroy(
    //    (HANDLE)_get_heap_handle() );
    //::Sleep( 10000 );
    //overflow(0);
*/
}
