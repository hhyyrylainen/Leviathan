#ifndef LEVIATHAN_GUI_BASE_RENDERABLE
#define LEVIATHAN_GUI_BASE_RENDERABLE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
//#include "Rendering\Graphics.h"
#include "BaseGuiObject.h"
#include "RenderBridge.h"

namespace Leviathan{

#define GUI_RENDERABLE_FORCEUPDATE_EVERY_N_FRAMES	5000

	class Graphics;

	class RenderableGuiObject : public BaseGuiObject{
	public:
		DLLEXPORT RenderableGuiObject::RenderableGuiObject();
		DLLEXPORT virtual RenderableGuiObject::~RenderableGuiObject();


		DLLEXPORT virtual void Release(Graphics* graph);
		DLLEXPORT virtual void Render(Graphics* graph);


		bool Hidden;
		int Zorder;

	protected:
		int NoNUpdatedFrames;
	};

}
#endif