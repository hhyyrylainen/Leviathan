# File that defines common project based on variables

# Define the project
if(WIN32 AND NOT CREATE_CONSOLE_APP)
    add_executable(${CurrentProjectName} WIN32 ${AllProjectFiles})
else()
    add_executable(${CurrentProjectName} ${AllProjectFiles})
endif()

set_target_properties(${CurrentProjectName} PROPERTIES DEBUG_POSTFIX D)

if(NOT SKIP_INSTALL)
  install(TARGETS ${CurrentProjectName} DESTINATION bin)

  # Strip symbols
  if(STRIP_SYMBOLS_ON_INSTALL AND UNIX)
    
    # Install code that strips the symbols
    install(CODE "execute_process(COMMAND strip ${CurrentProjectName} WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}/bin\")")

  endif()
endif()

# linking to engine
target_link_libraries(${CurrentProjectName} Engine ${ProjectCommonLibs} ${DEPENDENT_LIBS})

# speed up build
#cotire(${CurrentProjectName})

if(WIN32)

  # Set debugging work directory
  set_target_properties(${CurrentProjectName} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY
    "${CMAKE_BINARY_DIR}/bin")

endif(WIN32)

# Creating symbols after building
# When not USE_BREAKPAD this won't be ran automatically
if(UNIX)
add_custom_target(${CurrentProjectName}_Symbols ${SYMBOL_EXTRACTOR} "${CMAKE_BINARY_DIR}/bin/${CurrentProjectName}"
  DEPENDS ${CurrentProjectName} WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/Symbols VERBATIM)
endif()



