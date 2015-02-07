# PongServer game CMake

# Version
set(PROGRAM_VERSION_STABLE 0)
set(PROGRAM_VERSION_MAJOR 3)
set(PROGRAM_VERSION_MINOR 3)
set(PROGRAM_VERSION_PATCH 0)

set(PROGRAM_VERSION_STR ${PROGRAM_VERSION_STABLE}.${PROGRAM_VERSION_MAJOR}.${PROGRAM_VERSION_MINOR}.${PROGRAM_VERSION_PATCH})
set(PROGRAM_VERSION ${PROGRAM_VERSION_STABLE}.${PROGRAM_VERSION_MAJOR}${PROGRAM_VERSION_MINOR}${PROGRAM_VERSION_PATCH})

set(PROGRAM_VERSIONS "L\"${PROGRAM_VERSION_STR}\"")
set(PROGRAM_VERSIONS_ANSI "\"${PROGRAM_VERSION_STR}\"")

set(BaseProgramName "PONG_SERVER")
set(BaseIncludeFileName "PongIncludes.h")
set(BaseSubFolder "PongServer")

# Set all the settings
set(ProgramIncludesHeader "${BaseIncludeFileName}")
set(ProgramAppHeader "PongServer.h")
set(ProgramNetHeader "PongServerNetworking.h")
set(ProgramNamespace Pong)

# ------------------ ProgramConfiguration ------------------ #
set(PROGRAMCLASSNAME				PongServer)
set(PROGRAMNETWORKINGNAME			PongServerNetworking)
set(PROGRAMLOG						PongServer)
set(ENGINECONFIGURATION				"./EngineConf.conf")
set(PROGRAMCONFIGURATION			"./PongServer.conf")
set(PROGRAMKEYCONFIGURATION			"")
set(PROGRAMCHECKCONFIGFUNCNAME		"PongServer::CheckGameConfigurationVariables")
set(PROGRAMCHECKKEYCONFIGFUNCNAME	"PongServer::CheckGameKeyConfigVariables")
set(PROGRAMMASTERSERVERINFO			"MasterServerInformation(L\"PongServer.txt\", L\"Pong_\" GAME_VERSIONS, L\"http://boostslair.com/\", L\"/Pong/MastersList.txt\", L\"PongCrecentials.txt\", false)")
set(WINDOWTITLEGENFUNCTION			"PongServer::GenerateWindowTitle()")

set(USERREADABLEIDENTIFICATION		"L\"Pong server version \" GAME_VERSIONS")
set(GAMENAMEIDENTIFICATION			Pong)
set(GAMEVERSIONIDENTIFICATION		GAME_VERSIONS)

# We don't use a custom JS interface
set(PROGRAMUSE_CUSTOMJS             0)
set(PROGRAMCUSTOMJS_INCLUDE         ".h")
set(PROGRAMCUSTOMJS_CLASS           )
set(PROGRAMCUSTOMJS_EXTFILE         ".js")


# Configure the includes file
configure_file("${PROJECT_SOURCE_DIR}/File Templates/BaseLeviathanProjectIncludes.h.in" 
    "${PROJECT_SOURCE_DIR}/${BaseSubFolder}/${BaseIncludeFileName}")

# Configure the main file
configure_file("${PROJECT_SOURCE_DIR}/File Templates/BaseLeviathanProjectMain.cpp.in" 
    "${PROJECT_SOURCE_DIR}/${BaseSubFolder}/Main.cpp")



# get all files in their groups
file(GLOB PSourceFiles "*.cpp")
file(GLOB PHeaderFiles "*.h") 
file(GLOB Common "../PongCommon/*.h" "../PongCommon/*.cpp")

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
source_group("Common" FILES ${Common})


include_directories("${PROJECT_SOURCE_DIR}/PongCommon")


include_directories(${CMAKE_CURRENT_LIST_DIR})

# all Pong files in one list
set(AllProjectFiles ${PSourceFiles} ${PHeaderFiles} ${PResourceFiles} ${Additional} ${Common})

set(CurrentProjectName PongServer)

# Include the common file
include(LeviathanCoreProject)

# The project is now defined
