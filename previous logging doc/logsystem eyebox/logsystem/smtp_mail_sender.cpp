// remove warning C4127: conditional expression is constant
#pragma warning(disable: 4127)
#ifndef _SCL_SECURE_NO_DEPRECATE
// remove warning C4996: 'strcpy' was declared deprecated
#pragma warning( disable : 4996 )
#endif

#include "smtp_mail_sender.h"

#include <sstream>
#include <cassert>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#include <lmcons.h>
#elif defined POSIX
#include <sys/utsname.h>
#endif

#define BOUDARY_STRING "__AbdLK3254Onwx__bclO_Hx_458_hUI9hs_ckiu__"

#ifdef _DEBUG
#include <iostream>
using std::cerr;
#define DBG_ONLY(x) x
#else
#define DBG_ONLY(x)
#endif

class smtp_buffer
{
public:
    smtp_buffer() : size( 0 ), overflow( false )
    {
    }

    void reset( const char *Str1 = 0, const char *Str2 = 0, const char *Str3 = 0 )
    {
        this->size = 0;
        this->overflow = false;
        this->append( Str1, Str2, Str3 );
    }

    bool append( const char *Str1, const char *Str2 = 0, const char *Str3 = 0 )
    {                
        if ( this->overflow )
        {
            DBG_ONLY( cerr << "append : overflow" );
            return false;
        }
        const char *str[ 3 ] = { Str1, Str2, Str3 };
        for ( int i = 0; i < 3; ++i )
        {
            const char *s = str[ i ];
            if ( s )
            {
                const size_t free_size = sizeof this->data - size - 1;
                size_t s_len = strlen( s );
                if ( s_len > free_size )
                { 
                    DBG_ONLY( cerr << "append : overflow" );
                    this->overflow = true;
                    return false;
                }
                strcpy( &this->data[ size ], s );
                this->size += s_len;
            }
        }
        return true;
    }

    bool encode_base64_and_send( int SocketFD, const unsigned char *DataToEncode, size_t DataSize )
    {
        if ( SocketFD == INVALID_SOCKET )
        {
            DBG_ONLY( cerr << "send : SocketFD == INVALID_SOCKET" );
            return false;
        }

        if ( DataSize == 0 )
        {
            return true;
        }
        // the following code is based on:
        //*********************************************************************
        //* Base64 - a simple base64 encoder and decoder.
        //*
        //*     Copyright (c) 1999, Bob Withers - bwit@pobox.com
        //*
        //* This code may be freely used for any purpose, either personal
        //* or commercial, provided the authors copyright notice remains
        //* intact.
        //*********************************************************************
        const char fillchar = '=';
        static const char cvt[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";

        this->reset();
        for ( size_t i = 0; i < DataSize; ++i )
        {
            unsigned char c = (DataToEncode[ i ] >> 2) & 0x3F;
            this->data[ this->size ] = cvt[ c ];
            ++this->size;

            c = (DataToEncode[ i ] << 4) & 0x3F;
            if ( ++i < DataSize )
            {
                c |= (DataToEncode[ i ] >> 4) & 0x0f;
            }
            this->data[ this->size ] = cvt[ c ];
            ++this->size;

            if ( i < DataSize )
            {
                c = (DataToEncode[ i ] << 2) & 0x3F;
                if ( ++i < DataSize )
                {
                    c |= (DataToEncode[ i ] >> 6) & 0x03;
                }
                this->data[ this->size ] = cvt[ c ];
                ++this->size;
            }
            else
            {
                ++i;
                this->data[ this->size ] = fillchar;
                ++this->size;
            }
            if ( i < DataSize )
            {
                c = DataToEncode[ i ] & 0x3F;
                this->data[ this->size ] = cvt[ c ];
                ++this->size;
            }
            else
            {
                this->data[ this->size ] = fillchar;
                ++this->size;
            }

            if ( this->size > sizeof this->data - 4 )
            {
                if ( !this->send( SocketFD ) )
                {
                    return false;
                }
                // ici, this->size a été réinitialisé à zéro
            }
        }
        // s'il reste des données à envoyer, les envoyer
        if ( this->size > 0 )
        {
            return this->send( SocketFD );
        }
        return true;
    }

    bool send( int SocketFD )
    {
        if ( SocketFD == INVALID_SOCKET )
        {
            DBG_ONLY( cerr << "send : SocketFD == INVALID_SOCKET" );
            return false;
        }

        if ( this->size == 0 )
        {
            return true;
        }

        if ( this->overflow )
        {
            DBG_ONLY( cerr << "send : overflow" );
            return false;
        }

        for ( size_t i = 0; i < this->size; ++i )
        {
            std::cout << this->data[ i ];
        }
        int res = ::send( SocketFD, this->data, this->size, 0 );
        this->reset();
        return res != SOCKET_ERROR;
    }

    bool send_cmd_and_wait_for_reply( size_t SocketFD, const char *ExpectedReply, int TimeOutSec )
    {
        if ( SocketFD == INVALID_SOCKET )
        {
            DBG_ONLY( cerr << "send_cmd_and_wait_for_reply : SocketFD == INVALID_SOCKET" );
            return false;
        }

        if ( this->size == 0 )
        {
            return true;
        }

        if ( this->overflow )
        {
            DBG_ONLY( cerr << "send_cmd_and_wait_for_reply : overflow" );
            return false;
        }
        // envoyer
        for ( size_t i = 0; i < this->size; ++i )
        {
            std::cout << this->data[ i ];
        }

        int res = ::send( SocketFD, this->data, this->size, 0 );
        this->reset();
        if ( res == SOCKET_ERROR )
        {
            DBG_ONLY( cerr << "send_cmd_and_wait_for_reply : echec send() : " << WSAGetLastError() );
            return false;
        }   

        for (;;)
        {
            // attendre la réponse
            fd_set fdread;
            FD_ZERO( &fdread );
            FD_SET( SocketFD, &fdread );
            timeval timeout = { 0 };
            timeout.tv_sec = TimeOutSec;
            res = ::select( 1, &fdread, NULL, NULL, &timeout );
            if ( !res )
            {
                DBG_ONLY( cerr << "send_cmd_and_wait_for_reply : timeout" );
                return false;
            }
            // lire la réponse
            const size_t max_size = sizeof this->data - 1;
            res = ::recv( SocketFD, this->data, max_size, 0 );
            if ( res <= 0 )
            {
                DBG_ONLY( cerr << "send_cmd_and_wait_for_reply : echec recv() : " << WSAGetLastError() );
                return false;
            }
            if ( res > max_size )
            {
                // impossible normalement, mais...
                DBG_ONLY( cerr << "send_cmd_and_wait_for_reply : res > max_size" );
                return false;
            }
            this->data[ res ] = '\0';
            // recherche simpliste de la réponse attendue
            bool success = ( strstr( this->data, ExpectedReply ) != NULL );
            if ( success )
            {
                break;
            }
        }
        return true;
    }

private:
    char data[ 1024 ];
    size_t size;
    bool overflow;
};

namespace
{
    bool InitSocketSubsystem()
    {
        WSADATA data;
        return ::WSAStartup( MAKEWORD( 2,0 ), &data ) == 0;
    }

