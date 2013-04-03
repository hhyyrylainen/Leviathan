#ifndef LEVIATHAN_RENDERING_LIGHT
#define LEVIATHAN_RENDERING_LIGHT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include <d3dx10math.h>

namespace Leviathan{

	class RenderingLight : public Object{
	public:
		DLLEXPORT RenderingLight::RenderingLight();
		DLLEXPORT RenderingLight::~RenderingLight();

		DLLEXPORT void SetAmbientColor(float red, float green, float blue, float alpha);
		DLLEXPORT void SetDiffuseColor(float red, float green, float blue, float alpha);
		DLLEXPORT void SetDirection(float x, float y, float z);
		DLLEXPORT void SetSpecularColor(float red, float green, float blue, float alpha);
		DLLEXPORT void SetSpecularPower(float power);

		DLLEXPORT Float4 GetAmbientColor();
		DLLEXPORT Float4 GetDiffuseColor();
		DLLEXPORT Float3 GetDirection();
		DLLEXPORT Float4 GetSpecularColor();
		DLLEXPORT float GetSpecularPower();
	private:
		Float4 AmbientColor;
		Float4 DiffuseColor;
		Float3 Direction;
		Float4 SpecularColor;
		float SpecularPower;
	};

}
#endif