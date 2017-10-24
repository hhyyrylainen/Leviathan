# engine library Cmake file

# Generate additional source files
# This is here as an example to make this work later
#add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/Engine/Generated/Generated.h
#   COMMAND FileGenerator "V8EXT" "\"${PROJECT_SOURCE_DIR}/Engine/GeneratorInput/Generated.in.h\"" 
#   "\"${PROJECT_SOURCE_DIR}/Engine/Generated/Generated.h\""
#   DEPENDS FileGenerator "${PROJECT_SOURCE_DIR}/Engine/GeneratorInput/Generated.in.h"
#)

# get all files in their groups
if(CREATE_UE4_PLUGIN)

  set(GroupCore "Define.cpp" "Define.h" "FileSystem.cpp" "FileSystem.h" "ForwardDeclarations.h"
    "ErrorReporter.h" "Logger.cpp" "Logger.h" "utf8.h" "Include.h"
    "Exceptions.h" "Exceptions.cpp"
    "TimeIncludes.h" "TimeIncludes.cpp" "WindowsInclude.h"
    )
  
  set(GroupCommon "Common/BaseNotifiable.h" "Common/BaseNotifiableImpl.h"
    "Common/BaseNotifier.h" "Common/BaseNotifierImpl.h"
    "Common/ReferenceCounted.h"
    "Common/StringOperations.cpp" "Common/StringOperations.h"
    "Common/ExtraAlgorithms.h"
    "Common/ThreadSafe.h" 
    "Common/Types.h" "Common/Types.cpp"
    "Common/Visitor.cpp" "Common/Visitor.h"
    )
  
  set(GroupCommonData "Common/DataStoring/DataBlock.cpp" "Common/DataStoring/DataBlock.h"
    "Common/DataStoring/NamedVars.cpp" "Common/DataStoring/NamedVars.h"
    )
  
  set(GroupUtility "Utility/Convert.cpp" "Utility/Convert.h"
    "Utility/Random.cpp" "Utility/Random.h"
    )
  
  set(GroupMath "Math/CommonMath.cpp" "Math/CommonMath.h")

  set(GroupEntities "Entities/EntityCommon.h")

