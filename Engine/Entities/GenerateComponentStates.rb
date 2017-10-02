#!/usr/bin/env ruby
# Generates inbuilt Component state classes
require_relative '../../RubySetupSystem/RubyCommon.rb'
require_relative '../../Helpers/FileGen.rb'

abort "no target files provided" if ARGV.count < 1

generator = Generator.new ARGV[0], separateFiles: true

generator.useNamespace
generator.addInclude "Entities/ComponentState.h"
generator.addImplInclude "Entities/Components.h"

generator.add ComponentState.new("PositionState", members: [
                                   Variable.new("_Position", "Float3"),
                                   Variable.new("_Orientation", "Float4")],
                                 constructors: [
                                   ConstructorInfo.new(
                                     [
                                       Variable.new("tick",
                                                    "int"),
                                       Variable.new("data",
                                                    "Leviathan::Position")
                                     ],
                                     memberinitializers: [
                                       ["TickNumber", "tick"],
                                       ["_Position", "data.Members._Position"],
                                       ["_Orientation", "data.Members._Orientation"],
                                     ])
                                 ], methods: [
                                   GeneratedMethod.new(
                                     "DoesMatchState", "bool",
                                     [
                                       Variable.new("state", "Leviathan::Position")
                                     ], body: "return " + 
                                        genComparisonExpression(
                                          [
                                            ["_Position", "state.Members._Position"],
                                            ["_Orientation", "state.Members._Orientation"],
                                          ]) + ";"),
                                 ])



# Output the file
generator.run

# bindGenerator = Generator.new ARGV[1], bareOutput: true


# bindGenerator.add OutputText.new(worldClass.genAngelScriptBindings)


# bindGenerator.run
