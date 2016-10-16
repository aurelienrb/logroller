#include <iostream>
#include <string>
#include <sstream>
#include <list>
#include <map>
#include <vector>
#include <cassert>
#include <stack>
using namespace std;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define CPP11_MOVE_REF &&
#define UNUSED(arg) (void)arg

class log_output
{
public:
    virtual ~log_output() {}
    virtual void append(const std::string & text) = 0;

protected:
    log_output() {}

private:
    log_output(const log_output &);
    log_output & operator=(const log_output &);
};
typedef log_output * log_output_ptr;

class log_styled_output : public log_output
{
public:
    virtual void beginline() = 0;
    virtual void endline() = 0;
    virtual void push_style(const std::string & style) = 0;
    virtual void pop_style() = 0;
};
typedef log_styled_output * log_styled_output_ptr;

std::string skip_char(const std::string & s, char c)
{
    std::string result;
    result.reserve(s.size());

    for (std::string::const_iterator it = s.begin();
        it != s.end();
        it++)
    {
        if (*it != c) result += *it;
    }
    return result;
}

static void erase_spaces(std::vector<std::string> & strings, bool delete_empty = false)
{
    size_t nb_empty = 0;
    for (size_t i = 0; i < strings.size(); ++i)
    {
        strings[i].swap(
            skip_char(strings[i], ' '));
        if (strings[i].empty())
        {
            ++nb_empty;
        }
    }

    if (nb_empty > 0 && delete_empty)
    {
        size_t offset = 0;
        std::vector<std::string> result(strings.size() - nb_empty);
        for (size_t i = 0; i < strings.size(); ++i)
        {
            if (!strings[i].empty())
            {
                result[offset].swap(strings[i]);
                offset += 1;
            }
        }
        assert(offset == nb_empty);
        strings.swap(result);
    }
}

int to_int(const std::string & s, int value_if_error = 0)
{
    int result;
    std::istringstream iss(s);
    if (iss >> result)
    {
        return result;
    }
    return value_if_error;
}

class console_output : public log_styled_output
{
public:
    enum color_t
    {
        black = 0,
        darkblue = 1,
        darkgreen = 2,
        darkcyan = 3,
        darkred = 4,
        magenta = 5,
        brown = 6,
        grey = 7,
        darkgrey = 8,
        blue = 9,
        green = 10,
        cyan = 11,
        red = 12,
        pink = 13,
        yellow = 14,
        white = 15,
    };
    std::map<std::string, color_t> color_map;

    console_output()
    {
        color_map["black"] = black;
        color_map["darkblue"] = darkblue;
        color_map["darkgreen"] = darkgreen;
        color_map["darkcyan"] = darkcyan;
        color_map["darkred"] = darkred;
        color_map["magenta"] = magenta;
        color_map["brown"] = brown;
        color_map["grey"] = grey;
        color_map["darkgrey"] = darkgrey;
        color_map["blue"] = blue;
        color_map["green"] = green;
        color_map["cyan"] = cyan;
        color_map["red"] = red;
        color_map["pink"] = pink;
        color_map["yellow"] = yellow;
        color_map["white"] = white;

        current_style = "grey";
    }

    void set_color(color_t color)
    {
        ::SetConsoleTextAttribute(
            ::GetStdHandle(STD_OUTPUT_HANDLE),
            static_cast<WORD>(color));
    }

