#include <BUException/ExceptionBase.hh>
#include <BUTool/ToolException.hh>
#include <IPBusIO/IPBusIO.hh>
#include <IPBusIO/IPBusExceptions.hh>

#include <stdlib.h> //strtoul
#include <map> //map
#include <math.h> // for NAN
#include <cmath> // for pow
#include <arpa/inet.h> // for inet_ntoa and in_addr_t

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>

using boost::algorithm::iequals;

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
uint32_t IPBusIO::RegReadNode(uhal::Node const & node){
  CheckHW(hw); //Make sure the IPBus state is ok 
  uhal::ValWord<uint32_t> vw; //valword for transaction
  try{
    vw = node.read(); // start the transaction
    (*hw)->getClient().dispatch(); // force the transaction
  }catch (uhal::exception::ReadAccessDenied & e){
    BUException::REG_READ_DENIED e2;    
    e2.Append("failed Node read");
    throw e2;
  }

  return vw.value();
}

double IPBusIO::ConvertFloatingPoint16ToDouble(std::string const & reg){
  // Helper function to do the "fp16->double" conversion
  double doubleVal;

  union {
    struct {
      uint16_t significand   : 10;
      uint16_t exponent      : 5;
      uint16_t sign          : 1;
    } fp16;
    int16_t raw;
  } val;
  val.raw = RegReadRegister(reg);

  switch (val.fp16.exponent) {
  // Case where the exponent is minimum
  case 0:
    if (val.fp16.significand == 0) {
      doubleVal = 0.0;
    }
    else {
      doubleVal = pow(2,-14)*(val.fp16.significand/1024.0);
    }

    // Apply sign
    if (val.fp16.sign) {
      doubleVal *= -1.0;
    }
    break;
  // Case where the exponent is maximum
  case 31:
    if (val.fp16.significand == 0) {
      doubleVal = INFINITY;
      if (val.fp16.sign) {
	doubleVal *= -1.0;
      }   
    }
    else {
      doubleVal = NAN;
    }
    break;
  // Cases in between
  default:
    doubleVal = pow(2, val.fp16.exponent-15)*(1.0+(val.fp16.significand/1024.0));
    if (val.fp16.sign) {
      doubleVal *= -1.0;
    }
    break;   
  }
  return doubleVal;
}

double IPBusIO::ConvertIntegerToDouble(std::string const & reg, std::string const & format){
  // Helper function to convert an integer to float using the following format:
  // y = (sign)*(M_n/M_d)*x + (sign)*(b_n/b_d)
  //       [0]   [1] [2]        [3]   [4] [5]
  // sign == 0 -> negative, other values mean positive

  std::vector<uint64_t> mathValues;
  size_t iFormat=1;
 
  uint32_t rawVal = RegReadRegister(reg);
 
  while (mathValues.size() != 6 && iFormat < format.size()) {
    if (format[iFormat] == '_') {
      // Start parsing the value after the '_' and add the corresponding value to mathValues array
      for (size_t jFormat=++iFormat; jFormat < format.size(); jFormat++) {
        if ( (format[jFormat] == '_') || (jFormat == format.size() - 1) ) {
	  if (jFormat == format.size() - 1) { jFormat++; }
          // Convert the string to a number
          uint64_t val = strtoull(format.substr(iFormat, jFormat-iFormat).c_str(), NULL, 0);
          mathValues.push_back(val);
          iFormat = jFormat;
          break;
        }
      }
    }
    else {
      iFormat++;
    } 
  }

  // TODO: Some checks needed, check that mathValues.size() == 6, and no 0s in denominator

  // Compute the transformed value from the raw value
  // Will compute: (m*x) + b
  double transformedValue = rawVal;
  transformedValue *= double(mathValues[1]);
  transformedValue /= double(mathValues[2]);
  // Apply the sign of m
  if (mathValues[0] == 0) {
    transformedValue *= -1;
  }
  
  double b = double(mathValues[4]) / double(mathValues[5]);
  if (mathValues[3] == 0) {
    b *= -1;
  }
  transformedValue += b;

  return transformedValue;
}

