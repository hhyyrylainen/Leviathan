message(STATUS "Copying Leviathan files...")

######### Debugging Symbols
# Create debugging info directory
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/Symbols")
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/Symbols/Dumps")

file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/bin/Test")


# Copy the tools
# Set the symbol tool name
if(UNIX)
  set(SYMBOL_EXTRACTOR "./CreateSymbolsForTarget.sh")
elseif(WIN32)
  set(SYMBOL_EXTRACTOR "./CreateSymbolsForTarget.bat")
endif()

file(COPY "${LEVIATHAN_SRC}/MoveSymbolFile.sh"
  "${LEVIATHAN_SRC}/MoveSymbolFile.bat" 
  "${LEVIATHAN_SRC}/CreateSymbolsForTarget.sh"
  "${LEVIATHAN_SRC}/CreateSymbolsForTarget.bat"
  DESTINATION "${PROJECT_BINARY_DIR}/Symbols")


# copy data from bin directory
# This will be all the binaries that need to be copied to bin/lib and installed to bin/lib
# Glob them all
if(WIN32)
  file(GLOB ALL_DYNAMIC_LIBRARIES "${LEVIATHAN_SRC}/build/ThirdParty/lib/*.dll")
else()
  file(GLOB ALL_DYNAMIC_LIBRARIES "${LEVIATHAN_SRC}/build/ThirdParty/lib/*.so*")
endif()

# The bin-folder files
file(GLOB RootFolderFiles "bin/*.conf" "bin/*.txt")

file(COPY ${RootFolderFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin")
install(FILES ${RootFolderFiles} DESTINATION bin)


# copy data directory
file(GLOB DataMoveFiles "bin/Data/*")
file(COPY ${DataMoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin/Data")

# The script files folder
install(DIRECTORY "Scripts" DESTINATION bin/Data)
install(DIRECTORY "CoreOgreScripts" DESTINATION bin)
# we need to specifically install the directories
install(DIRECTORY "bin/Data/Fonts" DESTINATION bin/Data)
install(DIRECTORY "bin/Data/Models" DESTINATION bin/Data)
install(DIRECTORY "bin/Data/Sound" DESTINATION bin/Data)
install(DIRECTORY "bin/Data/Textures" DESTINATION bin/Data)
install(DIRECTORY "bin/Data/Screenshots" DESTINATION bin/Data)
install(DIRECTORY "bin/Data/Cache" DESTINATION bin/Data)

# Copy data from the scripts folder to the bin folder
file(GLOB ScriptsMoveFiles "Scripts/*")
file(COPY ${ScriptsMoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin/Data/Scripts")

# Copy the crucial Ogre scripts
file(GLOB CoreOgreScriptsMoveFiles "CoreOgreScripts/*")
file(COPY ${CoreOgreScriptsMoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin/CoreOgreScripts")
install(FILES ${CoreOgreScriptsMoveFiles} DESTINATION "bin/CoreOgreScripts")

# Boost files
# If we are not using static linking we need to copy everything
if(NOT Boost_USE_STATIC_LIBS AND COPY_BOOST_TO_PACKAGE AND NOT WIN32)
    
  list(APPEND ALL_DYNAMIC_LIBRARIES
    ${Boost_DATE_TIME_LIBRARY_RELEASE} ${Boost_CHRONO_LIBRARY_RELEASE}
	${Boost_THREAD_LIBRARY_RELEASE} ${Boost_SYSTEM_LIBRARY_RELEASE})

endif()

if(WIN32)

  if(USING_LEAP)
    list(APPEND ALL_DYNAMIC_LIBRARIES "Leap/lib/x64/Leap.dll")
  endif()
  
  file(GLOB THIRD_PARTY_DLLS "${LEVIATHAN_SRC}/build/ThirdParty/lib/**/*.dll")
  list(APPEND ALL_DYNAMIC_LIBRARIES ${THIRD_PARTY_DLLS})
  
else()

  # linux variants of the copy functions
  if(USING_LEAP)
    list(APPEND ALL_DYNAMIC_LIBRARIES "Leap/lib/x64/libLeap.so")
  endif()
  
  file(GLOB THIRD_PARTY_DLLS "${LEVIATHAN_SRC}/build/ThirdParty/lib/**/*.so*")
  list(APPEND ALL_DYNAMIC_LIBRARIES ${THIRD_PARTY_DLLS})
  
endif(WIN32)

if(UNIX)
  # Go through all the libraries and add all name variants to the moved files
  GlobAllVariants(ALL_DYNAMIC_LIBRARIES SanitizedList)
  set(ALL_DYNAMIC_LIBRARIES ${SanitizedList})
endif()

set(RawFilesToMove ${ALL_DYNAMIC_LIBRARIES} ${SDL_LIBRARIES})
  
MakeUniqueAndSanitizeLibraryList(RawFilesToMove)

#message(STATUS "Required library list is: ${RawFilesToMove}")

if(WIN32)
  # To be able to debug move all the dlls to the bin folder
  file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
  file(COPY ${RawFilesToMove} DESTINATION "${PROJECT_BINARY_DIR}/bin")
else()
  file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/bin/lib")
  file(COPY ${RawFilesToMove} DESTINATION "${PROJECT_BINARY_DIR}/bin/lib")
endif()

# Tools
file(GLOB AllTools "${LEVIATHAN_SRC}/build/ThirdParty/bin/*")

file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/tools")
file(COPY ${AllTools} DESTINATION "${PROJECT_BINARY_DIR}/tools")
  
