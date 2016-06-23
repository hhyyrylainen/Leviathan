# CMake file for generating all project files
cmake_minimum_required(VERSION 2.8.3)

project(Leviathan)

# version number
set(LEVIATHAN_VERSION_STABLE 0)
set(LEVIATHAN_VERSION_MAJOR 8)
set(LEVIATHAN_VERSION_MINOR 0)
set(LEVIATHAN_VERSION_PATCH 0)

set(LEVIATHAN_VERSION_STR ${LEVIATHAN_VERSION_STABLE}.${LEVIATHAN_VERSION_MAJOR}.${LEVIATHAN_VERSION_MINOR}.${LEVIATHAN_VERSION_PATCH})
set(LEVIATHAN_VERSION ${LEVIATHAN_VERSION_STABLE}.${LEVIATHAN_VERSION_MAJOR}${LEVIATHAN_VERSION_MINOR}${LEVIATHAN_VERSION_PATCH})

set(WORK_DIR "${PROJECT_BINARY_DIR}/bin")
file(TO_NATIVE_PATH ${PROJECT_BINARY_DIR} PROJECT_NATIVE_BINARY_DIR)

# modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/CMake" "${PROJECT_SOURCE_DIR}/LeviathanCMake")

# Windows find modules
if(WIN32)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/Windows/CMake" )
endif()

# load cotire
include(cotire)

# Load utility files
include(LeviathanUtility)

# Just in case some unity builds are broken
#set_property(DIRECTORY PROPERTY COTIRE_UNITY_LINK_LIBRARIES_INIT "COPY")

######### Options
    
option(CREATE_UE4_PLUGIN "If set to ON creates a static version without any external libraries" OFF)
    
option(USE_ANGELSCRIPT "For using AngelScript, required for normal builds" ON)
option(USE_BOOST "For using AngelScript, required for normal builds" ON)
option(USE_OGRE "For using Ogre and direct dependencies, required for normal builds" ON)
option(USE_NEWTON "For using Newton, required for normal builds" ON)
option(USE_CAUDIO "For using cAudio, required for normal builds" ON)
option(USE_SFML "For using SFML, required for normal builds" ON)
    
option(CREATE_STATIC "Set to ON to create static libraries instead of shared" OFF)
    
# \todo this option to copy right libs
option(CREATE_SDK "For creating SDKs for developers" ON)

# Automatically builds symbols as part of the build and enables google breakpad handler
option(USE_BREAKPAD "For enabling google breakpad crash handling, set off for local debugging" ON)

# Copies boost libraries to distributions
option(COPY_BOOST_TO_PACKAGE "For copying all boost libraries to run on system that don't have it" OFF)

set(CMAKE_INSTALL_PREFIX "./Install" CACHE FILEPATH "Install path")

if(CREATE_UE4_PLUGIN)
  
  message(STATUS "Creating UE4 Compatible Static Library")
  set(USE_ANGELSCRIPT OFF)
  set(USE_BOOST OFF)
  set(USE_OGRE OFF)
  set(USE_NEWTON OFF)
  set(COPY_BOOST_TO_PACKAGE OFF)
  set(CREATE_STATIC ON)
  set(USE_BREAKPAD OFF)
  set(USE_CAUDIO OFF)
  set(USE_SFML OFF)
  
  set(CMAKE_INSTALL_PREFIX "./UE4_Plugin")
  
elseif(CREATE_SDK)

  set(CMAKE_INSTALL_PREFIX "../SDK")
  
endif()


if(USE_BREAKPAD)

  if(NOT EXISTS "${CMAKE_SOURCE_DIR}/Breakpad/include")
    message(SEND_ERROR "Breakpad include dir is missing")
  endif()

endif()

# Whether to use Javascript or php search in doxygen
option(DOCUMENTATION_LOCAL "Set to OFF to create web documentation" ON)

# Strip symbols from install
option(STRIP_SYMBOLS_ON_INSTALL "For stripping debug symbols on install" ON)


# Check is leap motion used
option(USE_LEAP "Set to on when using Leap motion, will auto disable if files are not copied" ON)