double IPBusIO::ConvertLinear11ToDouble(std::string const & reg){
  // Helper function to convert linear11 format to double

  union {
    struct {
      int16_t integer  : 11;
      int16_t exponent :  5;
    } linear11;
    int16_t raw;
  } val;
  
  val.raw = RegReadRegister(reg);
  double floatingValue = double(val.linear11.integer) * pow(2, val.linear11.exponent);
  
  return floatingValue;
}

std::string IPBusIO::ConvertIPAddressToString(std::string const & reg){
  // Helper function to convert IP addresses to string
  struct in_addr addr;
  int16_t val = RegReadRegister(reg);
  addr.s_addr = in_addr_t(val);

  return inet_ntoa(addr);
}

std::string IPBusIO::ConvertEnumToString(std::string const & reg, std::string const & format){
  // Helper function to convert enum to std::string
  std::map<uint64_t, std::string> enumMap;
  
  // Parse the format argument for enumerations
  size_t iFormat = 1;
  while (iFormat < format.size()) {
    if (format[iFormat] == '_') {
      // Read the integer corresponding to this enum
      uint64_t val = 0;
      for (size_t jFormat = ++iFormat; jFormat < format.size(); jFormat++) {
        if ((format[jFormat] == '_') | (jFormat == format.size() - 1)) {
          if (jFormat == format.size() - 1) { jFormat++; }
          // Convert the value into number
          val = strtoul(format.substr(iFormat, jFormat-iFormat).c_str(), NULL, 0);
          iFormat = jFormat;
          break;
        }
      }
      // Now read the corresponding string
      for (size_t jFormat = ++iFormat; jFormat < format.size(); jFormat++) {
        if ((format[jFormat] == '_') || (jFormat == (format.size()-1))) {
          if (jFormat == format.size() - 1) { jFormat++; }
          // Get the string
          enumMap[val] = format.substr(iFormat, jFormat-iFormat);
          iFormat = jFormat;
          break;  
        }
      } 
    }
    else {
      iFormat++;
    }
  }

  // Now we have the enumeration map, read the integer value from the register
  // Then return the corresponding string
  uint32_t regValue = RegReadRegister(reg);
  if (enumMap.find(regValue) != enumMap.end()) {
    // If format starts with 't', just return the string value
    // Otherwise, return the value together with the number
    if (format[0] == 't') {
      return enumMap[regValue].c_str();
    }
    return (enumMap[regValue] + " " + std::to_string(regValue)).c_str();
  }

  // Cannot find it, TODO: better handle this possibility
  return "NOT_FOUND";
}

std::vector<std::string> IPBusIO::GetRegisterNamesFromTable(std::string const & tableName, int statusLevel){ 
  // Helper function to get a list of register names from a given table name 
  std::vector<std::string> registerNames;

  // All register names
  std::vector<std::string> allNames = myMatchRegex("*");

  for (size_t idx=0; idx < allNames.size(); idx++) {
    const uMap parameters = GetParameters(allNames[idx]);
    const std::string table = (parameters.find("Table") != parameters.end()) ? parameters.find("Table")->second : "Not found";
    if (table == tableName) {
      // Check the status level, if statusLevel < status,
      // we are not going to return this register
      const int status = (parameters.find("Status") != parameters.end()) ? std::stoi(parameters.find("Status")->second) : -1;
      if (statusLevel < status) { continue; }

      // Check the format, if there is no format string, skip listing this register
      if (parameters.find("Format") == parameters.end()) { continue; }

      registerNames.push_back(allNames[idx]);
    }
  }

  return registerNames;
}

std::string IPBusIO::RegReadConvertFormat(std::string const & reg){
  // From a given node address, retrieve the "Format" parameter of the node
  const uMap parameters = GetParameters(reg);
  std::string format = (parameters.find("Format") != parameters.end()) ? parameters.find("Format")->second : "none";
  return format; 
}

