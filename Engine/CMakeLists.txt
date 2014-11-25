# engine library Cmake file

# Generate additional source files
# This is here as an example to make this work later
#add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/Engine/Generated/Generated.h
#   COMMAND FileGenerator "V8EXT" "\"${PROJECT_SOURCE_DIR}/Engine/GeneratorInput/Generated.in.h\"" 
#   "\"${PROJECT_SOURCE_DIR}/Engine/Generated/Generated.h\""
#   DEPENDS FileGenerator "${PROJECT_SOURCE_DIR}/Engine/GeneratorInput/Generated.in.h"
#)

# get all files in their groups
set(GroupCore "Define.cpp" "Define.h" "Engine.cpp" "Engine.h" "FileSystem.cpp" "FileSystem.h" "ForwardDeclarations.h"
    "LeviathanMainDll.h" "Logger.cpp" "Logger.h"
    "TextureGenerator.cpp" "TextureGenerator.h" "Window.h" "Window.cpp" "utf8.h"
)
file(GLOB GroupRendering "Rendering/*.cpp" "Rendering/*.h")
file(GLOB GroupRenderingGUI "Rendering/GUI/*.cpp" "Rendering/GUI/*.h")
file(GLOB GroupScript "Script/*.cpp" "Script/*.h")
file(GLOB GroupGUI "GUI/*.cpp" "GUI/*.h")
file(GLOB GroupNewton "Newton/*.cpp" "Newton/*.h")
file(GLOB GroupEntities "Entities/*.cpp" "Entities/*.h")
file(GLOB GroupEntitiesBases "Entities/Bases/*.cpp" "Entities/Bases/*.h")
file(GLOB GroupEntitiesObjects "Entities/Objects/*.cpp" "Entities/Objects/*.h")
file(GLOB GroupEntitiesSerializers "Entities/Serializers/*.cpp" "Entities/Serializers/*.h")

file(GLOB GroupUtility "Utility/*.cpp" "Utility/*.h")
file(GLOB GroupUtilityFile "Utility/FileRelated/*.cpp" "Utility/FileRelated/*.h")
file(GLOB GroupUtilityData "Utility/DataHandling/*.cpp" "Utility/DataHandling/*.h")
file(GLOB GroupMath "Math/*.cpp" "Math/*.h")
file(GLOB GroupEvents "Events/*.cpp" "Events/*.h")
file(GLOB GroupExceptions "Exceptions/*.cpp" "Exceptions/*.h")
file(GLOB GroupApplication "Application/*.cpp" "Application/*.h")
file(GLOB GroupCommon "Common/*.cpp" "Common/*.h")
file(GLOB GroupCommonData "Common/DataStoring/*.cpp" "Common/DataStoring/*.h")
file(GLOB GroupSound "Sound/*.cpp" "Sound/*.h")
file(GLOB GroupStatistics "Statistics/*.cpp" "Statistics/*.h")
file(GLOB GroupInput "Input/*.cpp" "Input/*.h")
file(GLOB GroupLeap "Leap/*.cpp" "Leap/*.h")
file(GLOB GroupObjectFiles "ObjectFiles/*.cpp" "ObjectFiles/*.h")
file(GLOB GroupHandlers "Handlers/*.cpp" "Handlers/*.h")
file(GLOB GroupAddons "Addons/*.cpp" "Addons/*.h")
file(GLOB GroupNetworking "Networking/*.cpp" "Networking/*.h")
file(GLOB GroupThreading "Threading/*.cpp" "Threading/*.h")
file(GLOB GroupIterators "Iterators/*.cpp" "Iterators/*.h")

file(GLOB GroupGameplay "Gameplay/*.cpp" "Gameplay/*.h")
file(GLOB GroupPermissions "Gameplay/Permissions/*.cpp" "Gameplay/Permissions/*.h")



file(GLOB GeneratorInput "GeneratorInput/*.*")

file(GLOB GroupLZ4 "lz4/*.c" "lz4/*.cpp" "lz4/*.h")
file(GLOB GroupJsoncpp "jsoncpp/*.h" "jsoncpp/*.cpp")
file(GLOB Grouputf8 "utf8/*.h" "utf8/*.cpp")

# Generated files
set(GroupGeneratedFiles)
#set(GroupGeneratedFiles ${GroupGeneratedFiles} "${PROJECT_SOURCE_DIR}/Engine/Generated/Generated.h")



# AngelScript addon sources
file(GLOB_RECURSE GroupScriptAddons "${PROJECT_SOURCE_DIR}/AngelScript/add_on/*.cpp" "${PROJECT_SOURCE_DIR}/AngelScript/add_on/*.h")


# set groups
source_group("Math" FILES ${GroupMath})
source_group("Sound" FILES ${GroupSound})
source_group("Leap" FILES ${GroupLeap})
source_group("Scripting" FILES ${GroupScript})
source_group("Scripting\\AngelAddons" FILES ${GroupScriptAddons})
source_group("Addons" FILES ${GroupAddons})
source_group("Networking" FILES ${GroupNetworking})
source_group("Threading" FILES ${GroupThreading})

source_group("GeneratorInput" FILES ${GeneratorInput})
source_group("GeneratedFiles" FILES ${GroupGeneratedFiles})

source_group("Libraries\\lz4" FILES ${GroupLZ4})
source_group("Libraries\\json-cpp" FILES ${GroupJsoncpp})
source_group("Libraries\\Grouputf8" FILES ${Grouputf8})

