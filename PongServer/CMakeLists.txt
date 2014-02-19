# PongServer game CMake

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

# all Pong files in one list
set(AllProjectFiles ${PSourceFiles} ${PHeaderFiles} ${PResourceFiles} ${Additional} ${Common})

set(CurrentProjectName PongServer)

# Include the common file
include(LeviathanCoreProject)

# The project is now defined
