#ifndef __IPBUS_STATUS_HH__
#define __IPBUS_STATUS_HH__

#include <IPBusIO/IPBusIO.hh>
#include <StatusDisplay/StatusDisplay.hh>

#if UHAL_VER_MAJOR >= 2 && UHAL_VER_MINOR >= 8
#include <unordered_map>
typedef std::unordered_map<std::string, std::string> uMap;
#else
#include <boost/unordered_map.hpp>
typedef boost::unordered_map<std::string, std::string> uMap;
#endif
class IPBusStatus: public BUTool::StatusDisplay {
public:
  IPBusStatus(IPBusIO * _regIO);
  ~IPBusStatus(){};
private:
  IPBusStatus();
  void Process(std::string const & singleTable);
};
#endif
