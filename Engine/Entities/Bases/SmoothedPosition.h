#ifndef LEVIATHAN_SMOOTHEDPOSITION
#define LEVIATHAN_SMOOTHEDPOSITION
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "BasePositionable.h"

namespace Leviathan{

	class SmoothedPosition : public BasePositionable{
	public:
		DLLEXPORT SmoothedPosition::SmoothedPosition();
		DLLEXPORT virtual SmoothedPosition::~SmoothedPosition();

	protected:
		void SmoothValues();
		int s_X,s_Y,s_Z;
		int s_Pitch,s_Yaw,s_Roll;
		bool SmoothUpdated;
	};

}
#endif