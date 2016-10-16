#include "logroller.h"

#include <assert.h>

/*#include <pthread.h>
#define INLINE __inline__*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <libelf.h>

#if 0
/* compteur global */
static unsigned long nb_threads = 0;

typedef unsigned char log_thread_id;
#define LOG_MAX_THREAD UCHAR_MAX

static __thread bool this_thread_init_done = false;

/* Renvoie l'identifiant du thread courant à utiliser dans les logs.
    Supporte au maximum LOG_MAX_THREAD threads. Si cette limite est
    atteinte, renvoie LOG_MAX_THREAD comme identifiant des futurs threads. */
log_thread_id get_current_thread_id()
{
    /* compteur local à chaque thread */
    static __thread unsigned long current_thread_id = 0;

    if (current_thread_id == 0) {
        current_thread_id = atomic_inc(&nb_threads);
    }
    /* tronquer volontairement à LOG_MAX_THREAD */
    return (log_thread_id) current_thread_id;
}
*/
#endif

struct log_roll
{
    uint32_t capacity;
    uint32_t size;
    char * buffer;
};

struct log_entry16 * log_roller_alloc_entry16(
    struct log_roll * roll,
    enum entry_type type,
    uint16_t info,
    uint16_t value,
    unsigned short extra_size)
{
    struct log_entry16 *result;
    uint32_t required_size;

    /*if (!this_thread_init_done)*/
    {
        /* That's the first time we log from this thread: generate a system message
           giving details about this current thread */
    }

    /* alloc a new entry */
    required_size = sizeof(struct log_entry16) + extra_size;
    if (roll->size + required_size > roll->capacity)
    {
        roll->size = 0;
    }
    roll->size += required_size;

    assert(extra_size == 0 || value == extra_size);
    result = (struct log_entry16*)&roll->buffer[roll->size];
    result->is_extended = (extra_size != 0);
    result->type = type;
    result->info = info;
    result->value = value;
    return result;
}

/* The final mark is an int64_t we put at the end of the .logidx section.
 * It is used to easily compute a relative index for each log_message_info struct
 * preceding that mark.
 * The final mark expected value is 0x0123456789ABCDEF, which is useful to detect
 * any endianness issues. */
static const int64_t * get_final_mark_address()
{
    extern int64_t logidx_section_final_mark;
    __asm__(".pushsection .logidx, 1, \"aw\",@progbits");
    /*__asm__("msg_info_end_mark: .byte 0");*/
    __asm__("logidx_section_final_mark: .quad 0x0123456789ABCDEF");
    __asm__(".popsection");

    return &logidx_section_final_mark;
}

/* If we consider the .logidx section as an array of log_message_info, this
 * function returns what can be considered as an iterator that points
 * just beyond the end of the array (C++ STL like). */
static INLINE const struct log_message_info * get_message_info_end()
{
    return (const struct log_message_info *)get_final_mark_address();
}

static uint16_t compute_message_index_from_address(const struct log_message_info * msg_info_ptr)
{
    assert(msg_info_ptr < get_message_info_end());
    {
      uint32_t ptr_diff = get_message_info_end() - msg_info_ptr;
      /*printf("diff: %d\n", ptr_diff);*/
      assert(ptr_diff < 0xFFFF);
      return (uint16_t)ptr_diff;
    }
#if 0
    int i, fd;
    Elf * e;
    Elf_Kind ek;
    /*GElf_Ehdr ehdr;*/

    if (elf_version(EV_CURRENT) == EV_NONE)
    {
        /* ELF library initialization failed */
        elf_errmsg(-1);
        return 0xFFFF;
    }
    if ((fd = open("/proc/self/exe", O_RDONLY , 0)) < 0)
    {
        return 0xFFFF;
    }

    if ((e = elf_begin(fd, ELF_C_READ, NULL )) == NULL)
    {
        return 0xFFFF;
    }

    ek = elf_kind(e);
    if (ek != ELF_K_ELF)
    {
        return 0xFFFF;
    }

    if (gelf_getehdr(e, &ehdr) == NULL)
    {
        return 0xFFFF;
    }

    i = gelf_getclass(e);
    if (i == ELFCLASS32)
    {
        Elf32_Shdr *elf32_getshdr
    }
    else (i == ELFCLASS64)
    {
        Elf64_Shdr *elf64_getshdr
    }
    else
    {
        return 0xFFFF;
    }

    elf_end(e);
    close(fd);
    return 0xFFFF;
#endif
}

static const struct log_message_info * retrieve_address_from_message_index(uint16_t log_index)
{
    assert(log_index != 0xFFFF);
    return get_message_info_end() - log_index;
}

static void print_log_message(const struct log_message_info * msg_info)
{
    const char *msg = NULL, *file = NULL, *line = NULL;

    if (msg_info != NULL)
    {
        msg = msg_info->msg_data;
        file = msg + strlen(msg) + 1;
        line = file + strlen(file) + 1;
    }
    printf("[%s:%s] %s\n", file, line, msg);
}


struct log_roll g_roll = { 0, 0, 0};

void log_message(struct log_message_info * msg_info)
{
    if (g_roll.capacity == 0)
    {
      g_roll.capacity = 1000;
      g_roll.buffer = malloc(g_roll.capacity);
    }
    if (msg_info->msg_index == 0)
    {
        msg_info->msg_index = compute_message_index_from_address(msg_info);
        /*printf("Computed index: %d\n", msg_info->msg_index);*/
    }
    assert(msg_info->msg_index != 0xFFFF);
    if (msg_info->is_enabled)
    {
        struct log_entry16 * entry16;

        entry16 = log_roller_alloc_entry16(&g_roll, SCOPE_ENTER, 0, msg_info->msg_index, 0);

        print_log_message( retrieve_address_from_message_index(entry16->value) );
    }
}

