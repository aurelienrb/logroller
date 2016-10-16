#ifndef LOG_SCOPE_H
#define LOG_SCOPE_H

#include "log_sentence.h"

class LOGSYSTEM_EXPORT log_scope// : public boost::noncopyable
{
public:
    static const log_scope * GetThisThreadRootScope();
    static const log_scope * GetThisThreadCurrentScope();
    // on ne passe pas par des fonctions membres pour éviter de manipuler
    // des pointeurs invalides (car on ne contrôle pas la destruction des
    // log_scope)
    static const log_scope * Previous( const log_scope * );
    static const log_scope * Next( const log_scope * );

public:
    log_scope( const char FileName[], long LineNumber, const char FunctionName[] );
    ~log_scope();

    log_scope& operator=( log_sentence & );

    std::string msg() const
    {
        if ( !this->zLogMsg.empty() )
        {
            return this->zLogMsg.msg() + "\n";
        }
        return "";
    }

    bool in_same_scope( const log_scope *S ) const
    {
        return S != 0 &&
            S->zFileName == this->zFileName &&
                S->zFunctionName == this->zFunctionName;
    }

    std::string scope_info() const
    {
        return std::string( "* " ) + this->zFunctionName + "()";
    }

private:
    void save( const log_scope * );

private:
    const char      *zFileName;
    long            zLineNumber;
    const char      *zFunctionName;
    size_t          zScopeNum;
    log_sentence    zLogMsg;
};

#endif