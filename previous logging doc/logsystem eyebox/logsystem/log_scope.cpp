#include "log_scope.h"

#include <vector>
#include <cassert>

namespace
{
// tableau géré à la manière d'une stack, mais qu'on peut itérer
std::vector<log_scope*> this_thread_scopes;
}

const log_scope * log_scope::GetThisThreadRootScope()
{
    if ( this_thread_scopes.empty() )
    {
        return 0;
    }
    return this_thread_scopes.front();
}

const log_scope * log_scope::GetThisThreadCurrentScope()
{
    if ( this_thread_scopes.empty() )
    {
        return 0;
    }
    return this_thread_scopes.back();
}

const log_scope * log_scope::Previous( const log_scope *S )
{
    if ( this_thread_scopes.at( S->zScopeNum ) != S )
    {
        // S est invalide, il s'agit très certainement d'un objet détruit
        throw std::invalid_argument( "Invalid log_scope*" );
    }
    if ( S->zScopeNum > 0 )
    {
        return this_thread_scopes[ S->zScopeNum - 1 ];
    }
    return 0;
}

const log_scope * log_scope::Next( const log_scope *S )
{
    if ( this_thread_scopes.at( S->zScopeNum ) != S )
    {
        // S est invalide, il s'agit très certainement d'un objet détruit
        throw std::invalid_argument( "Invalid log_scope*" );
    }
    size_t size = this_thread_scopes.size();
    if ( size > 1 && S->zScopeNum < size - 1 )
    {
        return this_thread_scopes[ S->zScopeNum + 1 ];
    }
    return 0;}


log_scope::log_scope( const char FileName[], long LineNumber, const char FunctionName[] ):
    zFileName( FileName ),
    zLineNumber( LineNumber ),
    zFunctionName( FunctionName )
{
    assert( FileName );
    assert( FunctionName );
    this->zScopeNum = this_thread_scopes.size();
    this_thread_scopes.push_back( this );
}

log_scope::~log_scope()
{
    assert( this_thread_scopes.back() == this );
    this_thread_scopes.pop_back();
    if ( !this_thread_scopes.empty() )
    {
        this_thread_scopes.back()->save( this );
    }
    this->zScopeNum = static_cast<size_t>( -1 );
}

log_scope& log_scope::operator=( log_sentence & S )
{
    assert( !S.empty() );
    this->zLogMsg.swap( S );
    return *this;
}

void log_scope::save( const log_scope *S )
{
    this->zLogMsg.merge( S->zLogMsg );
}