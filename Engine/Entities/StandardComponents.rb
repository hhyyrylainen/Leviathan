# Component definitions for world generators of the inbuilt types
require_relative '../../Helpers/FileGen.rb'

COMPONENT_POSITION = EntityComponent.new(
  "Position",
  [ConstructorInfo.new(
     [
       Variable.new("position", "Float3",
                    memberaccess: "Members._Position"),
       Variable.new("orientation", "Float4",
                    memberaccess: "Members._Orientation")
     ], usedatastruct: true)
  ], statetype: true)

COMPONENT_RENDERNODE = EntityComponent.new(
  "RenderNode", [ConstructorInfo.new(
                   [
                     Variable.new("GetScene()", "",
                                  nonMethodParam: true),
                   ])], releaseparams: ["GetScene()"])

COMPONENT_SENDABLE = EntityComponent.new(
  "Sendable", [ConstructorInfo.new([])], nosynchronize: true)

COMPONENT_RECEIVED = EntityComponent.new(
  "Received", [ConstructorInfo.new([])], nosynchronize: true)

COMPONENT_MODEL = EntityComponent.new(
  "Model", [ConstructorInfo.new(
              [
                Variable.new("GetScene()", "",
                             nonMethodParam: true),
                Variable.new("GetComponent_RenderNode(id)", "",
                             nonMethodParam: true
                            ),
                Variable.new("model", "std::string",
                             memberaccess: "MeshName"),
                Variable.new("material", "bs::HMaterial",
                             memberaccess: "Material")
              ]
            )], releaseparams: [#["GetComponent_RenderNode(id)" # "GetScene()"
  ], nosynchronize: true # this is to get around not having implemented bsf serialization
)

COMPONENT_PHYSICS = EntityComponent.new(
  "Physics", [
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
  releaseparams: ["GetPhysicalWorld()"])

COMPONENT_BOXGEOMETRY = EntityComponent.new(
  "BoxGeometry",
  [ConstructorInfo.new(
     [
       Variable.new("size", "Float3", memberaccess: "Sizes"),
       Variable.new("material", "std::string", memberaccess: "Material")
     ], usedatastruct: false)])

# EntityComponent.new("ManualObject", [ConstructorInfo.new(
#                                        [
#                                          Variable.new("GetScene()", "",
#                                                       nonMethodParam: true),
#                                        ])],
#                     releaseparams: ["GetScene()"])

COMPONENT_CAMERA = EntityComponent.new(
  "Camera",
  [ConstructorInfo.new(
     [
       Variable.new("fov", "uint16_t", default: "90",
                    memberaccess: "FOV"),
       Variable.new("soundperceiver", "bool",
                    default: "true", memberaccess: "SoundPerceiver")
     ])
  ])

# EntityComponent.new("Plane",
#                     [ConstructorInfo.new(
#                        [
#                          Variable.new("GetScene()", "",
#                                       nonMethodParam: true),
#                          Variable.new("parent", "Ogre::SceneNode*",
#                                       noRef: true, nonserializeparam: true),
#                          Variable.new("material", "std::string", memberaccess: "Material"),
#                          Variable.new("plane", "Ogre::Plane",
#                                       memberaccess: "PlaneDefinition"),
#                          Variable.new("size", "Float2", memberaccess: "Size"),
#                          Variable.new("uvupvector", "Ogre::Vector3",
#                                       memberaccess: "UpVector",
#                                       default: "Ogre::Vector3::UNIT_Y"),
#                        ])
#                     ])

COMPONENT_ANIMATED = EntityComponent.new(
  "Animated",
  [ConstructorInfo.new(
     [
       Variable.new("GetComponent_RenderNode(id)", "",
                    nonMethodParam: true)
     ])
  ], releaseparams: []#["GetComponent_RenderNode(id)"]
)
