#ifndef LEVIATHAN_BASE_POSITIONABLE
#define LEVIATHAN_BASE_POSITIONABLE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Common/SFMLPackets.h"

#define BASEPOSITIONAL_CUSTOMMESSAGE_DATA_CHECK		{if(entitycustommessagetype == ENTITYCUSTOMMESSAGETYPE_CHANGEWORLDPOSITION){if(BasePositionableCustomMessage(entitycustommessagetype, dataptr)) return true;}}
#define BASEPOSITIONAL_CUSTOMMESSAGE_GET_CHECK		{if(tmprequest->RequestObjectPart == ENTITYDATA_REQUESTTYPE_WORLDPOSITION){if(BasePositionableCustomGetData(tmprequest)) return true;}}

namespace Leviathan{

	struct ObjectDataRequest;

    //! \brief Can hold all data used by BasePositionable
    struct BasePositionData{

        Float3 Position;
		Float4 QuatRotation;
    };

    //! \brief Base class for all entities that can be moved or rotated
	class BasePositionable{
	public:
		DLLEXPORT BasePositionable();
		// Uses provided values in the initializer list //
		DLLEXPORT BasePositionable(const Float3 &pos, const Float4 &orientation);
		DLLEXPORT virtual ~BasePositionable();

		DLLEXPORT virtual void SetPosComponents(const float &x, const float &y, const float &z);
		DLLEXPORT virtual void SetPos(const Float3 &pos);
        DLLEXPORT virtual void SetPosition(const Float3 &pos);
		DLLEXPORT virtual void SetOrientation(const Float4 &quaternionrotation);
        DLLEXPORT virtual void SetOrientationComponents(const float &x, const float &y, const float &z, const float &w);

		DLLEXPORT virtual Float4 GetOrientation();

        //! \brief Applies position and rotation from a BasePositionData
        DLLEXPORT virtual void ApplyPositionDataObject(const BasePositionData &pos);

		DLLEXPORT virtual void GetPosElements(float &outx, float &outy, float &outz);
		DLLEXPORT virtual Float3 GetPos();
		DLLEXPORT virtual float GetXPos();
		DLLEXPORT virtual float GetYPos();
		DLLEXPORT virtual float GetZPos();

		DLLEXPORT virtual void SetPosX(const float &x);
		DLLEXPORT virtual void SetPosY(const float &y);
		DLLEXPORT virtual void SetPosZ(const float &z);

        //! \brief Saves location and rotation to a packet
        DLLEXPORT void AddPositionAndRotationToPacket(sf::Packet &packet);

        //! \brief Sets the position and rotation to data retrieved from a packet
        //! \exception ExceptionInvalidArgument when the packet format is invalid
        DLLEXPORT void ApplyPositionAndRotationFromPacket(sf::Packet &packet) THROWS;

        //! \brief Loads position data into a BasePositionData
        //! \note This applies the values directly so if this returns false it is possible
        //! that the target has partially or entirely invalid data
        DLLEXPORT static bool LoadPositionFromPacketToHolder(sf::Packet &packet, BasePositionData &target);

	protected:
		virtual void PosUpdated();
		virtual void OrientationUpdated();

		bool BasePositionableCustomMessage(int message, void* data);
		bool BasePositionableCustomGetData(ObjectDataRequest* data);
        
		// ------------------------------------ //
        
		Float3 Position;
		Float4 QuatRotation;
	};

}
#endif
