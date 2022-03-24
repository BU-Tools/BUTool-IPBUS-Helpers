#include <BUException/ExceptionBase.hh>
#include <BUTool/ToolException.hh>
#include <IPBusIO/IPBusIO.hh>
#include <IPBusIO/IPBusExceptions.hh>


std::string IPBusIO::ReadString(std::string const & reg){
    uhal::Node const & node = GetNode(reg);
    uhal::defs::BlockReadWriteMode mode = node.getMode();
    size_t stringLen=GetRegSize(reg);
    //make sure the mode is non_incremental 
    if((uhal::defs::INCREMENTAL != mode) &&
       (!stringLen)){
      BUException::REG_READ_DENIED e2;    
      e2.Append(reg);
      e2.Append("is not a string\n");
      throw e2;
    }
    std::vector<uint32_t> val = BlockReadRegister(reg,stringLen);
    std::string ret( (char *) val.data(),stringLen*sizeof(uint32_t));
    return ret;
  }



IPBusIO::IPBusIO(std::shared_ptr<uhal::HwInterface> _hw):
  hw(_hw){
}


static void ReplaceStringInPlace(std::string& subject,
				 const std::string& search,
				 const std::string& replace) {
  size_t pos = 0;
  while ((pos = subject.find(search, pos)) != std::string::npos) {
    subject.replace(pos, search.length(), replace);
    pos += replace.length();
  }
}



std::vector<std::string> IPBusIO::myMatchRegex(std::string regex){
  std::string rx = regex;

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

uint32_t IPBusIO::ReadAddress(uint32_t addr){
  uhal::ValWord<uint32_t> vw; //valword for transaction
  try{
    vw = hw->getClient().read(addr); // start the transaction
    hw->getClient().dispatch(); // force the transaction
  }catch (uhal::exception::ReadAccessDenied & e){
    BUException::REG_READ_DENIED e2;    
    char str_addr[] = "0xXXXXXXXX";
    snprintf(str_addr,11,"0x%08X",addr);
    e2.Append(str_addr);
    throw e2;
  }

  return vw.value();
}
uint32_t IPBusIO::ReadRegister(std::string const & reg){
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
uint32_t IPBusIO::ReadNode(uhal::Node const & node){
  uhal::ValWord<uint32_t> vw; //valword for transaction
  try{
    vw = node.read(); // start the transaction
    hw->getClient().dispatch(); // force the transaction
  }catch (uhal::exception::ReadAccessDenied & e){
    BUException::REG_READ_DENIED e2;    
    e2.Append("failed Node read");
    throw e2;
  }

  return vw.value();
}

void IPBusIO::WriteAction(std::string const & reg){
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
void IPBusIO::WriteAddress(uint32_t addr,uint32_t data){
  try{
    hw->getClient().write( addr, data);
    hw->getClient().dispatch() ;
  }catch (uhal::exception::WriteAccessDenied & e){
    BUException::REG_WRITE_DENIED e2;
    char str_addr[] = "0xXXXXXXXX";
    snprintf(str_addr,11,"0x%08X",addr);
    e2.Append(str_addr);
    throw e2;
  }
}
void IPBusIO::WriteRegister(std::string const & reg, uint32_t data){
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
void IPBusIO::WriteNode(uhal::Node const & node,uint32_t data){
  try{
    node.write(data); // start the transaction
    hw->getClient().dispatch(); // force the transaction
  }catch (uhal::exception::ReadAccessDenied & e){
    BUException::REG_WRITE_DENIED e2;    
    e2.Append("failed Node write");
    throw e2;
  }
}

uint32_t IPBusIO::GetRegAddress(std::string const & reg){
  return hw->getNode(reg).getAddress();
}
uint32_t IPBusIO::GetRegMask(std::string const & reg){
  return hw->getNode(reg).getMask();
}
uint32_t IPBusIO::GetRegSize(std::string const & reg){
  return hw->getNode(reg).getSize();
}
std::string IPBusIO::GetRegMode(std::string const & reg){
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
std::string IPBusIO::GetRegPermissions(std::string const & reg){
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

std::string IPBusIO::GetRegDescription(std::string const & reg){
  return hw->getNode(reg).getDescription();
}

std::string IPBusIO::GetRegDebug(std::string const & reg){
  const uMap params = hw->getNode(reg).getParameters();
  std::string ret;
  for( uMap::const_iterator it = params.begin();
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

const uMap & IPBusIO::GetParameters(std::string const & reg){
  return hw->getNode(reg).getParameters();
}


uhal::Node const & IPBusIO::GetNode(std::string const & reg){
   return hw->getNode(reg);
}
