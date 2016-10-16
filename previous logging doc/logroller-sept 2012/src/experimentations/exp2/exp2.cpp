struct log_info_t
{
    const char * fileName;
    unsigned int lineNumber;
    const char * functionName;
    const char * logMessage;
};

int main(int argc, char **argv)
{
    static const log_info_t log_info = {
        __FILE__,
        __LINE__,
        __FUNCTION__,
        "Hello World!"
    };
    
    // if we receive an address to print on the commande line
    if (argc == 3 && string(argv[1]) == "-print")
    {
        // get the address to print
        istringstream iss(argv[2]) ;
        void *addr;
        if (iss >> addr)
        {
            log_info_t *info = (log_info_t*)addr;
            cout << info->fileName << " ["
                 << info->lineNumber << "] "
                 << info->functionName << "(): "
                 << info->logMessage
                 << endl;
        }
    }
    else
    {
        cout << (void *)&log_info;
    }  
}
