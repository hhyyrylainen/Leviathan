#ifndef LEVIATHAN_RESOLUTIONSCALING
#define LEVIATHAN_RESOLUTIONSCALING
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{

	class ResolutionScaling /*: public Object*/{
	public:

		DLLEXPORT static void SetResolution(int width, int height);

		//DLLEXPORT static float GetXScaleFactor();
		//DLLEXPORT static float GetYScaleFactor();


		//DLLEXPORT static float ScalePromilleToFactorX(int x);
		//DLLEXPORT static float ScalePromilleToFactorY(int y);

		//DLLEXPORT static float GetPromilleFactor();

		//DLLEXPORT static float ScaleAbsoluteXToPromille(int x);
		//DLLEXPORT static float ScaleAbsoluteYToPromille(int y);

		//DLLEXPORT static float ScaleAbsoluteXToFactor(int x);
		//DLLEXPORT static float ScaleAbsoluteYToFactor(int y);

		DLLEXPORT static float ScaleTextSize(float size);
		DLLEXPORT static float UnScaleTextFromSize(float size);
	private:

		DLLEXPORT ResolutionScaling::ResolutionScaling();
		DLLEXPORT ResolutionScaling::~ResolutionScaling();

		//static void CalculateFactors();
		// ------------------------ //
		//static float XScaleFactor;
		//static float YScaleFactor;


		static int Width;
		static int Height;
	};

}
#endif