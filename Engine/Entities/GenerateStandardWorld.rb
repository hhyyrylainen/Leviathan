#!/usr/bin/env ruby
# Generates basic GameWorld with all standard components

require_relative '../../RubySetupSystem/RubyCommon.rb'
require_relative '../../Helpers/FileGen.rb'

abort "no target files provided" if ARGV.count < 2

generator = Generator.new ARGV[0], separateFiles: true

generator.useNamespace
generator.addInclude "Entities/GameWorld.h"
generator.addInclude "Entities/Components.h"
generator.addInclude "Entities/Systems.h"
# Needs script include for stuff
generator.addInclude "Script/ScriptTypeResolver.h"

worldClass = GameWorldClass.new(
  "StandardWorld", componentTypes: [
    EntityComponent.new("Position",
                        [ConstructorInfo.new(
                           [
                             Variable.new("position", "Float3"),
                             Variable.new("orientation", "Float4")
                           ], usedatastruct: true)
                        ], statetype: true),
    EntityComponent.new("RenderNode", [ConstructorInfo.new(
                                         [
                                           Variable.new("GetScene()", "",
                                                        nonMethodParam: true),
                                         ])], releaseparams: ["GetScene()"]),
    EntityComponent.new("Sendable", [ConstructorInfo.new([])]),
    EntityComponent.new("Received", [ConstructorInfo.new([])]),
    EntityComponent.new("Model", [ConstructorInfo.new(
                                    [
                                      Variable.new("GetScene()", "",
                                                   nonMethodParam: true),
                                      Variable.new("parent", "Ogre::SceneNode*",
                                                   noRef: true
                                                  ),
                                      Variable.new("model", "std::string")
                                    ]
                                  )], releaseparams: ["GetScene()"]),
    EntityComponent.new("Physics", [
                          ConstructorInfo.new(
                            [
                              Variable.new("id", "ObjectID",
                                           nonMethodParam: true),
                              Variable.new("world", "GameWorld*", noRef: true),
                              Variable.new("updatepos", "Position",
                                           noConst: true,
                                           angelScriptUseInstead:
                                             Variable.new("updatepos", "Position*",
                                                          noRef: true)),
                              Variable.new("updatesendable", "Sendable*",
                                           noRef: true)
                            ], usedatastruct: true)], releaseparams: []),
    EntityComponent.new("BoxGeometry",
                        [ConstructorInfo.new(
                           [
                             Variable.new("size", "Float3"),
                             Variable.new("material", "std::string")
                           ], usedatastruct: false)]),
    EntityComponent.new("ManualObject", [ConstructorInfo.new(
                                           [
                                             Variable.new("GetScene()", "",
                                                          nonMethodParam: true),
                                           ])],
                        releaseparams: ["GetScene()"]),
    EntityComponent.new("Camera",
                        [ConstructorInfo.new(
                           [
                             Variable.new("fov", "uint8_t", default: "90"),
                             Variable.new("soundperceiver", "bool", default:
                                                                      "true")
                           ])
                        ]),
    EntityComponent.new("Plane",
                        [ConstructorInfo.new(
                           [
                             Variable.new("GetScene()", "",
                                          nonMethodParam: true),
                             Variable.new("parent", "Ogre::SceneNode*",
                                          noRef: true),
                             Variable.new("material", "std::string"),
                             Variable.new("plane", "Ogre::Plane"),
                             Variable.new("size", "Float2")
                           ])
                        ]),
    EntityComponent.new("Animated",
                        [ConstructorInfo.new(
                           [
                             Variable.new("item", "Ogre::Item*",
                                          noRef: true)
                           ])
                        ]),    
  ],
  systems: [
    EntitySystem.new("RenderingPositionSystem", ["RenderNode", "Position"],
                     runrender: {group: 10, parameters: [
                                   "PositionStates", "calculatedTick", "progressInTick"
                                 ]}),
    EntitySystem.new("RenderNodePropertiesSystem", [],
                     runrender: {group: 11, parameters: ["ComponentRenderNode.GetIndex()"]}),
    EntitySystem.new("AnimationTimeAdder", [],
                     runrender: {group: 60, parameters: ["ComponentAnimated.GetIndex()",
                                                         "calculatedTick", "progressInTick"]}),
    EntitySystem.new("ReceivedSystem", []), 
    EntitySystem.new("SendableSystem", []), 
    EntitySystem.new("PositionStateSystem", [], runtick: {
                       group: 50,
                       parameters: ["ComponentPosition.GetIndex()", "PositionStates",
                                    "tick"]}),
  ],
  systemspreticksetup: (<<-END
  const auto timeAndTickTuple = GetTickAndTime();
  const auto calculatedTick = std::get<0>(timeAndTickTuple);
  const auto progressInTick = std::get<1>(timeAndTickTuple);
  const auto tick = GetTickNumber();
END
                       ),
  framesystemrun: (<<-END
    // Client interpolation //
    if(!IsOnServer){

        //const float interpolatepercentage = std::max(0.f, timeintick / (float)TICKSPEED);

        _ReceivedSystem.Run(ComponentReceived.GetIndex(), *this);
    }

    // Skip in non-gui mode //
    if(!GraphicalMode)
        return;

    const auto timeAndTickTuple = GetTickAndTime();
    const auto calculatedTick = std::get<0>(timeAndTickTuple);
    const auto progressInTick = std::get<1>(timeAndTickTuple);
END
                 )
)


generator.add worldClass



# Output the file
generator.run

bindGenerator = Generator.new ARGV[1], bareOutput: true


bindGenerator.add OutputText.new(worldClass.genAngelScriptBindings)


bindGenerator.run