    void set_color(const std::string & color)
    {
        std::map<std::string, color_t>::const_iterator it = color_map.find(color);
        if (it != color_map.end())
        {
            set_color(it->second);
        }
    }
#if 0
    console_output(std::string styles)
    {
        std::list<std::string> l;

        std::string::size_type pos = styles.find(';');
        while (styles.size() > 1 && pos != std::string::npos)
        {
            if (pos > 0)
            {
                l.push_back(
                    styles.substr(0, pos));
            }
            styles = styles.substr(pos + 1);
            pos = styles.find(';');
        }
        if (!styles.empty())
            l.push_back(styles);

        
        for (std::list<std::string>::const_iterator it = l.begin();
            it != l.end();
            it++)
        {
            const std::string & s = *it;
            std::string::size_type pos = s.find(':');
            if (pos == std::string::npos || pos == 0 || pos == s.size() - 1)
            {
                continue;
            }

            std::string name = skip_char(s.substr(0 , pos), ' ');
            std::string str_value = skip_char(s.substr(pos + 1), ' ');
            if (name.empty() || str_value.empty())
            {
                continue;
            }
            int int_value;
            if (!to_int(str_value, int_value))
            {
                continue;
            }
            attributes[name] = int_value;
        }
    }
#endif
private:
    virtual void append(const std::string & text)
    {
        std::cout << text;
    }
    virtual void beginline()
    {
    }
    virtual void endline()
    {
        std::cout << std::endl;
    }
    virtual void push_style(const std::string & style)
    {
        styles.push(current_style);
        // if the received style is nul, we re-use the current style
        if (!style.empty())
        {            
            current_style = style;
        }        
        set_color(current_style);
    }
    virtual void pop_style()
    {
        assert(!styles.empty());
        current_style = styles.top();
        styles.pop();
        set_color(current_style);
    }

private:
    std::string current_style;
    std::stack<std::string> styles;
};


typedef std::map<std::string, std::string> log_msg_t;

class log_formater
{
public:
    virtual ~log_formater() {}
    virtual void format(log_msg_t & msg) = 0;

protected:
    log_formater() {}

private:    
    log_formater(const log_formater &);
    log_formater & operator=(const log_formater &);
};
typedef log_formater * log_formater_ptr;

static std::vector<std::string> explode(const std::string & s, char separator, bool skip_empty = false)
{
    std::vector<std::string> result;

    std::string remaining = s;
    while (!remaining.empty())
    {
        std::string::size_type pos = remaining.find(separator);
        if (pos == std::string::npos)
        {
            break;
        }
        if (pos == 0)
        {
            if (!skip_empty)
            {
                result.push_back(std::string());
            }
        }
        else
        {
            result.push_back(
                remaining.substr(0, pos));
        }
        remaining = remaining.substr(pos + 1);
    }
    if (remaining.size() > 0 || skip_empty == false)
    {
        result.push_back(remaining);
    }

    return result;
}

static inline bool split_in_two(const std::string & str, char separator, std::string & left, std::string & right)
{
    std::string::size_type pos = str.find(separator);
    if (pos == std::string::npos)
    {
        return false;
    }
    left = str.substr(0, pos);
    right = str.substr(pos+1);
    return true;
}

class styled_formater : public log_formater
{
public:
    styled_formater(
        const std::string & pattern,
        const std::string & pattern_style,
        char separator = '#',
        log_styled_output_ptr output = log_styled_output_ptr()):
    output(output)
    {
        std::vector<std::string> patterns = explode(pattern, separator);
        std::vector<std::string> styles = explode(pattern_style, separator);
        erase_spaces(styles);
        assert(patterns.size() == styles.size());

        blocks.reserve(patterns.size());
        for (size_t i = 0; i < patterns.size(); ++i)
        {
            blocks.push_back(std::make_pair(
                patterns[i],
                decode_style(styles.at(i))
            ));
        }
    }

    void add_output(log_styled_output_ptr out)
    {
        output = out;
    }

protected:
    std::string expand_pattern(size_t n, const log_msg_t & msg)
    {
        std::string result;

        std::string remaining = blocks.at(n).first;
        while (remaining.size() > 1)
        {
            std::string::size_type start = remaining.find('%');
            std::string::size_type end = remaining.find('%', start + 1);
            if (start == std::string::npos || end == std::string::npos)
            {
                break;
            }

            std::string before = remaining.substr(0, start);
            std::string exp = remaining.substr(start + 1, end - start - 1);
            remaining = remaining.substr(end + 1);

            if (!before.empty())
            {
                result += before;
            }

            log_msg_t::const_iterator it = msg.find(exp);
            if (it != msg.end())
            {
                result += it->second;
            }
            else
            {
                result += "%" + exp + "%";
            }
        }
        result += remaining;
        return result;
    }

    struct style_t
    {
        std::string color;
        int min_width;
        int max_width;
        bool textalign_right;
        bool visible;

        style_t() : min_width(-1), max_width(-1), textalign_right(false), visible(true)
        {
        }
    };
    virtual void customize_style(style_t & style, size_t n, log_msg_t & msg)
    {
        (void)style;
        (void)n;
        (void)msg;
    }

