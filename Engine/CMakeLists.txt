# engine library Cmake file

# Generate additional source files
# This is here as an example to make this work later
#add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/Engine/Generated/Generated.h
#   COMMAND FileGenerator "V8EXT" "\"${PROJECT_SOURCE_DIR}/Engine/GeneratorInput/Generated.in.h\""
#   "\"${PROJECT_SOURCE_DIR}/Engine/Generated/Generated.h\""
#   DEPENDS FileGenerator "${PROJECT_SOURCE_DIR}/Engine/GeneratorInput/Generated.in.h"
#)

# Generate additional source files
add_custom_command(OUTPUT "${PROJECT_SOURCE_DIR}/Engine/Generated/LeviathanV8CoreExt.h"
  COMMAND FileGenerator "V8EXT" "\"${PROJECT_SOURCE_DIR}/Engine/GeneratorInput/LeviathanCoreV8Extensions.js\"" 
  "\"${PROJECT_SOURCE_DIR}/Engine/Generated/LeviathanV8CoreExt.h\""
  DEPENDS FileGenerator "${PROJECT_SOURCE_DIR}/Engine/GeneratorInput/LeviathanCoreV8Extensions.js"
  )


set(GroupCore "Define.cpp" "Define.h" "FileSystem.cpp" "FileSystem.h"
  "ForwardDeclarations.h"
  "Logger.cpp" "Logger.h" "ErrorReporter.h"
  "utf8.h" "Include.h"
  "Exceptions.h" "Exceptions.cpp"
  "TimeIncludes.h" "TimeIncludes.cpp"
  "WindowsInclude.h"
  )

if(NOT LEVIATHAN_AS_LIBRARY)
  list(APPEND GroupCore "Engine.cpp" "Engine.h")
endif()

if(LEVIATHAN_USING_OGRE)
  list(APPEND GroupCore "TextureGenerator.cpp" "TextureGenerator.h" "Window.h" "Window.cpp")
endif()

if(LEVIATHAN_USING_CEF)
  list(APPEND GroupCore "GlobalCEFHandler.cpp" "GlobalCEFHandler.h")
endif()

set(GroupCommon "Common/BaseNotifiable.h" "Common/BaseNotifiableImpl.h"
  "Common/BaseNotifier.h" "Common/BaseNotifierImpl.h"
  "Common/ObjectPool.h" "Common/ObjectPoolThreadSafe.h"
  "Common/ReferenceCounted.h"
  "Common/StringOperations.cpp" "Common/StringOperations.h"
  "Common/ThreadSafe.h" 
  "Common/Types.h" "Common/Types.cpp"
  "Common/Visitor.cpp" "Common/Visitor.h"
  "Common/MimeTypes.cpp" "Common/MimeTypes.h"
  "Common/CommonMath.cpp" "Common/CommonMath.h"
  )

if(LEVIATHAN_USING_SFML)
  list(APPEND GroupCommon "Common/SFMLPackets.cpp" "Common/SFMLPackets.h")
endif()

set(GroupUtility "Utility/ComplainOnce.cpp" "Utility/ComplainOnce.h"
  "Utility/Convert.cpp" "Utility/Convert.h"
  "Utility/DebugVariableNotifier.cpp" "Utility/DebugVariableNotifier.h"
  "Utility/MD5Generator.cpp" "Utility/MD5Generator.h"
  "Utility/Random.cpp" "Utility/Random.h"
  "Utility/well.h" "Utility/InstanceCounter.h"
  "Utility/DebugObjectTracker.h"
  )

set(GroupCommonData "Common/DataStoring/DataBlock.cpp" "Common/DataStoring/DataBlock.h"
  "Common/DataStoring/NamedVars.cpp" "Common/DataStoring/NamedVars.h"
  "Common/DataStoring/DataStore.cpp" "Common/DataStoring/DataStore.h"
  )

