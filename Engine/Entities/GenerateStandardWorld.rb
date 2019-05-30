#!/usr/bin/env ruby
# Generates basic GameWorld with all standard components

require_relative '../../RubySetupSystem/RubyCommon.rb'
require_relative '../../Helpers/FileGen.rb'
require_relative "StandardWorld"

abort "no target files provided" if ARGV.count < 2

generator = Generator.new ARGV[0], separateFiles: true

generator.useNamespace
generator.addInclude "Entities/GameWorld.h"
generator.addInclude "Entities/Components.h"
generator.addInclude "Entities/Systems.h"
# Needed for standard type id
generator.addInclude "Entities/GameWorldFactory.h"

# Needs script include
generator.addInclude "Script/ScriptTypeResolver.h"


generator.add STANDARD_WORLD


# Output the file
generator.run

bindGenerator = Generator.new ARGV[1], bareOutput: true


bindGenerator.add OutputText.new(STANDARD_WORLD.genAngelScriptBindings)


bindGenerator.run





