
#ifndef __IPBUS_CONNECTION_HH__
#define __IPBUS_CONNECTION_HH__

#include <uhal/uhal.hpp>

class IPBusConnection{
public:
  IPBusConnection(std::string const & deviceTypeName,std::vector<std::string> const & args);
  IPBusConnection(std::shared_ptr<uhal::HwInterface> _hw); //To configure this from an externally created HW device
  virtual ~IPBusConnection();

  void Connect(std::vector<std::string> const & args);
  std::shared_ptr<uhal::HwInterface> GetHWInterface();//Get a const pointer to a pointer to a HW device so this class is still in control of that data.

  //The IPBus connection and read/write functions come from the IPBusIO class.
  //Look there for the details. 

private:
  IPBusConnection(); // do not implement
  std::string IPBusDeviceTypeName;
  std::shared_ptr<uhal::HwInterface> hw;    //This is first instance of the shared pointer. 
  // address table path (a bit ugly, but has to go somewhere!)
  std::string addressTablePath;
  std::string connectionFile;

};

#endif
