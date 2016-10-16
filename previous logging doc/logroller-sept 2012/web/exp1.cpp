#define LOGROLLER_ALLOW_LITERAL_STRING_AS_ARG
#include "logroller.h"

int main(int argc, char **argv)
{
    if (!log_init(argc, argv, inprocess_reader( text_printer(
            console_writer() + file_writer("%appname%.log") ) )
        return 0;

    if (!log_init(argc, argv, to_textfile("%appname%.log"))
        return 0;

    log_info("Hello World!");
       
    log_info("Syntaxe") << "à" << "la" << "istream";
    log_info("Syntaxe à la %1 (%2)").arg("Qt").arg("QString::arg");      
    log_info("Syntaxe") % "à" % "la" % "Qt (operateur %)";
    log_info("Syntaxe mixte %1 %2/%1 %3/%4").arg("Qt") % "arg" << "%" << "istream";
}

