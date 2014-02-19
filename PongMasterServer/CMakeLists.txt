# PongMasterServer CMake


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


