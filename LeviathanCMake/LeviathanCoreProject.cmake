# File that defines common project based on variables

message(STATUS "Adding project: " ${CurrentProjectName})

add_definitions(-DLEVIATHAN_BUILD)

if(MAKE_RELEASE)
  add_definitions(-DMAKE_RELEASE)
endif()

# # This doesn't work as this requires absolute paths...
# if(EXISTS /lib64/ld-linux-x86-64.so.2)
#   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--dynamic-linker=$ORIGIN/lib/ld-linux.so.2")
# endif()

# Define the project
if(WIN32 AND NOT CREATE_CONSOLE_APP)
    add_executable(${CurrentProjectName} WIN32 ${AllProjectFiles})
else()
    add_executable(${CurrentProjectName} ${AllProjectFiles})
endif()

set_target_properties(${CurrentProjectName} PROPERTIES DEBUG_POSTFIX D)
set_property(TARGET ${CurrentProjectName} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${CurrentProjectName} PROPERTY CXX_EXTENSIONS OFF)

# For alternative runtime library
# if(WIN32)
#   target_compile_options(${CurrentProjectName} PRIVATE $<$<CONFIG:Debug>:/MTd>)
#   target_compile_options(${CurrentProjectName} PRIVATE $<$<CONFIG:Release>:/MT>)
#   target_compile_options(${CurrentProjectName} PRIVATE $<$<CONFIG:RelWithDebInfo>:/MT>)
#   target_compile_options(${CurrentProjectName} PRIVATE $<$<CONFIG:MinSizeRel>:/MT>)
# endif()

install(TARGETS ${CurrentProjectName} DESTINATION bin)

# linking to engine
target_link_libraries(${CurrentProjectName} Engine)

target_link_libraries(${CurrentProjectName} ${LEVIATHAN_APPLICATION_LIBRARIES})

# speed up build
#cotire(${CurrentProjectName})

if(WIN32)

  # Set debugging work directory
  set_target_properties(${CurrentProjectName} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY
    "${CMAKE_BINARY_DIR}/bin")

endif(WIN32)


