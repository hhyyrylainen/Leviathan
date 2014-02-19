# LeviathanTest application CMake

# get all files in their groups
file(GLOB LTSourceFiles *.cpp)
file(GLOB LTHeaderFiles *.h) 

# file grouping
source_group("Source" FILES ${LTSourceFiles})
source_group("Header" FILES ${LTHeaderFiles})


# all LeviathanTest files in one list
set(AllProjectFiles ${LTSourceFiles} ${LTHeaderFiles})

set(CurrentProjectName LeviathanTest)

# Include the common file
include(LeviathanCoreProject)

# The project is now defined

# testing
add_test(LeviathanTestMainTest LeviathanTest CONFIGURATIONS Debug Release WORKING_DIRECTORY 
    "${PROJECT_BINARY_DIR}/bin")
set_tests_properties(LeviathanTestMainTest PROPERTIES PASS_REGULAR_EXPRESSION "testing completed succesfully")
