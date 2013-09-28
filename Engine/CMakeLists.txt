# engine library Cmake file


# get all files in their groups
file(GLOB GroupCore "*.cpp" "*.h")
file(GLOB GroupRendering "Rendering/*.cpp" "Rendering/*.h")
file(GLOB GroupRenderingGUI "Rendering/GUI/*.cpp" "Rendering/GUI/*.h")
file(GLOB GroupScript "Script/*.cpp" "Script/*.h")
file(GLOB GroupGUI "GUI/*.cpp" "GUI/*.h")
#file(GLOB GroupGUIObjects "GUI/Objects/*.cpp" "GUI/Objects/*.h")
#file(GLOB GroupGUIComponents "GUI/Components/*.cpp" "GUI/Components/*.h")
file(GLOB GroupEntities "Entities/*.cpp" "Entities/*.h")
file(GLOB GroupEntitiesBases "Entities/Bases/*.cpp" "Entities/Bases/*.h")
file(GLOB GroupUtility "Utility/*.cpp" "Utility/*.h")
file(GLOB GroupUtilityIte "Utility/Iterators/*.cpp" "Utility/Iterators/*.h")
file(GLOB GroupUtilityFile "Utility/FileRelated/*.cpp" "Utility/FileRelated/*.h")
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

# AngelScript addon sources
file(GLOB_RECURSE GroupScriptAddons "${PROJECT_SOURCE_DIR}/AngelScript/add_on/*.cpp" "${PROJECT_SOURCE_DIR}/AngelScript/add_on/*.h")


# set groups
source_group("Math" FILES ${GroupMath})
source_group("Sound" FILES ${GroupSound})
source_group("Leap" FILES ${GroupLeap})
source_group("Scripting" FILES ${GroupScript})
source_group("Scripting\\AngelAddons" FILES ${GroupScriptAddons})

source_group("Entities" FILES ${GroupEntities})
source_group("Entities\\Bases" FILES ${GroupEntitiesBases})

source_group("Utility" FILES ${GroupUtility})
source_group("Utility\\Statistics" FILES ${GroupStatistics})
source_group("Utility\\Iterators" FILES ${GroupUtilityIte})
source_group("Utility\\FileRelated" FILES ${GroupUtilityFile})

source_group("GUI" FILES ${GroupGUI})
#source_group("GUI\\Objects" FILES ${GroupGUIObjects})
#source_group("GUI\\Components" FILES ${GroupGUIComponents})

source_group("Core" FILES ${GroupCore})
source_group("Core\\Events" FILES ${GroupEvents})
source_group("Core\\Input" FILES ${GroupInput})
source_group("Core\\Handlers" FILES ${GroupHandlers})

source_group("Common" FILES ${GroupCommon})
source_group("Common\\Application" FILES ${GroupApplication})
source_group("Common\\Exceptions" FILES ${GroupExceptions})
source_group("Common\\ObjectFiles" FILES ${GroupObjectFiles})
source_group("Common\\DataStoring" FILES ${GroupCommonData})

source_group("Rendering" FILES ${GroupRendering})
source_group("Rendering\\GUI" FILES ${GroupRenderingGUI})

# collect all files to one group
set(EngineFiles ${GroupCore} ${GroupEntities} ${GroupUtility} ${GroupStatistics} ${GroupMath} ${GroupSound}
    ${GroupLeap} ${GroupScript} ${GroupGUI}  ${GroupEvents}
    ${GroupInput} ${GroupHandlers} ${GroupCommon} ${GroupApplication} ${GroupExceptions}
    ${GroupObjectFiles} ${GroupRendering} ${GroupCommonData} ${GroupUtilityIte} ${GroupUtilityFile}
    ${GroupEntitiesBases} ${GroupScriptAddons} ${GroupRenderingGUI})
    
#${GroupGUIObjects} ${GroupGUIComponents}
    
# precompiled headers
# remove from list before adding back
file(GLOB PrecompiledHeaderName "Include.h")
file(GLOB PrecompiledSourceName "Include.cpp")

list(REMOVE_ITEM EngineFiles ${PrecompiledSourceName})

# precompiled header

#ADD_MSVC_PRECOMPILED_HEADER(${PrecompiledHeaderName} ${PrecompiledSourceName} EngineFiles)
ADD_MSVC_PRECOMPILED_HEADER("Include.h" ${PrecompiledSourceName} EngineFiles)
#ADD_MSVC_PRECOMPILED_HEADER("Include.h" "Include.cpp" EngineFiles)

# engine exporting define
add_definitions(-DENGINE_EXPORTS)

# create library with all the files
add_library(Engine SHARED ${EngineFiles})

install(TARGETS Engine DESTINATION bin)

# post build copy
ADD_CUSTOM_COMMAND(TARGET Engine POST_BUILD COMMAND copy ARGS 
    "\"$(SolutionDir)Engine\\$(Configuration)\\$(TargetName).dll\" \"$(SolutionDir)bin\\$(TargetName).dll\""
)

