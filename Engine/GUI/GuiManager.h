#ifndef LEVIATHAN_GUI_MAIN
#define LEVIATHAN_GUI_MAIN
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Input\Key.h"

#include "Application\AppDefine.h"
#include "Common\DataStoring\DataStore.h"
#include "Common\DataStoring\DataBlock.h"

#include "GUI\Components\BaseGuiObject.h"

#include "GuiKeyListener.h"
#include "Rendering\Graphics.h"
#include "Script\ScriptInterface.h"

// objects //
#include "GUI\Objects\TextLabel.h"
#include "RocketSysInternals.h"
#include "OgreRenderQueueListener.h"

#define EVENT_SOURCE_MANAGER		1
#define EVENT_SOURCE_COLLECTION		2
#define EVENT_SOURCE_SELF			3
#define EVENT_SOURCE_MANAGINGOBJECT	4
#define EVENT_SOURCE_OTHEROBJECT	5

class RenderInterfaceOgre3D;

namespace Leviathan { namespace Gui{

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

		shared_ptr<ScriptScript> Scripting;
	};

	struct GuiEventListener{
		GuiEventListener(){
			Listen = NULL;
			Tolisten = EVENT_TYPE_ERROR;
		}
		GuiEventListener(BaseGuiEventable* listen, EVENT_TYPE tolisten){
			Listen = listen;
			Tolisten = tolisten;
		}
		BaseGuiEventable* Listen;
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

	class GuiManager : public EngineComponent, public Ogre::RenderQueueListener{
	public:
		DLLEXPORT GuiManager::GuiManager();
		DLLEXPORT GuiManager::~GuiManager();

		DLLEXPORT bool Init(AppDef* vars, Graphics* graph);
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

		DLLEXPORT void OnResize();

		// internal Gui element managing //
		DLLEXPORT bool AddGuiObject(BaseGuiObject* obj);
		DLLEXPORT bool AddGuiObject(BaseGuiObject* obj, int collectionid);
		DLLEXPORT void DeleteObject(int id);
		DLLEXPORT int GetObjectIndexFromId(int id);
		DLLEXPORT BaseGuiObject* GetObject(unsigned int index);

		DLLEXPORT void ExecuteGuiScript(const wstring &file);
		// function split into peaces //
		DLLEXPORT bool LoadCollection(vector<Int2> &membermapping, vector<shared_ptr<ObjectFileObject>> &data, ObjectFileObject &collectiondata);
		// TODO: actually implement this //
		DLLEXPORT void WriteGuiToFile(const wstring &file);

		DLLEXPORT bool HasForeGround();

		// event handler part //
		DLLEXPORT void AddListener(Gui::BaseGuiEventable* receiver, EVENT_TYPE tolisten);
		DLLEXPORT void RemoveListener(Gui::BaseGuiEventable* receiver, EVENT_TYPE type, bool all = false);

		// returns true if an object processed the event //
		DLLEXPORT bool CallEvent(Event* pEvent);
		DLLEXPORT int CallEventOnObject(Gui::BaseGuiEventable* receive, Event* pEvent); // used to send hide events to individual objects

		// collection managing //
		DLLEXPORT void CreateCollection(GuiCollection* add);
		DLLEXPORT GuiCollection* GetCollection(const int &id, const wstring &name = L"");

		DLLEXPORT void SetCollectionState(const int &id, const bool &visible, const bool &exclusive = false);


		DLLEXPORT static GuiManager* Get();
		DLLEXPORT Graphics* GetGraph(){return ThisRenderer; };

		virtual void renderQueueStarted(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& skipThisInvocation);

	private:
		// should be called before every vector operation //
		void UpdateArrays(); 

		// rendering //
		void BuildProjectionMatrix(Ogre::Matrix4& projection_matrix);
		void ConfigureRenderSystem();

		// ------------------------------------ //
		Gui::KeyListener* MainInput;

		RenderInterfaceOgre3D* RocketRenderer;
		RocketSysInternals* RocketInternals;

		// TODO: implement this in OverlayMaster to hide GUI in view ports that don't need it //
		bool Visible;

		Rocket::Core::Context* WindowContext;

		Graphics* ThisRenderer;

		// Gui elements //
		vector<BaseGuiObject*> Objects;
		// used to determine when to update (and sort) //
		bool ObjectAmountChanged : 1;
		// for easy iteration on draw //
		vector<RenderableGuiObject*> NeedRendering; 


