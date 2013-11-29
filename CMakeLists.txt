# CMake file for generating all project files
cmake_minimum_required(VERSION 2.8)

# we want to build with unicode setting, also add minimal rebuild, and exceptions from floating point operations, higher memory for compiling
add_definitions(-DUNICODE -D_UNICODE -fp:except -Zm162)

# program database flag for debug
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI -Gm /W3")

project(Leviathan)

set(WORK_DIR "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/Install")

option(INSTALL_ONLY_RELEASE "For creating release folders" ON)
option(USE_VLD "For including vld in the projects" ON)


if(USE_VLD)
set(L_VLD_STATE 1)
else(USE_VLD)
set(L_VLD_STATE 0)
endif(USE_VLD)

# version number
set(LEVIATHAN_VERSION_STABLE 0)
set(LEVIATHAN_VERSION_MAJOR 6)
set(LEVIATHAN_VERSION_MINOR 1)
set(LEVIATHAN_VERSION_PATCH 2)

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
file(GLOB VLDMoveFiles "bin/vld.ini" "bin/VLDExportSettings.txt" "bin/VLD data exporter.exe" 
    "bin/EngineConf.conf")
file(COPY ${VLDMoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin")

# copy data directory
file(GLOB DataMoveFiles "bin/Data/*")
file(COPY ${DataMoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin/Data")

# Copy data from the scripts folder to the bin folder
file(GLOB ScriptsMoveFiles "Scripts/*")
file(COPY ${ScriptsMoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin/Data/Scripts")

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

message(STATUS "Done copying")

# working directory setting
include(CreateLaunchers)
include(SetPrecompiledHeaders)

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
link_directories("${PROJECT_SOURCE_DIR}/Leap/lib/x86")
link_directories("${PROJECT_SOURCE_DIR}/Boost/Lib/debug")
link_directories("${PROJECT_SOURCE_DIR}/Boost/Lib/release")
link_directories("${PROJECT_SOURCE_DIR}/OGRE/lib/Debug")
link_directories("${PROJECT_SOURCE_DIR}/OGRE/lib/Release")
link_directories("${PROJECT_SOURCE_DIR}/OIS/lib/")
link_directories("${PROJECT_SOURCE_DIR}/libRocket/lib/")
link_directories("${PROJECT_SOURCE_DIR}/Newton/lib/Release")
link_directories("${PROJECT_SOURCE_DIR}/Newton/lib/Debug")
link_directories("${PROJECT_SOURCE_DIR}/SFML/lib/")

# engine linking
link_directories("${PROJECT_BINARY_DIR}")
link_directories("${PROJECT_BINARY_DIR}/Engine/Debug")
link_directories("${PROJECT_BINARY_DIR}/Engine/Release")

# engine project 
add_subdirectory(Engine)

# test project
enable_testing()
add_subdirectory(LeviathanTest)


# sandboxie example project
add_subdirectory(Project_sandboxie)

# pong example game
add_subdirectory(Pong)

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
# ogre tools
file(GLOB OgreTools "OGRE/bin/Release/*.exe")
file(GLOB SampleBrowserFile "OGRE/bin/Release/SampleBrowser.exe")
#list(REMOVE_ITEM OgreTools ${SampleBrowserFile})
install(FILES ${OgreTools} DESTINATION tools)

if(INSTALL_ONLY_RELEASE)
# only release versions of everything

# change some file lists around
file(GLOB LeapMoveFiles "Leap/lib/x86/Leap.dll")
# only release dlls
file(GLOB OgreMoveFiles "OGRE/bin/Release/*.dll")
# only release OIS
file(GLOB OISMoveFiles "OIS/Bin/OIS.dll")
# only release Rocket
file(GLOB RocketMoveFiles "libRocket/bin/RocketControls.dll" "libRocket/bin/RocketCore.dll" "libRocket/bin/RocketDebugger.dll")
# only release angelscript
#file(GLOB AngelMoveFiles "AngelScript/bin/angelscript.dll")

else(INSTALL_ONLY_RELEASE)
# install all files that debug distribution might need
install(FILES ${VLDMoveFiles} DESTINATION bin)

install(FILES ${NewtonMoveFilesDebug} DESTINATION bin)
install(FILES ${SMFLMoveFilesDebug} DESTINATION bin)

endif(INSTALL_ONLY_RELEASE)

install(FILES ${LeapMoveFiles} DESTINATION bin)
install(FILES ${OgreMoveFiles} DESTINATION bin)
install(FILES ${OISMoveFiles} DESTINATION bin)
install(FILES ${RocketMoveFiles} DESTINATION bin)
install(FILES ${NewtonMoveFilesRelease} DESTINATION bin)
install(FILES ${SMFLMoveFilesRelease} DESTINATION bin)

# testing support


# CPack support
include(InstallRequiredSystemLibraries)

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Leviathan Game Engine")
SET(CPACK_PACKAGE_VENDOR "Me, myself, and I")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/README.md")
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

endif(WIN32)

include(CPack)

