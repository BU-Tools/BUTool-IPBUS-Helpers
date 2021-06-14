#include <IPBusStatus/IPBusStatus.hh>

#ifdef USE_UIO_UHAL
#include <ProtocolUIO.hpp>
#endif

void IPBusStatus::Process(std::string const & singleTable){  
  //Build tables
  std::vector<std::string> Names = myMatchRegex("*");
  //process all the nodes and build table structure
  for(std::vector<std::string>::iterator itName = Names.begin();
      itName != Names.end();
      itName++){
    //Get the list of parameters for this node
    uMap parameters = GetParameters(*itName);

    //In IPBus the description is separate, so we add it to parameters here
    parameters["Description"] = GetRegDescription(*itName);

    if(parameters.size() != 0){      
      for(uMap::iterator itTable = parameters.begin();
	  itTable != parameters.end();
	  itTable++){
      }
    }
    //Look for a Status parameter
    if(parameters.find("Status") != parameters.end()){        
      //Check for an table name
      uMap::iterator itTable = parameters.find("Table");
      
      std::string tableName = itTable->second;
      //Add this Address to our Tables if it matches our singleTable option, or we are looking at all tables
      if( singleTable.empty() || TableNameCompare(tableName,singleTable)){
	uint32_t val;
	try{
	  val = RegReadRegister(*itName);
#ifdef USE_UIO_UHAL
	}catch(uhal::exception::UIOBusError & e){
	  continue;
#endif
	}catch(std::exception & e){
	  throw e;
	}
	tables[tableName].Add(*itName,
			      val,
			      GetRegMask(*itName),
			      parameters);	
      }
    }
  }
}
