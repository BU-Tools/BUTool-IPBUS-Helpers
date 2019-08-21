#ifndef __IPBUS_STATUS_HH__
#define __IPBUS_STATUS_HH__

#include <IPBusIO/IPBusIO.hh>
#include <BUTool/helpers/StatusDisplay/StatusDisplay.hh>
class IPBusStatus: public IPBusIO, public BUTool::StatusDisplay {
public:
  IPBusStatus(uhal::HwInterface * const * _hw){SetHWInterface(_hw);};
  ~IPBusStatus(){};
private:
  IPBusStatus();
  void Process(std::string const & singleTable);
};
#endif
