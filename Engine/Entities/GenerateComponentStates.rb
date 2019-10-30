#!/usr/bin/env ruby
# Generates inbuilt Component state classes
require_relative '../../RubySetupSystem/RubyCommon.rb'
require_relative '../../Helpers/FileGen.rb'

abort 'no target files provided' if ARGV.count < 1

generator = Generator.new ARGV[0], separateFiles: true

generator.useNamespace
generator.addInclude 'Entities/ComponentState.h'
generator.addImplInclude 'Entities/Components.h'

posState = ComponentState.new(
  'PositionState',
  members: [
    Variable.new('_Position', 'Float3'),
    Variable.new('_Orientation', 'Float4')
  ],
  copyconstructors: true,
  copyoperators: true,
  constructors: [
    # Empty unitialized constructor
    ConstructorInfo.new(
      []
    ),
    ConstructorInfo.new(
      [
        Variable.new('time', 'float', noRef: true),
        Variable.new('data', 'Leviathan::Position')
      ],
      baseparameters: 'time, COMPONENT_TYPE::Position',
      memberinitializers: [
        ['_Position', 'data.Members._Position'],
        ['_Orientation', 'data.Members._Orientation']
      ]
    ),
    ConstructorInfo.new(
      [
        Variable.new('time', 'float', noRef: true),
        Variable.new('_Position', 'Float3'),
        Variable.new('_Orientation', 'Float4')
      ],
      baseparameters: 'time, COMPONENT_TYPE::Position',
      memberinitializers: [
        %w[_Position _position],
        %w[_Orientation _orientation]
      ]
    )
  ], methods: [
    GeneratedMethod.new(
      'DoesMatchState', 'bool',
      [
        Variable.new('state', 'Leviathan::Position')
      ], body: 'return ' +
         genComparisonExpression(
           [
             ['_Position', 'state.Members._Position'],
             ['_Orientation', 'state.Members._Orientation']
           ]
         ) + ';'
    ),
    GeneratedMethod.new(
      'Interpolate', 'PositionState',
      [
        Variable.new('second', 'PositionState'),
        Variable.new('progress', 'float', noRef: true)
      ], body: <<-END
    return PositionState(StateTime, _Position.Lerp(second._Position, progress),
           _Orientation.Slerp(second._Orientation, progress));
      END

    )
  ]
)

generator.add posState

# Output the file
generator.run

# bindGenerator = Generator.new ARGV[1], bareOutput: true

# bindGenerator.add OutputText.new(worldClass.genAngelScriptBindings)

# bindGenerator.run
