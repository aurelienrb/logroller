/*
*  log_so_fast.c - Example of a backend for a "lazy logger", in C++
*  (C) Copyright 2007 Aurelien Regat-Barrel
*  For more information, please visit http://www.logroller.org
*
*  This sotware is provided "as is". You can freely copy, modify
*  or redistribute it.
*/

#include <stdio.h>

void log( const char *(Text&)[] );
void log_dump_bin( std::ostream & );

void log_format_bin( std::istream &, std::ostream & );

void install_posix_handler();

std::string backend_cmd;

void posix_handler()
{
    LOG( "executing posix_handler()" );
    
    // ooops, we are dieing, save the bin log
    std::ofstream bin_log( "crash.bin" );
    if ( bin_log )
    {
        log_dump_bin( bin_log );
        bin_log.close();
    }
    if ( bin_log )
    {
        // ok, restart ourself as a backend
        system( backend_cmd.c_str() );
    }
    // and die
    exit( -1 );
}

int main( int argc, char* argv[] )
{
    if ( argc == 2 )
    {
        if ( string( argv[1] ) == "-reportcrash" )
        {
            std::ifstream bin_log( "crash.bin" );
            if ( !bin_log )
            {
                return 1;
            }
            std::cout << "A crash occured, the following log file was generated:\n";
            log_format_bin( bin_log, cout );
            bin_log.close();
            unlink( "crash.bin" );
            std::cout << "Press enter to exit...\n";
            cin.ignore();
            return 0;
        }
    }
    install_posix_handler();
    
    LOG( "We are going to crash..." );
    int *ptr = 0;
    *ptr = 0; // bad memory access
    return 0;
}
