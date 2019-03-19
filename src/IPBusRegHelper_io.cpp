#include <IPBusRegHelper.hh>
#include <BUException/ExceptionBase.hh>

static void ReplaceStringInPlace(std::string& subject, const std::string& search,
			  const std::string& replace) {
  size_t pos = 0;
  while ((pos = subject.find(search, pos)) != std::string::npos) {
    subject.replace(pos, search.length(), replace);
    pos += replace.length();
  }
}



std::vector<std::string> IPBusRegHelper::myMatchRegex(std::string regex){
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

  return hw->getNodes( rx);
}

uint32_t IPBusRegHelper::RegReadAddress(uint32_t addr){ 
  uhal::ValWord<uint32_t> vw; //valword for transaction
  try{
    vw = hw->getClient().read(addr); // start the transaction
    hw->getClient().dispatch(); // force the transaction
  }catch (uhal::exception::ReadAccessDenied & e){
    BUException::REG_READ_DENIED e2;    
    char str_addr[] = "0xXXXXXXXX";
    snprintf(str_addr,10,"0x%08X",addr);
    e2.Append(str_addr);
    throw e2;
  }

  return vw.value();
}
uint32_t IPBusRegHelper::RegReadRegister(std::string const & reg){
  uhal::ValWord<uint32_t> ret;
  try{
    ret = hw->getNode( reg).read() ;
    hw->dispatch();
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
void IPBusRegHelper::RegWriteAction(std::string const & reg){
  //This is a funky uhal thing
  try{
    uint32_t addr = hw->getNode(reg).getAddress();
    uint32_t mask = hw->getNode(reg).getMask();
    hw->getClient().write(addr, mask);
    hw->dispatch();
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
void IPBusRegHelper::RegWriteAddress(uint32_t addr,uint32_t data){
  try{
    hw->getClient().write( addr, data);
    hw->getClient().dispatch() ;
  }catch (uhal::exception::WriteAccessDenied & e){
    BUException::REG_WRITE_DENIED e2;
    char str_addr[] = "0xXXXXXXXX";
    snprintf(str_addr,10,"0x%08X",addr);
    e2.Append(str_addr);
    throw e2;
  }
}
void IPBusRegHelper::RegWriteRegister(std::string const & reg, uint32_t data){
  try{
    hw->getNode( reg ).write( data );
    hw->dispatch() ;  
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

uint32_t IPBusRegHelper::GetRegAddress(std::string const & reg){return hw->getNode(reg).getAddress();}
uint32_t IPBusRegHelper::GetRegMask(std::string const & reg){return hw->getNode(reg).getMask();}
uint32_t IPBusRegHelper::GetRegSize(std::string const & reg){return hw->getNode(reg).getSize();}
std::string IPBusRegHelper::GetRegMode(std::string const & reg){
  std::string ret;
  uhal::defs::BlockReadWriteMode mode = hw->getNode(reg).getMode();
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
std::string IPBusRegHelper::GetRegPermissions(std::string const & reg){
  std::string ret;
  uhal::defs::NodePermission perm = hw->getNode(reg).getPermission();
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

std::string IPBusRegHelper::GetRegDescription(std::string const & reg){return hw->getNode(reg).getDescription();}

std::string IPBusRegHelper::GetRegDebug(std::string const & reg){
  const boost::unordered_map<std::string,std::string> params = hw->getNode(reg).getParameters();
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

  
  

  
