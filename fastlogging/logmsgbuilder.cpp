#include "logmsgbuilder.h"
#include <iostream>
#include <boost/format.hpp>
namespace logroller
{
logmsgbuilder::~logmsgbuilder()
{
    this->msg->dynText = this->str();

    std::cout << "FORMAT=" << this->msg->dynText << '\n';
    boost::format fmter( this->msg->dynText );
    for ( int i = 0; i < this->msg->nbStrings; ++i )
    {
        fmter % this->msg->staticStrings[ i ];
    }
    std::cout << fmter << '\n';
}
}