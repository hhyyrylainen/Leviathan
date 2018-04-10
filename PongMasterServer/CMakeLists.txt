# PongMasterServer CMake


# Version
set(PROGRAM_VERSION_STABLE 0)
set(PROGRAM_VERSION_MAJOR 3)
set(PROGRAM_VERSION_MINOR 0)
set(PROGRAM_VERSION_PATCH 0)

set(PROGRAM_VERSION_STR ${PROGRAM_VERSION_STABLE}.${PROGRAM_VERSION_MAJOR}.${PROGRAM_VERSION_MINOR}.${PROGRAM_VERSION_PATCH})
set(PROGRAM_VERSION ${PROGRAM_VERSION_STABLE}.${PROGRAM_VERSION_MAJOR}${PROGRAM_VERSION_MINOR}${PROGRAM_VERSION_PATCH})

set(PROGRAM_VERSIONS "\"${PROGRAM_VERSION_STR}\"")
set(PROGRAM_VERSIONS_ANSI "\"${PROGRAM_VERSION_STR}\"")

set(BaseProgramName "PONG_MASTER")
set(BaseIncludeFileName "PongMasterServerIncludes.h")
set(BaseSubFolder "PongMasterServer")

# Set all the settings
set(ProgramIncludesHeader "${BaseIncludeFileName}")
set(ProgramAppHeader "PongMasterServer.h")
set(ProgramNamespace Pong)

set(WorldFactoryClass "GameWorldFactory")
set(WorldFactoryInclude "Entities/GameWorldFactory.h")

# ------------------ ProgramConfiguration ------------------ #
set(PROGRAMCLASSNAME				PongMasterServer)
set(PROGRAMLOG						PongMaster)
set(ENGINECONFIGURATION				"./EngineConf.conf")
set(PROGRAMCONFIGURATION			"./PongMaster.conf")
set(PROGRAMKEYCONFIGURATION			"")
set(PROGRAMCHECKCONFIGFUNCNAME		"PongMasterServer::CheckGameConfigurationVariables")
set(PROGRAMCHECKKEYCONFIGFUNCNAME	"PongMasterServer::CheckGameKeyConfigVariables")
set(PROGRAMMASTERSERVERINFO			"MasterServerInformation(true, \"Pong_\" GAME_VERSIONS)")
set(WINDOWTITLEGENFUNCTION			"PongMasterServer::GenerateWindowTitle()")

set(USERREADABLEIDENTIFICATION		"\"Pong master version \" GAME_VERSIONS")
set(GAMENAMEIDENTIFICATION			Pong)
set(GAMEVERSIONIDENTIFICATION		GAME_VERSIONS)

set(PROGRAMUSE_CUSTOMJS 0)

# Configure the files
StandardConfigureExecutableMainAndInclude("${BaseIncludeFileName}" "Main.cpp"
  "${BaseSubFolder}" "${PROJECT_SOURCE_DIR}/${BaseSubFolder}")


# get all files in their groups
file(GLOB PSourceFiles *.cpp)
file(GLOB PHeaderFiles *.h) 


# resource files
file(GLOB PResourceFiles "resource.h" "*.rc")

# additional .txts
file(GLOB Additional "*.txt")
file(GLOB CMakeListFile "CMakeLists.txt")

list(REMOVE_ITEM Additional ${CMakeListFile})

# file grouping
source_group("Source" FILES ${PSourceFiles})
source_group("Header" FILES ${PHeaderFiles})
source_group("resources" FILES ${PResourceFiles})
source_group("Additional" FILES ${Additional})


# all PongMaster files in one list
set(AllProjectFiles ${PSourceFiles} ${PHeaderFiles} ${PResourceFiles} ${Additional})

set(CurrentProjectName PongMasterServer)
set(UsePrecompiledHeaderForProject TRUE)

set(PrecompiledHeaderName "PongMasterServerIncludes.h")
file(GLOB PrecompiledSourceName "PreCompiled.cpp")

# Include the common file
include(LeviathanCoreProject)

# The project is now defined


