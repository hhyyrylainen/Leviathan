#include "PongIncludes.h"
// ------------------------------------ //
#ifndef PONG_INPUTCONTROLLER
#include "GameInputController.h"
#endif
using namespace Pong;
// ------------------------------------ //
Pong::GameInputController::GameInputController(){
	// Create the default control groups //
	_SetupControlGroups();
}

Pong::GameInputController::~GameInputController(){

}
// ------------------------------------ //




// ------------------------------------ //
void Pong::GameInputController::_SetupControlGroups(){

	GroupToKeyMap = boost::assign::map_list_of(PLAYERCONTROLS_WASD, boost::assign::map_list_of
		(Window::ConvertWstringToOISKeyCode(L"A"), CONTROLKEYACTION_LEFT)
		(Window::ConvertWstringToOISKeyCode(L"D"), CONTROLKEYACTION_RIGHT)
		(Window::ConvertWstringToOISKeyCode(L"W"), CONTROLKEYACTION_POWERUPUP)
		(Window::ConvertWstringToOISKeyCode(L"S"), CONTROLKEYACTION_POWERUPDOWN)
		)(PLAYERCONTROLS_ARROWS, boost::assign::map_list_of
		(Window::ConvertWstringToOISKeyCode(L"LEFTARROW"), CONTROLKEYACTION_LEFT)
		(Window::ConvertWstringToOISKeyCode(L"RIGHTARROW"), CONTROLKEYACTION_RIGHT)
		(Window::ConvertWstringToOISKeyCode(L"UPARROW"), CONTROLKEYACTION_POWERUPUP)
		(Window::ConvertWstringToOISKeyCode(L"DOWNARROW"), CONTROLKEYACTION_POWERUPDOWN)
		)(PLAYERCONTROLS_IJKL, boost::assign::map_list_of
		(Window::ConvertWstringToOISKeyCode(L"J"), CONTROLKEYACTION_LEFT)
		(Window::ConvertWstringToOISKeyCode(L"L"), CONTROLKEYACTION_RIGHT)
		(Window::ConvertWstringToOISKeyCode(L"I"), CONTROLKEYACTION_POWERUPUP)
		(Window::ConvertWstringToOISKeyCode(L"K"), CONTROLKEYACTION_POWERUPDOWN)
		)(PLAYERCONTROLS_NUMPAD, boost::assign::map_list_of
		(Window::ConvertWstringToOISKeyCode(L"NUMPAD4"), CONTROLKEYACTION_LEFT)
		(Window::ConvertWstringToOISKeyCode(L"NUMPAD6"), CONTROLKEYACTION_RIGHT)
		(Window::ConvertWstringToOISKeyCode(L"NUMPAD8"), CONTROLKEYACTION_POWERUPUP)
		(Window::ConvertWstringToOISKeyCode(L"NUMPAD5"), CONTROLKEYACTION_POWERUPDOWN)
		);
}

std::map<OIS::KeyCode, CONTROLKEYACTION>& Pong::GameInputController::MapControlsToKeyGrouping(PLAYERCONTROLS controls) THROWS{

	return GroupToKeyMap[controls];
}

