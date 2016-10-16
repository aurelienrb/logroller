#ifndef _SCL_SECURE_NO_DEPRECATE
// warning C4996: 'strcpy' a été déclaré désapprouvé
#pragma warning( disable : 4996 )
#endif

#include "bug_report_sender.h"
#include "log_message_pool.h"

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#include <lmcons.h>
#define SECURITY_WIN32
#include <security.h>
#pragma comment( lib, "secur32.lib" )

#include <cassert>
#include <sstream>

namespace
{
    std::string         g_Header;
    std::string         g_HostName;    
    const int           g_SMTP_port = 25;

    bool SendHeader( SOCKET S,  bool HTML, const char * Subject );

    size_t append_enter_function_in_html( char *Buffer, size_t BufferSize, const char * /*FunctionName*/, size_t /*FunctionNameLength*/ )
    {
        static const char *str = "<ul>\n";
        static size_t str_len = strlen( str );

        if ( BufferSize < str_len + 1 )
        {
            return 0;   
        }
        strcat( Buffer, str );
        return str_len;
    }
    
    size_t append_leave_function_in_html( char *Buffer, size_t BufferSize, const char * /*FunctionName*/, size_t /*FunctionNameLength*/ )
    {
        static const char *str = "</ul>\n";
        static size_t str_len = strlen( str );

        if ( BufferSize < str_len + 1 )
        {
            return 0;   
        }
        strcat( Buffer, str );
        return str_len;

    }

    size_t append_log_message_in_html( char *Buffer, size_t BufferSize, const char *Msg, size_t MsgLen )
    {
        static const char *begin_str = "<li>";
        static size_t begin_str_len = strlen( begin_str );
        static const char *end_str = "</li>\n";
        static size_t end_str_len = strlen( end_str );

        const size_t total_len = begin_str_len + MsgLen + end_str_len;
        if ( BufferSize < total_len + 1 )
        {
            return 0;   
        }
        strcat( Buffer, begin_str );
        strncat( Buffer, Msg, MsgLen );
        strcat( Buffer, end_str );
        return strlen( Buffer ); // car MsgLen peut être trop grand
    }

    size_t append_crash_message_in_html( char *Buffer, size_t BufferSize, const char *Msg, size_t MsgLen )
    {
        static const char *begin_str = "<b><font color='red'>FIN D'EXECUTION SUITE A UN CRASH</font><br/>";
        static size_t begin_str_len = strlen( begin_str );
        static const char *end_str = "<br/>\n";
        static size_t end_str_len = strlen( end_str );

        const size_t total_len = begin_str_len + MsgLen + end_str_len;
        if ( BufferSize < total_len + 1 )
        {
            return 0;   
        }
        strcat( Buffer, begin_str );
        strncat( Buffer, Msg, MsgLen );
        strcat( Buffer, end_str );
        return strlen( Buffer ); // car MsgLen peut être trop grand
    }

    // renvoie le nombre de caractères copiés, ou zéro si pas assez de place
    size_t append_msg_in_html( char *Buffer, size_t BufferSize, const log_message *Msg )
    {
        if ( BufferSize < Msg->get_message_length() + 10 )
        {
            return 0;
        }
        switch ( Msg->type )
        {
        case ENTER_FUNCTION:   
            return append_enter_function_in_html( Buffer, BufferSize, Msg->get_message(), Msg->get_message_length() );
        case LEAVE_FUNCTION:
            return append_leave_function_in_html( Buffer, BufferSize, Msg->get_message(), Msg->get_message_length() );
        case TRACE:
            return append_log_message_in_html( Buffer, BufferSize, Msg->get_message(), Msg->get_message_length() );
        case CRASH:
            return append_crash_message_in_html( Buffer, BufferSize, Msg->get_message(), Msg->get_message_length() );
        }
        assert( 0 );
        return 0;
    }
}

bool bug_report_sender::init( const std::string & MailAddress )
{
    if ( MailAddress.empty() )
    {
        return false;
    }

    // valider l'adresse mail et récupérer le hostname du serveur mail
    size_t pos = MailAddress.find( '@' );
    if ( pos == std::string::npos || pos == 0 || pos == MailAddress.size() - 1 )
    {
        return false;
    }
    g_HostName = MailAddress.substr( pos + 1 );

    // récupérer le nom de la machine courante
    char user_name_buffer[ CNLEN + 1 + UNLEN + 1 ]; // "COMPUTER\USERNAME"
    ULONG user_name_buffer_len = sizeof user_name_buffer / sizeof( *user_name_buffer );
    if ( !GetUserNameExA( NameSamCompatible, user_name_buffer, &user_name_buffer_len ) )
    {
        assert( 0 );
        user_name_buffer_len = sizeof user_name_buffer / sizeof( *user_name_buffer );
        if ( !::GetUserNameA( user_name_buffer, &user_name_buffer_len ) )
        {
            return false;
        }
    }

    std::string user_name( user_name_buffer );
    
    SYSTEMTIME time;
    ::GetLocalTime( &time );

    std::string date_and_time;
    {
        std::ostringstream oss;
        oss << time.wDay << '/'
            << time.wMonth << '/'
            << time.wYear << ' '
            << time.wHour << ':'
            << time.wMinute;
        date_and_time = oss.str();
    }

    // Ok, on a tous les éléments, on génère le header
    std::ostringstream oss;
    oss << "EHLO " << user_name << "\r\n";
    oss << "MAIL FROM:<" << user_name << ">\r\n";
    oss << "RCPT TO:<" << MailAddress << ">\r\n";
    oss << "DATA\r\n";
    oss << "Date: " << date_and_time << "\r\n";
    oss << "From: " << user_name << "\r\n";
    
    g_Header = oss.str();

    return true;
}