		vector<shared_ptr<GuiReceivedKeyPress>> ReceivedPresses;
		bool GKeyPressesConsumed : 1;

		BaseGuiObject* Foreground;

		// we will soon need a  GuiManager for each window //
		int ID;


		// event handler //
		vector<GuiEventListener*> Listeners;

		// collections //
		vector<GuiCollection*> Collections;
		// ------------------------------------ //
		static GuiManager* staticaccess;

	public:
		// ------------------ animation handler part ------------------ //
		template<class TC>
		DLLEXPORT int HandleAnimation(AnimationAction* perform, TC* caller, const int &mspassed){
			switch(perform->GetType()){
			case GUI_ANIMATION_HIDE:
				{
					// set visibility //
					caller->SetHiddenState(true);

					// always finishes //
					return 1;
				}
			case GUI_ANIMATION_SHOW:
				{
					// set visibility //
					caller->SetHiddenState(false);

					// always finishes //
					return 1;
				}
			case GUI_ANIMATION_MOVE:
				{
					GuiAnimationTypeMove* data = static_cast<GuiAnimationTypeMove*>(perform->Data);

					float x = caller->GetValue(GUI_ANIMATEABLE_SEMANTIC_X);
					float y = caller->GetValue(GUI_ANIMATEABLE_SEMANTIC_Y);

					bool finished = false;

					float amount = mspassed*(data->Speed/1000.f)+0.0001f;

					// switch here based on move mode
					switch(data->Priority){
					case GUI_ANIMATION_TYPEMOVE_PRIORITY_X:
						{
							// this might be good to be done with 2d vector //
							Float2 movementvector(data->X-x, data->Y-y);

							// move x and maybe y according to vector //
							if(movementvector.X == 0)
								y += movementvector.Y*amount;
							else
								x += movementvector.X*amount;

							// check did we go over it //
							Float2 checkvector(data->X-x, data->Y-y);

							if(((movementvector.X < 0) != (checkvector.X < 0))){
								// went over the target, set to the target //
								x = data->X;
							}
							if(((movementvector.Y < 0) != (checkvector.Y < 0))){
								// went over the target, set to the target //
								y = data->Y;
							}

							// check for finishing //
							if((x == data->X) && (y == data->Y)){
								finished = true;
								break;
							}
						}
						break;
					case GUI_ANIMATION_TYPEMOVE_PRIORITY_Y:
						{
							// this might be good to be done with 2d vector //
							Float2 movementvector(data->X-x, data->Y-y);

							// move x and maybe y according to vector //
							if(movementvector.Y == 0)
								x += movementvector.X*amount;
							else
								y += movementvector.Y*amount;

							// check did we go over it //
							Float2 checkvector(data->X-x, data->Y-y);

							if(((movementvector.X < 0) != (checkvector.X < 0))){
								// went over the target, set to the target //
								x = data->X;
							}
							if(((movementvector.Y < 0) != (checkvector.Y < 0))){
								// went over the target, set to the target //
								y = data->Y;
							}

							// check for finishing //
							if((x == data->X) && (y == data->Y)){
								finished = true;
								break;
							}

						}
						break;
					case GUI_ANIMATION_TYPEMOVE_PRIORITY_SLOPE:
						{
							// this might be good to be done with 2d vector //
							Float2 movementvector(data->X-x, data->Y-y);
							// animation finish if no distance //
							if(movementvector.Length() == 0){
								finished = true;
								break;
							}

							// normalize so that speed is constant //
							movementvector = movementvector.Normalize();

							// move x and y according to vector //
							x += movementvector.X*amount;
							y += movementvector.Y*amount;

							// check did we go over it //
							Float2 checkvector(data->X-x, data->Y-y);

							if(((movementvector.X < 0) != (checkvector.X < 0))){
								// went over the target, set to the target //
								x = data->X;
								y = data->Y;

								finished = true;
								break;
							}


							// check for finishing //
							if((x == data->X) && (y == data->Y)){
								finished = true;
								break;
							}

						}
						break;
					}

					// send updated values //
					caller->SetValue(GUI_ANIMATEABLE_SEMANTIC_X, x);
					caller->SetValue(GUI_ANIMATEABLE_SEMANTIC_Y, y);

					if(finished){
						return 1;

					} else {
						return 0;
					}
				}
				break;
			default:
				return 404; // unrecognized type //
			}
		}
	};

}}
#endif