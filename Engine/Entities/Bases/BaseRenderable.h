#ifndef LEVIATHAN_BASE_RENDERABLE
#define LEVIATHAN_BASE_RENDERABLE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //

namespace Leviathan{

	class Graphics; // forward declaration to make this work //

	class BaseRenderable{
	public:
		DLLEXPORT BaseRenderable::BaseRenderable();
		DLLEXPORT virtual BaseRenderable::~BaseRenderable();

		DLLEXPORT virtual bool Render(Graphics* renderer, int mspassed) = 0;

		DLLEXPORT bool IsHidden();


	protected:
		int Frames;
		bool Hidden;
		bool Updated;
	};

}
#endif