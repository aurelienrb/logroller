template<char current>
struct countParams
{
    static const size_t nb = 0;
};

template<char current, char next>
struct countParams
{
    static const size_t nb = (size_t)(current == '%' &&
        (next == '0' || next == '1' || next == '2' || next == '3' || next == '4' ||
         next == '5' || next == '6' || next == '7' || next == '8' || next == '9' ));
};

template<char current, char next, char... following>
struct countParams
{
    static const size_t nb = countParams<current,next>::nb + countParams<next, following...>::nb;
};

template<char current, char... following>
struct countParams
{
    static const size_t nb = countParams<current, following...>::nb;
};

// renvoie le nombre de parametres attendus par la chaine
template<char... Str>
inline constexpr size_t
operator "" _nbparams() noexcept
{
    return countParams<Str>::nb;
}


// renvoie la longueur d'une chaine literale
size_t operator "" _len (const char*, size_t n) { return n; }


class argument_stream
{
public:
    template<typename T>
    argument_stream & operator<<(T t)
    {
        return *this;
    }
};

template<int nb_param = 0>
class argument_counter
{
public:
    static const int nb = nb_params;

    argument_counter & stream;
    
    argument_counter(argument_stream & s) : stream(s)
    {
    }

    template<typename T>
    argument_counter<nb_params + 1> operator<<(T t)
    {
        return argument_counter<nb_params + 1>(stream << t);
    }
};

template<int nb_params>
class log_entry_creator
{
public:
    template<size_t n>
    void operator=(argument_counter<n> & params)
    {
        static_assert(false, "Incorrect number of arguments for the log string");
    }

    void operator=(argument_counter<nb_params> & params)
    {
    }
};

#define log(x) log_entry_creator<x_nbparams> = argument_counter(argument_stream())