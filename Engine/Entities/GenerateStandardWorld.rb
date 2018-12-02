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
# Needed for standard type id
generator.addInclude "Entities/GameWorldFactory.h"

# Needs script include for stuff
generator.addInclude "Script/ScriptTypeResolver.h"

worldClass = GameWorldClass.new(
  "StandardWorld", componentTypes: [
    EntityComponent.new("Position",
                        [ConstructorInfo.new(
                           [
                             Variable.new("position", "Float3",
                                          memberaccess: "Members._Position"),
                             Variable.new("orientation", "Float4",
                                          memberaccess: "Members._Orientation")
                           ], usedatastruct: true)
                        ], statetype: true),
    EntityComponent.new("RenderNode", [ConstructorInfo.new(
                                         [
                                           Variable.new("GetScene()", "",
                                                        nonMethodParam: true),
                                         ])], releaseparams: ["GetScene()"]),
    EntityComponent.new("Sendable", [ConstructorInfo.new([])], nosynchronize: true),
    EntityComponent.new("Received", [ConstructorInfo.new([])], nosynchronize: true),
    EntityComponent.new("Model", [ConstructorInfo.new(
                                    [
                                      Variable.new("GetScene()", "",
                                                   nonMethodParam: true),
                                      Variable.new("parent", "Ogre::SceneNode*",
                                                   noRef: true, nonserializeparam: true
                                                  ),
                                      Variable.new("model", "std::string",
                                                   memberaccess: "MeshName")
                                    ]
                                  )], releaseparams: ["GetScene()"]),
    EntityComponent.new("Physics", [
                          ConstructorInfo.new(
                            [
                              Variable.new("id", "ObjectID",
                                           nonMethodParam: true, nonserializeparam: true),
                              Variable.new("this", "GameWorld*", noRef: true,
                                           nonMethodParam: true),
                              Variable.new("updatepos", "Position",
                                           noConst: true,
                                           nonserializeparam: true,
                                           angelScriptUseInstead:
                                             Variable.new("updatepos", "Position*",
                                                          noRef: true)),
                            ], usedatastruct: true)],
                        releaseparams: ["GetPhysicalWorld()"]),
    EntityComponent.new("BoxGeometry",
                        [ConstructorInfo.new(
                           [
                             Variable.new("size", "Float3", memberaccess: "Sizes"),
                             Variable.new("material", "std::string", memberaccess: "Material")
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
                             Variable.new("fov", "uint8_t", default: "90",
                                          memberaccess: "FOVY"),
                             Variable.new("soundperceiver", "bool",
                                          default: "true", memberaccess: "SoundPerceiver")
                           ])
                        ]),
    EntityComponent.new("Plane",
                        [ConstructorInfo.new(
                           [
                             Variable.new("GetScene()", "",
                                          nonMethodParam: true),
                             Variable.new("parent", "Ogre::SceneNode*",
                                          noRef: true, nonserializeparam: true),
                             Variable.new("material", "std::string", memberaccess: "Material"),
                             Variable.new("plane", "Ogre::Plane",
                                          memberaccess: "PlaneDefinition"),
                             Variable.new("size", "Float2", memberaccess: "Size"),
                             Variable.new("uvupvector", "Ogre::Vector3",
                                          memberaccess: "UpVector",
                                          default: "Ogre::Vector3::UNIT_Y"),
                           ])
                        ]),
    EntityComponent.new("Animated",
                        [ConstructorInfo.new(
                           [
                             Variable.new("item", "Ogre::Item*",
                                          noRef: true, nonserializeparam: true)
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
    EntitySystem.new("ReceivedSystem", [],
                     runtick: {group: 60,
                               parameters: ["ComponentReceived.GetIndex()"]}),

    # This needs to be ran before systems that unmark the Position
    EntitySystem.new("SendableMarkFromSystem<Position>", ["Sendable", "Position"],
                     runtick: {group: 20,
                               parameters: []}),
    EntitySystem.new("SendableSystem", [],
                     runtick: {group: 70,
                               parameters: ["ComponentSendable.GetIndex()"]}),
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
    if(GetNetworkSettings().DoInterpolation){

        //const float interpolatepercentage = std::max(0.f, timeintick / (float)TICKSPEED);

        // _ReceivedSystem.Run(*this, ComponentReceived.GetIndex());
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

worldClass.WorldType = "static_cast<int32_t>(Leviathan::INBUILT_WORLD_TYPE::Standard)"
generator.add worldClass



# Output the file
generator.run

bindGenerator = Generator.new ARGV[1], bareOutput: true


bindGenerator.add OutputText.new(worldClass.genAngelScriptBindings)


bindGenerator.run





