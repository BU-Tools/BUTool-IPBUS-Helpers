#ifndef __IPBUS_EXCEPTIONS_HH__
#define __IPBUS_EXCEPTIONS_HH__ 1

#include <BUException/ExceptionBase.hh>

namespace BUException{           
  ExceptionClassGenerator(IPBUS_CONNECTION_ERROR,"Error while connecting to IPBus device\n")
  ExceptionClassGenerator(BAD_STATE,"Data was in a bad state\n")
}
#endif