/*
INLINE char * get_checked_entry_buffer_ptr(struct log_entry * entry, uint16_t param_size)
{
    assert(param_size <= entry->u16.param_size);
    return (char*)entry + sizeof(struct log_entry);
}
#define get_entry_buffer_ptr(entry, type) (type*)get_checked_entry_buffer_ptr(entry, sizeof(type))


INLINE void
log_param_embeded(struct log_roll * roll, uint16_t param_value, enum log_param_type param_type)
{
    struct log_entry *entry;

    assert(param_type < SYSTEM);

    entry = log_roller_alloc_entry(roll, PARAM_EMBEDED, 0);
    entry->u4.param_type = param_type;
    entry->u16.param_value = param_value;
}

static INLINE void
log_param_int4(struct log_roll * roll, uint32_t param_value, bool is_signed)
{
    if (param_value < 0x0000FFFF)
    {
        log_param_embeded(roll, param_value, is_signed ? INT16 : UINT16);
    }
    else
    {
        struct log_entry *entry = log_roller_alloc_entry(roll, PARAM_EXTENDED, 4);
        assert(entry->u16.param_size == 4);
        entry->u4.param_type = is_signed ? INT_SIZEX : UINT_SIZEX;
        *get_entry_buffer_ptr(entry, uint32_t) = param_value;
    }
}

struct log_roll g_roll;

void log_param_int32(int32_t param)
{
    log_param_int4(&g_roll, param, true);
}

void log_param_uint32(uint32_t param)
{
    log_param_int4(&g_roll, param, false);
}

void log_scope_enter(const struct log_entry_point * entry_point)
{
    const char *msg, *file, *line;

    msg = entry_point->str;
    file = msg + strlen(msg) + 1;
    line = file + strlen(file) + 1;

    printf("[%s:%s] %s\n", file, line, msg);
}
*/
/*


template<typename T>
inline log_header_t * add_new_entry_embeded(header_type_e type, T value)
{
      log_header_t *header = alloc_entry(PARAM_EMBEDED);
      assert(header->header_data == 0);
      header->header_type_specific_info = type;
      header->header_data = static_cast<short unsigned>(value);
      assert(static_cast<short unsigned>(value) == header->header_data);
      return header;
}

template<typename T>
inline log_header_t * add_new_entry_param4(header_type_e type, T value)
{
      log_header_t *header = alloc_entry(PARAM_EXTENDED, 4);
      assert(header->header_data == 4);
      header->header_type_specific_info = type;
      *get_param_buffer<unsigned long>(header) = static_cast<unsigned long>(value);
      return header;
}

template<typename T>
inline log_header_t * add_new_entry_param8(header_type_e type, T value)
{
      log_header_t *header = alloc_entry(PARAM_EXTENDED, 8);
      assert(header->header_data == 8);
      header->header_type_specific_info = type;
      *get_param_buffer<uint64_t>(header) = static_cast<uint64_t>(value);
      return header;
}

inline log_header_t * log_param(short param)
{
    return add_new_entry_embeded(INT16, param);
}

inline log_header_t * log_param(unsigned short param)
{
    return add_new_entry_embeded(UINT16, param);
}

#ifdef IS32BITS
log_header_t * log_param(unsigned long);

inline log_header_t * log_param(long param)
{
    log_header_t * header = log_param(static_cast<unsigned long>(param));
    header->header_type_specific_info -= 1;
    return header;
}
#endif

log_header_t * log_param(uint64_t);
inline log_header_t * log_param(int64_t param)
{
    log_header_t * header = log_param(static_cast<uint64_t>(param));
    header->header_type_specific_info -= 1;
    return header;
}

log_header_t * log_param(unsigned long param)
{
    if (param <= 0x0000FFFF)
    {
        return add_new_entry_embeded(UINT16, param);
    }
    else
    {
        return add_new_entry_param4(UINT32, param);
    }
}

log_header_t * log_param(uint64_t param)
{
    if (param <= 0x00000000FFFFFFFFL)
    {
        if (param <= 0x000000000000FFFFL)
        {
            return add_new_entry_embeded(UINT16, param);
        }
        else
        {
            return add_new_entry_param4(UINT32, param);
        }
    }
    else
    {
        return add_new_entry_param8(UINT64, param);
    }
}

void log_param(const char *str, unsigned short len)
{
    log_header_t * header = alloc_entry(PARAM_EXTENDED, len);
    header->header_type_specific_info = STRING_CHAR8;
    if (len > 0)
    {
        memcpy(str, get_param_buffer<char>(header), header->header_data);
    }
}

void log_param(const unsigned short *str, unsigned short len)
{
    log_header_t * header = alloc_entry(PARAM_EXTENDED, len * 2);
    header->header_type_specific_info = STRING_WCHAR16;
    if (len > 0)
    {
        memcpy(str, get_param_buffer<char>(header), header->header_data);
    }
}

void log_param(const unsigned long *str, unsigned short len)
{
    log_header_t * header = alloc_entry(PARAM_EXTENDED, len * 4);
    header->header_type_specific_info = STRING_WCHAR32;
    if (len > 0)
    {
        memcpy(str, get_param_buffer<char>(header), header->header_data);
    }
}
*/