if(EXISTS "${PROJECT_SOURCE_DIR}/Leap/include/Leap.h")

  message(STATUS "Leap motion includes found")
  set(CAN_USE_LEAP ON)

  
else()

  if(NOT CREATE_UE4_PLUGIN)
    message(STATUS
      "Leap motion files aren't installed in leviathan/Leap, Leap motion support is disabled")
  endif()
  
  set(CAN_USE_LEAP OFF)
  
endif()

if(USE_LEAP AND CAN_USE_LEAP)
  set(USING_LEAP ON)
else()
  set(USING_LEAP OFF)
endif()


# For making libs with right names
set(CMAKE_DEBUG_POSTFIX "D")

# Output Paths #
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# First for the generic no-config case (e.g. with mingw)
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin )
# Second, for multi-config builds (e.g. msvc)
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_BINARY_DIR}/bin )
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_BINARY_DIR}/bin )
    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_BINARY_DIR}/bin )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

# Linking and platform specific
if(WIN32)
  # we want to build with unicode setting, also add minimal rebuild,
  # and exceptions from floating point operations, 
  # higher memory for compiling precompiled headers
  # Currently it seems that Zm250 should be enough but it might not be
  add_definitions(-DUNICODE -D_UNICODE -fp:except -Zm250)
  
  # program database flag for debug
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI -Gm /W3")
  
  # A policy is needed for launchers to work correctly
  
  # This allows the project target property be read the old way
  #cmake_policy(SET CMP0026 OLD)
  
  # This makes parameter evaluation expand things more aggressively(?)
  #cmake_policy(SET CMP0053 OLD)
  #cmake_policy(SET CMP0054 OLD)
  
else(WIN32)

  add_definitions(-fextended-identifiers)

  # Has to be on one line or else ';'s will be included
  # C++14
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wno-unused-function -Wno-unknown-pragmas -Wno-unused-variable -Wl,--no-undefined -Wl,--no-allow-shlib-undefined -Wno-pragma-once-outside-header")
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-reorder")
  
  # We need X11 on linux for window class to work
  find_package(X11)
  find_package(Threads)

  if(NOT X11_FOUND)
    message(SEND_ERROR "Failed to find X11")
  endif()

  # We need XMU X11 extension for clipboard
  if(NOT X11_Xmu_FOUND)
    message(SEND_ERROR "Failed to find X11-Xmu")
  endif()
  
  include_directories(${X11_INCLUDE_DIR})
endif(WIN32)

# When no configuration is set, enable RelWithDebInfo
if(CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
    "Set the build type, usually Release or RelWithDebInfo" FORCE)
endif(CMAKE_BUILD_TYPE STREQUAL "")