else()
  set(GroupCore "Define.cpp" "Define.h" "Engine.cpp" "Engine.h" "FileSystem.cpp" "FileSystem.h"
    "ForwardDeclarations.h"
    "Logger.cpp" "Logger.h" "ErrorReporter.h"
    "TextureGenerator.cpp" "TextureGenerator.h" "Window.h" "Window.cpp" "utf8.h" "Include.h"
    "Exceptions.h" "Exceptions.cpp"
    "TimeIncludes.h" "TimeIncludes.cpp"
    "CEGUIInclude.h" "WindowsInclude.h"
    )
  
  set(GroupCommon "Common/BaseNotifiable.h" "Common/BaseNotifiableImpl.h"
    "Common/BaseNotifier.h" "Common/BaseNotifierImpl.h"
    "Common/ObjectPool.h" "Common/ObjectPoolThreadSafe.h"
    "Common/ReferenceCounted.h"
    "Common/SFMLPackets.cpp" "Common/SFMLPackets.h"
    "Common/StringOperations.cpp" "Common/StringOperations.h"
    "Common/ThreadSafe.h" 
    "Common/Types.h" "Common/Types.cpp"
    "Common/Visitor.cpp" "Common/Visitor.h"
    )
  
  set(GroupMath "Math/CommonMath.cpp" "Math/CommonMath.h"
    "Math/RayTracing.cpp" "Math/RayTracing.h"
    "Math/TriangulateDelaunay.cpp" "Math/TriangulateDelaunay.h"
    )

  set(GroupUtility "Utility/ComplainOnce.cpp" "Utility/ComplainOnce.h"
    "Utility/Convert.cpp" "Utility/Convert.h"
    "Utility/DebugVariableNotifier.cpp" "Utility/DebugVariableNotifier.h"
    "Utility/MD5Generator.cpp" "Utility/MD5Generator.h"
    "Utility/Random.cpp" "Utility/Random.h"
    "Utility/well.h" "Utility/InstanceCounter.h"
    )
  
  set(GroupCommonData "Common/DataStoring/DataBlock.cpp" "Common/DataStoring/DataBlock.h"
    "Common/DataStoring/NamedVars.cpp" "Common/DataStoring/NamedVars.h"
    "Common/DataStoring/DataStore.cpp" "Common/DataStoring/DataStore.h"
    )

  add_custom_command(OUTPUT "${PROJECT_SOURCE_DIR}/Engine/Generated/StandardWorld.h"
    "${PROJECT_SOURCE_DIR}/Engine/Generated/StandardWorld.cpp"
    "${PROJECT_SOURCE_DIR}/Engine/Generated/StandardWorldBindings.h"
    COMMAND "ruby" "${PROJECT_SOURCE_DIR}/Engine/Entities/GenerateStandardWorld.rb"
    "${PROJECT_SOURCE_DIR}/Engine/Generated/StandardWorld"
    "${PROJECT_SOURCE_DIR}/Engine/Generated/StandardWorldBindings.h"
    DEPENDS "${PROJECT_SOURCE_DIR}/Engine/Entities/GenerateStandardWorld.rb"
    "${PROJECT_SOURCE_DIR}/Helpers/FileGen.rb"
    )

  add_custom_command(OUTPUT "${PROJECT_SOURCE_DIR}/Engine/Generated/ComponentStates.h"
    "${PROJECT_SOURCE_DIR}/Engine/Generated/ComponentStates.cpp"
    #"${PROJECT_SOURCE_DIR}/Engine/Generated/ComponentStatesBindings.h"
    COMMAND "ruby" "${PROJECT_SOURCE_DIR}/Engine/Entities/GenerateComponentStates.rb"
    "${PROJECT_SOURCE_DIR}/Engine/Generated/ComponentStates"
    #"${PROJECT_SOURCE_DIR}/Engine/Generated/StandardWorldBindings.h"
    DEPENDS "${PROJECT_SOURCE_DIR}/Engine/Entities/GenerateComponentStates.rb"
    "${PROJECT_SOURCE_DIR}/Helpers/FileGen.rb"
    )  
  
  set(GroupEntities "Entities/CommonStateObjects.cpp" "Entities/CommonStateObjects.h"
    "Entities/Components.cpp" "Entities/Components.h"
    "Entities/Component.h" "Entities/ComponentState.h" 
    "Entities/StateHolder.h" "Entities/StateHolder.cpp" 
    "Entities/StateInterpolator.h"
    "Entities/EntityCommon.h"
    "Entities/GameWorld.cpp" "Entities/GameWorld.h"
    "Entities/System.h" "Entities/Systems.cpp" "Entities/Systems.h"
    "Entities/GameWorldFactory.h" "Entities/GameWorldFactory.cpp"
    "Generated/StandardWorld.h" "Generated/StandardWorld.cpp"
    "Generated/ComponentStates.h" "Generated/ComponentStates.cpp"
    )
  
  file(GLOB GroupApplication "Application/AppDefine.cpp" "Application/AppDefine.h"
    "Application/Application.cpp" "Application/Application.h"
    "Application/GameConfiguration.cpp" "Application/GameConfiguration.h"
    "Application/KeyConfiguration.cpp" "Application/KeyConfiguration.h"
    "Application/MasterServerApplication.cpp" "Application/MasterServerApplication.h"
    "Application/ServerApplication.cpp" "Application/ServerApplication.h"
    "Application/ClientApplication.cpp" "Application/ClientApplication.h"
    "Application/ConsoleInput.cpp" "Application/ConsoleInput.h"
    )

  add_custom_command(OUTPUT "${PROJECT_SOURCE_DIR}/Engine/Generated/RequestImpl.h"
    COMMAND "ruby" "${PROJECT_SOURCE_DIR}/Engine/Networking/GenerateRequest.rb"
    "${PROJECT_SOURCE_DIR}/Engine/Generated/RequestImpl.h"
    DEPENDS "${PROJECT_SOURCE_DIR}/Engine/Networking/GenerateRequest.rb"
    "${PROJECT_SOURCE_DIR}/Helpers/FileGen.rb"
    )

  add_custom_command(OUTPUT "${PROJECT_SOURCE_DIR}/Engine/Generated/ResponseImpl.h"
    COMMAND "ruby" "${PROJECT_SOURCE_DIR}/Engine/Networking/GenerateResponse.rb"
    "${PROJECT_SOURCE_DIR}/Engine/Generated/ResponseImpl.h"
    DEPENDS "${PROJECT_SOURCE_DIR}/Engine/Networking/GenerateResponse.rb"
    "${PROJECT_SOURCE_DIR}/Helpers/FileGen.rb"
    )

  
  set(GroupNetworking
    "Networking/Connection.cpp" "Networking/Connection.h"
    "Networking/NetworkAckField.cpp" "Networking/NetworkAckField.h"
    "Networking/WireData.cpp" "Networking/WireData.h"
    "Networking/SentNetworkThing.cpp" "Networking/SentNetworkThing.h"
    "Networking/GameSpecificPacketHandler.cpp" "Networking/GameSpecificPacketHandler.h"
    "Networking/MasterServer.cpp" "Networking/MasterServer.h"
    "Networking/NetworkCache.cpp" "Networking/NetworkCache.h"
    "Networking/NetworkClientInterface.cpp" "Networking/NetworkClientInterface.h"
    "Networking/NetworkHandler.cpp" "Networking/NetworkHandler.h"
    "Networking/MasterServerInfo.h"
    "Networking/NetworkInterface.cpp" "Networking/NetworkInterface.h"
    "Networking/NetworkRequest.cpp" "Networking/NetworkRequest.h"
    "Networking/NetworkResponse.cpp" "Networking/NetworkResponse.h"
    "Networking/NetworkServerInterface.cpp" "Networking/NetworkServerInterface.h"
    "Networking/NetworkMasterServerInterface.cpp" "Networking/NetworkMasterServerInterface.h"
    "Networking/RemoteConsole.cpp" "Networking/RemoteConsole.h"
    "Networking/SyncedResource.cpp" "Networking/SyncedResource.h"
    "Networking/SyncedVariables.cpp" "Networking/SyncedVariables.h"
    "Networking/ConnectedPlayer.cpp" "Networking/ConnectedPlayer.h"
    "Networking/CommonNetwork.h"
    "Generated/RequestImpl.h" "Networking/GenerateRequest.rb"
    "Generated/ResponseImpl.h" "Networking/GenerateResponse.rb"
    )
  
  
  file(GLOB GroupRendering "Rendering/*.cpp" "Rendering/*.h")
  file(GLOB GroupRenderingGUI "Rendering/GUI/*.cpp" "Rendering/GUI/*.h")
  set(GroupScript
    "Script/Console.cpp" "Script/Console.h"
    "Script/ScriptArgumentsProvider.cpp" "Script/ScriptArgumentsProvider.h"
    "Script/ScriptExecutor.cpp" "Script/ScriptExecutor.h"
    "Script/ScriptModule.cpp" "Script/ScriptModule.h"
    "Script/ScriptNotifiers.cpp" "Script/ScriptNotifiers.h"
    "Script/ScriptRunningSetup.cpp" "Script/ScriptRunningSetup.h"
    "Script/ScriptScript.cpp" "Script/ScriptScript.h"
    )

  set(GroupScriptInterface
    "Script/Interface/ScriptEventListener.h"
    "Script/Interface/ScriptLock.h"
    "Script/Interface/ScriptDelegateSlot.h"
    )

  set(GroupScriptBindings  
    "Script/Bindings/BindHelpers.h"
    "Script/Bindings/CommonEngineBind.h" "Script/Bindings/CommonEngineBind.cpp"
    "Script/Bindings/GuiScriptBind.h" "Script/Bindings/GuiScriptBind.cpp"
    "Script/Bindings/TypesBind.h" "Script/Bindings/TypesBind.cpp"
    "Script/Bindings/EntityBind.h" "Script/Bindings/EntityBind.cpp"
    "Script/Bindings/StandardWorldBindHelper.h" "Generated/StandardWorldBindings.h"
    )
  


  set(GroupGUI 
    "GUI/BaseGuiObject.cpp" "GUI/BaseGuiObject.h"
    "GUI/GuiCollection.cpp" "GUI/GuiCollection.h"
    "GUI/GuiManager.cpp" "GUI/GuiManager.h"
    "GUI/AlphaHitCache.cpp" "GUI/AlphaHitCache.h"
    "GUI/VideoPlayer.cpp" "GUI/VideoPlayer.h"
    )

  set(GroupGUIWidgets
    "GUI/Widgets/AlphaHit.h" "GUI/Widgets/AlphaHit.cpp"
    "GUI/Widgets/CEGUIVideoPlayer.h" "GUI/Widgets/CEGUIVideoPlayer.cpp"    
    )  


  
  file(GLOB GroupNewton "Newton/*.cpp" "Newton/*.h")


  file(GLOB GroupEntitiesObjects "Entities/Objects/*.cpp" "Entities/Objects/*.h")
  file(GLOB GroupEntitiesSerializers "Entities/Serializers/*.cpp" "Entities/Serializers/*.h")


  
  
  set(GroupEvents
    "Events/AutoUpdateable.cpp" "Events/AutoUpdateable.h"
    "Events/CallableObject.cpp" "Events/CallableObject.h"
    "Events/DelegateSlot.cpp" "Events/DelegateSlot.h"
    "Events/EventableScriptObject.cpp" "Events/EventableScriptObject.h"
    "Events/Event.cpp" "Events/Event.h"
    "Events/EventHandler.cpp" "Events/EventHandler.h"
    )
  
  file(GLOB GroupExceptions "Exceptions/*.cpp" "Exceptions/*.h")
  set(GroupSound
    "Sound/SoundDevice.cpp" "Sound/SoundDevice.h"
    "Sound/SoundPlayingSlot.cpp" "Sound/SoundPlayingSlot.h"
    "Sound/SoundStream.h" "Sound/SoundStream.cpp"
    )

  file(GLOB GroupInput "Input/*.cpp" "Input/*.h")
  file(GLOB GroupHandlers "Handlers/*.cpp" "Handlers/*.h")
  file(GLOB GroupAddons "Addons/*.cpp" "Addons/*.h")
  file(GLOB GroupGameplay "Gameplay/*.cpp" "Gameplay/*.h")
  file(GLOB GroupPermissions "Gameplay/Permissions/*.cpp" "Gameplay/Permissions/*.h")


  file(GLOB GroupStatistics "Statistics/*.cpp" "Statistics/*.h")

  file(GLOB GroupUtilityFile "Utility/FileRelated/*.cpp" "Utility/FileRelated/*.h")
  file(GLOB GroupUtilityData "Utility/DataHandling/*.cpp" "Utility/DataHandling/*.h")

  if(USING_LEAP)
    file(GLOB GroupLeap "Leap/*.cpp" "Leap/*.h")
  endif()

