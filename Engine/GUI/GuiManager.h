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
#include "CEGUI/GUIContext.h"




namespace Leviathan {

namespace Gui{


	//! \brief Main GUI controller
	//! \todo Add GUI window objects to this which are associated with different windows
	class GuiManager : public EngineComponent, public ThreadSafe{
	public:
		DLLEXPORT GuiManager();
		DLLEXPORT ~GuiManager();

		DLLEXPORT bool Init(AppDef* vars, Graphics* graph, GraphicalInputEntity* window);
		DLLEXPORT void Release();

		//! \brief Called by Engine during Release to destroy static variables
		DLLEXPORT static void KillGlobalCache();

		DLLEXPORT void GuiTick(int mspassed);
		DLLEXPORT void Render();

		// key press receiving from listener //
		DLLEXPORT bool ProcessKeyDown(OIS::KeyCode key, int specialmodifiers);

		//! \todo Actually pass this to Views
		DLLEXPORT inline void SetVisible(bool visible){
			Visible = visible;
		}
		//! \brief Notifies internal browsers
		//! \todo Make CEGUI allow multiple windows
		DLLEXPORT void OnResize();
		//! \brief Notifies internal browsers
		DLLEXPORT void OnFocusChanged(bool focused);

		// internal Gui element managing //
		DLLEXPORT bool AddGuiObject(BaseGuiObject* obj);
		DLLEXPORT void DeleteObject(int id);
		DLLEXPORT int GetObjectIndexFromId(int id);
		DLLEXPORT BaseGuiObject* GetObject(unsigned int index);


		//! \brief Returns the main GUI context
		DLLEXPORT CEGUI::GUIContext* GetMainContext();

		// function split into peaces //
		DLLEXPORT bool LoadCollection(vector<shared_ptr<ObjectFileObject>> &data, ObjectFileObject &collectiondata);

		// file loading //

		//! \brief Loads a GUI file
		DLLEXPORT bool LoadGUIFile(const wstring &file, bool nochangelistener = false);

		//! \brief Unloads the currently loaded file
		DLLEXPORT void UnLoadGUIFile();

		// set to "none" to use default //
		DLLEXPORT void SetMouseTheme(const wstring &tname);
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

		//! \brief Returns a single CEGUI::Window matching the name
		//! \todo Allow error reporting
		DLLEXPORT CEGUI::Window* GetWindowByStringName(const string &namepath);


	private:
		// rendering //
		bool _CreateInternalOgreResources(Ogre::SceneManager* windowsscene);
		void _ReleaseOgreResources();

		void _FileChanged(const wstring &file, ResourceFolderListener &caller);

		// ------------------------------------ //


		//! \todo implement this in OverlayMaster to hide GUI in view ports that don't need it
		bool Visible;
		//! used to determine when to scan collections for active ones
		bool GuiMouseUseUpdated;
		//! set when containing window of the GUI shouldn't be allowed to capture mouse
		bool GuiDisallowMouseCapture;


		GraphicalInputEntity* ThisWindow;

		//! Gui elements
		vector<BaseGuiObject*> Objects;

		//! The corresponding CEGUI context
		CEGUI::GUIContext* GuiContext;

		//! Time of creation of this GuiManager
		WantedClockType::time_point LastTimePulseTime;

		//! The main file of the GUI from which is loaded from
		wstring MainGUIFile;

		//! Set when this is the first created gui manager
		//! \detail Used for injecting time pulses into CEGUI
		bool MainGuiManager;

		//! we will soon need a GuiManager for each window
		int ID;

		//! Used to stop listening for file changes
		int FileChangeID;

		// collections //
		std::vector<GuiCollection*> Collections;


		// ------------------------------------ //
		// Static animation files //
		//! Holds the loaded animation files, used to prevent loading a single file multiple times
		static std::vector<wstring> LoadedAnimationFiles;

		static boost::recursive_mutex GlobalGUIMutex;

		static bool IsAnimationFileLoaded(ObjectLock &lock, const wstring &file);

		//! \warning Won't check if the file is already in the vector, use IsAnimationFileLoaded
		static void SetAnimationFileLoaded(ObjectLock &lock, const wstring &file);
	};

}}
#endif
