# Sandboxie application CMake


# get all files in their groups
file(GLOB SBSourceFiles *.cpp)
file(GLOB SBHeaderFiles *.h) 

# resource files
file(GLOB SBResourceFiles "resource.h" "*.rc")

# file grouping
source_group("Source" FILES ${SBSourceFiles})
source_group("Header" FILES ${SBHeaderFiles})
source_group("resources" FILES ${SBResourceFiles})

# all Sandboxie files in one list
set(AllProjectFiles ${SBSourceFiles} ${SBHeaderFiles} ${SBResourceFiles})

set(CurrentProjectName Sandboxie)

# Include the common file
include(LeviathanCoreProject)

# The project is now defined
