#ifndef __IPBUS_IO_HH__
#define __IPBUS_IO_HH__

#include <uhal/uhal.hpp>

#if UHAL_VER_MAJOR >= 2 && UHAL_VER_MINOR >= 8
#include <unordered_map>
typedef std::unordered_map<std::string, std::string> uMap;
#else
#include <boost/unordered_map.hpp>
typedef boost::unordered_map<std::string, std::string> uMap;
#endif

#include <vector>
#include <string>
#include <stdint.h>

#define IPBUSIO_DEFAULT_DISPLAY_FORMAT "X"

//This class is an interface between the fundamental software library and what we want to use in our hardware class and the butool
//Right now this uses a dumb block/fifo read instead of the efficient calls built into IPBus.  
//These should be overloaded eventually and done so to work with the API from the BUTool Register Helper
class IPBusIO{
public:
  IPBusIO();
  virtual ~IPBusIO(){};
  std::vector<std::string> myMatchRegex(std::string regex);  
  //Misc markups
  uint32_t GetRegAddress(std::string const & reg);
  uint32_t GetRegMask(std::string const & reg);
  uint32_t GetRegSize(std::string const & reg);
  std::string GetRegMode(std::string const & reg);
  std::string GetRegPermissions(std::string const & reg);
  std::string GetRegDescription(std::string const & reg);
  std::string GetRegDebug(std::string const & reg);  
  const uMap & GetParameters(std::string const & reg);

  //numeric reads
  uint32_t RegReadAddress(uint32_t addr);
  //Named register reads
  uint32_t RegReadRegister(std::string const & reg);
  //Node reads
  uint32_t RegReadNode(uhal::Node const & node);

  // Conversion format
  enum ConvertType {NONE=0, UINT=1, INT=2, FP=4, STRING=8};
  ConvertType RegReadConvertType(std::string const & reg);
  std::string RegReadConvertFormat(std::string const & reg);
 
  // Named register read+conversion functions, overloaded depending on the conversion value type
  void RegReadConvert(std::string const & reg, unsigned int & val);
  void RegReadConvert(std::string const & reg, int & val);
  void RegReadConvert(std::string const & reg, double & val); 
  void RegReadConvert(std::string const & reg, std::string & val);

  //numeric, named register, action, and node writes
  void RegWriteAddress(uint32_t addr, uint32_t data);
  void RegWriteRegister(std::string const & reg, uint32_t data);
  void RegWriteAction(std::string const & reg);
  void RegWriteNode(uhal::Node const & node, uint32_t data);

  // Helper functions for converting
  double ConvertFloatingPoint16ToDouble(std::string const & reg);
  double ConvertLinear11ToDouble(std::string const & reg);
  double ConvertIntegerToDouble(std::string const & reg, std::string const & format);
  std::string ConvertEnumToString(std::string const & reg, std::string const & format);
  std::string ConvertIPAddressToString(std::string const & reg);

  // Helper function to return list of register names with a specified parameter
  std::vector<std::string> GetRegisterNamesFromTable(std::string const & tableName, int statusLevel=1);

  uhal::Node const & GetNode(std::string const & reg);
protected:
  void SetHWInterface(uhal::HwInterface * const * _hw);

private:
  //This is a const pointer to a pointer to a HWInterface so that the connection class controls it. 
  uhal::HwInterface * const * hw;  
};
#endif
