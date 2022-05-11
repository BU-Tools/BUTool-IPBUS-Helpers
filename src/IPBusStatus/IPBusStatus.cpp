#include <IPBusStatus/IPBusStatus.hh>

#ifdef USE_UIO_UHAL
#include <ProtocolUIO.hpp>
#endif

IPBusStatus::IPBusStatus(IPBusIO * _regIO):
  StatusDisplay(dynamic_cast<BUTool::RegisterHelperIO*>(_regIO)){
};

void IPBusStatus::Process(std::string const & singleTable){  
  // Build tables
  std::vector<std::string> Names = regIO->GetRegsRegex("*");
  // Process all the nodes and build table structure
  for(std::vector<std::string>::iterator itName = Names.begin();
      itName != Names.end();
      itName++){
    // Get the list of parameters for this node (this is only in IPBusIO)
    uMap parameters = dynamic_cast<IPBusIO*>(regIO)->GetParameters(*itName);

    //In IPBus the description is separate, so we add it to parameters here
    parameters["Description"] = regIO->GetRegDescription(*itName);

    //Look for a Status parameter
    if(parameters.find("Status") != parameters.end()){        
      // Check for a table name
      uMap::iterator itTable = parameters.find("Table");
      
      std::string tableName = itTable->second;
      //Add this Address to our Tables if it matches our singleTable option, or we are looking at all tables
      if( singleTable.empty() || TableNameCompare(tableName,singleTable)){
        // Add the register to the given table, with a pointer to a RegHelperIO
        // class to read values later.
        tables[tableName].Add(*itName, regIO, parameters);
        // tables[tableName].Add(*itName,
        // 		      val,
        // 		      regIO->GetRegMask(*itName),
        // 		      parameters);	
      }
    }
  }
}
