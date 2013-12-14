# CMake file for generating all project files
cmake_minimum_required(VERSION 2.8)

project(Leviathan)
set(WORK_DIR "${PROJECT_BINARY_DIR}/bin")

#######

if(WIN32)
# we want to build with unicode setting, also add minimal rebuild, and exceptions from floating point operations, higher memory for compiling
add_definitions(-DUNICODE -D_UNICODE -fp:except -Zm162)

# program database flag for debug
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI -Gm /W3")

set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/Install")
else(WIN32)
add_definitions(-std=c++11 -std=gnu++11 -fextended-identifiers)
# We need to get the Boost include directory from the user
set(BOOST_INCLUDE_DIR "NOTFOUND" CACHE FILEPATH "Path to Boost include files")

include_directories(${BOOST_INCLUDE_DIR})

# Linux library linking
#set(LINUX_LINK_LIBS_TOENGINE ${PROJECT_SOURCE_DIR}/Newton/lib/libNewton.so)
set(LINUX_LINK_LIBS_TOENGINE Newton)


#set(LINUX_LINK_LIBS_ENGINE_DEBUG ${PROJECT_SOURCE_DIR}libNewton.so ${PROJECT_SOURCE_DIR}/AngelScript/lib/libAngelscript.a
#	${PROJECT_SOURCE_DIR}/Leap/lib/x64/libLeap.so ${PROJECT_SOURCE_DIR}/OGRE/lib/libOgreMain_d.so 
#	${PROJECT_SOURCE_DIR}/OGRE/lib/libOgreOverlay_d.so ${PROJECT_SOURCE_DIR}/OGRE/lib/libOgrePaging_d.so
#	${PROJECT_SOURCE_DIR}/OGRE/lib/libOgreTerrain_d.so ${PROJECT_SOURCE_DIR}/OGRE/lib/libOgreRTShaderSystem_d.so
#	${PROJECT_SOURCE_DIR}/OGRE/lib/libPlugin_CgProgramManager_d.so ${PROJECT_SOURCE_DIR}/OGRE/lib/libPlugin_OctreeZone_d.so
#	${PROJECT_SOURCE_DIR}/OGRE/lib/libPlugin_ParticleFX_d.so ${PROJECT_SOURCE_DIR}/OGRE/lib/libRenderSystem_Direct3D11_d.so
#	${PROJECT_SOURCE_DIR}/OGRE/lib/libRenderSystem_GL_d.so ${PROJECT_SOURCE_DIR}/OIS/lib/libOIS_d.so 
#	${PROJECT_SOURCE_DIR}/OGRE/lib/libRocketCore_d.so ${PROJECT_SOURCE_DIR}/libRocket/lib/libRocketControls_d.so 
#	${PROJECT_SOURCE_DIR}/libRocket/lib/libRocketDebugger_d.so ${PROJECT_SOURCE_DIR}/SFML/lib/libsfml-audio.so 
#	${PROJECT_SOURCE_DIR}/SFML/lib/libsfml-network.so)

#set(LINUX_LINK_LIBS_ENGINE_RELEASE ${PROJECT_SOURCE_DIR}/AngelScript/lib/libAngelscript.a ${PROJECT_SOURCE_DIR}/Leap/lib/x64/libLeap.so 
#	${PROJECT_SOURCE_DIR}/OGRE/lib/libOgreMain.so ${PROJECT_SOURCE_DIR}/OGRE/lib/libOgreOverlay.so 
#	${PROJECT_SOURCE_DIR}/OGRE/lib/libOgrePaging.so ${PROJECT_SOURCE_DIR}/OGRE/lib/libOgreTerrain.so 
#	${PROJECT_SOURCE_DIR}/OGRE/lib/libOgreRTShaderSystem.so ${PROJECT_SOURCE_DIR}/OGRE/lib/libPlugin_CgProgramManager.so
#	${PROJECT_SOURCE_DIR}/OGRE/lib/libPlugin_OctreeZone.so ${PROJECT_SOURCE_DIR}/OGRE/lib/libPlugin_ParticleFX.so 
#	${PROJECT_SOURCE_DIR}/OGRE/lib/libRenderSystem_Direct3D11.so ${PROJECT_SOURCE_DIR}/OGRE/lib/libRenderSystem_GL.so
#	${PROJECT_SOURCE_DIR}/OIS/lib/libOIS.so 
#	${PROJECT_SOURCE_DIR}/libRocket/lib/libRocketCore.so ${PROJECT_SOURCE_DIR}/libRocket/lib/libRocketControls.so 
#	${PROJECT_SOURCE_DIR}/libRocket/lib/libRocketDebugger.so
#	${PROJECT_SOURCE_DIR}/Newton/lib/libNewton.so ${PROJECT_SOURCE_DIR}/SFML/lib/libsfml-audio.so 
#	${PROJECT_SOURCE_DIR}/SFML/lib/libsfml-network.so)

