# CMake file for generating all project files
cmake_minimum_required(VERSION 2.8)

project(Leviathan)
set(WORK_DIR "${PROJECT_BINARY_DIR}/bin")

# modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/CMake")

# Stuff from example ogre cmake configuration to hopefully fix linux build #
if(WIN32)
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "$ENV{OGRE_HOME}/CMake/")
    set(OGRE_BASEDIR "$ENV{OGRE_HOME}/")
endif(WIN32)

if(UNIX)
	if(EXISTS "/usr/local/lib/OGRE/cmake")

        set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/usr/local/lib/OGRE/cmake/")
        set(OGRE_BASEDIR "/usr/local/lib/OGRE/")
	elseif(EXISTS "/usr/lib/OGRE/cmake")

        set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/usr/lib/OGRE/cmake/")
        set(OGRE_BASEDIR "/usr/lib/OGRE/")
	else(EXISTS "/usr/local/lib/OGRE")
        message(SEND_ERROR "Failed to find Ogre module path")
	endif(EXISTS "/usr/local/lib/OGRE/cmake")
endif(UNIX)
 
if(CMAKE_BUILD_TYPE STREQUAL "")
  # CMake defaults to leaving CMAKE_BUILD_TYPE empty. This screws up
  # differentiation between debug and release builds.
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: None (CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif(CMAKE_BUILD_TYPE STREQUAL "")

# Find required libraries
find_package(OGRE REQUIRED Plugin_ParticleFX Plugin_CgProgramManager Plugin_OctreeZone Overlay Paging)

find_package(OIS REQUIRED)
 
if(NOT OIS_FOUND)
	message(SEND_ERROR "Failed to find OIS")
endif(NOT OIS_FOUND)


set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/Install")

# set some additional libraries
set(ADDITIONAL_OGRE ${OGRE_Plugin_ParticleFX_LIBRARIES} ${OGRE_Plugin_CgProgramManager_LIBRARIES}
    ${OGRE_Plugin_OctreeZone_LIBRARIES} ${OGRE_Overlay_LIBRARIES} ${Ogre_Paging_LIBRARIES})

# Find Boost
if(WIN32 OR APPLE)
    set(Boost_USE_STATIC_LIBS TRUE)
else(WIN32 OR APPLE)
    # Statically linking boost to a dynamic Ogre build doesn't work on Linux 64bit
    set(Boost_USE_STATIC_LIBS ${OGRE_STATIC})
endif(WIN32 OR APPLE)
if(MINGW)
    # this is probably a bug in CMake: the boost find module tries to look for
    # boost libraries with name libboost_*, but CMake already prefixes library
    # search names with "lib". This is the workaround.
    set(CMAKE_FIND_LIBRARY_PREFIXES ${CMAKE_FIND_LIBRARY_PREFIXES} "")
endif()
set(Boost_ADDITIONAL_VERSIONS "1.55" "1.44" "1.44.0" "1.42" "1.42.0" "1.41.0" "1.41" "1.40.0" "1.40" "1.39.0" "1.39" "1.38.0" "1.38" "1.37.0" "1.37" )
# Components that need linking (NB does not include header-only components like bind)
set(OGRE_BOOST_COMPONENTS thread date_time)

set(LEVIATHAN_BOOST_COMPONENTS chrono system)

set(REQUIRED_BOOST_COMPONENTS ${LEVIATHAN_BOOST_COMPONENTS} ${OGRE_BOOST_COMPONENTS})

find_package(Boost COMPONENTS ${REQUIRED_BOOST_COMPONENTS} QUIET)
if(NOT Boost_FOUND)
    message("Boost not found, retrying with different settings")
    # Try again with the other type of libs
    set(Boost_USE_STATIC_LIBS NOT ${Boost_USE_STATIC_LIBS})
    find_package(Boost COMPONENTS ${OGRE_BOOST_COMPONENTS} QUIET)
    
    find_package(Boost QUIET)
    
    if(NOT Boost_FOUND)
        message(SEND_ERROR "Failed to find Boost libraries: " ${REQUIRED_BOOST_COMPONENTS})        
    endif(NOT Boost_FOUND)
endif(NOT Boost_FOUND)


# Set up referencing of Boost
include_directories(${Boost_INCLUDE_DIR})
add_definitions(-DBOOST_ALL_NO_LIB)

  
if(WIN32)
# we want to build with unicode setting, also add minimal rebuild, and exceptions from floating point operations, 
# higher memory for compiling precompiled headers
add_definitions(-DUNICODE -D_UNICODE -fp:except -Zm182)

# program database flag for debug
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI -Gm /W3")

else(WIN32)
    add_definitions(-std=c++11 -std=gnu++11 -fextended-identifiers)

    # Linux library linking
    set(LINUX_LINK_LIBS_TOENGINE Newton)

    set(LINUX_LINK_LIBS_ENGINE Angelscript Leap RocketCore RocketControls RocketDebugger Newton 
        sfml-audio sfml-network sfml-system)

    # We need X11 on linux for window class to work
    find_package(X11)

    include_directories(${X11_INCLUDE_DIRS})
endif(WIN32)


option(INSTALL_ONLY_RELEASE "For creating release folders" ON)
if(WIN32)

option(USE_VLD "For including vld in the projects" ON)
endif(WIN32)

if(USE_VLD)
set(L_VLD_STATE 1)
else(USE_VLD)
set(L_VLD_STATE 0)
endif(USE_VLD)

# version number
set(LEVIATHAN_VERSION_STABLE 0)
set(LEVIATHAN_VERSION_MAJOR 6)
set(LEVIATHAN_VERSION_MINOR 3)
set(LEVIATHAN_VERSION_PATCH 0)

set(LEVIATHAN_VERSION_STR ${LEVIATHAN_VERSION_STABLE}.${LEVIATHAN_VERSION_MAJOR}.${LEVIATHAN_VERSION_MINOR}.${LEVIATHAN_VERSION_PATCH})
set(LEVIATHAN_VERSION ${LEVIATHAN_VERSION_STABLE}.${LEVIATHAN_VERSION_MAJOR}${LEVIATHAN_VERSION_MINOR}${LEVIATHAN_VERSION_PATCH})

set(LEVIATHAN_VERSIONS "L\"${LEVIATHAN_VERSION_STR}\"")
set(LEVIATHAN_VERSION_ANSIS "\"${LEVIATHAN_VERSION_STR}\"")

set(CMAKE_DEBUG_POSTFIX "D")



# create bin directory
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

message(STATUS "Copying files...")

# copy data from bin directory
if(WIN32)
    file(GLOB VLDMoveFiles "bin/vld.ini" "bin/VLDExportSettings.txt" "bin/VLD data exporter.exe" 
        "bin/EngineConf.conf")
else(WIN32)
    file(GLOB VLDMoveFiles "bin/EngineConf.conf")
endif(WIN32)
file(COPY ${VLDMoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin")

# copy data directory
file(GLOB DataMoveFiles "bin/Data/*")
file(COPY ${DataMoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin/Data")

# Copy data from the scripts folder to the bin folder
file(GLOB ScriptsMoveFiles "Scripts/*")
file(COPY ${ScriptsMoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin/Data/Scripts")



if(WIN32)
    # Boost files
    if(NOT Boost_USE_STATIC_LIBS)
        set(BOOST_MOVE_FILES_RELEASE ${Boost_DATE_TIME_LIBRARY_RELEASE} ${Boost_CHRONO_LIBRARY_RELEASE}
          ${Boost_THREAD_LIBRARY_RELEASE} ${Boost_SYSTEM_LIBRARY_RELEASE})
        set(BOOST_MOVE_FILES_DEBUG ${Boost_DATE_TIME_LIBRARY_DEBUG} ${Boost_CHRONO_LIBRARY_DEBUG}
          ${Boost_THREAD_LIBRARY_DEBUG} ${Boost_SYSTEM_LIBRARY_DEBUG})
          
        file(COPY ${BOOST_MOVE_FILES_RELEASE} DESTINATION "${PROJECT_BINARY_DIR}/bin")
        file(COPY ${BOOST_MOVE_FILES_DEBUG} DESTINATION "${PROJECT_BINARY_DIR}/bin")
    
    endif(NOT Boost_USE_STATIC_LIBS)
    
    message(STATUS "Copying libs")
    
    # leap DLLs
    file(GLOB LeapMoveFiles "Leap/lib/x86/Leap.dll" "Leap/lib/x86/Leapd.dll")
    file(COPY ${LeapMoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin")

    # libRocket dlls
    file(GLOB RocketMoveFiles "libRocket/bin/*")
    file(COPY ${RocketMoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin")
    
    # smfl
    file(GLOB SMFLMoveFilesRelease "SFML/bin/openal32.dll" "SFML/bin/libsndfile-1.dll" "SFML/bin/sfml-audio-2.dll" "SFML/bin/sfml-system-2.dll"
        "SFML/bin/sfml-network-2.dll")
    file(COPY ${SMFLMoveFilesRelease} DESTINATION "${PROJECT_BINARY_DIR}/bin")
    file(GLOB SMFLMoveFilesDebug "SFML/bin/sfml-audio-d-2.dll" "SFML/bin/sfml-system-d-2.dll"
        "SFML/bin/sfml-network-d-2.dll")
    file(COPY ${SMFLMoveFilesDebug} DESTINATION "${PROJECT_BINARY_DIR}/bin")
    
    # newton
    file(GLOB NewtonMoveFilesRelease "Newton/bin/Release/*")
    file(COPY ${NewtonMoveFilesRelease} DESTINATION "${PROJECT_BINARY_DIR}/bin")
    file(GLOB NewtonMoveFilesDebug "Newton/bin/Debug/*")
    file(COPY ${NewtonMoveFilesDebug} DESTINATION "${PROJECT_BINARY_DIR}/bin")
    
    # ogre dlls (this should be changed for public release to only copy required dlls)
    file(GLOB OgreMoveFiles "${OGRE_BASEDIR}bin/Debug/*.pdb" "OGRE/bin/Debug/*.dll" "OGRE/bin/Release/*.dll")
    file(COPY ${OgreMoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin")

else(WIN32)
    # linux variants of the copy functions
    file(GLOB LeapMoveFiles "Leap/lib/x64/libLeap.so")
    file(COPY ${LeapMoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin")

    # ogre sos (this should be changed for public release to only copy required sos)
    file(GLOB OgreMoveFiles "${OGRE_BASEDIR}/*.so*" "${OGRE_BASEDIR}/../*Ogre*.so*" "${OGRE_BASEDIR}/../libOIS*.so")
    file(COPY ${OgreMoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin")

    # libRocket sos
    file(GLOB RocketMoveFiles "libRocket/lib/*.so*")
    file(COPY ${RocketMoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin")

    # newton
    file(COPY "Newton/lib/libNewton.so" DESTINATION "${PROJECT_BINARY_DIR}/bin")

    # smfl
    file(GLOB SMFLMoveFiles "SFML/lib/*.so*")
    file(COPY ${SMFLMoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin")

endif(WIN32)
message(STATUS "Done copying")

# working directory setting
if(WIN32)
    include(CreateLaunchers)
    include(SetPrecompiledHeaders)
else(WIN32)

    set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
    set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

    set(CMAKE_RPATH "$ORIGIN/")
    set(CMAKE_INSTALL_RPATH "$ORIGIN/")
    set(CMAKE_BUILD_RPATH "$ORIGIN/")

    set(CMAKE_BUILD_WITH_INSTALL_RPATH true)

endif(WIN32)

configure_file(
    "${PROJECT_SOURCE_DIR}/Engine/Include.h.in"
    "${PROJECT_SOURCE_DIR}/Engine/Include.h")


# additional include directories
include_directories("${PROJECT_SOURCE_DIR}/AngelScript/include")
include_directories("${PROJECT_SOURCE_DIR}/AngelScript")
include_directories("${PROJECT_SOURCE_DIR}/Engine/")
include_directories("${PROJECT_SOURCE_DIR}/Leap/include")
include_directories("${PROJECT_SOURCE_DIR}/libRocket/include")
include_directories("${PROJECT_SOURCE_DIR}/Newton/include")
include_directories("${PROJECT_SOURCE_DIR}/SFML/include")
include_directories(${OIS_INCLUDE_DIRS}	${OGRE_INCLUDE_DIRS})

# libraries paths
link_directories("${PROJECT_SOURCE_DIR}/AngelScript/lib")
link_directories("${PROJECT_SOURCE_DIR}/libRocket/lib/")
link_directories("${PROJECT_SOURCE_DIR}/SFML/lib/")

if(WIN32)
    link_directories("${PROJECT_SOURCE_DIR}/Leap/lib/x86")
    link_directories("${PROJECT_SOURCE_DIR}/Newton/lib/Release")
    link_directories("${PROJECT_SOURCE_DIR}/Newton/lib/Debug")
else(WIN32)
    link_directories("${PROJECT_SOURCE_DIR}/Newton/lib")
    link_directories("${PROJECT_SOURCE_DIR}/Leap/lib/x64")
endif(WIN32)

# engine linking
link_directories("${PROJECT_BINARY_DIR}")
link_directories("${PROJECT_BINARY_DIR}/Engine/Debug")
link_directories("${PROJECT_BINARY_DIR}/Engine/Release")

# engine project 
add_subdirectory(Engine)

# test project
add_subdirectory(LeviathanTest)


# sandboxie example project
add_subdirectory(Project_sandboxie)

# pong example game
add_subdirectory(Pong)

# example pong server
add_subdirectory(PongServer)

# example pong master server
add_subdirectory(PongMasterServer)

# install support

# data folders
file(GLOB DataMoveFiles "bin/Data/*.*")
install(FILES ${DataMoveFiles} DESTINATION bin/Data)
# The script files folder
install(DIRECTORY "Scripts" DESTINATION bin/Data)
install(FILES "bin/EngineConf.conf" DESTINATION bin)
# we need to specifically install the directories
install(DIRECTORY "bin/Data/Fonts" DESTINATION bin/Data)
install(DIRECTORY "bin/Data/Models" DESTINATION bin/Data)
install(DIRECTORY "bin/Data/Scripts" DESTINATION bin/Data)
install(DIRECTORY "bin/Data/Sound" DESTINATION bin/Data)
install(DIRECTORY "bin/Data/Textures" DESTINATION bin/Data)
install(DIRECTORY "bin/Data/Screenshots" DESTINATION bin/Data)
install(DIRECTORY "bin/Data/Cache" DESTINATION bin/Data)
if(WIN32)
    # ogre tools
    file(GLOB OgreTools "${OGRE_BASEDIR}/bin/Release/*.exe")
    file(GLOB SampleBrowserFile "${OGRE_BASEDIR}/bin/Release/SampleBrowser.exe")
    #list(REMOVE_ITEM OgreTools ${SampleBrowserFile})
    install(FILES ${OgreTools} DESTINATION tools)
endif(WIN32)

if(INSTALL_ONLY_RELEASE)
    # only release versions of everything
    if(WIN32)
        # change some file lists around
        file(GLOB LeapMoveFiles "Leap/lib/x86/Leap.dll")
        # only release dlls
        file(GLOB OgreMoveFiles "${OGRE_BASEDIR}/bin/Release/*.dll")
        # only release Rocket
        file(GLOB RocketMoveFiles "libRocket/bin/RocketControls.dll" "libRocket/bin/RocketCore.dll" "libRocket/bin/RocketDebugger.dll")
    endif(WIN32)

else(INSTALL_ONLY_RELEASE)
    # install all files that debug distribution might need
    if(WIN32)
        install(FILES ${VLDMoveFiles} DESTINATION bin)

        install(FILES ${NewtonMoveFilesDebug} DESTINATION bin)
        install(FILES ${SMFLMoveFilesDebug} DESTINATION bin)
        
        if(NOT Boost_USE_STATIC_LIBS)
            install(FILES ${BOOST_MOVE_FILES_DEBUG} DESTINATION bin)
        endif(NOT Boost_USE_STATIC_LIBS)
    endif(WIN32)

endif(INSTALL_ONLY_RELEASE)

install(FILES ${LeapMoveFiles} DESTINATION bin)
install(FILES ${OgreMoveFiles} DESTINATION bin)
install(FILES ${RocketMoveFiles} DESTINATION bin)
if(WIN32)
    install(FILES ${NewtonMoveFilesRelease} DESTINATION bin)
    install(FILES ${SMFLMoveFilesRelease} DESTINATION bin)
    if(NOT Boost_USE_STATIC_LIBS)
        install(FILES ${BOOST_MOVE_FILES_RELEASE} DESTINATION bin)
    endif(NOT Boost_USE_STATIC_LIBS)
else(WIN32)
    install(FILES ${NewtonMoveFiles} DESTINATION bin)
    install(FILES ${SMFLMoveFiles} DESTINATION bin)
endif(WIN32)


# testing support


# CPack support
include(InstallRequiredSystemLibraries)

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Leviathan Game Engine")
SET(CPACK_PACKAGE_VENDOR "Me, myself, and I")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/README.MD")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/License.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${LEVIATHAN_VERSION_STABLE}.${LEVIATHAN_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${LEVIATHAN_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${LEVIATHAN_VERSION_PATCH}")


if(WIN32)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  set(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}\\\\LeviathanIcon.bmp")
  set(CPACK_NSIS_DISPLAY_NAME "Leviathan Game Engine")
  set(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.boostslair.com/projects/Leviathan/Help.php")
  set(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.boostslair.com/projects/Leviathan/")
  set(CPACK_NSIS_CONTACT "boost@boostslair.com")
  set(CPACK_NSIS_MODIFY_PATH OFF)
else(WIN32)
  set(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/LeviathanIcon.bmp")
  set(CPACK_NSIS_DISPLAY_NAME "Leviathan Game Engine")
  set(CPACK_NSIS_HELP_LINK "http://www.boostslair.com/projects/Leviathan/Help.php")
  set(CPACK_NSIS_URL_INFO_ABOUT "http://www.boostslair.com/projects/Leviathan/")
  set(CPACK_NSIS_CONTACT "boost@boostslair.com")
  set(CPACK_NSIS_MODIFY_PATH OFF)
endif(WIN32)

include(CPack)

