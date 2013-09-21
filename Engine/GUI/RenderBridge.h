#ifndef LEVIATHAN_RENDER_BRIDGE
#define LEVIATHAN_RENDER_BRIDGE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Rendering\GUI\GuiRendBlob.h"
#include "Rendering\GUI\OverlayMaster.h"
#include "OgreOverlayContainer.h"

namespace Leviathan{

	class RenderBridge : public Object{
	public:
		DLLEXPORT RenderBridge(int id, bool hidden, int zorder);
		DLLEXPORT ~RenderBridge();
		
		DLLEXPORT bool RenderActions(Rendering::OverlayMaster* rendering);

		DLLEXPORT void SetVisibility(const bool &visible);
		DLLEXPORT void SetHidden(int slot, bool hidden);
		DLLEXPORT size_t GetSlotIndex(int slot);
		//DLLEXPORT RenderingGBlob** GetDrawAction(const size_t &index);
		DLLEXPORT void DeleteBlobOnIndex(const size_t &index);

		DLLEXPORT inline bool DoesWantToClose(){
			return WantsToClose;
		}
		DLLEXPORT inline int GetID(){
			return ID;
		}

		DLLEXPORT inline void SetAsClosing(){
			WantsToClose = true;
		}
		DLLEXPORT inline void SetZOrder(const int &zorder){
			ZVal = zorder;
		}
		
		std::vector<RenderingGBlob*> DrawActions;

	protected:
		
		Ogre::OverlayContainer* BridgesContainer;

		Rendering::OverlayMaster* OwningOverlay;

		bool WantsToClose;
		int ZVal;
		bool Hidden;
		int ID;
	};

}
#endif