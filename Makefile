SHELL = bash

BUTOOL_PATH?=../../

UHAL_VER_MAJOR ?= 2
UHAL_VER_MINOR ?= 8

CXX?=g++
ifdef RUNTIME_LDPATH
RUNTIME_LDPATH_FLAG = -rpath=$(RUNTIME_LDPATH)
else
RUNTIME_LDPATH_FLAG = -rpath=$(BUTOOL_PATH)/lib,-rpath=$(CACTUS_ROOT)/lib
endif

LIB_IPBUS_IO = lib/libBUTool_IPBusIO.so
LIB_IPBUS_IO_SOURCES = $(wildcard src/IPBusIO/*.cpp)
LIB_IPBUS_IO_OBJECT_FILES = $(patsubst src/%.cpp,obj/%.o,${LIB_IPBUS_IO_SOURCES})

LIB_IPBUS_REG_HELPER = lib/libBUTool_IPBusRegHelpers.so
LIB_IPBUS_REG_HELPER_SOURCES = $(wildcard src/IPBusRegHelper/*.cpp)
LIB_IPBUS_REG_HELPER_OBJECT_FILES = $(patsubst src/%.cpp,obj/%.o,${LIB_IPBUS_REG_HELPER_SOURCES})

LIB_IPBUS_STATUS = lib/libBUTool_IPBusStatus.so
LIB_IPBUS_STATUS_SOURCES = $(wildcard src/IPBusStatus/*.cpp)
LIB_IPBUS_STATUS_OBJECT_FILES = $(patsubst src/%.cpp,obj/%.o,${LIB_IPBUS_STATUS_SOURCES})


INCLUDE_PATH += \
							-Iinclude  \
							-I$(BUTOOL_PATH)/include

LIBRARY_PATH += \
							-Llib \
							-L$(BUTOOL_PATH)/lib

ifdef BOOST_INC
INCLUDE_PATH +=-I$(BOOST_INC)
endif
ifdef BOOST_LIB
LIBRARY_PATH +=-L$(BOOST_LIB)
endif

LIBRARIES =    	-lboost_regex \
				-lBUTool_BUTextIO \
				-lBUTool_Helpers


INSTALL_PATH ?= ./install


CXX_FLAGS = -std=c++11 -g -O3 -rdynamic -Wall -MMD -MP -fPIC ${INCLUDE_PATH} -Werror -Wno-literal-suffix

CXX_FLAGS +=-fno-omit-frame-pointer -Wno-ignored-qualifiers -Werror=return-type -Wextra -Wno-long-long -Winit-self -Wno-unused-local-typedefs  -Woverloaded-virtual -DUHAL_VER_MAJOR=${UHAL_VER_MAJOR} -DUHAL_VER_MINOR=${UHAL_VER_MINOR} ${COMPILETIME_ROOT} ${FALLTHROUGH_FLAGS}

ifdef MAP_TYPE
CXX_FLAGS += ${MAP_TYPE}
endif

LINK_LIBRARY_FLAGS = -shared -Wl,--no-as-needed -fPIC -Wall -g -O3 -rdynamic ${LIBRARY_PATH} ${LIBRARIES} -Wl,${RUNTIME_LDPATH_FLAG} ${COMPILETIME_ROOT}



# ------------------------
# IPBUS stuff
# ------------------------
UHAL_LIBRARIES = -lcactus_uhal_log 		\
                 -lcactus_uhal_grammars 	\
                 -lcactus_uhal_uhal 	

ifdef UIO_UHAL_PATH
UHAL_LIBRARIES += -lUIOuHAL
CXX_FLAGS += -DUSE_UIO_UHAL
endif

# Search uHAL library from $IPBUS_PATH first then from $CACTUS_ROOT
ifdef IPBUS_PATH
UHAL_INCLUDE_PATH = \
	         					-isystem$(IPBUS_PATH)/uhal/uhal/include \
	         					-isystem$(IPBUS_PATH)/uhal/log/include \
	         					-isystem$(IPBUS_PATH)/uhal/grammars/include 
UHAL_LIBRARY_PATH = \
							-L$(IPBUS_PATH)/uhal/uhal/lib \
	         					-L$(IPBUS_PATH)/uhal/log/lib \
	         					-L$(IPBUS_PATH)/uhal/grammars/lib
else
UHAL_INCLUDE_PATH = \
	         					-isystem$(CACTUS_ROOT)/include

UHAL_LIBRARY_PATH = \
							-L$(CACTUS_ROOT)/lib  -Wl,-rpath=$(CACTUS_ROOT)/lib 
endif

ifdef UIO_UHAL_PATH
UHAL_INCLUDE_PATH += -isystem$(UIO_UHAL_PATH)/include
UHAL_LIBRARY_PATH += -L$(UIO_UHAL_PATH)/lib  -Wl,-rpath=$(UIO_UHAL_PATH)/lib
endif


UHAL_CXX_FLAGS = ${UHAL_INCLUDE_PATH}
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
_all: _cactus_env ${LIB_IPBUS_IO} ${LIB_IPBUS_REG_HELPER} ${LIB_IPBUS_STATUS}

_cactus_env:
ifdef IPBUS_PATH
	$(info using uHAL lib from user defined IPBUS_PATH=${IPBUS_PATH})
else ifdef CACTUS_ROOT
	$(info using uHAL lib from user defined CACTUS_ROOT=${CACTUS_ROOT})
else
	$(error Must define IPBUS_PATH or CACTUS_ROOT to include uHAL libraries (define through Makefile or command line\))
endif

# ------------------------
# IPBusRegHelper library
# ------------------------
${LIB_IPBUS_REG_HELPER}: ${LIB_IPBUS_REG_HELPER_OBJECT_FILES} 
	${CXX} ${LINK_LIBRARY_FLAGS} ${UHAL_LIBRARY_FLAGS} ${UHAL_LIBRARIES} $^ -o $@

${LIB_IPBUS_IO}: ${LIB_IPBUS_IO_OBJECT_FILES} 
	${CXX} ${LINK_LIBRARY_FLAGS} ${UHAL_LIBRARY_FLAGS} ${UHAL_LIBRARIES} $^ -o $@

${LIB_IPBUS_STATUS}: ${LIB_IPBUS_STATUS_OBJECT_FILES} 
	${CXX} ${LINK_LIBRARY_FLAGS} ${UHAL_LIBRARY_FLAGS} ${UHAL_LIBRARIES} $^ -o $@


# -----------------------
# install
# -----------------------
install: all
	 install -m 775 -d ${INSTALL_PATH}/lib
	 install -b -m 775 ./lib/* ${INSTALL_PATH}/lib
	 install -m 775 -d ${INSTALL_PATH}/include
	 cp -r include/* ${INSTALL_PATH}/include


#${LIB_IPBUS_REG_HELPER_OBJECT_FILES}: obj/%.o : src/%.cpp 
obj/%.o : src/%.cpp 
	mkdir -p $(dir $@)
	mkdir -p {lib,obj}
	${CXX} ${CXX_FLAGS} ${UHAL_CXX_FLAGS} -c $< -o $@



-include $(LIBRARY_OBJECT_FILES:.o=.d)

