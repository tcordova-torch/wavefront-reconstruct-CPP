CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(CXPFullExample)

#where to find local cmake modules
FILE(TO_CMAKE_PATH "$ENV{SISODIR5}" SISODIR5)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_INSTALL_PREFIX}/cmake/modules" "${SISODIR5}/cmake/modules")

INCLUDE(SisoPlatform)
FIND_PACKAGE(FgLib5 REQUIRED)
FIND_PACKAGE(SisoDisplayLib REQUIRED)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

INCLUDE_DIRECTORIES(
	${FgLib5_INCLUDE_DIR}
	${SISODISPLAYLIB_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../../include
	${CMAKE_CURRENT_SOURCE_DIR}/../CXPCommon
)

ADD_EXECUTABLE(CXPFullExample
    ${CMAKE_CURRENT_SOURCE_DIR}/../CXPCommon/CXPSDKCommon.cpp
	CXPFullExample.cpp)
	
TARGET_LINK_LIBRARIES(CXPFullExample
	${FgLib5_LIBRARIES}
	${SISODISPLAYLIB_LIBRARIES}
	${SISODIR5}/lib/interface/visualc/siso_genicam.lib
)

SET_TARGET_PROPERTIES(CXPFullExample PROPERTIES FOLDER "Examples/CameraInterfaces/CXP")


INSTALL(TARGETS
	CXPFullExample
	DESTINATION bin/examples/CXP)
