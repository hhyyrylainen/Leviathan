# File that defines common project based on variables

if(MAKE_RELEASE)
  add_definitions("-DMAKE_RELEASE")
endif()

# Define the project
if(WIN32 AND NOT CREATE_CONSOLE_APP)
    add_executable(${CurrentProjectName} WIN32 ${AllProjectFiles})
else()
    add_executable(${CurrentProjectName} ${AllProjectFiles})
endif()

set_target_properties(${CurrentProjectName} PROPERTIES DEBUG_POSTFIX D)
set_property(TARGET ${CurrentProjectName} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${CurrentProjectName} PROPERTY CXX_EXTENSIONS OFF)

if(NOT SKIP_INSTALL)
  install(TARGETS ${CurrentProjectName} DESTINATION bin)
endif()

# This now includes CEF
# linking to engine
target_link_libraries(${CurrentProjectName} Engine ${ProjectCommonLibs} ${DEPENDENT_LIBS})

# speed up build
#cotire(${CurrentProjectName})

if(WIN32)

  # Set debugging work directory
  set_target_properties(${CurrentProjectName} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY
    "${CMAKE_BINARY_DIR}/bin")

endif(WIN32)

