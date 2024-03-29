PROJECT(ConfigurationFile)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

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
)

ADD_EXECUTABLE(ConfigFileInit
	ConfigFileInit.cpp)
TARGET_LINK_LIBRARIES(ConfigFileInit
	${FgLib5_LIBRARIES}
	${SISODISPLAYLIB_LIBRARIES}
)
SET_TARGET_PROPERTIES(ConfigFileInit PROPERTIES FOLDER "Examples/Functionality/ConfigurationFile")

ADD_EXECUTABLE(ConfigFileLoad
	ConfigFileLoad.cpp)
TARGET_LINK_LIBRARIES(ConfigFileLoad
	${FgLib5_LIBRARIES}
	${SISODISPLAYLIB_LIBRARIES}
)
SET_TARGET_PROPERTIES(ConfigFileLoad PROPERTIES FOLDER "Examples/Functionality/ConfigurationFile")

INSTALL(TARGETS
	ConfigFileInit
	ConfigFileLoad
	DESTINATION bin/examples)
