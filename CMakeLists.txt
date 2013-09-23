# CMake File for wsPublish
# Written by Ethan "flibitijibibo" Lee

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(wsPublish)

# Build Options
OPTION(WSP_DEBUG "Build wsPublish with debugging symbols" ON)

# Architecture Flags
IF(APPLE)
	SET(ARCHFLAG "-arch i386 -arch x86_64")
	SET(CMAKE_EXECUTABLE_SUFFIX ".osx")
ELSEIF(WIN32)
	SET(ARCHFLAG "-m32")
ELSE()
	# For now...
	SET(ARCHFLAG "-m32")
	SET(CMAKE_EXECUTABLE_SUFFIX ".x86")
ENDIF()

# Compiler Flags
IF(WSP_DEBUG)
	SET(CMAKE_C_FLAGS "${ARCHFLAG} -g -Wall")
	SET(CMAKE_CXX_FLAGS "${ARCHFLAG} -g -Wall")
ELSE()
	SET(CMAKE_C_FLAGS "${ARCHFLAG} -O2 -Wall -pedantic")
	SET(CMAKE_CXX_FLAGS "${ARCHFLAG} -O2 -Wall")
ENDIF()

# Steam SDK Directories
IF(WIN32)
	SET(STEAMINC "C:\\Users\\Ethan\ Lee\\STEAMBUILD\\sdk\\public")
	SET(STEAMLIB "C:\\Users\\Ethan\ Lee\\STEAMBUILD\\sdk\\redistributable_bin")
ELSE()
	IF(APPLE)
		SET(HOMEDIR "Users")
		SET(OSDIR "osx32")
	ELSE()
		SET(HOMEDIR "home")
		SET(OSDIR "linux32")
	ENDIF()
	SET(STEAMINC "/${HOMEDIR}/flibitijibibo/Programming/cppLibraries/sdk/public")
	SET(STEAMLIB "/${HOMEDIR}/flibitijibibo/Programming/cppLibraries/sdk/redistributable_bin/${OSDIR}")
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
	cmdtool/platform.c
	cmdtool/main.c
)

ADD_LIBRARY(wsPublish SHARED ${WSP_SRC})
TARGET_LINK_LIBRARIES(wsPublish steam_api)

ADD_EXECUTABLE(cmdtool ${CMD_SRC})
TARGET_LINK_LIBRARIES(cmdtool wsPublish physfs)
