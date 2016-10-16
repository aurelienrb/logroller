#include <QtGui/QApplication>
#include "mainwindow.h"

//#include <libelf.h>
#include <assert.h>
#include <gelf.h>
#include <fcntl.h>
#include <stdint.h>

#include <iostream>
#include <sstream>

class ostream_wrapper
{
public:
    ostream_wrapper(std::ostream & os, const char * fmt) : os_(os), paramCount(0), format(fmt)
    {
    }
    ~ostream_wrapper()
    {
        os_ << std::endl;
    }

    template<typename T>
    ostream_wrapper & operator<<(T t)
    {
        formatParam();
        os_ << t << ' ';
        return *this;
    }

    ostream_wrapper & operator<<(const QString & s)
    {
        formatParam();
        os_ << s.toStdString() << ' ';
        return *this;
    }

private:
    void formatParam()
    {
        ++paramCount;

        // hexa printing for that param?
        std::ostringstream oss;
        oss << "%x" << paramCount;
        if (format.find(oss.str()) != std::string::npos)
        {
            os_ << "0x" << std::hex;
        }
        else
        {
            os_ << std::dec;
        }
    }

private:
    std::ostream & os_;
    int paramCount;
    std::string format;
};

inline ostream_wrapper log_error(const char * msg)
{
    std::cerr << msg << ' ';
    return ostream_wrapper(std::cerr, msg);
}

inline ostream_wrapper log_trace(const char * msg)
{
    std::cout << msg << ' ';
    return ostream_wrapper(std::cout, msg);
}

struct log_message_info64
{
    uint64_t msg_data;
    uint16_t msg_index;
    bool is_enabled;
};

class ElfReader
{
public:
    ElfReader();
    ~ElfReader();

    bool open(const char * fileName);

    bool isElf()
    {
        return elf_kind(elf) == ELF_K_ELF;
    }

    bool is64bits()
    {
        assert(isElf());

        int result = gelf_getclass(elf);
        if (result == ELFCLASSNONE)
        {
            log_error("gelf_getclass() failed:") << elf_errmsg(-1);
        }
        return result == ELFCLASS64;
    }

    size_t getNbProgramHeaders()
    {
        size_t nb;
        if (elf_getphdrnum(elf, &nb) != 0)
        {
            log_error("elf_getphdrnum() failed:") << elf_errmsg(-1);
        }
        return nb;

    }

    Elf_Data * findSectionByName(const std::string & sectionName)
    {
        size_t sectionIndex;
        if (elf_getshdrstrndx(elf, &sectionIndex) != 0)
        {
            log_error("elf_getshdrstrndx() failed:") << elf_errmsg(-1);
            return NULL;
        }

        Elf_Scn * pSection = NULL;
        while ((pSection = elf_nextscn(elf, pSection)) != NULL)
        {
            GElf_Shdr sectionHeader;
            if (gelf_getshdr(pSection, &sectionHeader) != &sectionHeader)
            {
                log_error("gelf_getshdr() failed:") << elf_errmsg(-1);
                return NULL;
            }

            std::string name = elf_strptr(elf, sectionIndex, sectionHeader.sh_name);
            if (name == sectionName)
            {
                log_trace("Found section") << sectionName;

                // we found the wanted section
                //Elf_Data * elfData = elf_rawdata(pSection, NULL);
                Elf_Data * elfData = elf_getdata(pSection, NULL);
                if (elfData->d_type != ELF_T_BYTE)
                {
                    log_error("Unexpected section type:") << elfData->d_type;
                    return NULL;
                }
                return elfData;
            }
        }
        log_trace("Section %1 not found") << sectionName;
        return NULL;
    }

    Elf_Scn * findSectionByVirtualAddress(uint64_t vaddr)
    {
        size_t sectionIndex;
        if (elf_getshdrstrndx(elf, &sectionIndex) != 0)
        {
            log_error("elf_getshdrstrndx() failed:") << elf_errmsg(-1);
            return NULL;
        }

        Elf_Scn * pSection = NULL;
        while ((pSection = elf_nextscn(elf, pSection)) != NULL)
        {
            GElf_Shdr sectionHeader;
            if (gelf_getshdr(pSection, &sectionHeader) != &sectionHeader)
            {
                log_error("gelf_getshdr() failed:") << elf_errmsg(-1);
                return NULL;
            }

            if (vaddr >= sectionHeader.sh_addr &&
                vaddr <  sectionHeader.sh_addr + sectionHeader.sh_size)
            {
                const char * name = elf_strptr(elf, sectionIndex, sectionHeader.sh_name);
                log_trace("Found the section, name is '%1'") << name;
                return pSection;
            }
        }
        log_trace("Section containing address %x1 not found") << vaddr;
        return NULL;
    }

private:
    int fd;
    Elf * elf;
    //GElf_Ehdr header;
};

ElfReader::ElfReader() : fd(-1), elf(0)
{
}

ElfReader::~ElfReader()
{
    if (elf)
    {
        elf_end(elf);
    }
    if (fd >= 0)
    {
        close(fd);
    }
}

bool ElfReader::open(const char * fileName)
{
    if (elf_version(EV_CURRENT) == EV_NONE)
    {
        log_error("ELF library initialization failed:") << elf_errmsg(-1);
        return false;
    }

    int fd = ::open(fileName, O_RDONLY , 0);
    if (fd < 0)
    {
        log_error("Failed to open file") << fileName;
        return false;
    }
    log_trace("Opened file") << fileName;

    elf = elf_begin(fd, ELF_C_READ, NULL);
    if (elf == NULL)
    {
        log_error("elf_begin() failed:") << elf_errmsg(-1);
        return false;
    }
/*
    if (gelf_getehdr(elf, &ehdr) == NULL)
    {
        log_error("gelf_getehdr() failed:") << elf_errmsg(-1);
        return false;
    }
*/

    // Ok!
    log_trace("Ok!");
    return true;
}

