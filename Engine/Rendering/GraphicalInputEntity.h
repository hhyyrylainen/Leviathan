#ifndef LEVIATHAN_GRAPHICALINPUTENTITY
#define LEVIATHAN_GRAPHICALINPUTENTITY
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Window.h"
#include "Input/InputController.h"
#include "GUI/GuiManager.h"
#include "Entities/Objects/ViewerCameraPos.h"
#include "CEGUI/RendererModules/Ogre/Renderer.h"

namespace Leviathan{

	//! \brief Represents a collection of objects that represents everything related to a single game's Windows window
	class GraphicalInputEntity : public ThreadSafe{
	public:
		//! \warning You can only create one window at a time since this is not thread safe
		DLLEXPORT GraphicalInputEntity(Graphics* windowcreater, AppDef* windowproperties);
		DLLEXPORT ~GraphicalInputEntity();

		DLLEXPORT void Tick(int mspassed);

		// This function uses the LinkObjects function objects //
		DLLEXPORT bool Render(int mspassed);

		// object linking //
		// This function also updates the camera aspect ratio //
		DLLEXPORT void LinkObjects(shared_ptr<ViewerCameraPos> camera, shared_ptr<GameWorld> world);

		// returns true if succeeds, false if another window has input //
		DLLEXPORT bool SetMouseCapture(bool state);

		DLLEXPORT void ReleaseLinked();

		DLLEXPORT void UnlinkAll();

		//! Returns how many windows have been created
		//! \see GlobalWindowCount
		DLLEXPORT static int GetGlobalWindowCount();

		// graphics related //
		DLLEXPORT float GetViewportAspectRatio();
		DLLEXPORT void SaveScreenShot(const string &filename);

		DLLEXPORT void OnResize(int width, int height);

		DLLEXPORT inline Window* GetWindow(){
			return DisplayWindow;
		}
		DLLEXPORT inline Gui::GuiManager* GetGUI(){
			return WindowsGui;
		}
		DLLEXPORT inline InputController* GetInputController(){
			return TertiaryReceiver;
		}
		DLLEXPORT inline shared_ptr<ViewerCameraPos> GetLinkedCamera(){
			return LinkedCamera;
		}
		DLLEXPORT void OnFocusChange(bool focused);

		DLLEXPORT CEGUI::OgreRenderer* GetCEGUIRenderer() const{
			return CEGUIRenderer;
		}


		//! \brief Overwrites the default InputController with a custom one
		//! \warning The controller will be deleted by this and there is no way to release it without deleting after this call
		DLLEXPORT void SetCustomInputController(InputController* controller);


	protected:



		Window* DisplayWindow;
		InputController* TertiaryReceiver;
		Gui::GuiManager* WindowsGui;
		CEGUI::OgreRenderer* CEGUIRenderer;


		shared_ptr<GameWorld> LinkedWorld;
		shared_ptr<ViewerCameraPos> LinkedCamera;
		// this count variable is needed to parse resource groups after first window //
		static int GlobalWindowCount;

		bool MouseCaptureState;
		static GraphicalInputEntity* InputCapturer;
	};

}
#endif