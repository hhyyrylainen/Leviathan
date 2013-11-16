#ifndef LEVIATHAN_GRAPHICALINPUTENTITY
#define LEVIATHAN_GRAPHICALINPUTENTITY
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Window.h"
#include "Input\InputController.h"
#include "GUI\GuiManager.h"
#include "Entities\Objects\ViewerCameraPos.h"

namespace Leviathan{

	class GameWorld;

	class GraphicalInputEntity : public Object{
	public:
		DLLEXPORT GraphicalInputEntity(Graphics* windowcreater, AppDef* windowproperties);
		DLLEXPORT ~GraphicalInputEntity();

		DLLEXPORT void Tick(int mspassed);

		// This function uses the LinkObjects function objects //
		DLLEXPORT void Render(int mspassed);

		// object linking //
		// This function also updates the camera aspect ratio //
		DLLEXPORT void LinkObjects(shared_ptr<ViewerCameraPos> camera, shared_ptr<GameWorld> world);

		// returns true if succeeds, false if another window has input //
		DLLEXPORT bool SetMouseCapture(bool state);

		DLLEXPORT void ReleaseLinked();

		DLLEXPORT void UnlinkAll();

		// graphics related //
		DLLEXPORT float GetViewportAspectRatio();
		DLLEXPORT void SaveScreenShot(const string &filename);

		DLLEXPORT void OnResize(int width, int height);

		DLLEXPORT inline Window* GetWindow(){
			return DisplayWindow;
		}
		DLLEXPORT inline Ogre::Viewport* GetMainViewport(){
			return MainViewport;
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
	protected:



		Window* DisplayWindow;
		InputController* TertiaryReceiver;
		Gui::GuiManager* WindowsGui;
		Ogre::Viewport* MainViewport;

		shared_ptr<GameWorld> LinkedWorld;
		shared_ptr<ViewerCameraPos> LinkedCamera;
		// this count variable is needed to parse resource groups after first window //
		static int GlobalWindowCount;

		bool MouseCaptureState;
		static GraphicalInputEntity* InputCapturer;
	};

}
#endif