void bug_report_sender::send_report( const log_message_pool *Pool, const char *report_title, int nb_msg_to_report )
{
    if ( g_Header.empty() || g_HostName.empty() || !report_title )
    {
        assert( 0 );
        return;
    }

    if ( nb_msg_to_report < 0 )
    {
        nb_msg_to_report = Pool->get_number_of_msg();
    }

    WSADATA wsaData;
    int res = WSAStartup( MAKEWORD(2,0), &wsaData );
    if ( res != 0 )
    {
        assert( 0 );
        return;
    }

    SOCKADDR_IN sin = { 0 };
    sin.sin_addr.s_addr	= inet_addr( g_HostName.c_str() );
    sin.sin_family		= AF_INET;
    sin.sin_port		= htons( g_SMTP_port ); // port SMTP
    SOCKET sock = socket( AF_INET, SOCK_STREAM, 0 );
    if ( sock == INVALID_SOCKET )
    {
        WSACleanup();
        return;
    }

    if ( connect( sock, (SOCKADDR *)&sin, sizeof sin ) == SOCKET_ERROR )
    {
        closesocket( sock );
        WSACleanup();
        return;
    }

    if ( !SendHeader( sock, true, report_title ) )
    {
        assert( 0 );
        closesocket( sock );
        WSACleanup();
        return;
    }

    // envoyer le début du message html
    const char msg_begin[] = "<html><body>";
    size_t msg_begin_size = ( sizeof msg_begin - 1 ) / sizeof( *msg_begin );

    res = send( sock, msg_begin, msg_begin_size, 0 );
    if ( res == SOCKET_ERROR )
    {
        closesocket( sock );
        WSACleanup();
        return;
    }

    const size_t buffer_max_size = 1024 * 10;
    static char buffer[ buffer_max_size ];

    char *ptr = buffer;
    size_t buffer_size = 0;
    res = SOCKET_ERROR;

    const log_message *msg = Pool->get_first();    
    int nb_msg_reported = 0;
    while ( msg && nb_msg_reported < nb_msg_to_report )
    {
        size_t n = append_msg_in_html( ptr, buffer_max_size - buffer_size, msg );
        if ( n > 0 )
        {
            // message copié avec succès
            buffer_size += n;
            ptr += n;
            ++nb_msg_reported;

            // passer au message suivant
            msg = Pool->get_next( msg );
        }
        else
        {
            // plus de place dans le buffer : l'envoyer pour le vider
            res = send( sock, buffer, buffer_size, 0 );
            ptr = buffer;
            buffer_size = 0;
            if ( res == SOCKET_ERROR )
            {
                closesocket( sock );
                WSACleanup();
                return;
            }
        }
    }

    // s'il reste des données à envoyer, le faire
    if ( buffer_size > 0 )
    {
        res = send( sock, buffer, buffer_size, 0 );
        if ( res == SOCKET_ERROR )
        {
            closesocket( sock );
            WSACleanup();
            return;
        }
    }

    // envoyer la fin du message
    const char msg_end[] = "</body></html>\r\n.\r\nQUIT\r\n";
    size_t msg_end_size = ( sizeof msg_end - 1 ) / sizeof( *msg_end );

    res = send( sock, msg_end, msg_end_size, 0 );
    if ( res == SOCKET_ERROR )
    {
        closesocket( sock );
        WSACleanup();
        return;
    }


    // shutdown the connection since no more data will be sent
    res = shutdown( sock, SD_SEND );
    if ( res == SOCKET_ERROR )
    {
        //printf("shutdown failed: %d\n", WSAGetLastError());
        closesocket( sock );
        WSACleanup();
        return;
    }

    // Receive until the peer closes the connection
    const int recvbuflen = 512;
    char recvbuf[ recvbuflen + 1 ];
    do
    {
        res = recv( sock, recvbuf, recvbuflen, 0 );
        recvbuf[ recvbuflen ] = 0;
    } while( res > 0 );

    closesocket( sock );
    WSACleanup();
}

namespace
{
bool SendHeader( SOCKET S,  bool HTML, const char *Subject )
{
    if ( !Subject )
    {
        return false;
    }

    int res = send( S, g_Header.c_str(), g_Header.size(), 0 );
    if ( res == SOCKET_ERROR )
    {
        return false;
    }

    const size_t max_buffer_size = 256;
    char buffer[ max_buffer_size + 1 ];

    // Content-Type
    if ( HTML )
    {
        strcpy( buffer, "Content-Type: text/html;charset=\"ISO-8859-1\"\r\n" );
    }
    else
    {
        strcpy( buffer, "Content-Type: text/plain;charset=\"ISO-8859-1\"\r\n" );
    }

    // Subject
    strcat( buffer, "Subject: " );    
    size_t buffer_size = strlen( buffer );
    
    const char *footer = "\r\n\r\n";
    const size_t length_of_footer = strlen( footer );
    const size_t max_subject_length = max_buffer_size - buffer_size - length_of_footer;

    strncat( buffer, Subject, max_subject_length );
    strcat( buffer, footer );

    buffer_size = strlen( buffer );
    res = send( S, buffer, buffer_size, 0 );

    return res != SOCKET_ERROR;
}
}