    void ShutdownSocketSubsystem()
    {
        ::WSACleanup();
    }

    const char * ValidateEMailAndExtractServerName( const char * FullMailAddress )
    {
        const char *arobase = strchr( FullMailAddress, '@' );
        if ( arobase == NULL || arobase == FullMailAddress )
        {
            DBG_ONLY( cerr << "ValidateEMailAndExtractServerName : adresse invalide : " << FullMailAddress );
            return 0;
        }
        const char *server_name = arobase + 1;
        if ( server_name[ 0 ] == '\0' )
        {
            DBG_ONLY( cerr << "ValidateEMailAndExtractServerName : adresse invalide : " << FullMailAddress );
            return 0;
        }
        return server_name;        
    }

    const char * GenerateComputerName()
    {
#ifdef _WIN32
        static char computer_name[ MAX_COMPUTERNAME_LENGTH + 1 ] = { 0 };
        DWORD size = sizeof computer_name;
        ::GetComputerNameA( computer_name, &size );
        return computer_name;
#elif defined POSIX
	struct utsname uts;
	uname(&uts);
	return uts.nodename;
#endif
    }
}

SMTPMailSender::SMTPMailSender():
    socket_fd( INVALID_SOCKET )
{
    InitSocketSubsystem();
}

SMTPMailSender::~SMTPMailSender()
{
    this->close_connection();
    ShutdownSocketSubsystem();
}

bool SMTPMailSender::open_smtp_connection( const char *DestMailAddress, const char *MailSubject, const char *SenderNameOrEmail, unsigned short SMTPPortNumber, int TimeOutSec )    
{
    const char *smtp_server = ValidateEMailAndExtractServerName( DestMailAddress );
    if ( smtp_server == 0 )
    {
        return false;
    }

    this->timeout_sec = TimeOutSec;
    if ( this->socket_fd != INVALID_SOCKET )
    {
        this->close_connection();
    }
    this->socket_fd = ::socket( AF_INET, SOCK_STREAM, 0 );
    if ( this->socket_fd == INVALID_SOCKET )
    {
        DBG_ONLY( cerr << "open_smtp_connection : echec socket() : " << WSAGetLastError() );
        return false;           
    }

    SOCKADDR_IN sin     = { 0 };
    sin.sin_addr.s_addr	= inet_addr( smtp_server );
    sin.sin_family		= AF_INET;
    sin.sin_port		= htons( SMTPPortNumber );

    if ( ::connect( 
        this->socket_fd, 
        reinterpret_cast<SOCKADDR*>( &sin ), 
        sizeof sin ) == SOCKET_ERROR )
    {
        DBG_ONLY( cerr << "open_smtp_connection : echec connect() : " << WSAGetLastError() );
        this->close_connection();
        return false;
    }

    // Envoyer le header
    char computer_name[ MAX_COMPUTERNAME_LENGTH + 1 ] = { 0 };
    {
        DWORD size = sizeof computer_name;
        ::GetComputerNameA( computer_name, &size );
    }

    static smtp_buffer buffer;
    
    buffer.reset( "EHLO ", computer_name, "\r\n" );
    if ( !buffer.send_cmd_and_wait_for_reply( this->socket_fd, "220", this->timeout_sec ) )
    {
        DBG_ONLY( "open_smtp_connection : echec send_cmd_and_wait_for_reply( 'EHLO', '220' )" );
        this->close_connection();
        return false;
    }

    if ( SenderNameOrEmail == 0 )
    {
        static char user_name[ UNLEN + 1 ] = { 0 };
	    DWORD size = sizeof user_name;
        ::GetUserNameA( user_name, &size );
        SenderNameOrEmail = user_name;
    }

    buffer.reset( "MAIL FROM:<", SenderNameOrEmail, ">\r\n" );
    if ( !buffer.send_cmd_and_wait_for_reply( this->socket_fd, "250", this->timeout_sec ) )
    {
        DBG_ONLY( "open_smtp_connection : echec send_cmd_and_wait_for_reply( 'MAIL FROM', '250' )" );
        this->close_connection();
        return false;
    }

    buffer.reset( "RCPT TO:<", DestMailAddress, ">\r\n" );
    if ( !buffer.send_cmd_and_wait_for_reply( this->socket_fd, "250", this->timeout_sec ) )
    {
        DBG_ONLY( "open_smtp_connection : echec send_cmd_and_wait_for_reply( 'RCPT TO', '250' )" );
        this->close_connection();
        return false;
    }


    buffer.reset( "DATA\r\n" );
    if ( !buffer.send_cmd_and_wait_for_reply( this->socket_fd, "354", this->timeout_sec ) )
    {
        DBG_ONLY( "open_smtp_connection : echec send_cmd_and_wait_for_reply( 'DATA', '354' )" );
        this->close_connection();
        return false;
    }

    SYSTEMTIME time;
    ::GetLocalTime( &time );
    char date_and_time[ 2 + 1 + 2 + 1 + 4 + 1 + 2 + 1 + 2 + 1 ] = { 0 };
    sprintf(
        date_and_time,
        "%02d/%02d/%04d %02d:%02d",
        time.wDay,
        time.wMonth,
        time.wYear,
        time.wHour,
        time.wMinute );

    buffer.reset( "Date: ", date_and_time, "\r\n" );

    buffer.append( "From: <", SenderNameOrEmail, ">\r\n" );
    buffer.append( "Subject: ", MailSubject, "\r\n" );
    buffer.append( "MIME-Version: 1.0\r\n" );
    buffer.append( "Content-Type: multipart/mixed;\r\n" );
    buffer.append( " boundary= \"" BOUDARY_STRING "\"\r\n" );
    buffer.append( "\r\n" );
    if ( !buffer.send( this->socket_fd ) )
    {
        DBG_ONLY( "open_smtp_connection : echec send( header )" );
        this->close_connection();
        return false;
    }

    return true;
}

bool SMTPMailSender::send_attached_rawdata( const char *AttachmentName, const char *Data, size_t DataSize, const char *MimeType )
{
    const unsigned char *data = reinterpret_cast<const unsigned char *>( Data );
    return this->send_attached_rawdata( AttachmentName, data, DataSize, MimeType );
}

bool SMTPMailSender::send_attached_rawdata( const char *AttachmentName, const unsigned char *Data, size_t DataSize, const char *MimeType )
{
    if ( this->socket_fd == INVALID_SOCKET )
    {
        return false;
    }

    static smtp_buffer buffer;

    buffer.append( "--" BOUDARY_STRING "\r\n" );
    buffer.append( "Content-Type: ", MimeType, "\r\n" );
    buffer.append( "Content-Transfer-Encoding: base64\r\n" );
    buffer.append( "Content-Disposition: attachment;\r\n filename= \"", AttachmentName, "\"\r\n\r\n" );
    if ( !buffer.send( this->socket_fd ) )
    {
        return false;
    }
    if ( !buffer.encode_base64_and_send( this->socket_fd, Data, DataSize ) )
    {
        return false;
    }
    buffer.reset( "\r\n" );
    return buffer.send( this->socket_fd );
}

bool SMTPMailSender::send_attached_file( const char *FileName, const char *FullPathFileName, const char *MimeType )
{
    if ( this->socket_fd == INVALID_SOCKET )
    {
        return false;
    }

    if ( FullPathFileName == 0 )
    {
        FullPathFileName = FileName;
    }

    // ouvrir
    HANDLE hFile = ::CreateFileA(
        FullPathFileName,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL );
    if ( hFile == INVALID_HANDLE_VALUE ) 
    { 
        return false;
    }
    
    // envoyer les infos du fichier
    static smtp_buffer buffer;

    buffer.append( "--" BOUDARY_STRING "\r\n" );
    buffer.append( "Content-Type: ", MimeType, "\r\n" );
    buffer.append( "Content-Transfer-Encoding: base64\r\n" );
    buffer.append( "Content-Disposition: attachment;\r\n filename= \"", FileName, "\"\r\n\r\n" );
    if ( !buffer.send( this->socket_fd ) )
    {
        return false;
    }

    // lire le fichier et l'envoyer
    const size_t read_buffer_size = 4096;
    unsigned char read_buffer[ read_buffer_size ];
    bool success = false;
    for (;;)
    {
        DWORD nb_read;
        if ( ::ReadFile( hFile, read_buffer, read_buffer_size, &nb_read, NULL ) )
        {
            if ( nb_read > 0 &&
                 !buffer.encode_base64_and_send( this->socket_fd, read_buffer, nb_read ) )
            {
                break;
            }
            if ( nb_read != read_buffer_size )
            {
                success = true;
                break;
            }
        } 
        else
        {
            // erreur de lecture
            break;
        }
    }
    ::CloseHandle( hFile );
    if ( !success )
    {
        return false;
    }

    // terminer    
    buffer.reset( "\r\n" );
    return buffer.send( this->socket_fd );
}

bool SMTPMailSender::send_text_body( const char *Text, const char *ContentType, const char *Charset, size_t TextLength )
{    
    if ( this->socket_fd == INVALID_SOCKET )
    {
        DBG_ONLY( cerr << "send_text_body : SocketFD == INVALID_SOCKET" );
        return false;
    }

    if ( TextLength == 0 )
    {
        TextLength = strlen( Text );
    }

    static smtp_buffer buffer;
    
    buffer.reset( "--" BOUDARY_STRING "\r\n" );
    buffer.append( "Content-Type: ", ContentType, ";" );
    buffer.append( " charset=\"", Charset, "\"\r\n" );
    buffer.append( "\r\n" );
    if ( !buffer.send( this->socket_fd ) )
    {
        DBG_ONLY( cerr << "send_text_body : !buffer.send" );
        return false;
    }

    // envoyer directement, on évite une recopie + une limitation de la taille du buffer
    int res = ::send( this->socket_fd, Text, TextLength, 0 );
    if ( res == SOCKET_ERROR )
    {
        DBG_ONLY( cerr << "send_text_body : echec send() : " << WSAGetLastError() );
        return false;
    }

    // terminer
    buffer.reset( "\r\n" );
    return buffer.send( this->socket_fd );
}

void SMTPMailSender::close_connection()
{
    if ( this->socket_fd != INVALID_SOCKET )
    {
        closesocket( this->socket_fd );
        this->socket_fd = INVALID_SOCKET;
    }
}

bool SMTPMailSender::terminate_and_close()
{
    if ( this->socket_fd == INVALID_SOCKET )
    {
        return false;
    }

    static smtp_buffer buffer;
    
    buffer.reset( "--" BOUDARY_STRING "--\r\n" );
    buffer.append( "\r\n.\r\n" );
    buffer.append( "QUIT\r\n" );
    bool res = buffer.send_cmd_and_wait_for_reply( this->socket_fd, "250", this->timeout_sec );
    this->wait_for_connection_closing();
    return res;
}

void SMTPMailSender::cancel_and_close()
{
    this->close_connection();
}

void SMTPMailSender::wait_for_connection_closing()
{
    if ( this->socket_fd == INVALID_SOCKET )
    {
        return;
    }

    // on fait des lectures jusqu'à ce que le serveur coupe la connexion
    const size_t buffer_size = 128;
    char buffer[ buffer_size ];
    int res;
    do
    {
        res = ::recv( this->socket_fd, buffer, buffer_size, 0 );
    } while ( res > 0 );
    if ( res != 0 )
    {
        DBG_ONLY( cerr << "wait_for_connection_closing : erreur recv : " << res );
    }
    this->close_connection();
}
