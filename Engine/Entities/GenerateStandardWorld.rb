#!/usr/bin/env ruby
# Generates basic GameWorld with all standard components

require_relative '../../RubySetupSystem/RubyCommon.rb'
require_relative '../../Helpers/FileGen.rb'

abort "no target file provided" if ARGV.count < 1

generator = Generator.new ARGV[0], true

generator.useNamespace
generator.addInclude "Entities/GameWorld.h"

generator.add GameWorldClass.new(
                "StandardWorld", componentTypes: [
                  EntityComponent.new("Position",
                                      [ConstructorInfo.new(
                                         [
                                           Variable.new("position", "Float3"),
                                           Variable.new("orientation", "Float4")
                                         ], usedatastruct: true)
                                      ]),
                  EntityComponent.new("RenderNode", [ConstructorInfo.new([])]),
                  EntityComponent.new("Sendable", [ConstructorInfo.new([])]),
                  EntityComponent.new("Received", [ConstructorInfo.new([])]),
                  EntityComponent.new("Model", [ConstructorInfo.new(
                                                  [
                                                    Variable.new("model", "std::string")]
                                                )]),
                  EntityComponent.new("Physics", [
                                        ConstructorInfo.new(
                                          [
                                            Variable.new("id", "ObjectID",
                                                         nonMethodParam: true),
                                            Variable.new("world", "GameWorld*", noRef: true),
                                            Variable.new("updatepos", "Position",
                                                         noConst: true),
                                            Variable.new("updatesendable", "Sendable*",
                                                         noRef: true)
                                          ], usedatastruct: true)]),
                  EntityComponent.new("BoxGeometry",
                                      [ConstructorInfo.new(
                                         [
                                           Variable.new("size", "Float3"),
                                           Variable.new("material", "std::string")
                                         ], usedatastruct: false)]),
                  EntityComponent.new("ManualObject", [ConstructorInfo.new([])]),
                  EntityComponent.new("Camera",
                                      [ConstructorInfo.new(
                                         [
                                           Variable.new("fov", "uint8_t", default: "90"),
                                           Variable.new("soundperceiver", "bool", default:
                                                                                    "true")
                                         ])
                                      ]),                  
                ],
                systems: [
                  EntitySystem.new("ReceivedSystem", []),
                  EntitySystem.new("RenderingPositionSystem", ["RenderNode", "Position"]),
                  EntitySystem.new("SendableSystem", []),
                  EntitySystem.new("RenderNodeHiderSystem", []),
                ],
                tickrunmethod: <<-END
    // Client interpolation //
    if(!IsOnServer){

        //const float interpolatepercentage = std::max(0.f, timeintick / (float)TICKSPEED);

        _ReceivedSystem.Run(ComponentReceived.GetIndex(), *this);
    }

    // Skip in non-gui mode //
    if(!GraphicalMode)
        return;

    _RenderNodeHiderSystem.Run(ComponentRenderNode.GetIndex(), *this);

    _RenderingPositionSystem.Run(*this);
END
              )




# Output the file
generator.run