source_group("Entities" FILES ${GroupEntities})
source_group("Entities\\Bases" FILES ${GroupEntitiesBases})
source_group("Entities\\Objects" FILES ${GroupEntitiesObjects})
source_group("Entities\\Serializers" FILES ${GroupEntitiesSerializers})

source_group("Utility" FILES ${GroupUtility})
source_group("Utility\\Statistics" FILES ${GroupStatistics})
source_group("Iterators" FILES ${GroupIterators})
source_group("Utility\\FileRelated" FILES ${GroupUtilityFile})
source_group("Utility\\DataHandling" FILES ${GroupUtilityData})

source_group("GUI" FILES ${GroupGUI})

source_group("Core" FILES ${GroupCore})
source_group("Core\\Events" FILES ${GroupEvents})
source_group("Core\\Input" FILES ${GroupInput})
source_group("Core\\Handlers" FILES ${GroupHandlers})
source_group("Newton" FILES ${GroupNewton})

source_group("Common" FILES ${GroupCommon})
source_group("Common\\Application" FILES ${GroupApplication})
source_group("Common\\Exceptions" FILES ${GroupExceptions})
source_group("Common\\DataStoring" FILES ${GroupCommonData})
source_group("Gameplay" FILES ${GroupGameplay})
source_group("Gameplay\\Permissions" FILES ${GroupPermissions})

source_group("ObjectFiles" FILES ${GroupObjectFiles})

source_group("Rendering" FILES ${GroupRendering})
source_group("Rendering\\GUI" FILES ${GroupRenderingGUI})

# collect all files to one group
set(EngineFiles ${GroupCore} ${GroupEntities} ${GroupUtility} ${GroupStatistics} ${GroupMath} ${GroupSound}
    ${GroupLeap} ${GroupScript} ${GroupGUI}  ${GroupEvents}
    ${GroupInput} ${GroupHandlers} ${GroupCommon} ${GroupApplication} ${GroupExceptions}
    ${GroupObjectFiles} ${GroupRendering} ${GroupCommonData} ${GroupUtilityIte} ${GroupUtilityFile}
    ${GroupEntitiesBases} ${GroupScriptAddons} ${GroupRenderingGUI} ${GroupNewton} ${GroupEntitiesObjects}
    ${GroupUtilityData} ${GroupAddons} ${GroupNetworking} ${GroupThreading} ${GroupIterators} ${GroupGameplay} 
    ${GroupPermissions} ${GroupEntitiesSerializers} ${GroupLZ4} ${GroupJsoncpp} ${GeneratorInput} ${GroupGeneratedFiles}
    ${Grouputf8})

    


# Don't use precompiled header on c source file
set(EngineFiles ${EngineFiles} )

# engine exporting define
add_definitions(-DENGINE_EXPORTS)

# create library with all the files
add_library(Engine SHARED ${EngineFiles})



set(FinalEngineLinkLibraries ${Boost_LIBRARIES} ${OGRE_LIBRARIES} ${OIS_LIBRARIES} ${ADDITIONAL_OGRE})

if(NOT WIN32)
# Link with X11 and threads
set(FinalEngineLinkLibraries ${FinalEngineLinkLibraries} ${X11_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

endif(NOT WIN32)

# Add the final libraries that have different debug and optimized values
set(FinalEngineLinkLibraries ${FinalEngineLinkLibraries} ${LINK_LIBS_ENGINE_DEBUG} ${LINK_LIBS_ENGINE_RELEASE})

target_link_libraries(Engine ${FinalEngineLinkLibraries})

# Avoid Xlib headers and windows header in the precompiled header to avoid macros destroying everything
set_target_properties(Engine PROPERTIES 
  COTIRE_PREFIX_HEADER_IGNORE_PATH
  "${X11_INCLUDE_DIR}/X11;${CMAKE_SOURCE_DIR};${CMAKE_BINARY_DIR}")


# Allow adding in folder contained libraries to be cotired
# And add the Include.h as a precompiled header file
set_target_properties(Engine PROPERTIES 
  COTIRE_PREFIX_HEADER_INCLUDE_PATH 
  "${PROJECT_SOURCE_DIR}/AngelScript;${PROJECT_SOURCE_DIR}/CEGUI;${PROJECT_SOURCE_DIR}/Newton;${PROJECT_SOURCE_DIR}/SFML;${PROJECT_SOURCE_DIR}/Leap;${CMAKE_SOURCE_DIR}/Engine/Include.h")


# speed up build
cotire(Engine)


# Possible debug installs
if(NOT INSTALL_ONLY_RELEASE OR INSTALL_CREATE_SDK)
    set(InstallConfigs "Release" "Debug")
else()
    set(InstallConfigs "Release")
endif()

install(TARGETS Engine DESTINATION bin CONFIGURATIONS ${InstallConfigs})

# post build copy
if(WIN32 AND NOT MINGW)
ADD_CUSTOM_COMMAND(TARGET Engine POST_BUILD COMMAND copy ARGS 
    "\"$(SolutionDir)bin\\$(Configuration)\\$(TargetName).dll\" \"$(SolutionDir)bin\\$(TargetName).dll\""
)
endif(WIN32 AND NOT MINGW)

# Symbol building
if(UNIX)
  add_custom_target(Engine_Symbols ${SYMBOL_EXTRACTOR} "${CMAKE_BINARY_DIR}/bin/libEngine.so"
    DEPENDS Engine WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/Symbols VERBATIM)
elseif(WIN32)
  add_custom_target(Engine_Symbols ${SYMBOL_EXTRACTOR} "${CMAKE_BINARY_DIR}/bin/Engine.dll"
    DEPENDS Engine WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/Symbols VERBATIM)
endif()

