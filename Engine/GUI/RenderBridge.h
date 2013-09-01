#ifndef LEVIATHAN_RENDER_BRIDGE
#define LEVIATHAN_RENDER_BRIDGE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Rendering\GuiRendBlob.h"

namespace Leviathan{

	class RenderBridge : public Object{
	public:
		DLLEXPORT RenderBridge::RenderBridge();
		DLLEXPORT RenderBridge::RenderBridge(int id, bool hidden, int zorder);
		DLLEXPORT RenderBridge::~RenderBridge();
		
		DLLEXPORT void SetHidden(int slot, bool hidden);
		DLLEXPORT size_t GetSlotIndex(int slot);
		
		vector<RenderingGBlob*> DrawActions;


		bool WantsToClose;
		int ZVal;
		bool Hidden;
		int ID;
	};

}
#endif