#ifndef __TOOLEXCEPTION_HH__
#define __TOOLEXCEPTION_HH__ 1

#include <BUException/ExceptionBase.hh>

namespace BUException{           
  ExceptionClassGenerator(REG_READ_DENIED,"Read access denied\n")
  ExceptionClassGenerator(REG_WRITE_DENIED,"Write access denied\n")
  ExceptionClassGenerator(BAD_REG_NAME,"Bad register name\n")
  ExceptionClassGenerator(IPBUS_CONNECTION_ERROR,"Error in connecting\n");
}




#endif
