#ifndef LEVIATHAN_BASE_POSITIONABLE
#define LEVIATHAN_BASE_POSITIONABLE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{

	class BasePositionable /*: public Object these classes are "components" and shouldn't inherit anything */{
	public:
		DLLEXPORT BasePositionable::BasePositionable();
		DLLEXPORT virtual BasePositionable::~BasePositionable();

		DLLEXPORT virtual void SetPos(const float &x, const float &y, const float &z);
		DLLEXPORT virtual void SetOrientation(int pitch, int yaw, int roll);

		DLLEXPORT virtual void GetOrientation(int &outpitch, int &outyaw, int &outroll);
		DLLEXPORT virtual int GetPitch();
		DLLEXPORT virtual int GetYaw();
		DLLEXPORT virtual int GetRoll();

		DLLEXPORT virtual void GetPos(float &outx, float &outy, float &outz);
		DLLEXPORT virtual float GetXPos();
		DLLEXPORT virtual float GetYPos();
		DLLEXPORT virtual float GetZPos();

		DLLEXPORT virtual void SetPosX(const float &x);
		DLLEXPORT virtual void SetPosY(const float &y);
		DLLEXPORT virtual void SetPosZ(const float &z);

		DLLEXPORT virtual void SetPitch(int pitch);
		DLLEXPORT virtual void SetYaw(int yaw);
		DLLEXPORT virtual void SetRoll(int roll);

	protected:
		virtual void PosUpdated();
		virtual void OrientationUpdated();
		// ------------- //
		float X,Y,Z;
		int Pitch,Yaw,Roll;
	};

}
#endif