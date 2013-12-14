#ifndef LEVIATHAN_BASE_SCALABLE
#define LEVIATHAN_BASE_SCALABLE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{

	class BaseScalable /*: public Object these classes are "components" and shouldn't inherit anything */{
	public:
		DLLEXPORT BaseScalable();
		DLLEXPORT ~BaseScalable();

		DLLEXPORT float GetScale();
		DLLEXPORT void GetScale(float &outx, float &outy, float &outz);

		DLLEXPORT float GetXScale();
		DLLEXPORT float GetYScale();
		DLLEXPORT float GetZScale();

		DLLEXPORT void SetScale(float all);
		DLLEXPORT void SetScale(float x, float y, float z);

		DLLEXPORT void SetXScale(float x);
		DLLEXPORT void SetYScale(float y);
		DLLEXPORT void SetZScale(float z);

	protected:
		void OnUpdateScale();
		bool IsScaleUpdated();

		// ------------------------------- //
		float XScale;
		float YScale;
		float ZScale;

		bool ScaleUpdated;
	};

}
#endif