class ElfSection
{
public:
    ElfSection(Elf_Scn * pSection, ElfReader & ) : elfData(NULL)
    {
        if (pSection)
        {
            if (gelf_getshdr(pSection, &elfHeader) != &elfHeader)
            {
                log_error("gelf_getshdr() failed:") << elf_errmsg(-1);
            }
            else
            {
                //name = elf_strptr(elf, sectionIndex, sectionHeader.sh_name);
                elfData = elf_getdata(pSection, NULL);
            }
        }
    }

    const unsigned char * data()
    {
        assert(elfData);
        return (unsigned char *)elfData->d_buf + elfData->d_off;
    }

    const char * virtualAddressToData(uint64_t vaddr)
    {
        if (vaddr <  elfHeader.sh_addr ||
            vaddr >= elfHeader.sh_addr + elfHeader.sh_size)
        {
            return NULL;
        }
        uint64_t offset = vaddr - elfHeader.sh_addr;

        assert(elfData);
        return (const char *)elfData->d_buf + elfData->d_off + offset;
    }

    GElf_Shdr elfHeader;
    Elf_Data * elfData;
    //std::string name;
};

bool readElf(const char *fileName)
{
    log_trace("readElf");

    ElfReader file;
    if (!file.open(fileName))
    {
        return false;
    }

    if (!file.isElf())
    {
        log_error("Not an ELF file!");
        return false;
    }

    if (Elf_Data * s = file.findSectionByName(".logidx"))
    {
        log_trace("Raw data size:") << s->d_size << "offset:" << s->d_off << "align:" << s->d_align;

        uint64_t nbElements = s->d_size / sizeof(log_message_info64);
        log_trace("Nb elements:") << nbElements;

        uint64_t remainingSize = s->d_size - sizeof(log_message_info64) * nbElements;
        log_trace("Remaining size:") << remainingSize;

        unsigned char *dataBegin = ((unsigned char *)s->d_buf) + s->d_off;
        log_message_info64 * elements = (log_message_info64*)dataBegin;

        if (nbElements == 0)
        {
            return true;
        }

        if (Elf_Scn * scn = file.findSectionByVirtualAddress(elements[0].msg_data))
        {
            ElfSection section(scn, file);

            if (section.elfData->d_type != ELF_T_BYTE)
            {
                log_error("Unexpected section type:") << section.elfData->d_type;
            }
            log_trace("Section address range: [%x1, %x2]") << section.elfHeader.sh_addr << section.elfHeader.sh_addr + section.elfHeader.sh_size;
            log_trace("Section size:") << section.elfHeader.sh_size;

            for (uint64_t i = 0; i < nbElements; ++i)
            {
                const char * data = section.virtualAddressToData(elements[i].msg_data);
                if (data)
                {
                    log_trace("Data %1: '%2") << i << data;
                }
                else
                {
                    log_error("Invalid vaddress for element %1: %2") << i << elements[i].msg_data;
                }
            }
            //log_trace("nb %1 = %2", int32(i), str(s))

        }
    }

    return true;
}

void log_strn(const char * str, size_t size) { std::cout << " " << std::string(str, size); }
void log_str(const char * str) { std::cout << " " << str; }
void log_int(int n) { std::cout << " " << n; }
void log_end(const char * params) { std::cout << "\t" << params << std::endl; }

#define LOG_ARG_COUNT(...) LOG_ARG_COUNT_(__VA_ARGS__, LOG_RSEQ_N())
#define LOG_ARG_COUNT_(...) LOG_ARG_N(__VA_ARGS__)
#define LOG_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define LOG_RSEQ_N() 8, 7, 6, 5, 4, 3, 2, 1, 0

#define CONCATENATE(arg1, arg2)   CONCATENATE1(arg1, arg2)
#define CONCATENATE1(arg1, arg2)  CONCATENATE2(arg1, arg2)
#define CONCATENATE2(arg1, arg2)  arg1##arg2

/* for some reason LOG_PARAM1 gets expanded with an empty parameter when no arg is provided
   so we need to define a token named log_ */
#define log_
#define LOG_PARAM3(p1, p2, p3) log_##p1 ; log_##p2 ; log_##p3 ; log_end("["#p1"] ["#p2"] ["#p3"]")
#define LOG_PARAM2(p1, p2) log_##p1 ; log_##p2 ; log_end("["#p1"] ["#p2"]")
#define LOG_PARAM1(p1) log_##p1 ; log_end("["#p1"]")
#define LOG_PARAMS(N, ...) CONCATENATE(LOG_PARAM, N)(__VA_ARGS__)

#define LOG(msg, ...) log_str(__PRETTY_FUNCTION__) ; log_str(__FILE__ "\t" msg); LOG_PARAMS(LOG_ARG_COUNT(__VA_ARGS__), __VA_ARGS__)

#define LOG1(msg, param1) log_str(msg); LOG_PARAM1(param1)

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    LOG("Hello");
    LOG("Hello", int(1));
    LOG("Hello", int(1), str("Hello"));
    LOG("Hello", strn("ABCDE", 3));
    LOG("Hello", int(1), int(2), int(3));

    #define DEBUG ".debug "
    LOG1(DEBUG "Hello", str("World"));

    //readElf("../lib/logtest");
    
    return a.exec();
}
