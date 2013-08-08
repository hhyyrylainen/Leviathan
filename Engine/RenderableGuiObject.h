#ifndef LEVIATHAN_GUI_BASERENDERABLE
#define LEVIATHAN_GUI_BASERENDERABLE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "RenderBridge.h"
#include "GuiBaseGraphicalComponent.h"

namespace Leviathan{
	class Graphics;
namespace Gui{

#define GUI_RENDERABLE_FORCEUPDATE_EVERY_N_FRAMES	5000
	

	class RenderableGuiObject{
	public:
		DLLEXPORT RenderableGuiObject::RenderableGuiObject(const int &zlocation, const size_t &gcomponentcount, const bool &hidden = false);
		DLLEXPORT virtual RenderableGuiObject::~RenderableGuiObject();

		// pure virtual function(s) to force all render ables to define these //
		//DLLEXPORT virtual void Release() = 0;
		DLLEXPORT virtual void Render(Graphics* graph) = 0;

		DLLEXPORT inline void SetHiddenState(bool hidden){
			Updated = true;
			Hidden = hidden;
		}


		bool Hidden;
		int Zorder;

	protected:
		// ensures that rendering bridge is created, or throws an exception //
		virtual bool VerifyRenderingBridge(Graphics* graph, const int &ID);

		// used to force all GUI objects to render at least every few minutes //
		int NoNUpdatedFrames;
		// used to see if hidden state has been changed between frames //
		bool OldHidden;
		bool Updated;

		// graphical components used to easily reuse graphical parts //
		vector<BaseGraphicalComponent*> GComponents;

		// rendering bridge //
		shared_ptr<RenderBridge> RBridge;
	};

}}
#endif