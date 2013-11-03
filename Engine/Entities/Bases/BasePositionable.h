#ifndef LEVIATHAN_BASE_POSITIONABLE
#define LEVIATHAN_BASE_POSITIONABLE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{

	class BasePositionable{
	public:
		DLLEXPORT BasePositionable();
		// Uses provided values in the initializer list //
		DLLEXPORT BasePositionable(const Float3 &pos, const Float4 &orientation);
		DLLEXPORT virtual ~BasePositionable();

		DLLEXPORT virtual void SetPos(const float &x, const float &y, const float &z);
		DLLEXPORT virtual void SetPos(const Float3 &pos);
		DLLEXPORT virtual void SetOrientation(const Float4 &quaternionrotation);

		DLLEXPORT virtual Float4 GetOrientation();


		DLLEXPORT virtual void GetPos(float &outx, float &outy, float &outz);
		DLLEXPORT virtual Float3 GetPos();
		DLLEXPORT virtual float GetXPos();
		DLLEXPORT virtual float GetYPos();
		DLLEXPORT virtual float GetZPos();

		DLLEXPORT virtual void SetPosX(const float &x);
		DLLEXPORT virtual void SetPosY(const float &y);
		DLLEXPORT virtual void SetPosZ(const float &z);

	protected:
		virtual void PosUpdated();
		virtual void OrientationUpdated();
		// ------------- //
		Float3 Position;
		Float4 QuatRotation;
	};

}
#endif