#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "Common/SFMLPackets.h"
#include "BaseObject.h"
#include "../../Common/Types.h"

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
	class BasePositionable : public virtual BaseObject{
	public:
		DLLEXPORT BasePositionable();
		// Uses provided values in the initializer list //
		DLLEXPORT BasePositionable(const Float3 &pos, const Float4 &orientation);
		DLLEXPORT virtual ~BasePositionable();

		DLLEXPORT void SetPosComponents(const float &x, const float &y, const float &z);
		DLLEXPORT void SetPos(const Float3 &pos);
        DLLEXPORT void SetPosition(const Float3 &pos);
		DLLEXPORT void SetOrientation(const Float4 &quaternionrotation);
        DLLEXPORT void SetOrientationComponents(const float &x, const float &y, const float &z, const float &w);

        DLLEXPORT void SetPos(const Float3 &pos, Lock &guard);
        DLLEXPORT void SetOrientation(const Float4 &quaternionrotation, Lock &guard);

		DLLEXPORT Float4 GetOrientation() const;
        DLLEXPORT Float4 GetOrientation(Lock &guard) const;
        DLLEXPORT Float4 GetRotation() const;
        DLLEXPORT void GetOrientation(Float4 &receiver) const;
        DLLEXPORT void GetRotation(Float4 &receiver) const;

        //! \brief Applies position and rotation from a BasePositionData
        DLLEXPORT void ApplyPositionDataObject(const BasePositionData &pos);

		DLLEXPORT void GetPosElements(float &outx, float &outy, float &outz);
		DLLEXPORT Float3 GetPos() const;
        DLLEXPORT Float3 GetPos(Lock &guard) const;
        DLLEXPORT Float3 GetPosition() const;
        DLLEXPORT void GetPos(Float3 &receiver) const;
		DLLEXPORT float GetPosX() const;
		DLLEXPORT float GetPosY() const;
		DLLEXPORT float GetPosZ() const;

		DLLEXPORT void SetPosX(const float &x);
		DLLEXPORT void SetPosY(const float &y);
		DLLEXPORT void SetPosZ(const float &z);

        //! \brief Saves location and rotation to a packet
        DLLEXPORT void AddPositionAndRotationToPacket(sf::Packet &packet);

        //! \brief Sets the position and rotation to data retrieved from a packet
        //! \exception ExceptionInvalidArgument when the packet format is invalid
        DLLEXPORT void ApplyPositionAndRotationFromPacket(sf::Packet &packet);

        //! \brief Interpolates between two states and sets this object's state to be between the states
        DLLEXPORT void InterpolatePositionableState(PositionableRotationableDeltaState &first,
            PositionableRotationableDeltaState &second, float progress);
        
        //! \brief Loads position data into a BasePositionData
        //! \note This applies the values directly so if this returns false it is possible
        //! that the target has partially or entirely invalid data
        DLLEXPORT static bool LoadPositionFromPacketToHolder(sf::Packet &packet,
            BasePositionData &target);
        

	protected:
		virtual void PosUpdated(Lock &guard);
		virtual void OrientationUpdated(Lock &guard);

		bool BasePositionableCustomMessage(int message, void* data);
		bool BasePositionableCustomGetData(ObjectDataRequest* data);
        
		// ------------------------------------ //
        
		Float3 Position;
		Float4 QuatRotation;
	};

}

