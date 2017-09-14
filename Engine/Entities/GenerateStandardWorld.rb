#!/usr/bin/env ruby
# Generates basic GameWorld with all standard components

require_relative '../../RubySetupSystem/RubyCommon.rb'
require_relative '../../Helpers/FileGen.rb'

abort "no target file provided" if ARGV.count < 1

generator = Generator.new ARGV[0], true

generator.useNamespace
generator.addInclude "Entities/GameWorld.h"

generator.add GameWorldClass.new(
                "StandardWorld", componentTypes=[
                  EntityComponent.new("Position"),
                  EntityComponent.new("RenderNode"),
                  EntityComponent.new("Sendable"),
                  EntityComponent.new("Received"),
                  EntityComponent.new("Model"),
                  EntityComponent.new("Physics"),
                  EntityComponent.new("BoxGeometry"),
                  EntityComponent.new("ManualObject"),
                ],
                systems=[
                  EntitySystem.new("ReceivedSystem"),
                  EntitySystem.new("RenderingPositionSystem"),
                  EntitySystem.new("SendableSystem"),
                  EntitySystem.new("RenderNodeHiderSystem"),
                ],
              )




# Output the file
generator.run