endif()

set(GroupObjectFiles
  "ObjectFiles/ObjectFile.cpp" "ObjectFiles/ObjectFile.h"
  "ObjectFiles/ObjectFileProcessor.cpp" "ObjectFiles/ObjectFileProcessor.h"
  )

file(GLOB GroupThreading "Threading/*.cpp" "Threading/*.h")
file(GLOB GroupIterators
  "Iterators/IteratorData.h"
  "Iterators/StringDataIterator.cpp" "Iterators/StringDataIterator.h"
  "Iterators/StringIterator.cpp" "Iterators/StringIterator.h"
  )

file(GLOB GeneratorInput "GeneratorInput/*.*")

file(GLOB GroupLZ4 "lz4/*.c" "lz4/*.cpp" "lz4/*.h")

if(NOT CREATE_UE4_PLUGIN)
    file(GLOB GroupJsoncpp "jsoncpp/*.h" "jsoncpp/*.cpp")
endif()

file(GLOB Grouputf8 "utf8/*.h" "utf8/*.cpp")

# Generated files
set(GroupGeneratedFiles)
#set(GroupGeneratedFiles ${GroupGeneratedFiles} "${PROJECT_SOURCE_DIR}/Engine/Generated/Generated.h")

# AngelScript addon sources
if(USE_ANGELSCRIPT)
  file(GLOB_RECURSE GroupScriptAddons "${LEVIATHAN_SRC}/build/ThirdParty/include/add_on/*.cpp"
    "${LEVIATHAN_SRC}/build/ThirdParty/include/add_on/*.h")

  if(NOT GroupScriptAddons)

    message(SEND_ERROR "AngelScript add_on files haven't been installed to the "
      "ThirdParty/include directory")
  endif()
