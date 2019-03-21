#ifndef __IPBUS_IO_HH__
#define __IPBUS_IO_HH__

#include <uhal/uhal.hpp>


#include <vector>
#include <string>
#include <stdint.h>

//This class is an interface between the fundamental software library and what we want to use in our hardware class and the butool
//Right now this uses a dumb block/fifo read instead of the efficient calls built into IPBus.  
//These should be overloaded eventually and done so to work with the API from the BUTool Register Helper
class IPBusIO{
public:
  IPBusIO();

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
  //Named register reads
  uint32_t RegReadRegister(std::string const & reg);
  
  void RegWriteAddress(uint32_t addr, uint32_t data);
  void RegWriteRegister(std::string const & reg, uint32_t data);
  void RegWriteAction(std::string const & reg);
protected:
  void SetHWInterface(uhal::HwInterface * const * _hw);

private:
  //This is a const pointer to a pointer to a HWInterface so that the connection class controls it. 
  uhal::HwInterface * const * hw;  
};
#endif
