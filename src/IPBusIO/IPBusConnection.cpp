#include <IPBusIO/IPBusConnection.hh>
#include <IPBusIO/IPBusExceptions.hh>

#include <boost/regex.hpp>

//for network hints
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

//for inet_ntoa
#include <netinet/in.h>
#include <arpa/inet.h>


static void hostnameToIp(char const * hostname, char *ip) {
  struct addrinfo hints, *servinfo, *p;  
  struct sockaddr_in *h;  
  int rv;  
 
  memset(&hints, 0, sizeof hints);  
  hints.ai_family = AF_INET; // use AF_INET assuming IPv4 or AF_UNSPEC if also accept IPv6 
  hints.ai_socktype = SOCK_STREAM;  
 
  if ( (rv = getaddrinfo( hostname , NULL , &hints , &servinfo) ) != 0) {   
    // ATTENTION: should throw exception and append gai_strerror(rv), print and return 1 placeholder for now   
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));  
    return;
  }   
  // loop through all the results and connect to the first we can 
  for(p = servinfo; p != NULL; p = p->ai_next) {   
    h = (struct sockaddr_in *) p->ai_addr;    
    //ATTENTION: Should check that ip is large enough to copy given IP address   
    strcpy(ip , inet_ntoa( h->sin_addr ) );   
  }   
  freeaddrinfo(servinfo); // all done with this structure    
}    




IPBusConnection::IPBusConnection(std::string const & deviceTypeName,
				 std::vector<std::string> const & args):
  IPBusDeviceTypeName(deviceTypeName)
{
  Connect(args);  
}
IPBusConnection::IPBusConnection(std::shared_ptr<uhal::HwInterface> _hw):
  IPBusDeviceTypeName("EXTERNAL PTR"),
  hw(_hw)
{
}

IPBusConnection::~IPBusConnection(){
}

#define FILE_ADDR_ARG 0
#define ADDR_TABLE_PATH_ARG 1
#define PREFIX_ARG 2
void IPBusConnection::Connect(std::vector<std::string> const & arg){
  //--- inhibit most noise from uHAL
  uhal::setLogLevelTo(uhal::Error());

  if(0 == arg.size()){
    BUException::IPBUS_CONNECTION_ERROR e;
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
      hw = std::make_shared<uhal::HwInterface>(
					       uhal::ConnectionManager::getDevice(IPBusDeviceTypeName.c_str(),
										  uri,
										  addrTableFull));
    } catch( uhal::exception::exception& e) {
      e.append("Module::Connect() creating hardware device");
      printf("Error creating uHAL hardware device\n");
    }

  } else if( boost::regex_match( connectionFile.c_str(), reMatch, reXMLFile) ) {
    //===========================================================================
    //Connection File  
    //===========================================================================
    printf("Using .xml connection file...\n");
    std::string prefix_connectionFile("file://");
    prefix_connectionFile+=connectionFile;
    std::string connectionFileEntry("test.0");
    //using the uhal library to get the device from the connection file
#ifdef USE_UIO_UHAL
    uhal::ConnectionManager manager( prefix_connectionFile.c_str(),std::vector<std::string>(1,"uioaxi-1.0"));
#else
    uhal::ConnectionManager manager( prefix_connectionFile.c_str());
#endif
    //expect an argument specifying an entry in the connection file following the connection file name
    //e.g.: >connect connectionFile.xml test.0
    if(1 == arg.size()){
      printf("Warning: Input is a connection file but no device entry specified, using default entry name: %s\n", connectionFileEntry.c_str());
    }
    else connectionFileEntry = arg[1];

    try {
      hw = std::make_shared<uhal::HwInterface>( manager.getDevice ( connectionFileEntry.c_str() ));
    }  catch (uhal::exception::exception& e) {
      e.append("Module::Connect() creating hardware device");
      printf("Error while creating uHAL hardware device %s from connection file %s\n", connectionFileEntry.c_str(), connectionFile.c_str());
    }

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
      hw = std::make_shared<uhal::HwInterface>( uhal::ConnectionManager::getDevice(IPBusDeviceTypeName.c_str(), uri, addrTableFull + "/" + IPBusDeviceTypeName + ".xml"));
    } catch( uhal::exception::exception& e) {
      e.append("Module::Connect() creating hardware device");
      printf("Error creating uHAL hardware device\n");
    }

  }
}
