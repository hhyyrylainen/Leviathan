#ifndef LEVIATHAN_RENDERING_SAVEDRESOURCE
#define LEVIATHAN_RENDERING_SAVEDRESOURCE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{

	class SavedResource : public EngineComponent{
	public:
		DLLEXPORT SavedResource::SavedResource();
        DLLEXPORT SavedResource::~SavedResource();

		DLLEXPORT virtual bool Init();
		DLLEXPORT virtual void Release();

	private:

	};

}
#endif