    virtual void format(log_msg_t & msg)
    {        
        output->beginline();

        for (size_t i = 0; i < blocks.size(); ++i)
        {
            style_t style = blocks[i].second;
            customize_style(style, i, msg);

            std::string final_text = expand_pattern(i, msg);
            
            if (style.min_width > -1 && static_cast<int>(final_text.size()) < style.min_width)
            {
                std::string diff(style.min_width - final_text.size(), ' ');
                if (style.textalign_right)
                {
                    final_text = diff + final_text;
                }
                else
                {
                    final_text += diff;
                }
            }
            if (style.max_width > -1 && static_cast<int>(final_text.size()) > style.max_width)
            {
                final_text.resize(style.max_width);
            }

            if (style.visible)
            {
                output->push_style(style.color);
                output->append(final_text);
                output->pop_style();
            }
            else
            {
                // TODO: make sure output does not print anything (use invisible background)
                std::string s(final_text.size(), ' ');
                output->append(s);
            }
        }        

        output->endline();
    }

private:
    static style_t decode_style(const std::string & str)
    {
        style_t result;

        std::vector<std::string> attributes = explode(
            skip_char(str, ' '), ';', true);

        for (size_t i = 0; i < attributes.size(); ++i)
        {
            std::string attr;
            std::string value;
            if (!split_in_two(attributes[i], ':', attr, value))
            {
                continue;
            }
            if (attr == "color")
            {
                result.color = value;
            }
            else if (attr == "min-width")
            {
                result.min_width = to_int(value, -1);
            }
            else if (attr == "max-width")
            {
                result.max_width = to_int(value, -1);
            }
            else if (attr == "width")
            {
                result.min_width = result.max_width = to_int(value, -1);
            }
            else if (attr == "text-align")
            {
                if (value == "right")
                {
                    result.textalign_right = true;
                }
            }
        }

        return result;
    }

    typedef std::pair<std::string, style_t> block_t;
    std::vector<block_t> blocks;    
    log_styled_output_ptr output;
};

class log_reader
{
public:
    log_reader(log_formater_ptr formater) : format(formater)
    {
    }

    void process()
    {
        log_msg_t msg;
        msg["filename"] = __FILE__;
        msg["function"] = __FUNCTION__;
        msg["text"] = "Hello World!";
        msg["tag"] = "info";
        
        format->format(msg);
        
        msg["tag"] = "error";
        msg["text"] = "Erreur!";
        format->format(msg);
    }

private:
    log_formater_ptr format;
};
typedef log_reader * log_reader_ptr;


#if 0
class myconsole : public text_formater
{
public:
    myconsole() : text_formater("[%filename%] %function%() %text%")
    {
        add_output(&console);
    }

private:
    console_output console;

    virtual void format(const msg_t * msg)
    {
        if (part_count() == 0)
            return;

        int i;
        for (i = 0; i < part_count() - 1; ++i)
        {
            std::string part = generate_part(msg, i);
            console.print(part);
        }

        std::string style;
        if (msg->tag == "error")
        {
            style = "red";
        }
        console.push_style(style);
        console.write(
            generate_part(msg, i));
        console.pop_style();
    }
};
#endif

class extra_fields : public log_formater
{
public:
    enum
    {
        //filenameonly = 1,
        funcnameonly = 2,
        classnameonly = 4,
        classnamedotdot = 8,
        classdotdot = 16,
    };

    extra_fields(unsigned int fields, log_formater_ptr formater) :
        fields(fields),
        formater(formater)
    {
        assert(formater);
    }

private:
    virtual void format(log_msg_t & msg)
    {
        if (fields & (classnameonly | classnamedotdot | classdotdot | funcnameonly))
        {
            std::string classname;
            size_t pos = msg["function"].find_last_of(':');
            if (pos > 1)
            {
                classname = msg["function"].substr(0, pos - 1);
            }
            if (fields & funcnameonly)
            {
                msg["funcnameonly"] = (pos > 1) ?
                    msg["function"].substr(pos + 1) :
                    msg["function"];
            }
            if (fields & classdotdot)
            {
                msg["classdotdot"] = (pos > 1) ? "::" : "";
            }
            if (fields & classnameonly)
            {
                msg["classnameonly"] = classname;
            }
            if (fields & classnamedotdot)
            {
                msg["classnamedotdot"] = classname + "::";
            }
        }
        formater->format(msg);
    }
private:
    unsigned int fields;
    log_formater_ptr formater;    
};

