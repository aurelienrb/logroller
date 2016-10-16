#include "log_message_pool.h"

#include <cassert>

log_message_pool::log_message_pool():
    first_msg_num( 0 ),
    msg_nb( 0 )
{
    this->set_pool_size_in_number_of_msg( 1000 );
}

void log_message_pool::set_pool_size_in_kb( size_t Kb )
{
    size_t nb_msg = ( Kb * 1024 ) / sizeof( log_message );
    this->set_pool_size_in_number_of_msg( nb_msg );
}

void log_message_pool::set_pool_size_in_number_of_msg( size_t Nb )
{
    this->pool.resize( Nb );
    this->first_msg_num = 0;
    this->msg_nb = 0;
}

log_message* log_message_pool::alloc()
{
    assert( this->first_msg_num == 0 || this->msg_nb == this->pool.size() );

    size_t max_nb = this->pool.size();
    // si plus de place
    if ( this->msg_nb == max_nb )
    {
        size_t old_first = this->first_msg_num;
        this->first_msg_num = ( this->first_msg_num + 1 ) % max_nb;
        assert( this->get_last() == &this->pool[ old_first ] );
    }
    else
    {
        assert( this->msg_nb < max_nb );
        ++this->msg_nb;
        assert( this->get_last() != 0 );
    }
    return const_cast<log_message*>(
        this->get_last() );
}

size_t log_message_pool::get_number_of_msg() const
{
    return this->msg_nb;
}

const log_message* log_message_pool::get_first() const
{
    assert( this->first_msg_num == 0 || this->msg_nb == this->pool.size() );
    if ( this->msg_nb == 0 )
    {
        return 0;
    }
    return &this->pool[ this->first_msg_num ];
}

const log_message* log_message_pool::get_last() const
{
    assert( this->first_msg_num == 0 || this->msg_nb == this->pool.size() );
    if ( this->msg_nb == 0 )
    {
        return 0;
    }
    size_t last_nb = ( this->first_msg_num + this->msg_nb - 1 ) % this->pool.size();
    return &this->pool[ last_nb ];
}

const log_message* log_message_pool::get_next( const log_message *Msg ) const
{
    assert( this->first_msg_num == 0 || this->msg_nb == this->pool.size() );

    if ( this->msg_nb == 0 )
    {
        assert( 0 );
        return 0;
    }

    // valider le pointeur
    if ( Msg < &this->pool[ 0 ] )
    {
        assert( 0 );
        return 0;
    }
    else if ( Msg > &this->pool[ this->msg_nb - 1 ] )
    {
        assert( 0 );
        return 0;
    }
    
    // chercher le suivant
    if ( Msg < this->get_last() )
    {
        return Msg + 1;
    }
    else if ( Msg == this->get_last() )
    {
        return 0;
    }
    else
    {
        assert( this->msg_nb == this->pool.size() );
        if ( Msg < &this->pool[ this->msg_nb - 1 ] )
        {
            return Msg + 1;
        }
        return &this->pool[ 0 ];
    }
}

const log_message* log_message_pool::get_previous( const log_message *Msg ) const
{
    assert( this->first_msg_num == 0 || this->msg_nb == this->pool.size() );

    if ( this->msg_nb == 0 )
    {
        assert( 0 );
        return 0;
    }

    // valider le pointeur
    if ( Msg < &this->pool[ 0 ] )
    {
        assert( 0 );
        return 0;
    }
    else if ( Msg > &this->pool[ this->msg_nb - 1 ] )
    {
        assert( 0 );
        return 0;
    }
    
    // chercher le précédent
    if ( Msg > this->get_first() )
    {
        return Msg - 1;
    }
    else if ( Msg == this->get_first() )
    {
        return 0;
    }
    else
    {
        assert( this->msg_nb == this->pool.size() );
        if ( Msg > &this->pool[ 0 ] )
        {
            return Msg - 1;
        }
        return &this->pool[ this->pool.size() - 1 ];
    }
}

