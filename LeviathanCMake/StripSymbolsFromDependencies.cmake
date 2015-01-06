# Get rid of all the large symbols


file(GLOB FilesToGetRidOf "${CMAKE_INSTALL_PREFIX}/bin/lib*.so*")

foreach(curlib ${FilesToGetRidOf})
  
  # Install code that strips the symbols
  execute_process(COMMAND strip ${curlib} WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/bin")

endforeach()
