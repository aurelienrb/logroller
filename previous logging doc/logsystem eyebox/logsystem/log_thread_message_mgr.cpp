#include "log_thread_message_mgr.h"
#include "log_message_pool.h"

#include <stack>
#include <vector>
#include <cassert>

class log_thread_message_mgr::pimpl
{
public:
    log_message_pool                        pool;
    std::stack<std::pair<const char*,int> > levels;
};

namespace
{
    std::string print_msg_in_html( const log_message *Msg )
    {
        switch ( Msg->type )
        {
        case ENTER_FUNCTION:
            return "<ul>\n";
        case LEAVE_FUNCTION:
            return "</ul>\n";
        case TRACE:
            return std::string( "<li>" ) + Msg->get_message() + "</li>\n";
        }
        assert( 0 );
        return "";
    }
}

log_thread_message_mgr::log_thread_message_mgr()
{
    this->p_ = new pimpl;
}

log_thread_message_mgr::~log_thread_message_mgr()
{
    delete this->p_;
}

void log_thread_message_mgr::enter_function( const char * function_name, const std::string & Msg )
{
    assert( function_name );

    bool new_function = true;
    if ( !this->p_->levels.empty() )
    {
        // on entre dans une fonction différente de l'actuelle ?
        std::pair<const char*, size_t> top = this->p_->levels.top();

        if ( function_name == top.first ||
             strcmp( function_name, top.first ) == 0 )
        {
            // non : on incrémente simplement le compteur d'entrée de la fonction actuelle
            this->p_->levels.pop();
            ++top.second;
            this->p_->levels.push( top );

            new_function = false;
        }
    }

    if ( new_function )
    {
        // on entre dans une fonction
        this->p_->levels.push( std::pair<const char*, size_t>( function_name, 1 ) );

        // générer un message
        log_message *enter_msg = this->p_->pool.alloc();
        enter_msg->type = ENTER_FUNCTION;
        enter_msg->set_message( function_name );
        enter_msg->level = this->p_->levels.size() - 1;
    }                

    // message en lui même
    log_message *msg = this->p_->pool.alloc();
    msg->type = TRACE;
    msg->set_message( Msg );
    msg->level = this->p_->levels.size() - 1;
}

void log_thread_message_mgr::leave_function( const char * function_name )
{
    assert( !this->p_->levels.empty() );
    std::pair<const char*, size_t> top = this->p_->levels.top();
    assert( top.first == function_name || strcmp( top.first, function_name ) == 0 );
    this->p_->levels.pop();
    if ( top.second > 1 )
    {
        --top.second;
        this->p_->levels.push( top );
    }
    else
    {
        // on a quitté la fonction: générer un message
        log_message *leave_msg = this->p_->pool.alloc();
        leave_msg->type = LEAVE_FUNCTION;
        leave_msg->set_message( function_name );
        leave_msg->level = this->p_->levels.size();
    }
}

std::string log_thread_message_mgr::dump_html()
{
    std::string dump;
    const log_message *msg = this->p_->pool.get_first();
    while ( msg )
    {
        dump += print_msg_in_html( msg );
        msg = this->p_->pool.get_next( msg );
    }
    return dump;
}

log_message_pool* log_thread_message_mgr::get_pool()
{
    return &this->p_->pool;
}

const log_message_pool * log_thread_message_mgr::get_pool() const
{
    return &this->p_->pool;
}
