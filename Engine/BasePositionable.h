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

		DLLEXPORT virtual void SetPos(int x, int y, int z) = 0;
		DLLEXPORT virtual void SetOrientation(int pitch, int yaw, int roll) = 0;

		DLLEXPORT virtual void GetOrientation(int &outpitch, int &outyaw, int &outroll);
		DLLEXPORT virtual int GetPitch();
		DLLEXPORT virtual int GetYaw();
		DLLEXPORT virtual int GetRoll();		

		DLLEXPORT virtual void GetPos(int &outx, int &outy, int &outz);
		DLLEXPORT virtual int GetXPos();
		DLLEXPORT virtual int GetYPos();
		DLLEXPORT virtual int GetZPos();

		DLLEXPORT virtual void SetPosX(int x);
		DLLEXPORT virtual void SetPosY(int y);
		DLLEXPORT virtual void SetPosZ(int z);

		DLLEXPORT virtual void SetPitch(int pitch);
		DLLEXPORT virtual void SetYaw(int yaw);
		DLLEXPORT virtual void SetRoll(int roll);

	protected:
		virtual void PosUpdated() = 0;
		virtual void OrientationUpdated() = 0;
		// ------------- //
		int X,Y,Z;
		int Pitch,Yaw,Roll;
	};

}
#endif