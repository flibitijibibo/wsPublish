# CMake File for wsPublish
# Written by Ethan "flibitijibibo" Lee

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(wsPublish)

# Architecture Flags
IF(APPLE)
	set(CMAKE_OSX_DEPLOYMENT_TARGET 10.7)
	SET(CMAKE_EXECUTABLE_SUFFIX ".osx")
	SET(OSDIR "osx32")
	SET(BIN_RPATH "@executable_path")
	set(CMAKE_SKIP_BUILD_RPATH TRUE)
	set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	set(CMAKE_INSTALL_RPATH ${BIN_RPATH})
	set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
ELSEIF(WIN32)
	# Nothing to see here...
ELSEIF(CMAKE_SIZEOF_VOID_P MATCHES "8")
	SET(OSDIR "linux64")
	SET(CMAKE_EXECUTABLE_SUFFIX ".x86_64")
ELSE()
	SET(OSDIR "linux32")
	SET(CMAKE_EXECUTABLE_SUFFIX ".x86")
ENDIF()

# Steam SDK Directories
IF(WIN32)
	SET(STEAMINC "${CMAKE_SOURCE_DIR}\\sdk\\public")
	SET(STEAMLIB "${CMAKE_SOURCE_DIR}\\sdk\\redistributable_bin")
ELSE()
	SET(STEAMINC "${CMAKE_SOURCE_DIR}\\sdk/public")
	SET(STEAMLIB "${CMAKE_SOURCE_DIR}\\sdk/redistributable_bin/${OSDIR}")
ENDIF()

INCLUDE_DIRECTORIES(
	./include
	${STEAMINC}
)
LINK_DIRECTORIES(${STEAMLIB})

# Source Lists
SET(WSP_SRC
	src/wsPublish.cpp
)

SET(CMD_SRC
	cmdtool/json.c
	cmdtool/miniz.c
	cmdtool/platform.c
	cmdtool/main.c
)

ADD_LIBRARY(wsPublish SHARED ${WSP_SRC})
TARGET_LINK_LIBRARIES(wsPublish steam_api)

ADD_EXECUTABLE(cmdtool ${CMD_SRC})
IF(WIN32)
	TARGET_LINK_LIBRARIES(cmdtool wsPublish)
ELSE()
	TARGET_LINK_LIBRARIES(cmdtool wsPublish m)
ENDIF()