if(LEVIATHAN_USING_OGRE)
  file(GLOB STANDARD_GENERATOR_FILES "${PROJECT_SOURCE_DIR}/Engine/Entities/*.rb")
  
  AddRubyGeneratedFile(OUTPUT "${PROJECT_SOURCE_DIR}/Engine/Generated/StandardWorld.h"
    "${PROJECT_SOURCE_DIR}/Engine/Generated/StandardWorld.cpp"
    "${PROJECT_SOURCE_DIR}/Engine/Generated/StandardWorldBindings.h"
    PARAMS "${PROJECT_SOURCE_DIR}/Engine/Entities/GenerateStandardWorld.rb"
    "${PROJECT_SOURCE_DIR}/Engine/Generated/StandardWorld"
    "${PROJECT_SOURCE_DIR}/Engine/Generated/StandardWorldBindings.h"
    DEPENDS ${STANDARD_GENERATOR_FILES}
    )
  
  AddRubyGeneratedFile(OUTPUT "${PROJECT_SOURCE_DIR}/Engine/Generated/ComponentStates.h"
    "${PROJECT_SOURCE_DIR}/Engine/Generated/ComponentStates.cpp"
    #"${PROJECT_SOURCE_DIR}/Engine/Generated/ComponentStatesBindings.h"
    PARAMS "${PROJECT_SOURCE_DIR}/Engine/Entities/GenerateComponentStates.rb"
    "${PROJECT_SOURCE_DIR}/Engine/Generated/ComponentStates"
    #"${PROJECT_SOURCE_DIR}/Engine/Generated/StandardWorldBindings.h"
    )  
  
  set(GroupEntities "Entities/Components.cpp" "Entities/Components.h"
    "Entities/Component.h"
    "Entities/ComponentState.cpp" "Entities/ComponentState.h"
    "Entities/StateHolder.h" "Entities/StateHolder.cpp" 
    "Entities/StateInterpolator.h"
    "Entities/EntityCommon.h"
    "Entities/WorldNetworkSettings.h"
    "Entities/PerWorldData.h" "Entities/PerWorldData.cpp"
    "Entities/GameWorld.cpp" "Entities/GameWorld.h"
    "Entities/ScriptComponentHolder.cpp" "Entities/ScriptComponentHolder.h"
    "Entities/ScriptSystemWrapper.cpp" "Entities/ScriptSystemWrapper.h"
    "Entities/System.h" "Entities/Systems.cpp" "Entities/Systems.h"
    "Entities/GameWorldFactory.h" "Entities/GameWorldFactory.cpp"
    "Generated/StandardWorld.h" "Generated/StandardWorld.cpp"
    "Generated/ComponentStates.h" "Generated/ComponentStates.cpp"
    )

endif()

if(NOT LEVIATHAN_AS_LIBRARY)
  file(GLOB GroupApplication "Application/AppDefine.cpp" "Application/AppDefine.h"
    "Application/Application.cpp" "Application/Application.h"
    "Application/GameConfiguration.cpp" "Application/GameConfiguration.h"
    "Application/KeyConfiguration.cpp" "Application/KeyConfiguration.h"
    "Application/MasterServerApplication.cpp" "Application/MasterServerApplication.h"
    "Application/ServerApplication.cpp" "Application/ServerApplication.h"
    "Application/ClientApplication.cpp" "Application/ClientApplication.h"
    "Application/ConsoleInput.cpp" "Application/ConsoleInput.h"
    "Application/CrashHandler.cpp" "Application/CrashHandler.h"
    )
endif()

