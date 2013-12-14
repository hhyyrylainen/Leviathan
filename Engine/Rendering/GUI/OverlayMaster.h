#ifndef LEVIATHAN_OVERLAYMASTER
#define LEVIATHAN_OVERLAYMASTER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include <Overlay/OgreOverlay.h>
#include <Overlay/OgreOverlayElement.h>
#include <Overlay/OgreOverlaySystem.h>
#include <Overlay/OgrePanelOverlayElement.h>
#include <OgreRenderTargetListener.h>
#include "FontManager.h"

namespace Leviathan{ 
namespace Gui{
	class GuiManager;
}
namespace Rendering{

	// credit for coming up with this class/method goes to arthare http://www.ogre3d.org/forums/viewtopic.php?f=5&p=486334#p486334
	// a class to make sure that only the pointed-to overlay gets painted
	class OverlayPreferer : public Ogre::RenderTargetListener{
	public:
		OverlayPreferer(Gui::GuiManager* target, Ogre::Viewport* activeviewport);
		virtual ~OverlayPreferer();
		virtual void preViewportUpdate(const Ogre::RenderTargetViewportEvent & evt) override;
		virtual void postViewportUpdate(const Ogre::RenderTargetViewportEvent & evt) override;
		static std::set<Gui::GuiManager*> setOverlays;

	private:
		Gui::GuiManager* RTarget;
		Ogre::Viewport* _Viewport;
	};
	// usage: ViewPort->getTarget()->addListener(new OverlayPreferer(panel ptr, same ViewPort));


	class OverlayMaster : public Object{
	public:
		DLLEXPORT OverlayMaster();
		DLLEXPORT ~OverlayMaster();

		DLLEXPORT void Release();

		DLLEXPORT void SetGUIVisibleInViewport(Gui::GuiManager* container, Ogre::Viewport* viewport);


		DLLEXPORT static inline Ogre::OverlayManager& GetManager(){
			return Ogre::OverlayManager::getSingleton();
		}

	private:

		// releases OverlayPreferers //
		list<unique_ptr<OverlayPreferer>> OverlaysPreferrers;
	};

}}
#endif