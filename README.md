## BUTool IPBus helpers

### Purpose
This helping code wraps IPBus and provides the functions needed to connect the IPBus reads to the BUTool read/write commands.

#### Files
> include/IPBusIO/IPBusConnection.hh:

This class holds the IPBus uhal::HwInterface class and some code for creating that object. 

It also has warpped read/write functions for IPBus that come from the IPBusIO class

> include/IPBusIO/IPBusIO.hh:

This class is handed a uhal::HwInterface and provides nice wrappers for read/write

> include/IPBusRegHelper/IPBusRegHelper.hh:

This class maps our wrapped IPBus functions to the API from BUTool for nice looking reads/writes

### Use

#### Plugin-dir
Check out BUTool from git, then checkout this repo in the BUTool plugins dir.

A "make" for BUTool will then build all subdirectories in the plugins dir. 

If building on a system with uHAL `2.8.x`, the variable `UHAL_VER_MINOR=8` needs to be set before building. By default this plugin expects uHAL `2.7.x`, so if the system is already using this version, no action is necessary. 

#### DIY
First you should check out the BUTool source code, build it and source its env.sh script to get the correct enviornment. 

Then, type "make" to build the library.
This will create a env.sh script to be sourced to propery include this code