IPBusIO::ConvertType IPBusIO::RegReadConvertType(std::string const & reg){
  // Return the conversion type
  std::string format = RegReadConvertFormat(reg);
  if ((format[0] == 'T') || (format[0] == 't') || ( iequals(format, std::string("IP")) ) || ( iequals(format, "X")) ) {
    return STRING;
  }
  if ((format[0] == 'M') | (format[0] == 'm') | (format == "fp16")) {
    return FP;
  }
  if ((format.size() == 1) & (format[0] == 'd')) {
    return INT;
  }
  if ((format.size() == 1) & (format[0] == 'u')) {
    return UINT;
  }
  return NONE;
}

void IPBusIO::RegReadConvert(std::string const & reg, unsigned int & val){
  // Read the value from the named register, and update the value in place
  uint32_t rawVal = RegReadRegister(reg);
  val = rawVal;
}

void IPBusIO::RegReadConvert(std::string const & reg, int & val){
  // Read the value from the named register, and update the value in place
  uint32_t rawVal = RegReadRegister(reg);

  // Now comes the check:
  // -55 is a placeholder for non-running fireflies
  // That would mean rawVal 256 + (-55) = 201 since the raw value is an unsigned int
  // We'll transform and return that value, in other cases our job is easier
  int MAX_8_BIT_INT = 256;
  if (rawVal == 201) {
    val = -(int)(MAX_8_BIT_INT - rawVal);
    return;
  }
  val = (int)rawVal;
 
}

void IPBusIO::RegReadConvert(std::string const & reg, double & val){
  // Read the value from the named register, and update the value in place
  // Check the conversion type we want:
  // Is it a "fp16", or will we do some transformations? (i.e."m_...")
  std::string format = RegReadConvertFormat(reg);
  // 16-bit floating point to double transformation
  if (iequals(format, "fp16")) {
    val = ConvertFloatingPoint16ToDouble(reg);
  }
  
  // Need to do some arithmetic to transform
  else if ((format[0] == 'M') | (format[0] == 'm')) {
    val = ConvertIntegerToDouble(reg, format);
  }

  else if (iequals(format, "linear11")) {
    val = ConvertLinear11ToDouble(reg);
  }

  // Undefined format, throw error
  else {
    BUException::FORMATTING_NOT_IMPLEMENTED e;
    e.Append("Format: " + format);
    e.Append("\n");
    throw e;
  }
}

void IPBusIO::RegReadConvert(std::string const & reg, std::string & val){
  // Read the value from the named register, and update the value in place
  std::string format = RegReadConvertFormat(reg);
  
  if ((format.size() > 1) && (('t' == format[0]) || ('T' == format[0]))) {
    val = ConvertEnumToString(reg, format);
  }
  // IP addresses
  else if (iequals(format, std::string("IP"))) {
    val = ConvertIPAddressToString(reg);
  }
  // Hex numbers in string
  else if (iequals(format, "x")) {
    val = RegReadRegister(reg);
  }
  // Undefined format, throw error
  else {
    BUException::FORMATTING_NOT_IMPLEMENTED e;
    e.Append("Format: " + format);
    e.Append("\n");
    throw e;
  }

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
void IPBusIO::RegWriteNode(uhal::Node const & node,uint32_t data){
  CheckHW(hw); //Make sure the IPBus state is ok 
  try{
    node.write(data); // start the transaction
    (*hw)->getClient().dispatch(); // force the transaction
  }catch (uhal::exception::ReadAccessDenied & e){
    BUException::REG_WRITE_DENIED e2;    
    e2.Append("failed Node write");
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
  const uMap params = (*hw)->getNode(reg).getParameters();
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
  CheckHW(hw); //Make sure the IPBus state is ok
  return (*hw)->getNode(reg).getParameters();
}


uhal::Node const & IPBusIO::GetNode(std::string const & reg){
  CheckHW(hw); //Make sure the IPBus state is ok
  return (*hw)->getNode(reg);
}
