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
  //Block reads
  size_t BlockReadAddress(uint32_t addr, size_t nWords, uint32_t* buffer);
  size_t BlockReadRegister(std::string const & reg, size_t nWords, uint32_t* buffer);

  //numeric, named register, action, block, and node writes
  void RegWriteAddress(uint32_t addr, uint32_t data);
  void RegWriteRegister(std::string const & reg, uint32_t data);
  void RegWriteAction(std::string const & reg);
  void RegWriteNode(uhal::Node const & node, uint32_t data);

  void BlockWriteAddress(uint32_t addr,uint32_t* data,size_t nWords);
  void BlockWriteRegister(const std::string& reg, size_t nWords, uint32_t* data);

  uhal::Node const & GetNode(std::string const & reg);
protected:
  void SetHWInterface(uhal::HwInterface * const * _hw);

private:
  //This is a const pointer to a pointer to a HWInterface so that the connection class controls it. 
  uhal::HwInterface * const * hw;  
};
#endif
