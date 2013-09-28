#ifndef LEVIATHAN_GUI_MAIN
#define LEVIATHAN_GUI_MAIN
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Application\AppDefine.h"
#include "GUI\BaseGuiObject.h"


// objects //
#include "RocketSysInternals.h"
#include "OgreRenderQueueListener.h"



class RenderInterfaceOgre3D;


namespace Leviathan {
	class Graphics;
namespace Gui{

	class GuiCollection;
	class GuiLoadedSheet;


	class GuiManager : public EngineComponent, public Ogre::RenderQueueListener{
	public:
		DLLEXPORT GuiManager::GuiManager();
		DLLEXPORT GuiManager::~GuiManager();

		DLLEXPORT bool Init(AppDef* vars, Graphics* graph);
		DLLEXPORT void Release();

		DLLEXPORT void GuiTick(int mspassed);
		DLLEXPORT void Render();

		// key press receiving from listener //
		DLLEXPORT bool ProcessKeyDown(OIS::KeyCode key, int specialmodifiers);


		DLLEXPORT void OnResize();

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


		// collection managing //
		DLLEXPORT void AddCollection(GuiCollection* add);
		DLLEXPORT GuiCollection* GetCollection(const int &id, const wstring &name = L"");

		// events //
		DLLEXPORT bool CallEvent(Event* pEvent);
		DLLEXPORT int CallEventOnObject(BaseGuiObject* receive, Event* pEvent);

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

		RenderInterfaceOgre3D* RocketRenderer;
		RocketSysInternals* RocketInternals;

		// TODO: implement this in OverlayMaster to hide GUI in view ports that don't need it //
		bool Visible;

		Rocket::Core::Context* WindowContext;
		// stored to use cursor visibility //
		Rocket::Core::ElementDocument* Cursor;


		Graphics* ThisRenderer;

		// Gui elements //
		vector<BaseGuiObject*> Objects;
		// used to determine when to update (and sort) //
		bool ObjectAmountChanged : 1;


		// we will soon need a GuiManager for each window //
		int ID;

		// collections //
		std::vector<GuiCollection*> Collections;
		std::vector<shared_ptr<GuiLoadedSheet>> GuiSheets;
		// ------------------------------------ //
		static GuiManager* staticaccess;
	};

}}
#endif