endif()

# set groups
source_group("Math" FILES ${GroupMath})
source_group("Sound" FILES ${GroupSound})
source_group("Scripting" FILES ${GroupScript})
source_group("Scripting\\AngelAddons" FILES ${GroupScriptAddons})
source_group("Scripting\\Interface" FILES ${GroupScriptInterface})
source_group("Scripting\\Bindings" FILES ${GroupScriptBindings})

source_group("Addons" FILES ${GroupAddons})
source_group("Networking" FILES ${GroupNetworking})
source_group("Threading" FILES ${GroupThreading})

source_group("GeneratorInput" FILES ${GeneratorInput})
source_group("GeneratedFiles" FILES ${GroupGeneratedFiles})

source_group("Libraries\\lz4" FILES ${GroupLZ4})
source_group("Libraries\\json-cpp" FILES ${GroupJsoncpp})
source_group("Libraries\\utf8" FILES ${Grouputf8})

source_group("Entities" FILES ${GroupEntities})
source_group("Entities\\Objects" FILES ${GroupEntitiesObjects})
source_group("Entities\\Serializers" FILES ${GroupEntitiesSerializers})

source_group("Utility" FILES ${GroupUtility})
source_group("Utility\\Statistics" FILES ${GroupStatistics})
source_group("Iterators" FILES ${GroupIterators})
source_group("Utility\\FileRelated" FILES ${GroupUtilityFile})
source_group("Utility\\DataHandling" FILES ${GroupUtilityData})

