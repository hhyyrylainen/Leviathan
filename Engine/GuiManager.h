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
#include "BaseGuiObject.h"

#include "GuiKeyListener.h"
#include "Rendering\Graphics.h"
#include "RenderableGuiObject.h"
#include "EventableGui.h"
#include "AnimateableGui.h"

#include "ScriptInterface.h"


// objects //
#include "TextLabel.h"

#define EVENT_SOURCE_MANAGER		1
#define EVENT_SOURCE_COLLECTION		2
#define EVENT_SOURCE_SELF			3
#define EVENT_SOURCE_MANAGINGOBJECT	4
#define EVENT_SOURCE_OTHEROBJECT	5

namespace Leviathan {
	struct GuiCollection{
		GuiCollection(){
			Name = Misc::GetErrString();
			ID = -1;
			Visible = false;
			Enabled = false;
		}
		GuiCollection(wstring name, int id, bool visible, wstring toggle, bool strict = false, bool exclusive = false, bool enabled = true){
			Name = name;
			ID = id;
			Visible = visible;
			Enabled = enabled;
			Exclusive = exclusive;

			wchar_t chara = L'\n';
			KEYSPECIAL additional = KEYSPECIAL_NONE;
			bool Shift = false;
			bool Alt = false;
			bool Ctrl = false;

			if(Misc::CountOccuranceWstring(toggle, L"+")){
				vector<wstring> words;
				Misc::CutWstring(toggle, L"+",words);



				for(unsigned int i = 0; i < words.size(); i++){
					if(words[i].size() == 0)
						continue;
					if(i+1 >= words.size()){
						// last must be character
						chara = words[i][0];
					}
					if(Misc::WstringCompareInsensitive(words[i], L"alt")){
						Alt = true;
						continue;
					}
					if(Misc::WstringCompareInsensitive(words[i], L"shift")){
						Shift = true;
						continue;
					}
					if(Misc::WstringCompareInsensitive(words[i], L"ctrl")){
						Ctrl = true;
						continue;
					}
				}

			} else {
				if(toggle.size() != 0)
					chara = toggle[0];
			}
			additional = Key::ConstructSpecial(Shift, Alt, Ctrl);
			Toggle = Key(chara, additional);

			Strict = strict;
		}
		~GuiCollection(){
			// release script //
			SAFE_DELETE(Scripting);

			// possibly release children here //

		}

		wstring Name;
		int ID;
		bool Visible;
		bool Enabled;
		bool Exclusive;

		bool Strict;
		Key Toggle;
		vector<BaseGuiObject*> children;

		ScriptObject* Scripting;

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

	class GuiManager : public EngineComponent{
	public:
		DLLEXPORT GuiManager::GuiManager();
		DLLEXPORT GuiManager::~GuiManager();


		DLLEXPORT bool Init(AppDef* vars);
		DLLEXPORT void Release();

		DLLEXPORT void GuiTick(int mspassed);
		DLLEXPORT void AnimationTick(int mspassed);
		DLLEXPORT void Render();

		DLLEXPORT void QuickSendPress(int keyinfo);

		DLLEXPORT void OnResize();

		
		DLLEXPORT bool AddGuiObject(BaseGuiObject* obj);
		DLLEXPORT bool AddGuiObject(BaseGuiObject* obj, int collectionid);

		DLLEXPORT void DeleteObject(int id);
		DLLEXPORT int GetObjectIndexFromId(int id);
		DLLEXPORT BaseGuiObject* GetObject(unsigned int index);

		DLLEXPORT void ExecuteGuiScript(const wstring &file);
		DLLEXPORT void WriteGuiToFile(wstring file);


		DLLEXPORT static GuiManager* Get();
		DLLEXPORT Graphics* GetGraph(){return graph; };
		DLLEXPORT bool HasForeGround();

		DLLEXPORT void AddKeyPress(int keyval);
		DLLEXPORT void AddKeyDown(int keyval);

		// event handler part //
		DLLEXPORT void AddListener(Gui::BaseEventable* receiver, EVENT_TYPE tolisten);
		DLLEXPORT void RemoveListener(Gui::BaseEventable* receiver, EVENT_TYPE type, bool all = false);

		DLLEXPORT void CallEvent(Event* pEvent);
		DLLEXPORT int CallEventOnObject(Gui::BaseEventable* receive, Event* pEvent); // used to send hide events to invidiual objects

		// animation handler part //
		DLLEXPORT int HandleAnimation(Gui::AnimationAction* perform, Gui::GuiAnimateable* caller, int mspassed);

		// collection managing //
		DLLEXPORT void CreateCollection(GuiCollection* add);
		DLLEXPORT int GetCollection(wstring name, int id);
		DLLEXPORT GuiCollection* GetCollection(int id);

		DLLEXPORT void GuiComboPress(Key key);

		DLLEXPORT void ActivateCollection(int id, bool exclusive);
		DLLEXPORT void DisableCollection(int id, bool fast);

		DLLEXPORT ScriptCaller* GetCollectionCall(GuiCollection* customize);


		

	private:
		void UpdateArrays(); // should be called before every vector operation

		// ---------------------- //
		Graphics* graph;

		Gui::KeyListener* MainInput;

		vector<BaseGuiObject*> Objects;
		vector<RenderableGuiObject*> NeedRendering; // for easy iteration on draw //

		vector<int> KeyPresses;
		vector<int> KeysDown;

		BaseGuiObject* Foreground;

		bool ObjectAmountChanged;

		ScriptCaller* CollectionCall;


		// event handler //
		vector<GuiEventListener*> Listeners;


		// collections //
		vector<GuiCollection*> Collections;

		// ----------------------- //
		static GuiManager* staticaccess;
	};

}
#endif