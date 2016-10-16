#ifndef LOG_SENTENCE_H
#define LOG_SENTENCE_H

#include <core_export.h>

#include <string>
#include <cassert>

class LOGSYSTEM_EXPORT log_sentence
{
public:
    enum log_type
    {
        EMPTY,
        TRACE,
    };

public:
    log_sentence( log_type Type = EMPTY ):
        zType( Type )
    {
    }

    bool empty() const
    {
        return this->zType == EMPTY;
    }

    void swap( log_sentence & S )
    {
        this->zType = S.zType;
        // soit NULL, soit pointeur vers chaîne littérale
        this->zMsg = S.zMsg;
    }

    template<size_t N>
    inline log_sentence & operator << ( const char (&Str)[N] )
    {
        this->zMsg = Str;
        return *this;
    }

    std::string msg() const
    {
        assert( !this->empty() );
        return this->zMsg;
    }

    void merge( const log_sentence & S )
    {
        this->zMsg.append( "\n" );
        this->zMsg.append( S.zMsg );
    }

private:
    log_type    zType;
    std::string zMsg;
};

#endif