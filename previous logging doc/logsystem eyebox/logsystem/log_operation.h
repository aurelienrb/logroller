#ifndef LOG_OPERATION_H
#define LOG_OPERATION_H
/*
#ifndef LOG_OPERATION_IDS_H
#define LOG_OPERATION_IDS_H

#define ZINTERNAL_LOG_OPERATION_ID_TRACE    'TRCE'
#define ZINTERNAL_LOG_OPERATION_ID_FAILURE  'FAIL'
#define ZINTERNAL_LOG_OPERATION_ID_CATCH    'CTCH'
#define ZINTERNAL_LOG_OPERATION_ID_WARNING  'WARN'
#define ZINTERNAL_LOG_OPERATION_ID_ERROR    'ERR#'

#define ZINTERNAL_LOG_PREFIX_TRACE          "TRACE:"
#define ZINTERNAL_LOG_PREFIX_FAILURE        "FAILURE:"
#define ZINTERNAL_LOG_PREFIX_CATCH          "CATCH:"
#define ZINTERNAL_LOG_PREFIX_INFO           "INFO:"
#define ZINTERNAL_LOG_PREFIX_WARNING        "WARNING:"
#define ZINTERNAL_LOG_PREFIX_ERROR          "ERROR:"
#define ZINTERNAL_LOG_PREFIX_UNKNOWN        "<???>:"

#define ZINTERNAL_LOG_PREFIX_IN_EXCEPTION   "(exception)"

#endif
#define log_trace()         log_operation( ZINTERNAL_LOG_OPERATION_ID_TRACE, __FILE__, __LINE__ )
#define log_failure()       log_operation( ZINTERNAL_LOG_OPERATION_ID_FAILURE, __FILE__, __LINE__ )
#define log_catch()         log_operation( ZINTERNAL_LOG_OPERATION_ID_CATCH, __FILE__, __LINE__ )
#define log_catch_unknown() log_operation( ZINTERNAL_LOG_OPERATION_ID_CATCH, __FILE__, __LINE__ )
#define log_warning()       log_operation( ZINTERNAL_LOG_OPERATION_ID_WARNING, __FILE__, __LINE__ )
#define log_error()         log_operation( ZINTERNAL_LOG_OPERATION_ID_ERROR, __FILE__, __LINE__ )

#define log_trace()         log_scope scope##__LINE__( __FILE__, __LINE__, __FUNCTION__ ) = log_sentence( ZINTERNAL_LOG_OPERATION_ID_TRACE )

class COMMON_EXPORT log_operation
{
public:
    template<size_t N>
    log_operation( unsigned long LogType, const char (&File)[N], long Line )
    {
        this->BeginLine( LogType, File, N-1, Line );
    }

    ~log_operation();

    template<typename T>
    inline log_operation & operator << ( const T & t )
    {
        std::ostringstream oss;
        oss << t;
        return this->operator <<( oss.str() );
    }

    // définir pour size_t pour éviter le warning C4267: conversion
    // de 'size_t' en 'unsigned int', perte possible de données
    inline log_operation & operator << ( const size_t & t )
    {
        return this->operator << <unsigned int>( t );
    }

    // définir pour char* pour éviter de passer par un ostringstream
    // version template pour les chaines littérales => déduit leur longueur
    template<size_t N>
    inline log_operation & operator << ( const char (&Str)[N] )
    {
        this->Write( Str, N-1 );
        return *this;
    }

    // définir pour char* pour éviter de passer par un ostringstream 
    inline log_operation & operator << ( const char * Str )
    {
        if ( Str )
        {
            this->Write( Str, strlen( Str ) );
        }
        return *this;
    }

    // définir pour std::string pour éviter de passer par un ostringstream 
    inline log_operation & operator << ( const std::string & Str )
    {
        this->Write( Str.c_str(), Str.length() );
        return *this;
    }

    // définir pour QString pour éviter de passer par un ostringstream 
    inline log_operation & operator << ( const QString & Str )
    {
        this->Write( Str.toAscii().data(), Str.length() );
        return *this;
    }

private:
    void BeginLine( unsigned long LogType, const char * File, size_t FileLen, long Line ) throw();
    void Write( const char *Str, size_t Len ) throw();
};
*/
#endif