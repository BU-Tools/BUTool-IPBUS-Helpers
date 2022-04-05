#include <IPBusRegHelper.hh>

IPBusRegHelper::IPBusRegHelper(std::shared_ptr<IPBusIO> _regIO) :
  //Recast the IPBusIO to its base class 
  RegisterHelper(std::shared_ptr<RegIO>(regIO)) {
}

std::shared_ptr<IPBusIO> & IPBusRegHelper::GetRegIO(){
  //return a reference since this in internal to the class and is just a re-casting
  return std::dynamic_pointer_cast<IPBusIO>(regIO);
}