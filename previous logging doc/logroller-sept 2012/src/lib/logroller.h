/*
*   LICENSE
*/
#ifndef LOGROLLER_H
#define LOGROLLER_H

/*
 * Linux dependencies: libelf-dev
 *
 *
*/

/*
*   logroller.h - Core of the logging infrastructure: data types definition, memory management, serialization.
*
*   This file is meant to be compatible with C89 and C++98. It must compile with "gcc -ansi -pedantic-errors".
*
*   Version: 1.0
*   Date: $date$
*
*   Verified systems & compilers:
*
*   =========================================================
*   x86 (16 bits)      FreeDOS         DJGPP       NOT TESTED
*   =========================================================
*   x86 (32 bits)       Windows XP     VC++ 2008   NOT TESTED
*                                      VC++ 2010   NOT TESTED
*                                      VC++ 2012   NOT TESTED
*                                      MinGW       NOT TESTED
*                                      Borland     NOT TESTED
*                       -------------------------------------
*                       Linux 3.2      GCC 4.6.3   NOT TESTED
*                                      Intel C++   NOT TESTED
*   =========================================================
*   x86-64 (64 bits)    Windows 8      VC++ 2008   NOT TESTED
*                                      VC++ 2010   NOT TESTED
*                                      VC++ 2012   NOT TESTED
*                                      MinGW       NOT TESTED
*                                      Borland     NOT TESTED
*                       -------------------------------------
*                       Linux 3.2      GCC 4.6.3   NOT TESTED
*   =========================================================
*/

#include <stdbool.h>


#ifdef _MSC_VER
    #define LOGROLLER_BEGIN_PACK __pragma( pack(push, 1) )
    #define LOGROLLER_END_PACK   __pragma( pack(pop) )
#elif defined(__GNUC__)
    #define LOGROLLER_BEGIN_PACK
    #define LOGROLLER_END_PACK   __attribute__((packed))

    #include <stdint.h> /* bool, uint16_t */
#endif

/* 4 values: 2 bits for storage */
enum log_param_type16
{
/* Some parameters can be directly embeded in the log_entry16 header depending
*  on their size / value. BOOL1 is always embeded. INT and UINT
*  are eligible if their actual value can be encoded
*  with 10 bits or less (i.e. if UINT < 1024). STRING8 as well if it is empty
*  or has a single character (CHAR are stored as STRING8 of length 1).
*  The is_extended field tells if the param was embeded in the log_entry header
*  or if it is attached in a following buffer whose size is specified
*  in the value field.
*  As a result, the size of an attached param should be always >= 2.
*/
    INT,
    UINT,
    REAL,
    STRING8 /* caution: nul terminator is not encoded */
};

enum entry_type
{
    SCOPE_ENTER,
    SCOPE_LEAVE,
    MSG_PARAM,
    CUSTOM
};

/*
*  log_entry 16 bits
*  To be used in restricted / embedded systems.
*  No unicode string support (only narrow/ansi strings).
*  No thread id: logging is thread safe, but messages are merged all together.
*  Number max of distinct log messages: 8192.
*  Max size of each parameter value: 2048.
*/
LOGROLLER_BEGIN_PACK
struct log_entry16
{
    bool is_extended : 1; /* if true, the structure is bigger than 2 octets */
    uint32_t type    : 2; /* entry_type */
    uint32_t info    : 2; /* type specific info */
    uint32_t value   :11; /* if is_extended: size of the following buffer. */
                          /* else: type specific info */
}
LOGROLLER_END_PACK;

struct log_message_info
{
    const char * msg_data;
    uint16_t msg_index;
    bool is_enabled;
};

#define STR_EXPAND(tok) #tok
#define MAKESTR(tok) STR_EXPAND(tok)

#define CONCAT2(x, y) x##y
#define CONCAT(x,y) CONCAT2(x, y)
#define BUILD_NAME(x) CONCAT(x,__LINE__)

#define put_in_section(decl, name)\
    decl __attribute__ ((section (name)))

#define log_str_const(str) \
  static const char\
  put_in_section(BUILD_NAME(data_str)[], ".logdata") = str "\0" __FILE__ "\0" MAKESTR(__LINE__) "\0",\
  put_in_section(* BUILD_NAME(data_ref), ".logidx") = BUILD_NAME(data_str);\
  log_message(BUILD_NAME(data_ref));

#define log_str(str) \
  static const char\
  put_in_section(BUILD_NAME(data_str)[], ".logdata") = str "\0" __FILE__ "\0" MAKESTR(__LINE__) "\0";\
  static struct log_message_info put_in_section(BUILD_NAME(data_ref), ".logidx") = { BUILD_NAME(data_str), 0, true };\
  log_message(&BUILD_NAME(data_ref));

#define log_debug(msg) log_str(".debug " msg)

void log_message(struct log_message_info * info);

#endif
