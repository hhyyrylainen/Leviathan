# CMake file for generating all project files
cmake_minimum_required(VERSION 2.8.3)

project(Leviathan)

# version number
set(LEVIATHAN_VERSION_STABLE 0)
set(LEVIATHAN_VERSION_MAJOR 6)
set(LEVIATHAN_VERSION_MINOR 9)
set(LEVIATHAN_VERSION_PATCH 7)

set(LEVIATHAN_VERSION_STR ${LEVIATHAN_VERSION_STABLE}.${LEVIATHAN_VERSION_MAJOR}.${LEVIATHAN_VERSION_MINOR}.${LEVIATHAN_VERSION_PATCH})
set(LEVIATHAN_VERSION ${LEVIATHAN_VERSION_STABLE}.${LEVIATHAN_VERSION_MAJOR}${LEVIATHAN_VERSION_MINOR}${LEVIATHAN_VERSION_PATCH})

set(LEVIATHAN_VERSIONS "L\"${LEVIATHAN_VERSION_STR}\"")
set(LEVIATHAN_VERSION_ANSIS "\"${LEVIATHAN_VERSION_STR}\"")

set(WORK_DIR "${PROJECT_BINARY_DIR}/bin")
file(TO_NATIVE_PATH ${PROJECT_BINARY_DIR} PROJECT_NATIVE_BINARY_DIR)

# modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/CMake" "${PROJECT_SOURCE_DIR}/LeviathanCMake")

# The CEGUI build might have some weird suffix
set(CEGUI_SUFFIX "-9999" CACHE STRING "The suffix of cegui builds, for example -0.8 or -9999")

#set(CEGUI_SUFFIX "")

# When no configuration is set, enable RelWithDebInfo
if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Set the build type, usually Release or RelWithDebInfo" FORCE)
endif(CMAKE_BUILD_TYPE STREQUAL "")

