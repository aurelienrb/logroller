

void Test()
{
    for ( int i = 0; i < 10; ++i )
    {
        log_iteration( i );
    }
}

int Recursive( int N )
{
    log_reentrance();
    log_performance();

    if ( N == 0 )
    {
        return N;
    }
    return N + Recursive( N - 1 );
}

int Recursive( int N )
{
    reentrance_logger r;
    log_
    assert( r.deepness() < 20 );

    static const location_infos infos_line28 = {};
    static per_thread_counter counter( &infos_line28 );
    static per_thread_infos<&infos_line28> this_infos;

    static int counter = 0;

    static const loc_infos
    reentrance_tracker check_point;
    log_trace() << check_point.current_reentrance_level();


    if ( N == 0 )
    {
        return N;
    }
    return N + Recursive( N - 1 );
}