#set(LINUX_LINK_LIBS_ENGINE_DEBUG libNewton.so libAngelscript.a libLeap.so libOgreMain_d.so libOgreOverlay_d.so libOgrePaging_d.so
#	libOgreTerrain_d.so libOgreRTShaderSystem_d.so libPlugin_CgProgramManager_d.so libPlugin_OctreeZone_d.so libPlugin_ParticleFX_d.so
#	libRenderSystem_Direct3D11_d.so libRenderSystem_GL_d.so libOIS_d.so libRocketCore_d.so
#	libRocketControls_d.so libRocketDebugger_d.so libsfml-audio.so libsfml-network.so)

#set(LINUX_LINK_LIBS_ENGINE_RELEASE libAngelscript.a libLeap.so libOgreMain.so libOgreOverlay.so libOgrePaging.so libOgreTerrain.so 
#	libOgreRTShaderSystem.so libPlugin_CgProgramManager.so libPlugin_OctreeZone.so libPlugin_ParticleFX.so 
#	libRenderSystem_Direct3D11.so libRenderSystem_GL.so libOIS.so libRocketCore.so libRocketControls.so libRocketDebugger.so
#	libNewton.so libsfml-audio.so libsfml-network.so)

#set(LINUX_LINK_LIBS_ENGINE_DEBUG Newton Angelscript Leap OgreMain_d OgreOverlay_d OgrePaging_d OgreTerrain_d OgreRTShaderSystem_d
# 	Plugin_CgProgramManager_d Plugin_OctreeZone_d Plugin_ParticleFX_d RenderSystem_Direct3D11_d RenderSystem_GL_d 
#	OIS_d RocketCore_d RocketControls_d RocketDebugger_d sfml-audio sfml-network)

#set(LINUX_LINK_LIBS_ENGINE_RELEASE Angelscript Leap OgreMain OgreOverlay OgrePaging OgreTerrain OgreRTShaderSystem  
#	Plugin_CgProgramManager Plugin_OctreeZone Plugin_ParticleFX RenderSystem_Direct3D11 RenderSystem_GL OIS RocketCore
#	RocketControls RocketDebugger Newton sfml-audio sfml-network)

set(LINUX_LINK_LIBS_ENGINE_DEBUG OgreMain_d Newton Angelscript Leap 
	OgreOverlay_d OgrePaging_d OgreRTShaderSystem_d
 	OIS_d RocketCore_d RocketControls_d RocketDebugger_d sfml-audio sfml-network)

set(LINUX_LINK_LIBS_ENGINE_RELEASE OgreMain Angelscript Leap OgreOverlay OgrePaging OgreRTShaderSystem  
	OIS RocketCore RocketControls RocketDebugger Newton sfml-audio sfml-network)


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

# modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/CMake")

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
# leap DLLs
file(GLOB LeapMoveFiles "Leap/lib/x86/Leap.dll" "Leap/lib/x86/Leapd.dll")
file(COPY ${LeapMoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin")

# ogre dlls (this should be changed for public release to only copy required dlls)
file(GLOB OgreMoveFiles "OGRE/bin/Debug/*.pdb" "OGRE/bin/Debug/*.dll" "OGRE/bin/Release/*.dll")
file(COPY ${OgreMoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin")

# OIS dll
file(GLOB OISMoveFiles "OIS/Bin/*")
file(COPY ${OISMoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin")

# libRocket dlls
file(GLOB RocketMoveFiles "libRocket/bin/*")
file(COPY ${RocketMoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin")

# newton
file(GLOB NewtonMoveFilesRelease "Newton/bin/Release/*")
file(COPY ${NewtonMoveFilesRelease} DESTINATION "${PROJECT_BINARY_DIR}/bin")
file(GLOB NewtonMoveFilesDebug "Newton/bin/Debug/*")
file(COPY ${NewtonMoveFilesDebug} DESTINATION "${PROJECT_BINARY_DIR}/bin")

# smfl
file(GLOB SMFLMoveFilesRelease "SFML/bin/openal32.dll" "SFML/bin/libsndfile-1.dll" "SFML/bin/sfml-audio-2.dll" "SFML/bin/sfml-system-2.dll"
    "SFML/bin/sfml-network-2.dll")
file(COPY ${SMFLMoveFilesRelease} DESTINATION "${PROJECT_BINARY_DIR}/bin")
file(GLOB SMFLMoveFilesDebug "SFML/bin/sfml-audio-d-2.dll" "SFML/bin/sfml-system-d-2.dll"
    "SFML/bin/sfml-network-d-2.dll")
file(COPY ${SMFLMoveFilesDebug} DESTINATION "${PROJECT_BINARY_DIR}/bin")
else(WIN32)
# linux variants of the copy functions
file(GLOB LeapMoveFiles "Leap/lib/x64/libLeap.so")
file(COPY ${LeapMoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin")

# ogre oses (this should be changed for public release to only copy required oses)
file(GLOB OgreMoveFiles "OGRE/lib/*.so")
file(COPY ${OgreMoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin")

# OIS dll
file(GLOB OISMoveFiles "OIS/lib/*.so")
file(COPY ${OISMoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin")

# libRocket dlls
file(GLOB RocketMoveFiles "libRocket/lib/*.so")
file(COPY ${RocketMoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin")

# newton
file(GLOB NewtonMoveFilesRelease "Newton/lib/*.so")
file(COPY ${NewtonMoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin")

# smfl
file(GLOB SMFLMoveFiles "SFML/lib/libsfml-audio.so.2.1" "SFML/lib/libsfml-network.so.2.1" "SFML/lib/libsfml-network.so.2.1")
file(COPY ${SMFLMoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin")

endif(WIN32)
message(STATUS "Done copying")

# working directory setting
if(WIN32)
include(CreateLaunchers)
include(SetPrecompiledHeaders)
else(WIN32)


endif(WIN32)

configure_file(
    "${PROJECT_SOURCE_DIR}/Engine/Include.h.in"
    "${PROJECT_SOURCE_DIR}/Engine/Include.h"
)


# additional include directories
include_directories("${PROJECT_SOURCE_DIR}/AngelScript/include")
include_directories("${PROJECT_SOURCE_DIR}/AngelScript")
include_directories("${PROJECT_SOURCE_DIR}/Engine/")
include_directories("${PROJECT_SOURCE_DIR}/Leap/include")
include_directories("${PROJECT_SOURCE_DIR}/OIS/include")
include_directories("${PROJECT_SOURCE_DIR}/libRocket/include")
include_directories("${PROJECT_SOURCE_DIR}/Newton/include")
include_directories("${PROJECT_SOURCE_DIR}/SFML/include")

include_directories("${PROJECT_SOURCE_DIR}/OGRE/include")
include_directories("${PROJECT_SOURCE_DIR}/OGRE/include/OGRE")
include_directories("${PROJECT_SOURCE_DIR}/OGRE/include/OGRE/Overlay")
include_directories("${PROJECT_SOURCE_DIR}/OGRE/include/OGRE/Terrain")
include_directories("${PROJECT_SOURCE_DIR}/OGRE/include/OGRE/Paging")
include_directories("${PROJECT_SOURCE_DIR}/OGRE/include/OGRE/Threading")
include_directories("${PROJECT_SOURCE_DIR}/OGRE/include/OGRE/Volume")

# libraries paths
link_directories("${PROJECT_SOURCE_DIR}/AngelScript/lib")
link_directories("${PROJECT_SOURCE_DIR}/OIS/lib/")
link_directories("${PROJECT_SOURCE_DIR}/libRocket/lib/")
link_directories("${PROJECT_SOURCE_DIR}/SFML/lib/")

if(WIN32)
link_directories("${PROJECT_SOURCE_DIR}/Leap/lib/x86")
link_directories("${PROJECT_SOURCE_DIR}/Boost/Lib/debug")
link_directories("${PROJECT_SOURCE_DIR}/Boost/Lib/release")
link_directories("${PROJECT_SOURCE_DIR}/OGRE/lib/Debug")
link_directories("${PROJECT_SOURCE_DIR}/OGRE/lib/Release")
link_directories("${PROJECT_SOURCE_DIR}/Newton/lib/Release")
link_directories("${PROJECT_SOURCE_DIR}/Newton/lib/Debug")
else(WIN32)
link_directories("${PROJECT_SOURCE_DIR}/Newton/lib")
link_directories("${PROJECT_SOURCE_DIR}/Boost/lib")
link_directories("${PROJECT_SOURCE_DIR}/Leap/lib/x64")
link_directories("${PROJECT_SOURCE_DIR}/OGRE/lib")
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
file(GLOB OgreTools "OGRE/bin/Release/*.exe")
file(GLOB SampleBrowserFile "OGRE/bin/Release/SampleBrowser.exe")
#list(REMOVE_ITEM OgreTools ${SampleBrowserFile})
install(FILES ${OgreTools} DESTINATION tools)
endif(WIN32)

if(INSTALL_ONLY_RELEASE)
# only release versions of everything
if(WIN32)
# change some file lists around
file(GLOB LeapMoveFiles "Leap/lib/x86/Leap.dll")
# only release dlls
file(GLOB OgreMoveFiles "OGRE/bin/Release/*.dll")
# only release OIS
file(GLOB OISMoveFiles "OIS/Bin/OIS.dll")
# only release Rocket
file(GLOB RocketMoveFiles "libRocket/bin/RocketControls.dll" "libRocket/bin/RocketCore.dll" "libRocket/bin/RocketDebugger.dll")
endif(WIN32)

else(INSTALL_ONLY_RELEASE)
# install all files that debug distribution might need
if(WIN32)
install(FILES ${VLDMoveFiles} DESTINATION bin)

install(FILES ${NewtonMoveFilesDebug} DESTINATION bin)
install(FILES ${SMFLMoveFilesDebug} DESTINATION bin)
endif(WIN32)

endif(INSTALL_ONLY_RELEASE)

install(FILES ${LeapMoveFiles} DESTINATION bin)
install(FILES ${OgreMoveFiles} DESTINATION bin)
install(FILES ${OISMoveFiles} DESTINATION bin)
install(FILES ${RocketMoveFiles} DESTINATION bin)
if(WIN32)
install(FILES ${NewtonMoveFilesRelease} DESTINATION bin)
install(FILES ${SMFLMoveFilesRelease} DESTINATION bin)
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

