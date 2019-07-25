#include <BUException/ExceptionBase.hh>
#include <BUTool/ToolException.hh>
#include <IPBusIO/IPBusIO.hh>
#include <IPBusIO/IPBusExceptions.hh>


inline void CheckHW(uhal::HwInterface * const * hw){
  if(NULL == hw){
    BUException::BAD_STATE e;
    e.Append("IPBusIO HwInterface * * is NULL!\nClass hasn't been configured!\n");
  }else if(NULL == *hw){
    BUException::BAD_STATE e;
    e.Append("IPBusIO HwInterface * is NULL!\nHWInterface has been invaliddated\n");    
  }
}


IPBusIO::IPBusIO(){
  hw = NULL;
}


void IPBusIO::SetHWInterface(uhal::HwInterface * const * _hw){
  hw = _hw;
}

static void ReplaceStringInPlace(std::string& subject, const std::string& search,
				 const std::string& replace) {
  size_t pos = 0;
  while ((pos = subject.find(search, pos)) != std::string::npos) {
    subject.replace(pos, search.length(), replace);
    pos += replace.length();
  }
}



std::vector<std::string> IPBusIO::myMatchRegex(std::string regex){
  CheckHW(hw); //Make sure the IPBus state is ok
  std::string rx = regex;

  std::transform( rx.begin(), rx.end(), rx.begin(), ::toupper);

  if( rx.size() > 6 && rx.substr(0,5) == "PERL:") {
    printf("Using PERL-style regex unchanged\n");
    rx = rx.substr( 5);
  } else {
    ReplaceStringInPlace( rx, ".", "#");
    ReplaceStringInPlace( rx, "*",".*");
    ReplaceStringInPlace( rx, "#","\\.");
  }

  return (*hw)->getNodes( rx);
}

uint32_t IPBusIO::RegReadAddress(uint32_t addr){
  CheckHW(hw); //Make sure the IPBus state is ok 
  uhal::ValWord<uint32_t> vw; //valword for transaction
  try{
    vw = (*hw)->getClient().read(addr); // start the transaction
    (*hw)->getClient().dispatch(); // force the transaction
  }catch (uhal::exception::ReadAccessDenied & e){
    BUException::REG_READ_DENIED e2;    
    char str_addr[] = "0xXXXXXXXX";
    snprintf(str_addr,11,"0x%08X",addr);
    e2.Append(str_addr);
    throw e2;
  }

  return vw.value();
}
uint32_t IPBusIO::RegReadRegister(std::string const & reg){
  CheckHW(hw); //Make sure the IPBus state is ok
  uhal::ValWord<uint32_t> ret;
  try{
    ret = (*hw)->getNode( reg).read() ;
    (*hw)->dispatch();
  }catch (uhal::exception::ReadAccessDenied & e){
    BUException::REG_READ_DENIED e2;    
    e2.Append(reg);
    throw e2;
  }catch (uhal::exception::NoBranchFoundWithGivenUID & e){
    BUException::BAD_REG_NAME e2;
    e2.Append(reg);
    throw e2;
  }
  return ret.value();
}
void IPBusIO::RegWriteAction(std::string const & reg){
  CheckHW(hw); //Make sure the IPBus state is ok
  //This is a funky uhal thing
  try{
    uint32_t addr = (*hw)->getNode(reg).getAddress();
    uint32_t mask = (*hw)->getNode(reg).getMask();
    (*hw)->getClient().write(addr, mask);
    (*hw)->dispatch();
  }catch (uhal::exception::NoBranchFoundWithGivenUID & e){
    BUException::BAD_REG_NAME e2;
    e2.Append(reg);
    throw e2;
  }catch (uhal::exception::WriteAccessDenied & e){
    BUException::REG_WRITE_DENIED e2;
    e2.Append(reg);
    throw e2;
  }
}
void IPBusIO::RegWriteAddress(uint32_t addr,uint32_t data){
  CheckHW(hw); //Make sure the IPBus state is ok
  try{
    (*hw)->getClient().write( addr, data);
    (*hw)->getClient().dispatch() ;
  }catch (uhal::exception::WriteAccessDenied & e){
    BUException::REG_WRITE_DENIED e2;
    char str_addr[] = "0xXXXXXXXX";
    snprintf(str_addr,11,"0x%08X",addr);
    e2.Append(str_addr);
    throw e2;
  }
}
void IPBusIO::RegWriteRegister(std::string const & reg, uint32_t data){
  CheckHW(hw); //Make sure the IPBus state is ok
  try{
    (*hw)->getNode( reg ).write( data );
    (*hw)->dispatch() ;  
  }catch (uhal::exception::NoBranchFoundWithGivenUID & e){
    BUException::BAD_REG_NAME e2;
    e2.Append(reg);
    throw e2;
  }catch (uhal::exception::WriteAccessDenied & e){
    BUException::REG_WRITE_DENIED e2;
    e2.Append(reg);
    throw e2;
  }
}

uint32_t IPBusIO::GetRegAddress(std::string const & reg){
  CheckHW(hw); //Make sure the IPBus state is ok
  return (*hw)->getNode(reg).getAddress();
}
uint32_t IPBusIO::GetRegMask(std::string const & reg){
  CheckHW(hw); //Make sure the IPBus state is ok
  return (*hw)->getNode(reg).getMask();
}
uint32_t IPBusIO::GetRegSize(std::string const & reg){
  CheckHW(hw); //Make sure the IPBus state is ok
  return (*hw)->getNode(reg).getSize();
}
std::string IPBusIO::GetRegMode(std::string const & reg){
  CheckHW(hw); //Make sure the IPBus state is ok
  std::string ret;
  uhal::defs::BlockReadWriteMode mode = (*hw)->getNode(reg).getMode();
  switch( mode) {
  case uhal::defs::INCREMENTAL:
    ret += " inc";
    break;
  case uhal::defs::NON_INCREMENTAL:
    ret += " non-inc";
    break;
  case uhal::defs::HIERARCHICAL:
  case uhal::defs::SINGLE:
  default:
    break;
  }
  return ret;
}
std::string IPBusIO::GetRegPermissions(std::string const & reg){
  CheckHW(hw); //Make sure the IPBus state is ok
  std::string ret;
  uhal::defs::NodePermission perm = (*hw)->getNode(reg).getPermission();
  switch( perm) {
  case uhal::defs::READ:
    ret += " r";
    break;
  case uhal::defs::WRITE:
    ret += " w";
    break;
  case uhal::defs::READWRITE:
    ret += " rw";
    break;
  default:
    ;
  }
  return ret;
}

std::string IPBusIO::GetRegDescription(std::string const & reg){
  CheckHW(hw); //Make sure the IPBus state is ok
  return (*hw)->getNode(reg).getDescription();
}

std::string IPBusIO::GetRegDebug(std::string const & reg){
  CheckHW(hw); //Make sure the IPBus state is ok
  const boost::unordered_map<std::string,std::string> params = (*hw)->getNode(reg).getParameters();
  std::string ret;
  for( boost::unordered_map<std::string,std::string>::const_iterator it = params.begin();
       it != params.end();
       it++) {
    ret+="   ";
    ret+=it->first.c_str();
    ret+=" = ";
    ret+=it->second.c_str();
    ret+="\n";
  }    
  return ret;
}

  
  

  
