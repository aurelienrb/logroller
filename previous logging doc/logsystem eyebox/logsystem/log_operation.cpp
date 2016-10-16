//#include "common.h"
////#include "log_operation.h"
//
//#include "log_console.h"
//#include "log_server.h"
//
//log_operation::~log_operation()
//{
//    this->Write( "\n", 1 );
//}
//
//void log_operation::BeginLine( unsigned long LogType, const char *File, size_t FileLen, long Line ) throw()
//{
//    assert( File );
//    try
//    {
//        std::ostringstream oss;
//
//        if ( std::uncaught_exception() )
//        {
//            oss << ZINTERNAL_LOG_PREFIX_IN_EXCEPTION << ' ';
//        }
//
//        switch ( LogType )
//        {
//        case ZINTERNAL_LOG_OPERATION_ID_TRACE:
//            oss << ZINTERNAL_LOG_PREFIX_TRACE;
//            break;
//        case ZINTERNAL_LOG_OPERATION_ID_FAILURE:
//            oss << ZINTERNAL_LOG_PREFIX_FAILURE;
//            break;
//        case ZINTERNAL_LOG_OPERATION_ID_CATCH:
//            oss << ZINTERNAL_LOG_PREFIX_CATCH;
//            break;
//        case ZINTERNAL_LOG_OPERATION_ID_WARNING:
//            oss << ZINTERNAL_LOG_PREFIX_WARNING;
//            break;
//        case ZINTERNAL_LOG_OPERATION_ID_ERROR:
//            oss << ZINTERNAL_LOG_PREFIX_ERROR;
//            break;
//        default:
//            oss << ZINTERNAL_LOG_PREFIX_UNKNOWN;
//        }
//        oss << ' ';
//
//        // extraire le nom du fichier sans le path
//        size_t pos = 0;
//        for ( size_t i = FileLen - 1; i > 0; --i )
//        {
//            if ( File[ i ] == '\\' || File[ i ] == '/' )
//            {
//                pos = i + 1;
//                break;
//            }
//        }
//        const char *file_name = &File[ pos ];
//        oss << file_name << '(' << Line << ") : ";
//        std::string s = oss.str();
//        this->Write( s.c_str(), s.size() );
//    }
//    catch ( ... )
//    {
//        // TODO: quoi faire ?
//    }
//}
//
//void log_operation::Write( const char *Str, size_t Len ) throw()
//{
//    log_server::Write( Str, Len );
//    log_console::Write( Str, Len );
//}
//