source_group("GUI" FILES ${GroupGUI})
source_group("GUI\\Widgets" FILES ${GroupGUIWidgets})

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

if(USE_LEAP)
  source_group("Leap" FILES ${GroupLeap})
endif()

# collect all files to one group
set(EngineFiles ${GroupCore} ${GroupEntities} ${GroupUtility} ${GroupStatistics}
  ${GroupMath} ${GroupSound}
  ${GroupScript} ${GroupScriptInterface} ${GroupScriptBindings}  ${GroupScriptAddons} 
  ${GroupGUI} ${GroupGUIWidgets} ${GroupEvents}
  ${GroupInput} ${GroupHandlers} ${GroupCommon} ${GroupApplication} ${GroupExceptions}
  ${GroupObjectFiles} ${GroupRendering} ${GroupCommonData} ${GroupUtilityIte}
  ${GroupUtilityFile}
  ${GroupRenderingGUI} ${GroupNewton} ${GroupEntitiesObjects}
  ${GroupUtilityData} ${GroupAddons} ${GroupNetworking} ${GroupThreading} ${GroupIterators}
  ${GroupGameplay} 
  ${GroupPermissions} ${GroupEntitiesSerializers} ${GroupLZ4} ${GroupJsoncpp}
  ${GeneratorInput} ${GroupGeneratedFiles}
  ${Grouputf8}
  )

if(USE_LEAP)
  set(EngineFiles ${EngineFiles} ${GroupLeap})
endif()

    
# engine exporting define
add_definitions(-DENGINE_EXPORTS)
add_definitions(-DLEVIATHAN_BUILD)

# create library with all the files
if(CREATE_STATIC)
  add_library(Engine ${EngineFiles})
else()
  add_library(Engine SHARED ${EngineFiles})
endif()

# Link the engine against dependencies
target_link_libraries(Engine ${LEVIATHAN_ENGINE_LIBRARIES})

if(NOT WIN32)
  # Link with X11 and threads
  target_link_libraries(Engine ${X11_LIBRARIES} ${X11_Xmu_LIB}
    ${CMAKE_THREAD_LIBS_INIT})
  
endif(NOT WIN32)