class my_formater : public styled_formater
{
public:
    my_formater(log_styled_output_ptr output) : styled_formater(
        "[#%filename%#] #%classdotdot%#%funcnameonly%()# #%text%",
        " #color:green;width:14# #color:blue;width:2#color:darkblue;width:15;text-align:right# #color:white;width:44;text-align:right",
        '#',
        output)
    {
    }
protected:
    virtual void customize_style(style_t & style, size_t n, log_msg_t & msg)
    {
        if (msg["tag"] == "error")
        {
            style.color = "red";
            if (n <= 3)
            {
                style.visible = false;
            }
        }
    }
};

int main(int argc, char **argv)
{
    log_reader_ptr reader(
        new log_reader(
            new extra_fields(
                extra_fields::classdotdot|extra_fields::funcnameonly,
                new my_formater(
                    new console_output()
                )
            )
        )
    );
    reader->process();
#if 0
    log_archiver archiver;
    archiver.start_archiver();

    log_init(archiver.input_stream());

    class log_roller
    {
    public:
        inline max_alloc_size() const
        {
            return z_max_alloc_size;
        }

        void * alloc(unsigned short size)
        {
            return alloc(size, true);
        }

    protected:
        void * alloc(unsigned short size, bool first_attempt)
        {
            if (size > z_max_alloc_size)
            {
                return NULL;
            }

            // ! multithreading context: tcm_current_rollcan change at any time !
            roll_t * current_roll = tcm_current_roll;
            assert(current_roll);

            void * result = current_roll->alloc(size);
            if (result == NULL)
            {
                if (first_attempt)
                {
                    // we reached the capacity of the roll we used: change it
                    change_roll(current_roll);
                    result = alloc(size, false);
                }
                else
                {
                    // to avoid any infinite recursion, we fail here
                    return NULL;
                }
            }
            return result;
        }
    protected:
        struct roll_t
        {
            // data buffer
            unsigned char * data;
            // capacity of the buffer
            const unsigned int capacity;
            // remaining free space in the buffer
            volatile int tcm_remaining; // thread concurrent modification

            void * alloc(unsigned short size)
            {
                void * result = NULL;
                if (tca_remaining >= size)
                {
                    int remaining = atomic_sub(tcm_remaining, size);
                    if (remaining >= size)
                    {
                        unsigned int offset = capacity - remaining - size;
                        result = data[offset];
                    }
                }
                return result;
            }
        };

        log_roll(unsigned int max_alloc_size) :
             z_current_roll(NULL),
             z_max_alloc_size(max_alloc_size)
        {
        }

        ~log_roll() // virtual not needed
        {
        }

        virtual void change_roll(roll_t *) = 0;

    protected:
        volatile roll_t * tcm_current_roll; // thread concurrent modification

    private:
        const unsigned int z_max_alloc_size;
    };

    log_init(archiver.get_roll());

    new file_writer(
        new console_output(
            bind( field_value("tag", "error"), field_style("text", "color:red") )
            new styled_formater(
                new extrafields(

            bind( field_name("func"), field_pattern("[%function%]"))

    new_field("func", "[%function%]").set_style("color:blue");
    update_field("message", "%func% %message%")

    log_stream & log_console(log_stream & stream)
    {
        stream.append(new console_output());
        return stream;
    }

    log_stream & set_field(log_stream & stream, const char *name, const char *value)
    {
        stream.set_field(name, value);
        return stream;
    }

    log_stream stream1;
    stream1 << set_field("#1", "[%function%]")
        >> set_field("message", "%#1% %text%")
        >> set_style("#1", "color:blue")
        >> set_style("text", "color:white")
        >> set_style_if("%tag%", "error", "message", "color:red")
        >> log_console() >> log_textfile("%appname%.log"));

    log_init(stream1);

    log_init(
        extrafields_generator() >> styled_formater() >> console_output() >> file_writer() );

    log_init() >> console_output();

    child_process_redirector redirect;
    if (redirect.fork())
    {
        redirect.send_to() >> console_output();
    }
    log_init(redirect);

    //log_reader_ptr reader(
    //    new log_reader(
    //        new myconsole())
    //);
#endif    
}
