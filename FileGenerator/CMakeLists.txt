# FileGenerator game CMake
# This project will generate various headers and resources required by other projects

# get all files in their groups
file(GLOB PSourceFiles "*.cpp")
file(GLOB PHeaderFiles "*.h") 


# additional .txts
file(GLOB Additional "*.txt")
file(GLOB CMakeListFile "CMakeLists.txt")

list(REMOVE_ITEM Additional ${CMakeListFile})

# file grouping
source_group("Source" FILES ${PSourceFiles})
source_group("Header" FILES ${PHeaderFiles})
source_group("Additional" FILES ${Additional})

# all Pong files in one list
set(AllProjectFiles ${PSourceFiles} ${PHeaderFiles} ${Additional})

# Add the program
if(WIN32)
  add_executable(FileGenerator WIN32 ${AllProjectFiles})
else(WIN32)
  add_executable(FileGenerator ${AllProjectFiles})
endif(WIN32)

target_link_libraries(FileGenerator ${Boost_LIBRARIES})

set_target_properties(FileGenerator PROPERTIES DEBUG_POSTFIX D)

