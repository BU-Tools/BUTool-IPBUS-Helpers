BUTOOL_PATH?=../../

CXX?=g++

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




CXX_FLAGS = -std=c++11 -g -O3 -rdynamic -Wall -MMD -MP -fPIC ${INCLUDE_PATH} -Werror -Wno-literal-suffix

CXX_FLAGS +=-fno-omit-frame-pointer -Wno-ignored-qualifiers -Werror=return-type -Wextra -Wno-long-long -Winit-self -Wno-unused-local-typedefs  -Woverloaded-virtual ${COMPILETIME_ROOT} ${FALLTHROUGH_FLAGS}

LINK_LIBRARY_FLAGS = -shared -fPIC -Wall -g -O3 -rdynamic ${LIBRARY_PATH} ${LIBRARIES} -Wl, ${COMPILETIME_ROOT}



# ------------------------
# IPBUS stuff
# ------------------------
UHAL_LIBRARIES = -lcactus_uhal_log 		\
                 -lcactus_uhal_grammars 	\
                 -lcactus_uhal_uhal 		

# Search uHAL library from $IPBUS_PATH first then from $CACTUS_ROOT
ifdef IPBUS_PATH
$(info using uHAL lib from user defined IPBUS_PATH=${IPBUS_PATH})
UHAL_INCLUDE_PATH = \
	         					-isystem$(UHAL_PATH)/uhal/include \
	         					-isystem$(UHAL_PATH)/log/include \
	         					-isystem$(UHAL_PATH)/grammars/include 
UHAL_LIBRARY_PATH = \
							-L$(UHAL_PATH)/uhal/lib \
	         					-L$(UHAL_PATH)/log/lib \
	         					-L$(UHAL_PATH)/grammars/lib 
else
ifdef CACTUS_ROOT
$(info using uHAL lib from user defined CACTUS_ROOT=${CACTUS_ROOT})
UHAL_INCLUDE_PATH = \
	         					-isystem$(CACTUS_ROOT)/include 

UHAL_LIBRARY_PATH = \
							-L$(CACTUS_ROOT)/lib 
else
$(error Must define IPBUS_PATH or CACTUS_ROOT to include uHAL libraries (define through Makefile or command line\))
endif
endif

UHAL_CXX_FLAGHS = ${UHAL_INCLUDE_PATH}

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
_all: ${LIB_IPBUS_IO}


# ------------------------
# IPBusRegHelper library
# ------------------------
#${LIB_IPBUS_REG_HELPER}: ${LIB_IPBUS_REG_HELPER_OBJECT_FILES} 
#	${CXX} ${LINK_LIBRARY_FLAGS} ${UHAL_LIBRARY_FLAGS} ${UHAL_LIBRARIES} ${LIB_IPBUS_REG_HELPER_LDFLAGS} $^ -o $@

${LIB_IPBUS_IO}: ${LIB_IPBUS_IO_OBJECT_FILES} 
	${CXX} ${LINK_LIBRARY_FLAGS} ${UHAL_LIBRARY_FLAGS} ${UHAL_LIBRARIES} ${LIB_IPBUS_REG_HELPER_LDFLAGS} $^ -o $@


#${LIB_IPBUS_REG_HELPER_OBJECT_FILES}: obj/%.o : src/%.cpp 
obj/%.o : src/%.cpp 
	mkdir -p $(dir $@)
	mkdir -p {lib,obj}
	${CXX} ${CXX_FLAGS} ${UHAL_CXX_FLAGHS} -c $< -o $@



-include $(LIBRARY_OBJECT_FILES:.o=.d)

