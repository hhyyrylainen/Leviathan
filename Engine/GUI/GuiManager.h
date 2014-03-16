#ifndef LEVIATHAN_GUI_MAIN
#define LEVIATHAN_GUI_MAIN
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Application/AppDefine.h"
#include "GUI/BaseGuiObject.h"


// objects //
#include "OgreRenderQueueListener.h"
#include "Common/ThreadSafe.h"




namespace Leviathan {
	class GraphicalInputEntity;
	class Graphics;
namespace Gui{

	class GuiCollection;
	class GuiLoadedSheet;

	//! \brief Main GUI controller
	//! \todo Add GUI window objects to this which are associated with different windows
	class GuiManager : public EngineComponent, public Ogre::RenderQueueListener, public ThreadSafe{
	public:
		DLLEXPORT GuiManager();
		DLLEXPORT ~GuiManager();

		DLLEXPORT bool Init(AppDef* vars, Graphics* graph, GraphicalInputEntity* window);
		DLLEXPORT void Release();

		DLLEXPORT void GuiTick(int mspassed);
		DLLEXPORT void Render();

		// key press receiving from listener //
		DLLEXPORT bool ProcessKeyDown(OIS::KeyCode key, int specialmodifiers);

		DLLEXPORT inline void SetVisible(bool visible){
			Visible = visible;
		}

		DLLEXPORT void OnResize(int width, int height);

		// internal Gui element managing //
		DLLEXPORT bool AddGuiObject(BaseGuiObject* obj);
		DLLEXPORT void DeleteObject(int id);
		DLLEXPORT int GetObjectIndexFromId(int id);
		DLLEXPORT BaseGuiObject* GetObject(unsigned int index);

		// function split into peaces //
		DLLEXPORT bool LoadCollection(vector<shared_ptr<ObjectFileObject>> &data, ObjectFileObject &collectiondata);

		// file loading //
		DLLEXPORT bool LoadGUIFile(const wstring &file);
		// set to "none" to use default //
		DLLEXPORT void SetMouseFile(const wstring &file);
		DLLEXPORT void SetMouseFileVisibleState(bool state);

		// collection managing //
		DLLEXPORT void AddCollection(GuiCollection* add);
		DLLEXPORT GuiCollection* GetCollection(const int &id, const wstring &name = L"");

		DLLEXPORT void SetCollectionStateProxy(string name, bool state);
		DLLEXPORT void SetCollectionState(const wstring &name, bool state);
		DLLEXPORT void SetCollectionAllowEnableState(const wstring &name, bool allow = true);
		DLLEXPORT inline void PossiblyGUIMouseDisable(){
			GuiMouseUseUpdated = true;
		}
		// called when mouse cannot be captured (should force at least one collection on) //
		DLLEXPORT void OnForceGUIOn();

		// events //
		DLLEXPORT bool CallEvent(Event* pEvent);
		DLLEXPORT int CallEventOnObject(BaseGuiObject* receive, Event* pEvent);


		DLLEXPORT inline shared_ptr<GuiLoadedSheet> GetSheet(int id){
			try{

				return GuiSheets[id];

			} catch (...){
				return nullptr;
			}
		}


		//! \brief Used to update chrome before having to render it
		virtual void preRenderQueues();

		virtual void renderQueueStarted(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& skipThisInvocation);

	private:
		// rendering //
		bool _CreateInternalOgreResources(Ogre::SceneManager* windowsscene);
		void _ReleaseOgreResources();

		// ------------------------------------ //


		//! \todo implement this in OverlayMaster to hide GUI in view ports that don't need it
		bool Visible;
		//! used to determine when to scan collections for active ones
		bool GuiMouseUseUpdated;
		//! set when containing window of the GUI shouldn't be allowed to capture mouse
		bool GuiDisallowMouseCapture;


		// Rendering resources //
		Ogre::ManualObject* CEFOverlayQuad;


		GraphicalInputEntity* ThisWindow;

		//! Gui elements
		vector<BaseGuiObject*> Objects;


		//! we will soon need a GuiManager for each window
		int ID;

		// collections //
		std::vector<GuiCollection*> Collections;
		std::map<int, shared_ptr<GuiLoadedSheet>> GuiSheets;
	};

}}
#endif
