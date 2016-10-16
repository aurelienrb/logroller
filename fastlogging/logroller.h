#ifndef LOGROLLER_H
#define LOGROLLER_H

#include "logmsgbuilder.h"

#define LOG_TRACE(msg) LOG( "TRACE", msg )
#define LOG_ERROR(msg) LOG( "ERROR", msg )

namespace logroller
{
void pass( const logpoint & );

logmsgbuilder log( const logpoint * logpt );
}

#define CONCAT(x, y) x##y
#define _STR(x) #x
#define STR(x) _STR(x)
#define _LOG( type, msg, line, file ) \
    static const logroller::logpoint CONCAT(logpt, line) = \
    { file, STR(line), msg, type }; \
    logroller::pass(CONCAT(logpt, line))
#define _LOG_STREAM( type, line, file ) \
    static const logroller::logpoint CONCAT(logpt, line) = \
    { file, STR(line), 0, type };\
    logroller::log( &CONCAT(logpt, line) )

#define LOG(type, msg) _LOG(type, msg, __LINE__, __FILE__)

#define log_trace() \
    _LOG_STREAM("TRACE", __LINE__, __FILE__)
    

#endif