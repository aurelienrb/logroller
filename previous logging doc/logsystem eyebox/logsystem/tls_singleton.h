#ifndef TLS_SINGLETON_H
#define TLS_SINGLETON_H

#include <list>

#ifdef _DEBUG
#include <cassert>
#include <algorithm>
#endif

class tls_index
{
public:
    tls_index();
    ~tls_index(); 
    void set( void* );
    void* get() const;

private:
    unsigned long zTlsIndex;
};

template<typename T>
class tls_singleton
{
public:
    ~tls_singleton() // détruit les instances allouées non détruites
    {
        this->delete_all_instances();
    }

    void set_this_thread_instance( T *t )
    {
        this->zIndex.set( t );
        this->zInstances.push_back( t );
    }

    T* get_this_thread_instance() const
    {
        return static_cast<T*>( this->zIndex.get() );        
    }

    void delete_this_thread_instance()
    {
        T *t = this->get_this_thread_instance();
        this->zIndex.set( 0 );
#ifdef _DEBUG
        std::list<T*>::iterator i = std::find(
            this->zInstances.begin(),
            this->zInstances.end(),
            t );
        assert( i != this->zInstances.end() );
#endif
        this->zInstances.remove( t );
        delete t;
    }

    std::list<T*> get_undeleted_instances() const
    {
        return this->zInstances;
    }

    void delete_all_instances()
    {
        for ( std::list<T*>::iterator i = this->zInstances.begin(),
              end = this->zInstances.end(); i != end; i++ )
        {
            delete *i;
        }
        this->zInstances.clear();
    }

private:
    std::list<T*>   zInstances;    
    tls_index       zIndex;
};

#endif