if(LEVIATHAN_USING_SFML)
  AddRubyGeneratedFile(OUTPUT "${PROJECT_SOURCE_DIR}/Engine/Generated/RequestImpl.h"
    PARAMS "${PROJECT_SOURCE_DIR}/Engine/Networking/GenerateRequest.rb"
    "${PROJECT_SOURCE_DIR}/Engine/Generated/RequestImpl.h"
    )

  AddRubyGeneratedFile(OUTPUT "${PROJECT_SOURCE_DIR}/Engine/Generated/ResponseImpl.h"
    PARAMS "${PROJECT_SOURCE_DIR}/Engine/Networking/GenerateResponse.rb"
    "${PROJECT_SOURCE_DIR}/Engine/Generated/ResponseImpl.h"
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
endif()

if(LEVIATHAN_USING_OGRE)
  set(GroupRendering
    "Rendering/GeometryHelpers.h" "Rendering/GeometryHelpers.cpp"
    "Rendering/Graphics.cpp" "Rendering/Graphics.h"
    "Rendering/GUIOverlayRenderer.cpp" "Rendering/GUIOverlayRenderer.h"
    "Rendering/RotatingBufferHelper.h"
    )
endif()

if(LEVIATHAN_USING_ANGELSCRIPT)
  set(GroupScript
    "Script/Console.cpp" "Script/Console.h"
    "Script/ScriptArgumentsProvider.cpp" "Script/ScriptArgumentsProvider.h"
    "Script/ScriptExecutor.cpp" "Script/ScriptExecutor.h"
    "Script/ScriptTypeResolver.cpp" "Script/ScriptTypeResolver.h"
    "Script/ScriptModule.cpp" "Script/ScriptModule.h"
    "Script/ScriptNotifiers.cpp" "Script/ScriptNotifiers.h"
    "Script/ScriptRunningSetup.cpp" "Script/ScriptRunningSetup.h"
    "Script/ScriptScript.cpp" "Script/ScriptScript.h"
    "Script/AccessMask.cpp" "Script/AccessMask.h"
    "Script/NonOwningScriptCallback.cpp" "Script/NonOwningScriptCallback.h"
    "Script/AddonTypes.h"
    "Script/ScriptCallingHelpers.h"
    "Script/CustomScriptRunHelpers.h"
    "Script/ScriptConversionHelpers.h"
    )

  set(GroupScriptInterface
    "Script/Interface/ScriptEventListener.h"
    "Script/Interface/ScriptLock.h"
    "Script/Interface/ScriptDelegateSlot.h"
    )

  set(GroupScriptBindings  
    "Script/Bindings/BindHelpers.h"
    "Script/Bindings/BindStandardFunctions.h" "Script/Bindings/BindStandardFunctions.cpp"
    "Script/Bindings/CommonEngineBind.h" "Script/Bindings/CommonEngineBind.cpp"
    "Script/Bindings/BSFBind.h" "Script/Bindings/BSFBind.cpp"
    "Script/Bindings/PhysicsBind.h" "Script/Bindings/PhysicsBind.cpp"
    "Script/Bindings/GuiScriptBind.h" "Script/Bindings/GuiScriptBind.cpp"
    "Script/Bindings/TypesBind.h" "Script/Bindings/TypesBind.cpp"
    "Script/Bindings/EntityBind.h" "Script/Bindings/EntityBind.cpp"
    "Script/Bindings/StandardWorldBindHelper.h" "Generated/StandardWorldBindings.h"
    )
endif()


if(LEVIATHAN_USING_GUI)
  set(GroupGUI 
    "GUI/GuiManager.cpp" "GUI/GuiManager.h"
    "GUI/VideoPlayer.cpp" "GUI/VideoPlayer.h"
    "GUI/GuiCEFApplication.cpp" "GUI/GuiCEFApplication.h"
    "GUI/CEFLocalResourceRequest.cpp" "GUI/CEFLocalResourceRequest.h"
    "GUI/CEFConversionHelpers.cpp" "GUI/CEFConversionHelpers.h"
    "GUI/GuiLayer.cpp" "GUI/GuiLayer.h"
    "GUI/GuiWidgetLayer.cpp" "GUI/GuiWidgetLayer.h"
    "GUI/GuiView.cpp" "GUI/GuiView.h"
    "GUI/JavaScriptHelper.cpp" "GUI/JavaScriptHelper.h"
    "GUI/JSNativeCoreAPI.cpp" "GUI/JSNativeCoreAPI.h"
    "GUI/LeviathanJavaScriptAsync.cpp" "GUI/LeviathanJavaScriptAsync.h"
    "GUI/KeyMapping.cpp" "GUI/KeyMapping.h"
    "GUI/GuiInputSettings.h"
    "GUI/JSProxyable.h"
    )

  set(GroupGUIWidgets
    "GUI/Widgets/Widget.h" "GUI/Widgets/Widget.cpp"
    "GUI/Widgets/VideoPlayerWidget.h" "GUI/Widgets/VideoPlayerWidget.cpp"
    )

endif()

if(LEVIATHAN_USING_BULLET)
  file(GLOB GroupPhysics 
    "Physics/PhysicalMaterial.cpp" "Physics/PhysicalMaterial.h"
    "Physics/PhysicsMaterialManager.cpp" "Physics/PhysicsMaterialManager.h"    
    "Physics/PhysicalWorld.cpp" "Physics/PhysicalWorld.h"
    "Physics/PhysicsShape.cpp" "Physics/PhysicsShape.h"
    "Physics/PhysicsBody.cpp" "Physics/PhysicsBody.h"
    )
endif()


set(GroupEvents
  "Events/AutoUpdateable.cpp" "Events/AutoUpdateable.h"
  "Events/DelegateSlot.cpp" "Events/DelegateSlot.h"
  "Events/Event.cpp" "Events/Event.h"
  "Events/EventHandler.cpp" "Events/EventHandler.h"
  )

if(NOT LEVIATHAN_AS_LIBRARY)
  list(APPEND GroupEvents "Events/CallableObject.cpp" "Events/CallableObject.h")
endif()

if(LEVIATHAN_USING_ANGELSCRIPT)
  list(APPEND GroupEvents "Events/EventableScriptObject.cpp" "Events/EventableScriptObject.h")
endif()

file(GLOB GroupExceptions "Exceptions/*.cpp" "Exceptions/*.h")

if(LEVIATHAN_USING_DEPENDENCIES)
  set(GroupSound
    "Sound/SoundDevice.cpp" "Sound/SoundDevice.h"
    "Sound/AudioSource.h" "Sound/AudioSource.cpp"
    "Sound/AudioBuffer.h" "Sound/AudioBuffer.cpp"
    "Sound/ProceduralSound.h" "Sound/ProceduralSound.cpp"
    )
endif()

set(GroupInput
  "Input/InputController.cpp"
  "Input/InputController.h"
  "Input/Key.cpp"
  "Input/Key.h"
  )

if(LEVIATHAN_USING_DEPENDENCIES)  
  file(GLOB GroupHandlers "Handlers/*.cpp" "Handlers/*.h")

  set(GroupAddons
    "Addons/GameModule.cpp" "Addons/GameModule.h"
    "Addons/GameModuleLoader.cpp" "Addons/GameModuleLoader.h"
    )

  file(GLOB GroupGameplay "Gameplay/*.cpp" "Gameplay/*.h")
  file(GLOB GroupPermissions "Gameplay/Permissions/*.cpp" "Gameplay/Permissions/*.h")
endif()

file(GLOB GroupStatistics "Statistics/*.cpp" "Statistics/*.h")

file(GLOB GroupUtilityData "Utility/DataHandling/*.cpp" "Utility/DataHandling/*.h")

if(LEVIATHAN_USING_DEPENDENCIES)
  set(GroupEditor
    "Editor/Editor.h" "Editor/Editor.cpp"
    "Editor/Importer.h" "Editor/Importer.cpp"
    )
endif()

# Generated files
if(LEVIATHAN_USING_CEF)
  set(GroupGeneratedFiles "Generated/LeviathanV8CoreExt.h")
endif()

if(LEVIATHAN_USING_LEAP)
  file(GLOB GroupLeap "Leap/*.cpp" "Leap/*.h")
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


set(GroupJsoncpp "jsoncpp.cpp" "json/json.h" "json/json-forwards.h")
# TODO: does this work as static library on windows?
# For building jsoncpp must have "JSON_DLL_BUILD" defined
set_source_files_properties("jsoncpp.cpp" PROPERTIES
  COMPILE_DEFINITIONS JSON_DLL_BUILD)

file(GLOB Grouputf8 "utf8/*.h" "utf8/*.cpp")

# AngelScript addon sources
if(LEVIATHAN_USING_ANGELSCRIPT)
  file(GLOB_RECURSE GroupScriptAddons "${LEVIATHAN_SRC}/build/ThirdParty/include/add_on/*.cpp"
    "${LEVIATHAN_SRC}/build/ThirdParty/include/add_on/*.h")

  if(NOT GroupScriptAddons)

    message(SEND_ERROR "AngelScript add_on files haven't been installed to the "
      "ThirdParty/include directory")
  endif()

  add_library(AngelScriptAddons ${GroupScriptAddons})
  set_target_properties(AngelScriptAddons PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
  
endif()

# set groups
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

source_group("Utility" FILES ${GroupUtility})
source_group("Utility\\Statistics" FILES ${GroupStatistics})
source_group("Iterators" FILES ${GroupIterators})
source_group("Utility\\DataHandling" FILES ${GroupUtilityData})

source_group("GUI" FILES ${GroupGUI})
source_group("GUI\\Widgets" FILES ${GroupGUIWidgets})

source_group("Core" FILES ${GroupCore})
source_group("Core\\Events" FILES ${GroupEvents})
source_group("Core\\Input" FILES ${GroupInput})
source_group("Core\\Handlers" FILES ${GroupHandlers})
source_group("Physics" FILES ${GroupPhysics})

source_group("Common" FILES ${GroupCommon})
source_group("Common\\Application" FILES ${GroupApplication})
source_group("Common\\Exceptions" FILES ${GroupExceptions})
source_group("Common\\DataStoring" FILES ${GroupCommonData})
source_group("Gameplay" FILES ${GroupGameplay})
source_group("Gameplay\\Permissions" FILES ${GroupPermissions})

source_group("ObjectFiles" FILES ${GroupObjectFiles})

source_group("Rendering" FILES ${GroupRendering})
source_group("Rendering\\GUI" FILES ${GroupRenderingGUI})

source_group("Editor" FILES ${GroupEditor})

if(USE_LEAP)
  source_group("Leap" FILES ${GroupLeap})
endif()

# collect all files to one group
set(EngineFiles ${GroupCore} ${GroupEntities} ${GroupUtility} ${GroupStatistics}
  ${GroupSound}
  ${GroupScript} ${GroupScriptInterface} ${GroupScriptBindings}
  ${GroupGUI} ${GroupGUIWidgets} ${GroupEvents}
  ${GroupInput} ${GroupHandlers} ${GroupCommon} ${GroupApplication} ${GroupExceptions}
  ${GroupObjectFiles} ${GroupRendering} ${GroupCommonData} ${GroupUtilityIte}
  ${GroupUtilityFile}
  ${GroupRenderingGUI} ${GroupPhysics}
  ${GroupUtilityData} ${GroupAddons} ${GroupNetworking} ${GroupThreading} ${GroupIterators}
  ${GroupGameplay} 
  ${GroupPermissions} ${GroupLZ4} ${GroupJsoncpp}
  ${GeneratorInput} ${GroupGeneratedFiles}
  ${Grouputf8} ${GroupEditor} ${GroupGeneratedFiles}
  )

if(LEVIATHAN_USING_LEAP)
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

# Anything linking against this gets the right include folder
target_include_directories(Engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# Link the engine against dependencies
target_link_libraries(Engine PRIVATE ${LEVIATHAN_ENGINE_LIBRARIES})

if(NOT WIN32)
  # Link with X11 and threads
  target_link_libraries(Engine PRIVATE ${X11_LIBRARIES} ${X11_Xmu_LIB} ${X11_Xfixes_LIB}
    ${CMAKE_THREAD_LIBS_INIT})

  # GCC 8 fix (can be removed once GCC 9 is prevalent)
  target_link_libraries(Engine PRIVATE $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>)
  
endif(NOT WIN32)

if(WIN32 AND LEVIATHAN_USING_ANGELSCRIPT)
  
  # Angelscript needs /LTCG flag
  # It now actually doesn't with the new build setup stuff
  # set_target_properties(Engine PROPERTIES LINK_FLAGS "/LTCG")
  
endif()

set_property(TARGET Engine PROPERTY CXX_STANDARD 17)
set_property(TARGET Engine PROPERTY CXX_EXTENSIONS OFF)

# Avoid Xlib headers and windows header in the precompiled header to
# avoid macros destroying everything
set_target_properties(Engine PROPERTIES 
  COTIRE_PREFIX_HEADER_IGNORE_PATH
  "${X11_INCLUDE_DIR}/X11;${CMAKE_SOURCE_DIR};${CMAKE_BINARY_DIR}")


# Allow adding in folder contained libraries to be cotired
# And add the Include.h as a precompiled header file
# TODO: update this if using cotire
set_target_properties(Engine PROPERTIES 
  COTIRE_PREFIX_HEADER_INCLUDE_PATH
  "${PROJECT_SOURCE_DIR}/build;${CMAKE_SOURCE_DIR}/Engine/Include.h")

# speed up build
#cotire(Engine)


install(TARGETS Engine DESTINATION bin)

# # TODO: header installs
# if()
#   RemoveSourceFilesFromList(GroupCore)
  
#   install(FILES ${GroupCore} DESTINATION "include/Leviathan")
  
#   RemoveSourceFilesFromList(GroupCommonData)
#   install(FILES ${GroupCommonData} DESTINATION "include/Leviathan/Common/DataStoring")
  
#   RemoveSourceFilesFromList(GroupObjectFiles)
#   install(FILES ${GroupObjectFiles} DESTINATION "include/Leviathan/ObjectFiles")

#   RemoveSourceFilesFromList(GroupThreading)
#   install(FILES ${GroupThreading} DESTINATION "include/Leviathan/Threading")
  
#   RemoveSourceFilesFromList(GroupIterators)
#   install(FILES ${GroupIterators} DESTINATION "include/Leviathan/Iterators")
  
#   RemoveSourceFilesFromList(GroupLZ4)
#   install(FILES ${GroupLZ4} DESTINATION "include/Leviathan/lz4")
  
#   RemoveSourceFilesFromList(Grouputf8)
#   install(FILES ${Grouputf8} DESTINATION "include/Leviathan/utf8")

#   RemoveSourceFilesFromList(GroupCommon)
#   install(FILES ${GroupCommon} DESTINATION "include/Leviathan/Common")

#   RemoveSourceFilesFromList(GroupUtility)
#   install(FILES ${GroupUtility} DESTINATION "include/Leviathan/Utility")
  
#   install(FILES ${GroupEntities} DESTINATION "include/Leviathan/Entities")
  
  
# endif()

