Purpose:
This helping code wraps IPBus and provides the functions needed to connect the IPBus reads to the BUTool read/write commands.

include/IPBusIO/IPBusConnection.hh:
This class holds the IPBus uhal::HwInterface class and some code for creating that object. 
It also has warpped read/write functions for IPBus that come from the IPBusIO class

include/IPBusIO/IPBusIO.hh:
This class is handed a uhal::HwInterface and provides nice wrappers for read/write

include/IPBusRegHelper/IPBusRegHelper.hh:
This class maps our wrapped IPBus functions to the API from BUTool for nice looking reads/writes

Use:
First you should check out the BUTool source code, build it and source its env.sh script to get the correct enviornment. 

Then, type "make" to build the library.
This will create a env.sh script to be sourced to propery include this code
