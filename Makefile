PackagePath = $(shell pwd)


LIB_IPBUS_IO = lib/libBUTool_IPBusIO.so
LIB_IPBUS_IO_SOURCES = $(wildcard src/IPBusIO/*.cpp)
LIB_IPBUS_IO_OBJECT_FILES = $(patsubst src/%.cpp,obj/%.o,${LIB_IPBUS_IO_SOURCES})
LIB_IPBUS_IO_DEPLIBS = libToolException.so
LIB_IPBUS_IO_LDFLAGS = $(patsubt lib%.so/-l%,${LIB_IPBUS_IO_DEPLIBS})

LIB_IPBUS_REG_HELPER = lib/libBUTool_IPBusRegHelpers.so
LIB_IPBUS_REG_HELPER_SOURCES = $(wildcard src/IPBusRegHelper/*.cpp)
LIB_IPBUS_REG_HELPER_OBJECT_FILES = $(patsubst src/%.cpp,obj/%.o,${LIB_IPBUS_REG_HELPER_SOURCES})
LIB_IPBUS_REG_HELPER_DEPLIBS = libToolException.so libBUTool_Helpers.so libBUTool_IPBusIO.so
LIB_IPBUS_REG_HELPER_LDFLAGS = $(patsubt lib%.so/-l%,${LIB_IPBUS_REG_HELPER_DEPLIBS})

INCLUDE_PATH = \
							-Iinclude  \
							-I$(BUTOOL_PATH)/include

LIBRARY_PATH = \
							-Llib \
							-L$(BUTOOL_PATH)/lib

ifdef BOOST_INC
INCLUDE_PATH +=-I$(BOOST_INC)
endif
ifdef BOOST_LIB
LIBRARY_PATH +=-L$(BOOST_LIB)
endif

LIBRARIES =    	-lboost_regex




CPP_FLAGS = -std=c++11 -g -O3 -rdynamic -Wall -MMD -MP -fPIC ${INCLUDE_PATH} -Werror -Wno-literal-suffix

CPP_FLAGS +=-fno-omit-frame-pointer -Wno-ignored-qualifiers -Werror=return-type -Wextra -Wno-long-long -Winit-self -Wno-unused-local-typedefs  -Woverloaded-virtual

LINK_LIBRARY_FLAGS = -shared -fPIC -Wall -g -O3 -rdynamic ${LIBRARY_PATH} ${LIBRARIES} -Wl,-rpath=$(BUTOOL_PATH)/lib,-rpath=$(CACTUS_ROOT)/lib



# ------------------------
# IPBUS stuff
# ------------------------
UHAL_LIBRARIES = -lcactus_uhal_log 		\
                 -lcactus_uhal_grammars 	\
                 -lcactus_uhal_uhal 		

UHAL_INCLUDE_PATH = \
	         					-isystem$(CACTUS_ROOT)/include 

UHAL_LIBRARY_PATH = \
							-L$(CACTUS_ROOT)/lib 

UHAL_CPP_FLAGHS = ${UHAL_INCLUDE_PATH}

UHAL_LIBRARY_FLAGS = ${UHAL_LIBRARY_PATH}





.PHONY: all _all clean _cleanall build _buildall _cactus_env

default: build
clean: _cleanall
_cleanall:
	rm -rf obj
	rm -rf bin
	rm -rf lib


all: _all
build: _all
buildall: _all
#_all: _cactus_env ${LIB_IPBUS_REG_HELPER} ${LIB_IPBUS_IO}
_all: _cactus_env ${LIB_IPBUS_IO}

_cactus_env: 
ifndef CACTUS_ROOT
	$(warning CACTUS_ROOT enviornment variable is not set)
	$(warning CACTUS_ROOT must list the path containing the CACTUS libraries and must be added to the LD_LIBRARY_PATH)
	$(error   CACTUS NOT FOUND)
endif

# ------------------------
# IPBusRegHelper library
# ------------------------
#${LIB_IPBUS_REG_HELPER}: ${LIB_IPBUS_REG_HELPER_OBJECT_FILES} 
#	g++ ${LINK_LIBRARY_FLAGS} ${UHAL_LIBRARY_FLAGS} ${UHAL_LIBRARIES} ${LIB_IPBUS_REG_HELPER_LDFLAGS} $^ -o $@

${LIB_IPBUS_IO}: ${LIB_IPBUS_IO_OBJECT_FILES} 
	g++ ${LINK_LIBRARY_FLAGS} ${UHAL_LIBRARY_FLAGS} ${UHAL_LIBRARIES} ${LIB_IPBUS_REG_HELPER_LDFLAGS} $^ -o $@


#${LIB_IPBUS_REG_HELPER_OBJECT_FILES}: obj/%.o : src/%.cpp 
obj/%.o : src/%.cpp 
	mkdir -p $(dir $@)
	mkdir -p {lib,obj}
	g++ ${CPP_FLAGS} ${UHAL_CPP_FLAGHS} -c $< -o $@



-include $(LIBRARY_OBJECT_FILES:.o=.d)

