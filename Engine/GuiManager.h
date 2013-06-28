#ifndef LEVIATHAN_GUI_MAIN
#define LEVIATHAN_GUI_MAIN
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#define GOBJECT_TYPE_TEXTLABEL 1

#include "ObjectFileProcessor.h"

#include "Key.h"
#include "AppDefine.h"
#include "DataStore.h"
#include "DataBlock.h"
#include "BaseGuiObject.h"

#include "GuiKeyListener.h"
#include "Rendering\Graphics.h"
#include "RenderableGuiObject.h"
#include "EventableGui.h"
#include "AnimateableGui.h"

#include "ScriptInterface.h"
#include "LineTokenizer.h"


// objects //
#include "TextLabel.h"

#define EVENT_SOURCE_MANAGER		1
#define EVENT_SOURCE_COLLECTION		2
#define EVENT_SOURCE_SELF			3
#define EVENT_SOURCE_MANAGINGOBJECT	4
#define EVENT_SOURCE_OTHEROBJECT	5

namespace Leviathan {

	// usability type define //
	typedef Key<int> GKey;

	struct GuiCollection{
		GuiCollection();
		GuiCollection(const wstring &name, int id, bool visible, const wstring &toggle, bool strict = false, bool exclusive = false, 
			bool enabled = true);
		~GuiCollection();

		wstring Name;
		int ID;
		bool Visible : 1;
		bool Enabled : 1;
		bool Exclusive : 1;
		bool Strict : 1;

		GKey Toggle;

		vector<BaseGuiObject*> children;

		shared_ptr<ScriptObject> Scripting;
	};

	struct GuiEventListener{
		GuiEventListener(){
			Listen = NULL;
			Tolisten = EVENT_TYPE_ERROR;
		}
		GuiEventListener(Gui::BaseEventable* listen, EVENT_TYPE tolisten){
			Listen = listen;
			Tolisten = tolisten;
		}
		Gui::BaseEventable* Listen;
		EVENT_TYPE Tolisten;
	};

#define GUI_KEYSTATE_TYPE_KEYPRESS	0
#define GUI_KEYSTATE_TYPE_KEYDOWN	1

	struct GuiReceivedKeyPress{
		GuiReceivedKeyPress(short whichcase, int key, InputEvent* receivedevent){
			Type = whichcase;
			KeyCode = key;
			MatchingEvent = receivedevent;
		}

		short Type;
		int KeyCode;
		InputEvent* MatchingEvent;
	};

	class GuiManager : public EngineComponent{
	public:
		DLLEXPORT GuiManager::GuiManager();
		DLLEXPORT GuiManager::~GuiManager();

		DLLEXPORT bool Init(AppDef* vars);
		DLLEXPORT void Release();

		DLLEXPORT void GuiTick(int mspassed);
		DLLEXPORT void AnimationTick(int mspassed);
		DLLEXPORT void Render();

		// key press receiving from listener //
		DLLEXPORT void ClearKeyReceivingState();
		DLLEXPORT void ProcessKeyPresses();
		DLLEXPORT void AddKeyPress(int keyval, InputEvent* originalevent);
		DLLEXPORT void AddKeyDown(int keyval, InputEvent* originalevent);
		DLLEXPORT bool IsEventConsumed(InputEvent** ev);
		DLLEXPORT void SetKeyContainedValuesAsConsumed(const GKey &k);

		DLLEXPORT bool GuiComboPress(GKey &key);

		// send "fake" key press to Gui //
		//DLLEXPORT void QuickSendPress(int keyinfo);

		DLLEXPORT void OnResize();

		// internal Gui element managing //
		DLLEXPORT bool AddGuiObject(BaseGuiObject* obj);
		DLLEXPORT bool AddGuiObject(BaseGuiObject* obj, int collectionid);
		DLLEXPORT void DeleteObject(int id);
		DLLEXPORT int GetObjectIndexFromId(int id);
		DLLEXPORT BaseGuiObject* GetObject(unsigned int index);

		DLLEXPORT void ExecuteGuiScript(const wstring &file);
		// TODO: actually implement this //
		DLLEXPORT void WriteGuiToFile(const wstring &file);

		DLLEXPORT bool HasForeGround();

		// event handler part //
		DLLEXPORT void AddListener(Gui::BaseEventable* receiver, EVENT_TYPE tolisten);
		DLLEXPORT void RemoveListener(Gui::BaseEventable* receiver, EVENT_TYPE type, bool all = false);

		// returns true if an object processed the event //
		DLLEXPORT bool CallEvent(Event* pEvent);
		DLLEXPORT int CallEventOnObject(Gui::BaseEventable* receive, Event* pEvent); // used to send hide events to individual objects

		// animation handler part //
		DLLEXPORT int HandleAnimation(Gui::AnimationAction* perform, Gui::GuiAnimateable* caller, int mspassed);

		// collection managing //
		DLLEXPORT void CreateCollection(GuiCollection* add);
		DLLEXPORT int GetCollection(const wstring &name, int id);
		DLLEXPORT GuiCollection* GetCollection(int id);

		DLLEXPORT void ActivateCollection(int id, bool exclusive);
		DLLEXPORT void DisableCollection(int id, bool fast);


		DLLEXPORT static GuiManager* Get();
		DLLEXPORT Graphics* GetGraph(){return graph; };

	private:
		// should be called before every vector operation //
		void UpdateArrays(); 

		// ------------------------------------ //
		Graphics* graph;
		Gui::KeyListener* MainInput;

		// Gui elements //
		vector<BaseGuiObject*> Objects;
		// used to determine when to update (and sort) //
		bool ObjectAmountChanged : 1;
		// for easy iteration on draw //
		vector<RenderableGuiObject*> NeedRendering; 


		vector<shared_ptr<GuiReceivedKeyPress>> ReceivedPresses;
		bool GKeyPressesConsumed : 1;

		BaseGuiObject* Foreground;


		// event handler //
		vector<GuiEventListener*> Listeners;

		// collections //
		vector<GuiCollection*> Collections;
		// ------------------------------------ //
		static GuiManager* staticaccess;
	};

}
#endif