#!/usr/bin/env ruby
# Generates inbuilt Component state classes
require_relative '../../RubySetupSystem/RubyCommon.rb'
require_relative '../../Helpers/FileGen.rb'

abort "no target files provided" if ARGV.count < 1

generator = Generator.new ARGV[0], separateFiles: true

generator.useNamespace
generator.addInclude "Entities/ComponentState.h"

generator.add ComponentState.new("PositionState", members: [
                                   Variable.new("_Position", "Float3"),
                                   Variable.new("_Orientation", "Float4")])



# Output the file
generator.run

# bindGenerator = Generator.new ARGV[1], bareOutput: true


# bindGenerator.add OutputText.new(worldClass.genAngelScriptBindings)


# bindGenerator.run
