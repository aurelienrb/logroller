//#include "common.h"
//#include "send_mail.h"
//
//#define WIN32_LEAN_AND_MEAN
//#include <winsock2.h>
//#pragma comment(lib, "ws2_32.lib")
//
//#include <iostream>
//using namespace std;
///*
//void bcopy( void * source, void * destination, int size )
//{
//   char * src = ( char * ) source;
//   char * dst = ( char * ) destination;
//
//   for( int i=0; i<size; i++ )
//      dst[i] = src[i];
//}
//
//void bzero( void * destination, int size )
//{
//   char * dst = ( char * ) destination;
//
//   for( int i=0; i<size; i++ )
//      dst[i] = 0x00;
//}
//
//void Process( char * buffer )
//{
//   Sleep( 1000 );
//   cerr << "Envoye   < " << buffer << endl;
//   int    size   = strlen( buffer );
//   int    retVal = send( to_server_socket, buffer, size, 0 );
//
//   char buf[ 1024 ];
//   buf[0] = 0x00;
//   while( !buf[0] )
//      int yeah = recv( to_server_socket, buf, 1024, 0 );
//   cerr << "Recu     > " << buf << endl << endl;
//}
//
//
//SOCKET to_server_socket = 0;
//char   server_name[]    = "192.168.0.1";
//int    port             = 25;
//*/
//
//void send_mail()
//{
//    WSADATA wsaData;
//    int res = WSAStartup( MAKEWORD(2,0), &wsaData );
//    if ( res != 0 )
//    {
//       return;
//    }
//
//    int iResult;
//
//    SOCKADDR_IN sin = { 0 };
//    sin.sin_addr.s_addr	= inet_addr( "192.168.0.1" );
//    sin.sin_family		= AF_INET;
//    sin.sin_port		= htons( 25 ); // port SMTP
//    SOCKET sock = socket( AF_INET, SOCK_STREAM, 0 );
//    if ( sock == INVALID_SOCKET )
//    {
//        WSACleanup();
//        return;
//    }
//
//    if ( connect( sock, (SOCKADDR *)&sin, sizeof sin ) == SOCKET_ERROR )
//    {
//        closesocket( sock );
//        WSACleanup();
//        return;
//    }
//
//    SYSTEMTIME time;
//    ::GetLocalTime( &time );
//
//    std::string date_and_time;
//    {
//        std::ostringstream oss;
//        oss << time.wDay << '/'
//            << time.wMonth << '/'
//            << time.wYear << ' '
//            << time.wHour << ':'
//            << time.wMinute;
//        date_and_time = oss.str();
//    }
//
//    const char * user_name = "Aurelien";
//    const char * mail_server_host_name = "192.168.0.1";
//    const char * mail_server_mailbox = "bugreport";
//    const char * mail_subject = "Test HTML";
//    const char * mail_body = "<html><body><b>Hello</b><br/>ligne2</body></html>";
//
//    std::ostringstream oss;
//    oss << "EHLO " << user_name << "\r\n";
//    oss << "MAIL FROM:<" << user_name << ">\r\n";
//    oss << "RCPT TO:<" << mail_server_mailbox << '@' << mail_server_host_name << ">\r\n";
//    oss << "DATA\r\n";
//    oss << "Date: " << date_and_time << "\r\n";
//    oss << "From: " << user_name << "\r\n";
//    //oss << "Content-Type: text/plain;charset=\"ISO-8859-1\"\r\n";
//    oss << "Content-Type: text/html;charset=\"ISO-8859-1\"\r\n";
//    oss << "Subject: " << mail_subject << "\r\n\r\n";
//    oss << mail_body << "\r\n";
//    oss << "\r\n.\r\n";
//    oss << "QUIT\r\n";
//
//    std::string msg = oss.str();
//
//    // Send an initial buffer
//    iResult = send( sock, msg.c_str(), msg.size(), 0 );
//    if ( iResult == SOCKET_ERROR ) 
//    {
//        //printf("send failed: %d\n", WSAGetLastError());
//        closesocket( sock );
//        WSACleanup();
//        return;
//    }
//
//    //printf("Bytes Sent: %ld\n", iResult);
//
//    // shutdown the connection since no more data will be sent
//    iResult = shutdown( sock, SD_SEND );
//    if ( iResult == SOCKET_ERROR )
//    {
//        //printf("shutdown failed: %d\n", WSAGetLastError());
//        closesocket( sock );
//        WSACleanup();
//        return;
//    }
//
//    // Receive until the peer closes the connection
//    const int recvbuflen = 512;
//    char recvbuf[ recvbuflen ];
//    do
//    {
//        iResult = recv( sock, recvbuf, recvbuflen, 0 );
//        if ( iResult > 0 )
//        {
//            //printf("Bytes received: %d\n", iResult);
//        }
//        else if ( iResult == 0 )
//        {
//            //printf("Connection closed\n");
//        }
//        else
//        {
//            //printf("recv failed: %d\n", WSAGetLastError());
//        }
//
//    } while( iResult > 0 );
//
//    closesocket( sock );
//    WSACleanup();
//}
//
