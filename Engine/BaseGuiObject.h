#ifndef LEVIATHAN_BASE_GUI
#define LEVIATHAN_BASE_GUI
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ScriptObject.h"


#define GUI_OBJECT_LEVEL_BASE			1
//#define GUI_OBJECT_LEVEL_AUTOUPDATEABLE	2
#define GUI_OBJECT_LEVEL_RENDERABLE		3
#define GUI_OBJECT_LEVEL_EVENTABLE		4
#define GUI_OBJECT_LEVEL_ANIMATEABLE	5

namespace Leviathan{

	class BaseGuiObject : public Object{
	public:
		DLLEXPORT BaseGuiObject::BaseGuiObject();
		DLLEXPORT virtual BaseGuiObject::~BaseGuiObject();

		DLLEXPORT int CompareType(int compare);

		int Objecttype;
		bool HigherLevel;
		int ObjectLevel;

		int ID;

		shared_ptr<ScriptObject> Scripting;
	protected:

	};

}
#endif