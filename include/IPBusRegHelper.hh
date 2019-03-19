#ifndef __IPBUS_REG_HELPER_HH__
#define __IPBUS_REG_HELPER_HH__

#include <helpers/register_helper.hh>
#include <uhal/uhal.hpp>

#include <vector>
#include <string>
#include <stdint.h>

class IPBusRegHelper: public BUTool::RegisterHelper{
public:
  IPBusRegHelper();
  IPBusRegHelper(std::string const & deviceTypeName);
  
  std::vector<std::string> myMatchRegex(std::string regex);

  //Misc markups
  uint32_t GetRegAddress(std::string const & reg);
  uint32_t GetRegMask(std::string const & reg);
  uint32_t GetRegSize(std::string const & reg);
  std::string GetRegMode(std::string const & reg);
  std::string GetRegPermissions(std::string const & reg);
  std::string GetRegDescription(std::string const & reg);
  std::string GetRegDebug(std::string const & reg);  

  //Named register writes
  uint32_t RegReadAddress(uint32_t addr);
  std::vector<uint32_t> RegReadAddressesInc   (uint32_t addr,size_t count);
  std::vector<uint32_t> RegReadAddressesNonInc(uint32_t addr,size_t count);
  //Named register reads
  uint32_t RegReadRegister(std::string const & reg);
  std::vector<uint32_t> RegReadAddressesInc   (std::string const & reg,size_t count);
  std::vector<uint32_t> RegReadAddressesNonInc(std::string const & reg,size_t count);
  
  void RegWriteAddress(uint32_t addr, uint32_t data);
  void RegWriteRegister(std::string const & reg, uint32_t data);
  void RegWriteAction(std::string const & reg);
protected:
  void Connect(std::vector<std::string> args);
  uhal::HwInterface *hw;

  std::string IPBusDeviceTypeName;
  
  // address table path (a bit ugly, but has to go somewhere!)
  std::string addressTablePath;
  std::string connectionFile;
private:
  void Init(std::string const & deviceTypeName);
};
#endif
