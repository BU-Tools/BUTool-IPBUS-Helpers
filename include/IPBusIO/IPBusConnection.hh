#ifndef __IPBUS_CONNECTION_HH__
#define __IPBUS_CONNECTION_HH__

#include <uhal/uhal.hpp>

#include <IPBusIO/IPBusIO.hh>

class IPBusConnection: public IPBusIO{
public:
  IPBusConnection();
  IPBusConnection(std::string const & deviceTypeName);
  IPBusConnection(uhal::HwInterface * _hw); //To configure this from an externally created HW device
  ~IPBusConnection();

  void Connect(std::vector<std::string> args);
  uhal::HwInterface * const * GetHWInterface();//Get a const pointer to a pointer to a HW device so this class is still in control of that data.

  //The IPBus connection and read/write functions come from the IPBusIO class.
  //Look there for the details. 

private:
  uhal::HwInterface *hw;  //the real HW device  (does this need a smart pointer?)
  // address table path (a bit ugly, but has to go somewhere!)
  std::string addressTablePath;
  std::string connectionFile;
  std::string IPBusDeviceTypeName;
};

#endif
