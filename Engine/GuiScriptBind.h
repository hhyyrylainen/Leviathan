#ifndef LEVIATHAN_GUISCRIPTBIND
#define LEVIATHAN_GUISCRIPTBIND
{
	if(engine->RegisterGlobalFunction("bool Gui_SetObjectText(int ID, string toset, bool doupdate = true)", asFUNCTION(Gui_SetObjectText), asCALL_CDECL) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: registerglobal failed in file "+Convert::StringToWstringNonRef(__FILE__)+L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	// register GUI_ANIMATION_ACTION enum and it's values //
	if(engine->RegisterEnum("GUI_ANIMATION_ACTION") < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: registerglobal failed in file "+Convert::StringToWstringNonRef(__FILE__)+L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	if(engine->RegisterEnumValue("GUI_ANIMATION_ACTION", "GUI_ANIMATION_ERROR", 0) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: registerglobal failed in file "+Convert::StringToWstringNonRef(__FILE__)+L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	if(engine->RegisterEnumValue("GUI_ANIMATION_ACTION", "GUI_ANIMATION_FADE_OUT", 1) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: registerglobal failed in file "+Convert::StringToWstringNonRef(__FILE__)+L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	if(engine->RegisterEnumValue("GUI_ANIMATION_ACTION", "GUI_ANIMATION_GENERAL", 2) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: registerglobal failed in file "+Convert::StringToWstringNonRef(__FILE__)+L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	if(engine->RegisterEnumValue("GUI_ANIMATION_ACTION", "GUI_ANIMATION_FADE_IN", 3) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: registerglobal failed in file "+Convert::StringToWstringNonRef(__FILE__)+L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	if(engine->RegisterEnumValue("GUI_ANIMATION_ACTION", "GUI_ANIMATION_MOVE", 4) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: registerglobal failed in file "+Convert::StringToWstringNonRef(__FILE__)+L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	if(engine->RegisterEnumValue("GUI_ANIMATION_ACTION", "GUI_ANIMATION_GLOW", 5) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: registerglobal failed in file "+Convert::StringToWstringNonRef(__FILE__)+L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	if(engine->RegisterEnumValue("GUI_ANIMATION_ACTION", "GUI_ANIMATION_HIDE", 6) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: registerglobal failed in file "+Convert::StringToWstringNonRef(__FILE__)+L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	if(engine->RegisterEnumValue("GUI_ANIMATION_ACTION", "GUI_ANIMATION_SHOW", 7) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: registerglobal failed in file "+Convert::StringToWstringNonRef(__FILE__)+L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	// end of gui enum action enum
	// register gui typemove Enum //
	//GUI_ANIMATION_TYPEMOVE_PRIORITY
	if(engine->RegisterEnum("GUI_ANIMATION_TYPEMOVE_PRIORITY") < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: registerglobal failed in file "+Convert::StringToWstringNonRef(__FILE__)+L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	if(engine->RegisterEnumValue("GUI_ANIMATION_TYPEMOVE_PRIORITY", "GUI_ANIMATION_TYPEMOVE_PRIORITY_X", GUI_ANIMATION_TYPEMOVE_PRIORITY_X) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: registerglobal failed in file "+Convert::StringToWstringNonRef(__FILE__)+L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	if(engine->RegisterEnumValue("GUI_ANIMATION_TYPEMOVE_PRIORITY", "GUI_ANIMATION_TYPEMOVE_PRIORITY_Y", GUI_ANIMATION_TYPEMOVE_PRIORITY_Y) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: registerglobal failed in file "+Convert::StringToWstringNonRef(__FILE__)+L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	if(engine->RegisterEnumValue("GUI_ANIMATION_TYPEMOVE_PRIORITY", "GUI_ANIMATION_TYPEMOVE_PRIORITY_BOTH", GUI_ANIMATION_TYPEMOVE_PRIORITY_BOTH) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: registerglobal failed in file "+Convert::StringToWstringNonRef(__FILE__)+L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	if(engine->RegisterEnumValue("GUI_ANIMATION_TYPEMOVE_PRIORITY", "GUI_ANIMATION_TYPEMOVE_PRIORITY_SLOPE", GUI_ANIMATION_TYPEMOVE_PRIORITY_SLOPE) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: registerglobal failed in file "+Convert::StringToWstringNonRef(__FILE__)+L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	if(engine->RegisterEnumValue("GUI_ANIMATION_TYPEMOVE_PRIORITY", "GUI_ANIMATION_TYPEMOVE_PRIORITY_SLOPEY", GUI_ANIMATION_TYPEMOVE_PRIORITY_SLOPEY) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: registerglobal failed in file "+Convert::StringToWstringNonRef(__FILE__)+L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	if(engine->RegisterEnumValue("GUI_ANIMATION_TYPEMOVE_PRIORITY", "GUI_ANIMATION_TYPEMOVE_PRIORITY_SMOOTH_DIVIDE", GUI_ANIMATION_TYPEMOVE_PRIORITY_SMOOTH_DIVIDE) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: registerglobal failed in file "+Convert::StringToWstringNonRef(__FILE__)+L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	// end of Gui type move enum //
	// Gui animation call binds //
	if(engine->RegisterGlobalFunction("int Gui_QueueAnimationActionMove(int ID, int xtarget, int ytarget, int whichfirst, float speed, bool allowsimult = false, int special = 0)",
		asFUNCTION(Gui_QueueAnimationActionMove), asCALL_CDECL) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: registerglobal failed in file "+Convert::StringToWstringNonRef(__FILE__)+L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	if(engine->RegisterGlobalFunction("int Gui_QueueAnimationActionVisibility(int ID, bool visible)",
		asFUNCTION(Gui_QueueAnimationActionVisibility), asCALL_CDECL) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: registerglobal failed in file "+Convert::StringToWstringNonRef(__FILE__)+L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	if(engine->RegisterGlobalFunction("int Gui_QueuedAnimationClear(int ID)",
		asFUNCTION(Gui_QueuedAnimationClear), asCALL_CDECL) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: registerglobal failed in file "+Convert::StringToWstringNonRef(__FILE__)+L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	if(engine->RegisterGlobalFunction("int Gui_QueuedAnimationUpdate(int ID, int passedms)",
		asFUNCTION(Gui_QueuedAnimationUpdate), asCALL_CDECL) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: registerglobal failed in file "+Convert::StringToWstringNonRef(__FILE__)+L" on line "+Convert::IntToWstring(__LINE__), false);
		return false;
	}
//	int Gui_QueueAnimationActionMove(int ID, int xtarget, int ytarget, int whichfirst, float speed, bool allowsimult = false, int special = 0);
//int Gui_QueueAnimationActionVisibility(int ID, bool visible);
//int Gui_QueuedAnimationClear(int ID);
//int Gui_QueuedAnimationUpdate(int ID, int passedms);

	// end of gui animation call binds //
}
#endif