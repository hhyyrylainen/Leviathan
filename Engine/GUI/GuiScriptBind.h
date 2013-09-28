#ifndef LEVIATHAN_GUISCRIPTBIND
#define LEVIATHAN_GUISCRIPTBIND


#include "angelscript.h"
#include "GuiAnimation.h"
#include "GuiScriptInterface.h"
#include "BaseGuiObject.h"

bool BindGUIObjects(asIScriptEngine* engine){

	// bind GuiCollection action, this is released by gui object //
	if(engine->RegisterObjectType("GuiCollection", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	//if(engine->RegisterObjectBehaviour("GuiCollection", asBEHAVE_FACTORY, "AnimationAction@ f(float xtarget, float ytarget, int whichfirst, "
	//	"float speed, bool allowsimult = false)", asFUNCTION(Gui::CreateAnimationActionMove), asCALL_CDECL) < 0){
	//		Logger::Get()->Error(L"ScriptExecutor: AngelScript: register script interface: failed, file: " __WFILE__ L", line: " __SWLINE__, false);
	//}
	//if(engine->RegisterObjectBehaviour("GuiCollection", asBEHAVE_FACTORY, "AnimationAction@ f(bool visible)", 
	//	asFUNCTION(Gui::CreateAnimationActionVisibility), asCALL_CDECL) < 0){
	//		Logger::Get()->Error(L"ScriptExecutor: AngelScript: register script interface: failed, file: " __WFILE__ L", line: " __SWLINE__, false);
	//}


	// bind BaseGuiObject //
	if(engine->RegisterObjectType("BaseGuiObject", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	// no factory function to prevent scripts from creating these functions //

	if(engine->RegisterObjectBehaviour("BaseGuiObject", asBEHAVE_ADDREF, "void f()", asMETHOD(Gui::BaseGuiObject, AddRefProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("BaseGuiObject", asBEHAVE_RELEASE, "void f()", asMETHOD(Gui::BaseGuiObject, ReleaseProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("BaseGuiObject", "int GetID()", asMETHOD(Gui::BaseGuiObject, GetID), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("BaseGuiObject", "int SetInternalElementRML(string rmlcode)", asMETHOD(Gui::BaseGuiObject, SetInternalRMLWrapper), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("BaseGuiObject", "VariableBlock@ GetAndPopFirstUpdated()", asMETHOD(Gui::BaseGuiObject, SetInternalRMLWrapper), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	

	


	return true;
}


bool BindGUIScriptCommon(asIScriptEngine* engine){
	// register GUI_ANIMATION_ACTION enum and it's values //
	if(engine->RegisterEnum("GUI_ANIMATION_ACTION") < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: register global failed in file " __WFILE__ L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	if(engine->RegisterEnumValue("GUI_ANIMATION_ACTION", "GUI_ANIMATION_ERROR", Gui::GUI_ANIMATION_ERROR) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: register global failed in file " __WFILE__ L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	if(engine->RegisterEnumValue("GUI_ANIMATION_ACTION", "GUI_ANIMATION_FADE_OUT", Gui::GUI_ANIMATION_FADE_OUT) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: register global failed in file " __WFILE__ L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	if(engine->RegisterEnumValue("GUI_ANIMATION_ACTION", "GUI_ANIMATION_GENERAL", Gui::GUI_ANIMATION_GENERAL) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: register global failed in file " __WFILE__ L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	if(engine->RegisterEnumValue("GUI_ANIMATION_ACTION", "GUI_ANIMATION_FADE_IN", Gui::GUI_ANIMATION_FADE_IN) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: register global failed in file " __WFILE__ L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	if(engine->RegisterEnumValue("GUI_ANIMATION_ACTION", "GUI_ANIMATION_MOVE", Gui::GUI_ANIMATION_MOVE) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: register global failed in file " __WFILE__ L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	if(engine->RegisterEnumValue("GUI_ANIMATION_ACTION", "GUI_ANIMATION_GLOW", Gui::GUI_ANIMATION_GLOW) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: register global failed in file " __WFILE__ L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	if(engine->RegisterEnumValue("GUI_ANIMATION_ACTION", "GUI_ANIMATION_HIDE", Gui::GUI_ANIMATION_HIDE) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: register global failed in file " __WFILE__ L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	if(engine->RegisterEnumValue("GUI_ANIMATION_ACTION", "GUI_ANIMATION_SHOW", Gui::GUI_ANIMATION_SHOW) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: register global failed in file " __WFILE__ L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	// end of Gui enum action enum
	// register Gui type move Enum //
	if(engine->RegisterEnum("GUI_ANIMATION_TYPEMOVE_PRIORITY") < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: register global failed in file " __WFILE__ L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	if(engine->RegisterEnumValue("GUI_ANIMATION_TYPEMOVE_PRIORITY", "GUI_ANIMATION_TYPEMOVE_PRIORITY_X", GUI_ANIMATION_TYPEMOVE_PRIORITY_X) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: register global failed in file " __WFILE__ L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	if(engine->RegisterEnumValue("GUI_ANIMATION_TYPEMOVE_PRIORITY", "GUI_ANIMATION_TYPEMOVE_PRIORITY_Y", GUI_ANIMATION_TYPEMOVE_PRIORITY_Y) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: register global failed in file " __WFILE__ L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	if(engine->RegisterEnumValue("GUI_ANIMATION_TYPEMOVE_PRIORITY", "GUI_ANIMATION_TYPEMOVE_PRIORITY_SLOPE", GUI_ANIMATION_TYPEMOVE_PRIORITY_SLOPE) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: register global failed in file " __WFILE__ L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	// end of Gui type move enum //


	// succeeded //
	return true;
}


void RegisterGUIScriptTypeNames(asIScriptEngine* engine, std::map<int, wstring> &typeids){

	typeids.insert(make_pair(engine->GetTypeIdByDecl("GuiCollection"), L"GuiCollection"));
	typeids.insert(make_pair(engine->GetTypeIdByDecl("BaseGuiObject"), L"BaseGuiObject"));
}

#endif