CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(CXP)

#where to find local cmake modules
FILE(TO_CMAKE_PATH "$ENV{SISODIR5}" SISODIR5)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_INSTALL_PREFIX}/cmake/modules" "${SISODIR5}/cmake/modules")

INCLUDE(SisoPlatform)
FIND_PACKAGE(FgLib5 REQUIRED)
FIND_PACKAGE(SisoDisplayLib REQUIRED)
FIND_PACKAGE(SisoGenicamLib REQUIRED)

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

INCLUDE_DIRECTORIES(
	${FgLib5_INCLUDE_DIR}
	${SISODISPLAYLIB_INCLUDE_DIR}
	${SisoGenicam_INCLUDE_DIR}	
	${CMAKE_CURRENT_SOURCE_DIR}/../include
	${CMAKE_CURRENT_SOURCE_DIR}/CXPCommon
)

#--------------------------------------------------------------------
ADD_EXECUTABLE(FullExample
	${CMAKE_CURRENT_SOURCE_DIR}/CXPCommon/CXPSDKCommon.cpp
	FullExample/CXPFullExample.cpp)
	
TARGET_LINK_LIBRARIES(FullExample
	${FgLib5_LIBRARIES}
	${SISODISPLAYLIB_LIBRARIES}
	${SisoGenicam_LIBRARIES}
)
SET_TARGET_PROPERTIES(FullExample PROPERTIES FOLDER "Examples/CameraInterfaces/CXP")


#--------------------------------------------------------------------

ADD_EXECUTABLE(GenicamCameraAccess
	${CMAKE_CURRENT_SOURCE_DIR}/CXPCommon/CXPSDKCommon.cpp
	GenicamCameraAccess/CXPGenicamCameraAccess.cpp)
	
TARGET_LINK_LIBRARIES(GenicamCameraAccess
	${FgLib5_LIBRARIES}
	${SISODISPLAYLIB_LIBRARIES}
	${SisoGenicam_LIBRARIES}
)
SET_TARGET_PROPERTIES(GenicamCameraAccess PROPERTIES FOLDER "Examples/CameraInterfaces/CXP")

#--------------------------------------------------------------------
ADD_EXECUTABLE(FreeRun
	${CMAKE_CURRENT_SOURCE_DIR}/CXPCommon/CXPSDKCommon.cpp
	GrabModes/FreeRun/CXPFreeRun.cpp)
	
TARGET_LINK_LIBRARIES(FreeRun
	${FgLib5_LIBRARIES}
	${SISODISPLAYLIB_LIBRARIES}
	${SisoGenicam_LIBRARIES}
)
SET_TARGET_PROPERTIES(FreeRun PROPERTIES FOLDER "Examples/CameraInterfaces/CXP/GrabModes")

#--------------------------------------------------------------------
ADD_EXECUTABLE(CXPDiscovery
	${CMAKE_CURRENT_SOURCE_DIR}/CXPCommon/CXPSDKCommon.cpp
	LinkDiscovery/CXPDiscovery/CXPDiscovery.cpp)
TARGET_LINK_LIBRARIES(CXPDiscovery
	${FgLib5_LIBRARIES}
	${SISODISPLAYLIB_LIBRARIES}
	${SisoGenicam_LIBRARIES}
)
SET_TARGET_PROPERTIES(CXPDiscovery PROPERTIES FOLDER "Examples/CameraInterfaces/CXP/LinkDiscovery")

#--------------------------------------------------------------------
ADD_EXECUTABLE(CXPManualLinkConfig
	${CMAKE_CURRENT_SOURCE_DIR}/CXPCommon/CXPSDKCommon.cpp
	LinkDiscovery/CXPManualLinkConfig/CXPManualLinkConfig.cpp)
	
TARGET_LINK_LIBRARIES(CXPManualLinkConfig
	${FgLib5_LIBRARIES}
	${SISODISPLAYLIB_LIBRARIES}
	${SisoGenicam_LIBRARIES}
	)
SET_TARGET_PROPERTIES(CXPManualLinkConfig PROPERTIES FOLDER "Examples/CameraInterfaces/CXP/LinkDiscovery")

#--------------------------------------------------------------------


INSTALL(TARGETS
	FullExample
	GenicamCameraAccess
	FreeRun
	CXPDiscovery
	CXPManualLinkConfig
	DESTINATION bin/examples/CXP)
