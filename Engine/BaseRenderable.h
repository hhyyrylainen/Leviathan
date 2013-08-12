#ifndef LEVIATHAN_BASE_RENDERABLE
#define LEVIATHAN_BASE_RENDERABLE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
//#include "Rendering\Graphics.h"
#include "Rendering/RenderingPassInfo.h"
namespace Leviathan{

	class Graphics; // forward declaration to make this work //

	class BaseRenderable /*: public Object these classes are "components" and shouldn't inherit anything */{
	public:
		DLLEXPORT BaseRenderable::BaseRenderable();
		DLLEXPORT virtual BaseRenderable::~BaseRenderable();

		DLLEXPORT virtual bool Render(Graphics* renderer, int mspassed, const RenderingPassInfo &info) = 0;

		DLLEXPORT bool IsHidden();


	protected:
		D3DXMATRIX OwnWorld;
		int Frames;
		bool Hidden;
		bool Updated;
	};

}
#endif