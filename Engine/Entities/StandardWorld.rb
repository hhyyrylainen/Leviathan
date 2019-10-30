# Definition of StandardWorld world type
require_relative 'StandardComponents'
require_relative 'StandardSystems'

STANDARD_WORLD = GameWorldClass.new(
  'StandardWorld',
  componentTypes: [
    COMPONENT_POSITION,
    COMPONENT_RENDERNODE,
    COMPONENT_SENDABLE,
    COMPONENT_RECEIVED,
    COMPONENT_MODEL,
    COMPONENT_PHYSICS,
    COMPONENT_BOXGEOMETRY,
    COMPONENT_CAMERA,
    COMPONENT_ANIMATED
  ],
  systems: [
    SYSTEM_RENDERINGPOSITION,
    SYSTEM_RENDERNODEPROPERTIES,
    SYSTEM_ANIMATION,
    SYSTEM_RECEIVED,
    SYSTEM_SENDABLEMARK_POSITION,
    SYSTEM_SENDABLE,
    SYSTEM_POSITIONSTATE,
    SYSTEM_MODELPROPERTIES
  ],
  framesystemrun: <<-END
    // Client interpolation //
    if(GetNetworkSettings().DoInterpolation){

        //const float interpolatepercentage = std::max(0.f, timeintick / (float)TICKSPEED);

        // _ReceivedSystem.Run(*this, ComponentReceived.GetIndex());
    }

    // Skip in non-gui mode //
    if(!GraphicalMode)
        return;
  END

)

STANDARD_WORLD.WorldType = 'static_cast<int32_t>(Leviathan::INBUILT_WORLD_TYPE::Standard)'
