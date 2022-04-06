#ifndef __IPBUS_REG_HELPER_HH__
#define __IPBUS_REG_HELPER_HH__

#include <RegisterHelper/RegisterHelper.hh>
#include <IPBusIO/IPBusIO.hh>


class IPBusRegHelper: public BUTool::RegisterHelper{
public:
  IPBusRegHelper(std::shared_ptr<IPBusIO> _regIO,
		 std::shared_ptr<BUTextIO> _textIO);
  //This class is a bridge to connect the API for the BUTool reg helpers with the IPBus IO functions.
  //This gets its API from the RegisterHelper so that BUTool gets a consistant set of commands for free.
  //The implementation of these functions is taken from the IPBusIO class, but for the inheritance to work, we 
  //need to make local overloads of the RegisterHelper API that call the equivalent functions from IPBusIO.
  //This means that the function names and signatures from IPBusIO don't have to match RegisterHelper, 
  //but let's make life easier and say the must be the same
protected:

  std::shared_ptr<IPBusIO> GetRegIO();
    
  
};
#endif