if(NOT WIN32 AND ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	# Not recommended configuration
	message(STATUS "Configuration is set to: " ${CMAKE_BUILD_TYPE})
	message(SEND_ERROR "On linux you should only try to build with Release or RelWithDebInfo")

endif(NOT WIN32 AND ${CMAKE_BUILD_TYPE} STREQUAL "Debug")


# locating the Ogre folder
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

# Ogre folder fine

# Find required libraries

# Ogre is required
find_package(OGRE REQUIRED Plugin_ParticleFX Plugin_CgProgramManager Plugin_OctreeZone Overlay Paging)

# OIS is required for input
find_package(OIS REQUIRED)

# check if it failed
if(NOT OIS_FOUND)
	message(SEND_ERROR "Failed to find OIS")
endif(NOT OIS_FOUND)


# set some additional libraries
set(ADDITIONAL_OGRE ${OGRE_Plugin_ParticleFX_LIBRARIES} ${OGRE_Plugin_CgProgramManager_LIBRARIES}
    ${OGRE_Plugin_OctreeZone_LIBRARIES} ${OGRE_Overlay_LIBRARIES} ${Ogre_Paging_LIBRARIES})

# Find Boost
# Might be a good idea to dynamically link Boost
set(Boost_USE_STATIC_LIBS FALSE)

set(Boost_ADDITIONAL_VERSIONS "1.55" "1.53")
# Components that need linking (NB does not include header-only components like bind)
set(OGRE_BOOST_COMPONENTS thread date_time)

# Other than these that are required are header-only libraries
set(LEVIATHAN_BOOST_COMPONENTS chrono system regex)

set(REQUIRED_BOOST_COMPONENTS ${LEVIATHAN_BOOST_COMPONENTS} ${OGRE_BOOST_COMPONENTS})

find_package(Boost COMPONENTS ${REQUIRED_BOOST_COMPONENTS})
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

# Linking and platform specific
if(WIN32)
    # we want to build with unicode setting, also add minimal rebuild, and exceptions from floating point operations, 
    # higher memory for compiling precompiled headers
    add_definitions(-DUNICODE -D_UNICODE -fp:except -Zm216)
    
    # program database flag for debug
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI -Gm /W3")
    
else(WIN32)
    add_definitions(-fextended-identifiers)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	
    # We need X11 on linux for window class to work
    find_package(X11)
    find_package(Threads)

    include_directories(${X11_INCLUDE_DIRS})
endif(WIN32)

# CEGUI libraries will have some suffixes
set(CEGUI_ENGINE_RELEASE "CEGUIBase" "CEGUICommonDialogs" "CEGUIOgreRenderer")
set(TmpProcessQueueList)
set(CEGUI_ENGINE_DEBUG)

foreach(ceglib ${CEGUI_ENGINE_RELEASE})
    set(TmpProcessQueueList ${TmpProcessQueueList} ${ceglib}${CEGUI_SUFFIX})
    set(CEGUI_ENGINE_DEBUG ${CEGUI_ENGINE_DEBUG} ${ceglib}${CEGUI_SUFFIX}_d)
endforeach()

set(CEGUI_ENGINE_RELEASE ${TmpProcessQueueList})
# Add the differently named ones
set(CEGUI_ENGINE_RELEASE ${CEGUI_ENGINE_RELEASE} "CEGUICoreWindowRendererSet" "CEGUIExpatParser")
set(CEGUI_ENGINE_DEBUG ${CEGUI_ENGINE_DEBUG} "CEGUICoreWindowRendererSet_d" "CEGUIExpatParser_d")



# Library linking
set(LINK_LIBS_TOENGINE_RELEASE optimized Newton)
set(LINK_LIBS_ENGINE_RELEASE Angelscript Leap Newton sfml-audio sfml-network sfml-system ${CEGUI_ENGINE_RELEASE})

if(WIN32)
# Debug libraries
set(LINK_LIBS_TOENGINE_DEBUG debug Newton_d)
set(LINK_LIBS_ENGINE_DEBUG Angelscriptd Leapd Newton_d sfml-audio-d sfml-network-d sfml-system-d ${CEGUI_ENGINE_DEBUG})
endif(WIN32)

# Prepend the right keywords


# define a temp value
set(TmpProcessQueueList)

foreach(curlibname ${LINK_LIBS_ENGINE_RELEASE})
    set(TmpProcessQueueList ${TmpProcessQueueList} optimized ${curlibname})
endforeach()

set(LINK_LIBS_ENGINE_RELEASE ${TmpProcessQueueList})

if(WIN32)
	set(TmpProcessQueueList)

	foreach(curlibname ${LINK_LIBS_ENGINE_DEBUG})
    	set(TmpProcessQueueList ${TmpProcessQueueList} debug ${curlibname})
	endforeach()

	set(LINK_LIBS_ENGINE_DEBUG ${TmpProcessQueueList})
endif(WIN32)


######### Options
    
option(INSTALL_ONLY_RELEASE "For creating release folders" ON)
# \todo this option to copy right libs
option(INSTALL_CREATE_SDK   "For creating SDKs for developers" OFF)

# VisualLeakDetector setting
if(WIN32)
    option(USE_VLD "For including vld in the projects" ON)
endif()

# For making libs with right names
set(CMAKE_DEBUG_POSTFIX "D")

if(USE_VLD)
    set(L_VLD_STATE 1)
else(USE_VLD)
    set(L_VLD_STATE 0)
endif(USE_VLD)

# create the bin directory
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

# Configure engine include file
configure_file("${PROJECT_SOURCE_DIR}/Engine/Include.h.in" "${PROJECT_SOURCE_DIR}/Engine/Include.h")

# configure the doxygen file
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file("${PROJECT_SOURCE_DIR}/LeviathanDoxy.in" "${PROJECT_BINARY_DIR}/LeviathanDoxy" @ONLY)
    add_custom_target(doc
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/LeviathanDoxy
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif(DOXYGEN_FOUND)

set(CEGUI_INSTALL_DIR "" CACHE FILEPATH "Path to the root directory of CEGUI install folder")
set(CEGUI_BUILD_BINARY_DIR "" CACHE FILEPATH "Path to the bin directory that has .pdb debug files")
option(CEGUI_COPY_DEBUGFILES "Whether to copy the .pdb files and others that are required for proper debugging" ON) 
configure_file("${PROJECT_SOURCE_DIR}/CopyCEGUI.bat.in" "${PROJECT_SOURCE_DIR}/CopyCEGUI.bat" @ONLY)

message(STATUS "Copying files...")

# copy data from bin directory

# The bin-folder files
if(WIN32)
    file(GLOB RootFolderFiles "bin/*.ini" "bin/*.txt" "bin/VLD data exporter.exe" 
        "bin/*.conf")
else(WIN32)
    file(GLOB RootFolderFiles "bin/*.conf" "bin/*.txt")
endif(WIN32)

# Move files
file(COPY ${RootFolderFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin")

# copy data directory
file(GLOB DataMoveFiles "bin/Data/*")
file(COPY ${DataMoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin/Data")

# Copy data from the scripts folder to the bin folder
file(GLOB ScriptsMoveFiles "Scripts/*")
file(COPY ${ScriptsMoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin/Data/Scripts")

# Copy the crucial Ogre scripts
file(GLOB CoreOgreScriptsMoveFiles "CoreOgreScripts/*")
file(COPY ${CoreOgreScriptsMoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin/CoreOgreScripts")

# Boost files
# If we are not using static linking we need to copy everything
if(NOT Boost_USE_STATIC_LIBS)
    
    message(STATUS "Copying Boost libs")

	set(BOOST_MOVE_FILES_RELEASE ${Boost_DATE_TIME_LIBRARY_RELEASE} ${Boost_CHRONO_LIBRARY_RELEASE}
	  ${Boost_THREAD_LIBRARY_RELEASE} ${Boost_SYSTEM_LIBRARY_RELEASE} ${Boost_REGEX_LIBRARY_RELEASE})
	set(BOOST_MOVE_FILES_DEBUG ${Boost_DATE_TIME_LIBRARY_DEBUG} ${Boost_CHRONO_LIBRARY_DEBUG}
	  ${Boost_THREAD_LIBRARY_DEBUG} ${Boost_SYSTEM_LIBRARY_DEBUG} ${Boost_REGEX_LIBRARY_DEBUG})
      
    # Setup to target the actually wanted files
	if(WIN32)
	    set(TmpDoStuff)
    
    	foreach(libname ${BOOST_MOVE_FILES_RELEASE})
        	# Replace the library extension with the platform extension
        	get_filename_component(LIBS_BASE_NAME ${libname} NAME_WE)
        	get_filename_component(LIBS_PATH ${libname} PATH)
           	set(TmpDoStuff ${TmpDoStuff} ${LIBS_PATH}/${LIBS_BASE_NAME}.dll)
    	endforeach()
    
    	set(BOOST_MOVE_FILES_RELEASE ${TmpDoStuff})

		set(TmpDoStuff)
		foreach(libname ${BOOST_MOVE_FILES_DEBUG})
			# Replace the library extension with the platform extension
			get_filename_component(LIBS_BASE_NAME ${libname} NAME_WE)
			get_filename_component(LIBS_PATH ${libname} PATH)
			set(TmpDoStuff ${TmpDoStuff} ${LIBS_PATH}/${LIBS_BASE_NAME}.dll)
		endforeach()

	    set(BOOST_MOVE_FILES_DEBUG ${TmpDoStuff})
    endif(WIN32)
      
    if(WIN32)  
		file(COPY ${BOOST_MOVE_FILES_RELEASE} DESTINATION "${PROJECT_BINARY_DIR}/bin")
    	file(COPY ${BOOST_MOVE_FILES_DEBUG} DESTINATION "${PROJECT_BINARY_DIR}/bin")
	endif(WIN32)

endif(NOT Boost_USE_STATIC_LIBS)

message(STATUS "Copying libs")

if(WIN32)

    # leap DLLs
    file(GLOB LeapMoveFilesRelease "Leap/lib/x86/Leap.dll")
    file(GLOB LeapMoveFilesDebug "Leap/lib/x86/Leapd.dll")

    # CEGUI dlls
    file(GLOB CEGUIMoveFilesDebug "CEGUI/bin/*_d.dll")
    file(GLOB CEGUIMoveFilesRelease "CEGUI/bin/*.dll")
    
    # Remove the debug ones from the release ones
    list(REMOVE_ITEM CEGUIMoveFilesRelease ${CEGUIMoveFilesDebug})

    # smfl
    file(GLOB SMFLMoveFilesRelease "SFML/bin/openal32.dll" "SFML/bin/libsndfile-1.dll" "SFML/bin/sfml-audio-2.dll" "SFML/bin/sfml-system-2.dll"
        "SFML/bin/sfml-network-2.dll")

    file(GLOB SMFLMoveFilesDebug "SFML/bin/sfml-audio-d-2.dll" "SFML/bin/sfml-system-d-2.dll"
        "SFML/bin/sfml-network-d-2.dll")
    
    # newton
    file(GLOB NewtonMoveFilesRelease "Newton/bin/Release/*")

    file(GLOB NewtonMoveFilesDebug "Newton/bin/Debug/*")

    # ogre dlls (this should be changed for public release to only copy required dlls)
    file(GLOB OgreMoveFilesDebug "${OGRE_BASEDIR}/bin/Debug/*.pdb" "${OGRE_BASEDIR}/bin/Debug/*.dll")
    file(GLOB OgreMoveFilesRelease "${OGRE_BASEDIR}/bin/Release/*.dll")

else(WIN32)
    # linux variants of the copy functions
    file(GLOB LeapMoveFilesRelease "Leap/lib/x64/libLeap.so")
    
    # ogre sos (this should be changed for public release to only copy required sos)
    file(GLOB OgreMoveFilesRelease "${OGRE_BASEDIR}/*.so*" "${OGRE_BASEDIR}/../*Ogre*.so*" "${OGRE_BASEDIR}/../libOIS*.so")

    # CEGUI sos
    file(GLOB CEGUIMoveFilesRelease "CEGUI/lib/*.so*")

    # newton
    file(GLOB NewtonMoveFilesRelease "Newton/lib/libNewton.so")

    # smfl
    file(GLOB SMFLMoveFilesRelease "SFML/lib/*.so*")

endif(WIN32)

message(STATUS "Moving the bulk...")

if(WIN32)
	
	file(COPY ${CEGUIMoveFilesDebug} DESTINATION "${PROJECT_BINARY_DIR}/bin")
	file(COPY ${LeapMoveFilesDebug} DESTINATION "${PROJECT_BINARY_DIR}/bin")
	file(COPY ${SMFLMoveFilesDebug} DESTINATION "${PROJECT_BINARY_DIR}/bin")
	file(COPY ${NewtonMoveFilesDebug} DESTINATION "${PROJECT_BINARY_DIR}/bin")
	file(COPY ${OgreMoveFilesDebug} DESTINATION "${PROJECT_BINARY_DIR}/bin")

endif(WIN32)


file(COPY ${CEGUIMoveFilesRelease} DESTINATION "${PROJECT_BINARY_DIR}/bin")
file(COPY ${LeapMoveFilesRelease} DESTINATION "${PROJECT_BINARY_DIR}/bin")
file(COPY ${SMFLMoveFilesRelease} DESTINATION "${PROJECT_BINARY_DIR}/bin")
file(COPY ${NewtonMoveFilesRelease} DESTINATION "${PROJECT_BINARY_DIR}/bin")
file(COPY ${OgreMoveFilesRelease} DESTINATION "${PROJECT_BINARY_DIR}/bin")

message(STATUS "Done copying")

# working directory setting
if(WIN32)
    include(CreateLaunchers)
    include(SetPrecompiledHeaders)
else(WIN32)

    set(CMAKE_RPATH "$ORIGIN/")
    set(CMAKE_INSTALL_RPATH "$ORIGIN/")
    set(CMAKE_BUILD_RPATH "$ORIGIN/")

    set(CMAKE_BUILD_WITH_INSTALL_RPATH true)

endif(WIN32)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)


# additional include directories
include_directories("${PROJECT_SOURCE_DIR}/AngelScript/include")
include_directories("${PROJECT_SOURCE_DIR}/AngelScript")
include_directories("${PROJECT_SOURCE_DIR}/Engine/")
include_directories("${PROJECT_SOURCE_DIR}/Leap/include")
include_directories("${PROJECT_SOURCE_DIR}/Newton/include")
include_directories("${PROJECT_SOURCE_DIR}/SFML/include")
include_directories(${OIS_INCLUDE_DIRS}	${OGRE_INCLUDE_DIRS})
include_directories("${PROJECT_SOURCE_DIR}/CEGUI/include")

# libraries paths
link_directories("${PROJECT_SOURCE_DIR}/AngelScript/lib")
link_directories("${PROJECT_SOURCE_DIR}/libRocket/lib/")
link_directories("${PROJECT_SOURCE_DIR}/SFML/lib/")
link_directories("${PROJECT_SOURCE_DIR}/CEGUI/lib/")
link_directories("${PROJECT_SOURCE_DIR}/Newton/lib/")
link_directories("${PROJECT_SOURCE_DIR}/Newton/bin/")

if(WIN32)
    link_directories("${PROJECT_SOURCE_DIR}/Leap/lib/x86")
else(WIN32)
    link_directories("${PROJECT_SOURCE_DIR}/Leap/lib/x64")
endif(WIN32)

# engine linking
link_directories("${PROJECT_BINARY_DIR}")
# these two might not be necessary anymore
link_directories("${PROJECT_BINARY_DIR}/Engine/Debug")
link_directories("${PROJECT_BINARY_DIR}/Engine/Release")
link_directories("${PROJECT_BINARY_DIR}/bin")

# First project must be the file generator
add_subdirectory(FileGenerator)

# engine project 
add_subdirectory(Engine)

# test project
add_subdirectory(LeviathanTest)

# sandboxie example project
add_subdirectory(Sandboxie)

# example pong master server
add_subdirectory(PongMasterServer)

# example pong server
add_subdirectory(PongServer)

# pong example game
add_subdirectory(Pong)

# install support

# data folders
file(GLOB DataMoveFiles "bin/Data/*.*")
install(FILES ${DataMoveFiles} DESTINATION bin/Data)


# The script files folder
install(DIRECTORY "Scripts" DESTINATION bin/Data)
install(DIRECTORY "CoreOgreScripts" DESTINATION bin)
install(FILES ${RootFolderFiles} DESTINATION bin)
# we need to specifically install the directories
install(DIRECTORY "bin/Data/Fonts" DESTINATION bin/Data)
install(DIRECTORY "bin/Data/Models" DESTINATION bin/Data)
install(DIRECTORY "bin/Data/Sound" DESTINATION bin/Data)
install(DIRECTORY "bin/Data/Textures" DESTINATION bin/Data)
install(DIRECTORY "bin/Data/Screenshots" DESTINATION bin/Data)
install(DIRECTORY "bin/Data/Cache" DESTINATION bin/Data)

if(WIN32)
    # ogre tools
    file(GLOB OgreTools "${OGRE_BASEDIR}/bin/Release/*.exe")
    file(GLOB SampleBrowserFile "${OGRE_BASEDIR}/bin/Release/SampleBrowser.exe")
    install(FILES ${OgreTools} DESTINATION tools)
    
endif(WIN32)

# Potentially install debug versions
if((NOT INSTALL_ONLY_RELEASE OR INSTALL_CREATE_SDK) AND WIN32)
    # install all files that debug distribution might need
    install(FILES ${VLDMoveFiles} DESTINATION bin)
    install(FILES ${LeapMoveFilesDebug} DESTINATION bin)
    install(FILES ${NewtonMoveFilesDebug} DESTINATION bin)
    install(FILES ${SMFLMoveFilesDebug} DESTINATION bin)
    install(FILES ${OgreMoveFilesDebug} DESTINATION bin)
	install(FILES ${CEGUIMoveFilesDebug} DESTINATION bin)

    
    if(NOT Boost_USE_STATIC_LIBS)
        install(FILES ${BOOST_MOVE_FILES_DEBUG} DESTINATION bin)
    endif(NOT Boost_USE_STATIC_LIBS)

endif((NOT INSTALL_ONLY_RELEASE OR INSTALL_CREATE_SDK) AND WIN32)

# Always install release libraries
install(FILES ${LeapMoveFilesRelease} DESTINATION bin)
install(FILES ${OgreMoveFilesRelease} DESTINATION bin)
install(FILES ${CEGUIMoveFilesRelease} DESTINATION bin)
install(FILES ${NewtonMoveFilesRelease} DESTINATION bin)
install(FILES ${SMFLMoveFilesRelease} DESTINATION bin)

if(NOT Boost_USE_STATIC_LIBS AND WIN32)
    install(FILES ${BOOST_MOVE_FILES_RELEASE} DESTINATION bin)
endif(NOT Boost_USE_STATIC_LIBS AND WIN32)


# testing support


# CPack support
include(InstallRequiredSystemLibraries)

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Leviathan Game Engine")
SET(CPACK_PACKAGE_VENDOR "Henri Hyyryläinen")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/README.MD")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/License.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${LEVIATHAN_VERSION_STABLE}.${LEVIATHAN_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${LEVIATHAN_VERSION_MINOR}")

if(INSTALL_CREATE_SDK)
    set(CPACK_PACKAGE_VERSION_PATCH "${LEVIATHAN_VERSION_PATCH}-sdk")
else(INSTALL_CREATE_SDK)
    set(CPACK_PACKAGE_VERSION_PATCH "${LEVIATHAN_VERSION_PATCH}")
endif(INSTALL_CREATE_SDK)


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

