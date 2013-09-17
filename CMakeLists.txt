# CMake file for generating all project files
cmake_minimum_required(VERSION 2.8)

# we want to build with unicode setting, also add minimal rebuild, and exceptions from floating point operations
add_definitions(-DUNICODE -D_UNICODE -Gm -fp:except /ZI)

project(Leviathan)

set(WORK_DIR ${PROJECT_BINARY_DIR})
set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/Install")
#set(PATHS_OGRE_COPY_DLL_PATH "H:/c++ resources/OgreSDK_vc10_v1-9-0unstable/bin" CACHE PATH "path for ogre binarie/bin dir")

# version number
set(LEVIATHAN_VERSION 0.530)
set(LEVIATHAN_VERSIONS "L\"0.5.3.0\"")
set(LEVIATHAN_VERSION_ANSIS "\"0.5.3.0\"")

set(CMAKE_DEBUG_POSTFIX "D")

# modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/CMake")

# create bin directory
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

message(STATUS "Copying files...")

# copy data from bin directory
file(GLOB MoveFiles "bin/vld.ini" "bin/VLDExportSettings.txt" "bin/VLD data exporter.exe" 
    "bin/EngineConf.conf")
file(COPY ${MoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin")

# copy data directory
file(GLOB MoveFiles "bin/Data/*")
file(COPY ${MoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin/Data")

# leap DLLs
file(GLOB MoveFiles "Leap/lib/x86/Leap.dll" "Leap/lib/x86/Leapd.dll")
file(COPY ${MoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin")

# ogre dlls (this should be changed for public release to only copy required dlls)
file(GLOB MoveFiles "OGRE/bin/Debug/*.pdb" "OGRE/bin/Debug/*.dll" "OGRE/bin/Release/*.dll")
file(COPY ${MoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin")


message(STATUS "Done copying")

# working directory setting
include(CreateLaunchers)
include(SetPrecompiledHeaders)

configure_file(
    "${PROJECT_SOURCE_DIR}/Engine/Include.h.in"
    "${PROJECT_SOURCE_DIR}/Engine/Include.h"
)


# additional include directories
include_directories("${PROJECT_SOURCE_DIR}/AngelScript/")
include_directories("${PROJECT_SOURCE_DIR}/Engine/")
include_directories("${PROJECT_SOURCE_DIR}/Leap/include")
include_directories("${PROJECT_SOURCE_DIR}/OGRE/include")
include_directories("${PROJECT_SOURCE_DIR}/OGRE/include/OGRE")
include_directories("${PROJECT_SOURCE_DIR}/OGRE/include/OGRE/Overlay")
include_directories("${PROJECT_SOURCE_DIR}/OGRE/include/OGRE/Terrain")
include_directories("${PROJECT_SOURCE_DIR}/OGRE/include/OGRE/Paging")
include_directories("${PROJECT_SOURCE_DIR}/OGRE/include/OGRE/Threading")
include_directories("${PROJECT_SOURCE_DIR}/OGRE/include/OGRE/Volume")

# libraries paths
link_directories("${PROJECT_SOURCE_DIR}/AngelScript")
link_directories("${PROJECT_SOURCE_DIR}/Leap/lib/x86")
link_directories("${PROJECT_SOURCE_DIR}/Boost/Lib/debug")
link_directories("${PROJECT_SOURCE_DIR}/Boost/Lib/release")
link_directories("${PROJECT_SOURCE_DIR}/OGRE/lib/Debug")
link_directories("${PROJECT_SOURCE_DIR}/OGRE/lib/Release")
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