if(NOT WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Debug")
	# Not recommended configuration
	message(STATUS "Configuration is set to: " ${CMAKE_BUILD_TYPE})
	message(SEND_WARNING "On linux you should only try to build with Release or RelWithDebInfo")

endif()


# locating the Ogre folder
if(UNIX)
    if(USE_OGRE)
        if(EXISTS "/usr/local/lib/OGRE/cmake")

            set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/usr/local/lib/OGRE/cmake/")
            set(OGRE_BASEDIR "/usr/local/lib/OGRE/")
            
        elseif(EXISTS "/usr/lib/OGRE/cmake")
        
            set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/usr/lib/OGRE/cmake/")
            set(OGRE_BASEDIR "/usr/lib/OGRE/")
        else(EXISTS "/usr/local/lib/OGRE")
            message(SEND_ERROR "Failed to find Ogre module path")
        endif(EXISTS "/usr/local/lib/OGRE/cmake")
    endif()
endif(UNIX)

# Find required libraries

# Ogre is required
if(USE_OGRE AND UNIX)
    
    if(NOT USE_BOOST)
        message(SEND_ERROR "USE_BOOST required for Ogre")
    endif()
    
    if(NOT USE_NEWTON)
        message(SEND_ERROR "USE_NEWTON required for Ogre")
    endif()
    
    # Components that need linking (NB does not include header-only components like bind)
    set(OGRE_BOOST_COMPONENTS thread date_time)

    find_package(OGRE REQUIRED Plugin_ParticleFX Plugin_CgProgramManager Plugin_OctreeZone
      Overlay Paging RenderSystem_GL)

    # OIS is required for input
    find_package(OIS REQUIRED)

    # check if it failed
    if(NOT OIS_FOUND)
        message(SEND_ERROR "Failed to find OIS")
    endif(NOT OIS_FOUND)

    # Find CEGUI
    set(CEGUI_VERSION_MAJOR_DEFAULT 9999)
    find_package(CEGUI REQUIRED OgreRenderer CoreWindowRendererSet ExpatParser FreeImageImageCodec)
    # TODO: allow other parsers/codecs

    if(NOT CEGUI_FOUND)
      message(SEND_ERROR "Failed to locate CEGUI")
    endif()

    set(ALL_CEGUI_LIBRARIES ${CEGUI_LIBRARIES} ${CEGUI_OgreRenderer_LIBRARIES}
      ${CEGUI_CoreWindowRendererSet_LIBRARIES} 
      ${CEGUI_ExpatParser_LIBRARIES} ${CEGUI_FreeImageImageCodec_LIBRARIES})

    # set some additional libraries
    set(ADDITIONAL_OGRE ${OGRE_Plugin_ParticleFX_LIBRARIES} ${OGRE_Plugin_CgProgramManager_LIBRARIES}
      ${OGRE_Plugin_OctreeZone_LIBRARIES} ${OGRE_Overlay_LIBRARIES} ${OGRE_Paging_LIBRARIES}
      ${OGRE_RenderSystem_GL_LIBRARIES})

    # Find sfml
    if(USE_SFML)
        find_package(SFML 2 COMPONENTS network system)

        if(NOT SFML_FOUND)
            message(SEND_ERROR "Failed to find SFML 2")
        endif()
    endif()

    # Find cAudio
    if(USE_CAUDIO)

      find_package(cAudio)
      
      if(NOT cAudio_FOUND)
        message(SEND_ERROR "Failed to find cAudio")
      endif()
    endif()
      
      
endif()

# Find Boost
if(USE_BOOST)
    # Might be a good idea to dynamically link Boost
    set(Boost_USE_STATIC_LIBS FALSE)

    set(Boost_ADDITIONAL_VERSIONS "1.55" "1.53")

    # Other than these that are required are header-only libraries
    set(LEVIATHAN_BOOST_COMPONENTS chrono system filesystem)

    set(REQUIRED_BOOST_COMPONENTS ${LEVIATHAN_BOOST_COMPONENTS} ${OGRE_BOOST_COMPONENTS})

    find_package(Boost COMPONENTS ${REQUIRED_BOOST_COMPONENTS} QUIET)

    if(NOT Boost_FOUND)
        message("Boost not found, retrying with different settings")
        # Try again with the other type of libs
        set(Boost_USE_STATIC_LIBS NOT ${Boost_USE_STATIC_LIBS})
        find_package(Boost COMPONENTS ${OGRE_BOOST_COMPONENTS})
        
        if(NOT Boost_FOUND)
            message(SEND_ERROR "Failed to find Boost libraries: " ${REQUIRED_BOOST_COMPONENTS})        
        endif(NOT Boost_FOUND)
    endif(NOT Boost_FOUND)

    # Boost is found or the configuration has already failed

    # Set up referencing of Boost
    include_directories(${Boost_INCLUDE_DIR})

    # All libs are linked by cmake
    add_definitions(-DBOOST_ALL_NO_LIB)
endif(USE_BOOST)

#
# Worlds dirtiest hacks for windows
#
# This is because it is such a pain
if(WIN32)

    set(OGRE_LIBRARIES OgreMain.lib RenderSystem_GL.lib RenderSystem_GL3Plus.lib Plugin_ParticleFX.lib
        OIS.lib CEGUIBase-9999.lib CEGUICommonDialogs-9999.lib CEGUICoreWindowRendererSet.lib 
        CEGUIExpatParser.lib CEGUIOgreRenderer-9999.lib CEGUISILLYImageCodec.lib
    )
    
    set(SFML_LIBRARIES sfml-network.lib sfml-system.lib)
    set(cAudio_LIBRARIES cAudio.lib)
    
    include_directories("${CMAKE_SOURCE_DIR}/Windows/ThirdParty/include")
    include_directories("${CMAKE_SOURCE_DIR}/Windows/ThirdParty/include/cAudio")
    include_directories("${CMAKE_SOURCE_DIR}/Windows/ThirdParty/include/OIS")
    include_directories("${CMAKE_SOURCE_DIR}/Windows/ThirdParty/include/OGRE")
    link_directories("${CMAKE_SOURCE_DIR}/Windows/ThirdParty/lib")
    

endif()

# Library linking
# And Debug libraries for windows
if(USE_NEWTON)
    list(APPEND LINK_LIBS_TOENGINE_RELEASE optimized Newton)
    list(APPEND LINK_LIBS_TOENGINE_DEBUG debug Newton)
    
    list(APPEND LINK_LIBS_ENGINE_RELEASE optimized Newton)
    list(APPEND LINK_LIBS_ENGINE_DEBUG debug Newton)
endif()

if(USE_ANGELSCRIPT)

    if(WIN32)
        list(APPEND LINK_LIBS_ENGINE_RELEASE optimized angelscript64)
        list(APPEND LINK_LIBS_ENGINE_DEBUG debug angelscript64d)
        
    else()
        list(APPEND LINK_LIBS_ENGINE_RELEASE optimized angelscript)
    endif()
endif()

if(USING_LEAP)
  list(APPEND LINK_LIBS_ENGINE_RELEASE optimized Leap)
  #list(APPEND LINK_LIBS_ENGINE_DEBUG debug Leapd)
endif()

# Breakpad libraries
if(USE_BREAKPAD)
  if(UNIX)
    list(APPEND LINK_LIBS_TOENGINE_RELEASE optimized breakpad_client)
  elseif(WIN32)
    list(APPEND LINK_LIBS_TOENGINE_RELEASE optimized exception_handler
      optimized common optimized crash_generation_client)
  else()
  
  endif()
endif()

# create the bin directory
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

# Tests can crash if this folder doesn't exist
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/bin/Test")

# Setup predefined macros

DefinePreprocessorMacro(USE_ANGELSCRIPT LEVIATHAN_USING_ANGELSCRIPT)
DefinePreprocessorMacro(USE_BOOST LEVIATHAN_USING_BOOST)
DefinePreprocessorMacro(USE_OGRE LEVIATHAN_USING_OGRE)
DefinePreprocessorMacro(USE_NEWTON LEVIATHAN_USING_NEWTON)
DefinePreprocessorMacro(USE_SFML LEVIATHAN_USING_SFML)
DefinePreprocessorMacro(USE_LEAP LEVIATHAN_USING_LEAP)

DefinePreprocessorMacro(USE_BREAKPAD LEVIATHAN_USING_BREAKPAD)



DefinePreprocessorMacro(CREATE_UE4_PLUGIN LEVIATHAN_CREATE_UE4_PLUGIN)

# Configure engine include file
configure_file("${PROJECT_SOURCE_DIR}/Engine/Include.h.in" "${PROJECT_SOURCE_DIR}/Engine/Include.h")

# configure the doxygen file
find_package(Doxygen)
if(DOXYGEN_FOUND)

  if(DOCUMENTATION_LOCAL)
    set(DOXYGEN_SERVER_SEARCH "FALSE")
  else()
    set(DOXYGEN_SERVER_SEARCH "TRUE")
  endif()
  
  configure_file("${PROJECT_SOURCE_DIR}/LeviathanDoxy.in"
    "${PROJECT_BINARY_DIR}/LeviathanDoxy" @ONLY)
    add_custom_target(doc
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/LeviathanDoxy
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif(DOXYGEN_FOUND)

message(STATUS "Copying files...")

######### Debugging Symbols
# Create debugging info directory
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/Symbols")
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/Symbols/Dumps")

file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

file(COPY "${PROJECT_SOURCE_DIR}/MoveSymbolFile.sh"
  "${PROJECT_SOURCE_DIR}/MoveSymbolFile.bat" 
  "${PROJECT_SOURCE_DIR}/CreateSymbolsForTarget.sh"
  "${PROJECT_SOURCE_DIR}/CreateSymbolsForTarget.bat"
  DESTINATION "${PROJECT_BINARY_DIR}/Symbols")

# Copy the tools
# Set the symbol tool name

if(UNIX)
  set(SYMBOL_EXTRACTOR "./CreateSymbolsForTarget.sh")
elseif(WIN32)
  set(SYMBOL_EXTRACTOR "./CreateSymbolsForTarget.bat")
endif()

# copy data from bin directory
# This will be all the binaries that need to be copied to bin/lib and installed to bin/lib
set(ALL_DYNAMIC_LIBRARIES)


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
    
  if(USE_NEWTON)
    list(APPEND ALL_DYNAMIC_LIBRARIES
      "Newton/bin/Newton.dll" "Newton/bin/Newton.dll")
  endif()
  
  file(GLOB THIRD_PARTY_DLLS "${CMAKE_SOURCE_DIR}/Windows/ThirdParty/bin/*")
  list(APPEND ALL_DYNAMIC_LIBRARIES ${THIRD_PARTY_DLLS})
  
else()

  # linux variants of the copy functions
  if(USING_LEAP)
    list(APPEND ALL_DYNAMIC_LIBRARIES "Leap/lib/x64/libLeap.so")
  endif()
  
  # newton
  if(USE_NEWTON)
    list(APPEND ALL_DYNAMIC_LIBRARIES "Newton/bin/libNewton.so")
  endif()
  
endif(WIN32)

if(USE_OGRE AND NOT WIN32)

  set(RawFilesToMove ${CEGUI_LIBRARIES} ${CEGUI_OgreRenderer_LIBRARIES}
    ${CEGUI_CoreWindowRendererSet_LIBRARIES} 
    ${CEGUI_ExpatParser_LIBRARIES} ${CEGUI_FreeImageImageCodec_LIBRARIES}
    ${CEGUI_OgreRenderer_LIBRARIES} ${NewtonMoveFiles} ${LeapMoveFiles} 
    ${OGRE_LIBRARIES} ${OGRE_Plugin_ParticleFX_LIBRARIES}
    ${OGRE_Plugin_CgProgramManager_LIBRARIES} 
    ${OGRE_Plugin_OctreeZone_LIBRARIES} ${OGRE_Overlay_LIBRARIES} ${OGRE_Paging_LIBRARIES}
    ${OGRE_RenderSystem_GL_LIBRARIES} ${OGRE_RenderSystem_GL3_LIBRARIES}
    ${SFML_LIBRARIES} ${OIS_LIBRARIES} ${cAudio_LIBRARIES})
  
  MakeUniqueAndSanitizeLibraryList(RawFilesToMove)
  list(APPEND ALL_DYNAMIC_LIBRARIES ${RawFilesToMove})
endif()


if(UNIX)
  # Go through all the libraries and add all name variants to the moved files
  GlobAllVariants(ALL_DYNAMIC_LIBRARIES SanitizedList)
  set(ALL_DYNAMIC_LIBRARIES ${SanitizedList})
endif()


if(WIN32)
  # To be able to debug move all the dlls to the bin folder 
  file(COPY ${ALL_DYNAMIC_LIBRARIES} DESTINATION "${PROJECT_BINARY_DIR}/bin")
else()
  file(COPY ${ALL_DYNAMIC_LIBRARIES} DESTINATION "${PROJECT_BINARY_DIR}/bin/lib")
endif()
file(COPY ${ALL_DYNAMIC_LIBRARIES} DESTINATION "${PROJECT_BINARY_DIR}/bin/lib")
install(FILES ${ALL_DYNAMIC_LIBRARIES} DESTINATION "bin/lib")

if(WIN32)

    # ogre tools
    file(GLOB OgreTools "${OGRE_BASEDIR}/bin/Release/*.exe")
    install(FILES ${OgreTools} DESTINATION tools)
    
endif(WIN32)


# working directory setting
if(WIN32)
    
else(WIN32)

    set(CMAKE_RPATH "$ORIGIN/:$ORIGIN/lib")
    set(CMAKE_INSTALL_RPATH "$ORIGIN/:$ORIGIN/lib")
    #set(CMAKE_BUILD_RPATH "$ORIGIN/:$ORIGIN/lib")

    set(CMAKE_BUILD_WITH_INSTALL_RPATH true)

endif(WIN32)


# additional include directories
include_directories("${PROJECT_SOURCE_DIR}/Engine/")

if(NOT WIN32)

    if(USE_CAUDIO)
      include_directories(${cAudio_INCLUDE_DIRS})
    endif()

    include_directories(${SFML_INCLUDE_DIR})

    if(USE_BREAKPAD)
      include_directories("${CMAKE_SOURCE_DIR}/Breakpad/include")
    endif()


    if(USE_OGRE)
        include_directories(${OIS_INCLUDE_DIRS}	${CEGUI_INCLUDE_DIRS} ${OGRE_INCLUDE_DIRS})
    endif()
endif()

if(USE_ANGELSCRIPT)
    include_directories("${PROJECT_SOURCE_DIR}/AngelScript/include")
    include_directories("${PROJECT_SOURCE_DIR}/AngelScript")
endif()

if(USE_NEWTON)
    include_directories("${PROJECT_SOURCE_DIR}/Newton/include")
endif()

if(USING_LEAP)
    include_directories("${PROJECT_SOURCE_DIR}/Leap/include")
    include_directories("${PROJECT_SOURCE_DIR}/Leap")
endif()

if(USE_ANGELSCRIPT)
    link_directories("${PROJECT_SOURCE_DIR}/AngelScript/lib")
endif()

if(USE_NEWTON)
    link_directories("${PROJECT_SOURCE_DIR}/Newton/lib/")
    link_directories("${PROJECT_SOURCE_DIR}/Newton/bin/")
endif()

if(USE_LEAP)
    link_directories("${PROJECT_SOURCE_DIR}/Leap/lib/x64")
endif()

# engine linking
link_directories("${PROJECT_BINARY_DIR}")
# these two might not be necessary anymore
link_directories("${PROJECT_BINARY_DIR}/Engine/Debug")
link_directories("${PROJECT_BINARY_DIR}/Engine/Release")
link_directories("${PROJECT_BINARY_DIR}/bin")

# Controls how LeviathanCoreProject.cmake works
set(CREATE_CONSOLE_APP OFF)

# First project must be the file generator
add_subdirectory(FileGenerator)

# engine project 
add_subdirectory(Engine)

# test project
add_subdirectory(LeviathanTest)

if(NOT CREATE_UE4_PLUGIN)
  # example pong master server
  add_subdirectory(PongMasterServer)

  # example pong server
  add_subdirectory(PongServer)

  # pong example game
  add_subdirectory(Pong)
endif()

# Strip symbols
if(STRIP_SYMBOLS_ON_INSTALL AND UNIX)
  
  # Install code that strips the symbols
  install(SCRIPT LeviathanCMake/StripSymbolsFromDependencies.cmake)

endif()

##### All symbol creating target
if(${USE_BREAKPAD})
  add_custom_target(All_Symbols ALL DEPENDS Engine_Symbols Pong_Symbols PongServer_Symbols PongMasterServer_Symbols)
else()
  add_custom_target(All_Symbols DEPENDS Engine_Symbols Pong_Symbols PongServer_Symbols PongMasterServer_Symbols)
endif()

# .desktop files
if(UNIX)
  
  add_custom_target(Test_desktopfile "./CreateLinuxIcon.rb"
    "${CMAKE_BINARY_DIR}/bin/LeviathanTest" "--name" "LeviathanTesting application" "-t"
    DEPENDS LeviathanTest WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/" VERBATIM)

  add_custom_target(Pong_desktopfile "./CreateLinuxIcon.rb"
    "${CMAKE_BINARY_DIR}/bin/Pong" "${CMAKE_SOURCE_DIR}/Pong/ICON1.ico"
    "--name" "Pong client application" "--prefix" "Pong"
    DEPENDS Pong WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/" VERBATIM)

  add_custom_target(PongServer_desktopfile "./CreateLinuxIcon.rb"
    "${CMAKE_BINARY_DIR}/bin/PongServer" "${CMAKE_SOURCE_DIR}/PongServer/ICON1.ico"
    "--name" "Pong server application" "--prefix" "PongServer"
    DEPENDS PongServer WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/" VERBATIM)

    add_custom_target(PongMasterServer_desktopfile "./CreateLinuxIcon.rb"
    "${CMAKE_BINARY_DIR}/bin/PongMasterServer" "${CMAKE_SOURCE_DIR}/PongMasterServer/ICON1.ico"
    "--name" "Pong masterserver application" "--prefix" "PongMaster"
    DEPENDS PongMasterServer WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/" VERBATIM)
  
  add_custom_target(All_desktopfile ALL DEPENDS Test_desktopfile Pong_desktopfile
    PongServer_desktopfile PongMasterServer_desktopfile)

  install(FILES "${CMAKE_BINARY_DIR}/bin/LeviathanTest.desktop" DESTINATION bin)
  
endif()


# CPack support
if(NOT CREATE_UE4_PLUGIN)
    include(InstallRequiredSystemLibraries)

    SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Leviathan Game Engine")
    SET(CPACK_PACKAGE_VENDOR "Henri Hyyryläinen")
    SET(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/README.md")
    set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/License.txt")
    set(CPACK_PACKAGE_VERSION_MAJOR "${LEVIATHAN_VERSION_STABLE}.${LEVIATHAN_VERSION_MAJOR}")
    set(CPACK_PACKAGE_VERSION_MINOR "${LEVIATHAN_VERSION_MINOR}")

    if(INSTALL_CREATE_SDK)
        set(CPACK_PACKAGE_VERSION_PATCH "${LEVIATHAN_VERSION_PATCH}-sdk")
    else(INSTALL_CREATE_SDK)
        set(CPACK_PACKAGE_VERSION_PATCH "${LEVIATHAN_VERSION_PATCH}")
    endif(INSTALL_CREATE_SDK)

    if(STRIP_SYMBOLS_ON_INSTALL)
      set(CPACK_STRIP_FILES true)
    endif()

    if(WIN32)
      # There is a bug in NSIS that does not handle full unix paths properly. Make
      # sure there is at least one set of four (4) backlasshes.
      set(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}\\\\LeviathanIcon.bmp")
      set(CPACK_NSIS_DISPLAY_NAME "Leviathan Game Engine")
      set(CPACK_NSIS_HELP_LINK "http:\\\\\\\\boostslair.com/Projects/Leviathan/doxygen/")
      set(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.boostslair.com/Projects/Leviathan/")
      set(CPACK_NSIS_CONTACT "boost@boostslair.com")
      set(CPACK_NSIS_MODIFY_PATH OFF)
    else(WIN32)
      set(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/LeviathanIcon.bmp")
      set(CPACK_NSIS_DISPLAY_NAME "Leviathan Game Engine")
      set(CPACK_NSIS_HELP_LINK "http://www.boostslair.com/Projects/Leviathan/doxygen/")
      set(CPACK_NSIS_URL_INFO_ABOUT "http://www.boostslair.com/Projects/Leviathan/")
      set(CPACK_NSIS_CONTACT "boost@boostslair.com")
      set(CPACK_NSIS_MODIFY_PATH OFF)
    endif(WIN32)

    include(CPack)

else()

    # Includes for UE4 are setup in Engine cmake file

endif()
