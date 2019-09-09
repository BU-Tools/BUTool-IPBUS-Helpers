#include <IPBusStatus/IPBusStatus.hh>
#include <uhal/ProtocolUIO.hpp>

void IPBusStatus::Process(std::string const & singleTable){  
  //Build tables
  std::vector<std::string> Names = myMatchRegex("*");
  //process all the nodes and build table structure
  for(std::vector<std::string>::iterator itName = Names.begin();
      itName != Names.end();
      itName++){
    //Get the list of parameters for this node
    boost::unordered_map<std::string,std::string>  parameters = GetParameters(*itName);
    if(parameters.size() != 0){      
      for(boost::unordered_map<std::string,std::string>::iterator itTable = parameters.begin();
	  itTable != parameters.end();
	  itTable++){
      }
    }
    //Look for a Status parameter
    if(parameters.find("Status") != parameters.end()){        
      //Check for an table name
      boost::unordered_map<std::string,std::string>::iterator itTable = parameters.find("Table");
      
      std::string tableName = itTable->second;
      //Add this Address to our Tables if it matches our singleTable option, or we are looking at all tables
      if( singleTable.empty() || TableNameCompare(tableName,singleTable)){
	uint32_t val;
	try{
	  val = RegReadRegister(*itName);
	}catch(uhal::exception::UIOBusError & e){
	  continue;
	}
	tables[tableName].Add(*itName,
			      val,
			      GetRegMask(*itName),
			      parameters);	
      }
    }
  }
}
