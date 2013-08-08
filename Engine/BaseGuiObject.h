#ifndef LEVIATHAN_GUI_BASEOBJECT
#define LEVIATHAN_GUI_BASEOBJECT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ScriptScript.h"
#include "ObjectFileObject.h"
#include "ReferenceCounted.h"

namespace Leviathan{ namespace Gui{
	// object types //
#define GUIOBJECTTYPE_TEXTLABEL		1000


	enum GUIOBJECTHAS {
		GUIOBJECTHAS_BASE = 0x1, 
		GUIOBJECTHAS_RENDERABLE = 0x2, 
		GUIOBJECTHAS_EVENTABLE = 0x4, 
		GUIOBJECTHAS_ANIMATEABLE = 0x8,
		GUIOBJECTHAS_POSITIONABLE = 0x10, 
		GUIOBJECTHAS_BASEGRAPHICAL = 0x20
		//0x40
		//0x80 // first byte full
		//0x100
		//0x200
		//0x400
		//0x800
		//0x1000
		//0x2000
		//0x4000
		//0x8000 // second byte full (int range might stop here(
		//0x10000
		//0x20000
		//0x40000
		//0x80000
		//0x100000
		//0x200000
		//0x400000
		//0x800000 // third byte full
		//0x1000000
		//0x2000000
		//0x4000000
		//0x8000000
		//0x10000000
		//0x20000000
		//0x40000000
		//0x80000000 // fourth byte full (will need QWORD here)
		//0x100000000
	};

	class GuiManager;
	// this class' functions are thread safe (*should* be) //// mutex for reference counting and possibly for other functions for thread safety //
	class BaseGuiObject : public Object, public ReferenceCounted{
	public:
		DLLEXPORT BaseGuiObject::BaseGuiObject(GuiManager* owner, const int &flags, const int &id, const int &objecttype, 
			shared_ptr<ScriptScript> script = NULL);
		DLLEXPORT virtual BaseGuiObject::~BaseGuiObject();

		DLLEXPORT static bool LoadFromFileStructure(GuiManager* owner, vector<BaseGuiObject*> &tempobjects, vector<Int2> &idmappairs, 
			ObjectFileObject& dataforthis);

		// this should be const, but it isn't because I like assignment operators //
		/*const*/ int ObjectFlags;
		
		DLLEXPORT inline int GetID(){
			return ID;
		}

	protected:

		int ID;
		int ObjectType;


		GuiManager* OwningInstance;
		shared_ptr<ScriptScript> Scripting;
	};

}}
#endif