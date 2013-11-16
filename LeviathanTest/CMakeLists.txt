# testing application CMake



# get all files in their groups
file(GLOB LTSourceFiles *.cpp)
file(GLOB LTHeaderFiles *.h) 


# file grouping
source_group("Source" FILES ${LTSourceFiles})
source_group("Header" FILES ${LTHeaderFiles})


# all LeviathanTest files in one list
set(LeviathanTestFiles ${LTSourceFiles} ${LTHeaderFiles})

add_executable(LeviathanTest WIN32 ${LeviathanTestFiles})
set_target_properties(LeviathanTest PROPERTIES DEBUG_POSTFIX D)

# Possible debug installs
set(InstallConfigurationNames "Release")
if(NOT INSTALL_ONLY_RELEASE)
# Append the Debug configuration if we don't want only release release
list(APPEND InstallConfigurationNames "Debug")
endif(NOT INSTALL_ONLY_RELEASE)

install(TARGETS LeviathanTest DESTINATION bin CONFIGURATIONS ${InstallConfigurationNames})

# linking to engine
target_link_libraries(LeviathanTest Engine)

# add as default startup project
create_target_launcher(LeviathanTest RUNTIME_LIBRARY_DIRS "${PROJECT_BINARY_DIR}/bin/" WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/bin/")


# post build copy
ADD_CUSTOM_COMMAND(TARGET LeviathanTest POST_BUILD COMMAND copy ARGS 
    "\"$(SolutionDir)LeviathanTest\\$(Configuration)\\$(TargetName).exe\" \"$(SolutionDir)bin\\$(TargetName).exe\""
)

# testing
add_test(LeviathanTestMainTest LeviathanTest CONFIGURATIONS Debug Release WORKING_DIRECTORY 
    "${PROJECT_BINARY_DIR}/bin" COMMAND "TestOutDefault.txt")
set_tests_properties(LeviathanTestMainTest PROPERTIES PASS_REGULAR_EXPRESSION "testing completed succesfully")