if(WIN32 AND LEVIATHAN_FULL_BUILD)

  # Angelscript needs /LTCG flag
  set_target_properties(Engine PROPERTIES LINK_FLAGS "/LTCG")
  
endif()

# Avoid Xlib headers and windows header in the precompiled header to avoid macros destroying everything
set_target_properties(Engine PROPERTIES 
  COTIRE_PREFIX_HEADER_IGNORE_PATH
  "${X11_INCLUDE_DIR}/X11;${CMAKE_SOURCE_DIR};${CMAKE_BINARY_DIR}")


# Allow adding in folder contained libraries to be cotired
# And add the Include.h as a precompiled header file
# TODO: update this if using cotire
set_target_properties(Engine PROPERTIES 
  COTIRE_PREFIX_HEADER_INCLUDE_PATH  "${PROJECT_SOURCE_DIR}/AngelScript;${PROJECT_SOURCE_DIR}/Newton;${PROJECT_SOURCE_DIR}/Leap;${CMAKE_SOURCE_DIR}/Engine/Include.h")

# speed up build
#cotire(Engine)


install(TARGETS Engine DESTINATION bin)

if(CREATE_UE4_PLUGIN)
  RemoveSourceFilesFromList(GroupCore)
  list(REMOVE_ITEM GroupCore "Exceptions.h")
  list(REMOVE_ITEM GroupCore "Logger.h")
  
  install(FILES ${GroupCore} DESTINATION "include/leviathan")
  
  RemoveSourceFilesFromList(GroupCommonData)
  install(FILES ${GroupCommonData} DESTINATION "include/leviathan/Common/DataStoring")
  
  RemoveSourceFilesFromList(GroupObjectFiles)
  install(FILES ${GroupObjectFiles} DESTINATION "include/leviathan/ObjectFiles")

  RemoveSourceFilesFromList(GroupThreading)
  install(FILES ${GroupThreading} DESTINATION "include/leviathan/Threading")
  
  RemoveSourceFilesFromList(GroupIterators)
  install(FILES ${GroupIterators} DESTINATION "include/leviathan/Iterators")
  
  RemoveSourceFilesFromList(GroupLZ4)
  install(FILES ${GroupLZ4} DESTINATION "include/leviathan/lz4")
  
  RemoveSourceFilesFromList(Grouputf8)
  install(FILES ${Grouputf8} DESTINATION "include/leviathan/utf8")

  RemoveSourceFilesFromList(GroupCommon)
  install(FILES ${GroupCommon} DESTINATION "include/leviathan/Common")

  RemoveSourceFilesFromList(GroupUtility)
  install(FILES ${GroupUtility} DESTINATION "include/leviathan/Utility")
  
  RemoveSourceFilesFromList(GroupMath)
  install(FILES ${GroupMath} DESTINATION "include/leviathan/Math")
  
  install(FILES ${GroupEntities} DESTINATION "include/leviathan/Entities")
  
  
endif()

# Strip symbols
if(STRIP_SYMBOLS_ON_INSTALL AND UNIX AND NOT CREATE_STATIC)
  
  # Install code that strips the symbols
  install(CODE "execute_process(COMMAND strip libEngine.so WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}/bin\")")

endif()

# post build copy
if(WIN32 AND NOT MINGW)
#ADD_CUSTOM_COMMAND(TARGET Engine POST_BUILD COMMAND copy ARGS 
#    "\"$(SolutionDir)bin\\$(Configuration)\\$(TargetName).dll\" \"$(SolutionDir)bin\\$(TargetName).dll\""
#)
endif(WIN32 AND NOT MINGW)

# Symbol building
if(UNIX)
  add_custom_target(Engine_Symbols ${SYMBOL_EXTRACTOR} "${CMAKE_BINARY_DIR}/bin/libEngine.so"
    DEPENDS Engine WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/Symbols VERBATIM)
elseif(WIN32)
  add_custom_target(Engine_Symbols ${SYMBOL_EXTRACTOR} "${CMAKE_BINARY_DIR}/bin/RelWithDebInfo/Engine.dll"
    DEPENDS Engine WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/Symbols VERBATIM)
endif()

