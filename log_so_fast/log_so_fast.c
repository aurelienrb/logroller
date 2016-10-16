/*
*  log_so_fast.c - Demonstrate the concept of "lazy logging", in C
*  (C) Copyright 2007 Aurelien Regat-Barrel
*  For more information, please visit http://www.logroller.org
*
*  This sotware is provided "as is". You can freely copy, modify
*  or redistribute it.
*/

#include <stdio.h>

/* structure to hold information to log */
typedef struct
{
    const char *file_name;
    const char *log_text;
} log_msg_t;

/* log buffer where log msg are stored */
#define BUFFER_SIZE 100
static const log_msg_t * log_msg_buffer[ BUFFER_SIZE ];
static unsigned int nb_msg = 0;

/* function to add a msg in the log buffer */
void log_so_fast( const log_msg_t *new_msg )
{
    if ( nb_msg < BUFFER_SIZE )
    {
        log_msg_buffer[ nb_msg ] = new_msg;
        ++nb_msg;
    }
}

/* function to print the content of the log buffer to a stream */
void dump_log( FILE *output )
{
    unsigned int i;
    for ( i = 0; i < nb_msg; ++i )
    {
        fprintf( output, "%s\n", log_msg_buffer[ i ]->log_text );
    }
}

/* demonstrate the basic usage */
void test1(void)
{
    static const log_msg_t msg_line38 = { __FILE__, "Hello from test1()" };
    log_so_fast( &msg_line38 );
}

/* demonstrate a possible use of a macro LOG() to simplify the code */
#define LOG(x)\
    {static log_msg_t msg_line__LINE__ = { __FILE__, x };\
    log_so_fast( &msg_line__LINE__ );}

void test2(void)
{
    LOG( "Hello from test2()" );
    LOG( "Hello again from test2()" );
}

int main()
{
    test1();
    test2();
    dump_log( stdout );
    return 0;
}