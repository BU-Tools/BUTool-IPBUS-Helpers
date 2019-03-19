#include <IPBus_reg_helper.hh>
#include <BUException/ExceptionBase.hh>

//
// read one or more registers
// one argument may be: address, single name or regular expression
// second argument is count to read from each address
// last argument may be "D" for doublewords (64-bits)
//

IPBusRegHelper::IPBusRegHelper() : BUTool::RegisterHelper(RegisterNameCase::UPPER){
  Init("IPBUS_DEVICE");
}
IPBusRegHelper::IPBusRegHelper(std::string const & deviceTypeName):RegisterHelper(RegisterNameCase::UPPER){
  Init("IPBUS_DEVICE");
}

void IPBusRegHelper::Init(std::string const & deviceTypeName){
  hw = NULL;
  IPBusDeviceTypeName = deviceTypeName;
}


#define FILE_ADDR_ARG 0
#define ADDR_TABLE_PATH_ARG 1
#define PREFIX_ARG 2
void IPBusRegHelper::Connect(std::vector<std::string> args){
  //--- inhibit most noise from uHAL
  uhal::setLogLevelTo(uhal::Error());

  if(0 == arg.size()){
    BUException::DEVICE_CREATION_ERROR e;
    e.Append("No file/address specified\n");
    throw e;
  }
  
  //We have a file/address
  connectionFile = arg[0];
 
  // Determine the address table path
  // 1st, non empty ADDR_TABLE_PATH_ARG
  // 2nd, enviornment variable "IPBusDeviceTypeName"_ADDRESS_TABLE_PATH
  // 3rd, empty
  //Generate the env name in case we need it.
  std::string ENVVariableName(IPBusDeviceTypeName);
  ENVVariableName.append("_ADDRESS_TABLE_PATH");
  if((ADDR_TABLE_PATH_ARG < arg.size()) && (!arg[ADDR_TABLE_PATH_ARG].empty())){
    //An create argument that is non-empty
    addressTablePath = arg[ADDR_TABLE_PATH_ARG];
  } else if (NULL != getenv(ENVVariableName.c_str())){
    //Enviornment variable
    addressTablePath.assign(getenv(ENVVariableName.c_str()));
  } else {
    addressTablePath.assign("");
  }

  // check for numeric IP address using Boost
  // separate last octet so we can increment it
  static const boost::regex reIPAddress("(\\d{1,3}.\\d{1,3}.\\d{1,3}.)(\\d{1,3})(/[cC])?");
  static const boost::regex reXMLFile("\\S*\\.[xX][mM][lL]");
  static boost::cmatch reMatch;
  static char uri[256];


  if( boost::regex_match( connectionFile.c_str(), reMatch, reIPAddress)) {
    //=============================================================================
    //IP Address

    //Get the IP
    std::string ip_addr(reMatch[1].first, reMatch[1].second); // extract 1st 3 octets                               
    std::string ip_last(reMatch[2].first, reMatch[2].second); // extract last octet                                 
    uint8_t oct_last = atoi( ip_last.c_str());
    
    //Check for control hub option
    bool use_ch = reMatch[3].matched; // check for /c suffix                                                     
    if( use_ch){
      printf("use_ch true\n");
    } else {
      printf("use_ch false\n");
    }

    // specify protocol prefix                                                                                
    std::string proto = use_ch ? "chtcp-2.0://localhost:10203?target=" : "ipbusudp-2.0://";

    snprintf( uri, 255, "%s%s%d:50001", proto.c_str(), ip_addr.c_str(), oct_last);

    printf("Created URI from IP address:\n  %s\n",uri);

    std::string addrTableFull = "file://" + addressTablePath;

    printf("Address table name is %s\n", addrTableFull.c_str());

    try {
      hw = new uhal::HwInterface( uhal::ConnectionManager::getDevice(IPBusDeviceTypeName.c_str(), uri, addrTableFull));
    } catch( uhal::exception::exception& e) {
      e.append("Module::Connect() creating hardware device");
      printf("Error creating uHAL hardware device\n");
    }

  } else if( boost::regex_match( connectionFile.c_str(), reMatch, reXMLFile) ) {
    //===========================================================================
    //Connection File  
    //===========================================================================
    printf("This mode needs to be re-implemented\n");    
    //
    //    printf("Using .xml connection file...\n");
    //    if(3 >  arg.size()){ 
    //      // the third argument is the prefix
    //      // use default "T1" and "T2" xml IDs                                                     
    //      amc13 = new AMC13( connectionFile);
    //    } else { 
    //      // specified xml ID prefix                                                                          
    //      const std::string t1id = arg[PREFIX_ARG] + ".T1";
    //      const std::string t2id = arg[PREFIX_ARG] + ".T2";
    //      amc13 = new AMC13( connectionFile, t1id, t2id);
    //  }
  } else { // hostname less '_t2' and '_t1'                                                                    
    //===========================================================================
    // Hostname
    //===========================================================================
    
    printf("does NOT match\n");
    // define necessary variables                                                                             
    char ip[16];
    std::string name;
    name = connectionFile;

    // convert hostname to ips                                                                                
    printf("converting host name...\n");
    hostnameToIp( name.c_str(), ip);
    printf("making proto...\n");

    // specify protocol prefix                                                                                
    std::string proto = "ipbusudp-2.0://"; // ATTENTION: may need to add check for chtcp protocol             

    printf("copying uris");
    // copy ip address to appropriate uris                                                                    
    snprintf( uri, 255, "%s%s:50001", proto.c_str(), ip );
    printf("Created URI from IP address: %s\n",uri);

    std::string addrTableFull = "file://" + addressTablePath;
    
    try {
      hw = new uhal::HwInterface( uhal::ConnectionManager::getDevice(IPBusDeviceTypeName.c_str(), uri, addrTableFull + "/" + IPBusDeviceTypeName + ".xml"));
    } catch( uhal::exception::exception& e) {
      e.append("Module::Connect() creating hardware device");
      printf("Error creating uHAL hardware device\n");
    }

  }


  if( hw == NULL){
    BUException::DEVICE_CREATION_ERROR e;
    e.Append("Unable to create HWInterface\n");
    throw e;    
  }

}

}

  
