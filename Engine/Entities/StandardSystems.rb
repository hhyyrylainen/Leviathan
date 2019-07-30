# Standard system definitions for world generators
SYSTEM_RENDERINGPOSITION = EntitySystem.new(
  "RenderingPositionSystem", ["RenderNode", "Position"],
  runrender: {group: 10, parameters: [
                "PositionStates", "calculatedTick", "progressInTick"
              ]})

SYSTEM_RENDERNODEPROPERTIES = EntitySystem.new(
  "RenderNodePropertiesSystem", [],
  runrender: {group: 11, parameters: ["ComponentRenderNode.GetIndex()"]})

SYSTEM_ANIMATION = EntitySystem.new(
  "AnimationSystem", [],
  runrender: {group: 60, parameters: ["ComponentAnimated.GetIndex()",
                                      "calculatedTick", "progressInTick"]})

SYSTEM_RECEIVED = EntitySystem.new(
  "ReceivedSystem", [],
  runtick: {group: 55,
            parameters: ["ComponentReceived.GetIndex()"]})

# This needs to be ran before systems that unmark the Position
SYSTEM_SENDABLEMARK_POSITION = EntitySystem.new(
  "SendableMarkFromSystem<Position>", ["Sendable", "Position"],
  runtick: {group: 20,
            parameters: []})

SYSTEM_SENDABLE = EntitySystem.new(
  "SendableSystem", [],
  runtick: {group: 70,
            parameters: ["ComponentSendable.GetIndex()"]})

SYSTEM_POSITIONSTATE = EntitySystem.new(
  "PositionStateSystem", [], runtick: {
    group: 50,
    parameters: ["ComponentPosition.GetIndex()", "PositionStates",
                 "tick"]})

SYSTEM_MODELPROPERTIES = EntitySystem.new(
  "ModelPropertiesSystem", [], runtick: {
    group: 56,
    parameters: ["ComponentModel.GetIndex()"]})


