#include <IPBusStatus/IPBusStatus.hh>

#include <BUTool/ToolException.hh>

IPBusStatus::IPBusStatus(IPBusIO * _regIO):
  StatusDisplay(dynamic_cast<BUTool::RegisterHelperIO*>(_regIO)){
};