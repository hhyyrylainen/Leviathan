#ifndef LEVIATHAN_GUIBASEGRAPHICALCOMPONENT
#define LEVIATHAN_GUIBASEGRAPHICALCOMPONENT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Rendering\Graphics.h"

namespace Leviathan{ namespace Gui{

#define GUI_GRAPHICALCOMPONENT_TYPE_BASE		0
#define GUI_GRAPHICALCOMPONENT_TYPE_BACKGROUND	1
#define GUI_GRAPHICALCOMPONENT_TYPE_SIMPLETEXT	2

	class BaseGraphicalComponent : public virtual Object{
	public:
		DLLEXPORT BaseGraphicalComponent::BaseGraphicalComponent(int slot, int zorder);
		DLLEXPORT virtual BaseGraphicalComponent::~BaseGraphicalComponent();

		// release used to explicitly release render bridge blob, (will always be released when render bridge is deleted) //
		DLLEXPORT virtual void Release(RenderBridge* bridge) = 0;

		// rendering function //
		DLLEXPORT virtual void Render(RenderBridge* bridge, Graphics* graph) = 0;

		DLLEXPORT inline bool IsRenderNeeded(){

			return RUpdated;
		}

		DLLEXPORT inline void SetZOrder(int newz){

			ZOrder = newz;
			// updated //
			RUpdated = true;
		}

	protected:
		int CType;

		// slot used in determining which rendering bridge blob matches this //
		int Slot;
		int ZOrder;

		// flag which remembers if render update is needed //
		bool RUpdated;
	};

}}
#endif