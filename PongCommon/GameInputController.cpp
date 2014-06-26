#include "PongIncludes.h"
// ------------------------------------ //
#ifndef PONG_INPUTCONTROLLER
#include "GameInputController.h"
#endif
#ifdef PONG_VERSION
#include "PongGame.h"
#include "PongNetHandler.h"
#else
#include "PongServer.h"
#include "PongServerNetworking.h"
#endif // PONG_VERSION
using namespace Pong;
// ------------------------------------ //
Pong::GameInputController::GameInputController() : NetworkedInputHandler(PongInputFactory::Get(), 
#ifdef PONG_VERSION
	PongGame::Get()->GetInterface()
#else
	PongServer::Get()->GetServerNetworkInterface()
#endif // PONG_VERSION
	)
{
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
// ------------------ PongInputFactory ------------------ //
DLLEXPORT unique_ptr<NetworkedInput> Pong::PongInputFactory::CreateNewInstanceForLocalStart(int inputid, bool isclient){
	throw std::exception("The method or operation is not implemented.");
}

DLLEXPORT unique_ptr<NetworkedInput> Pong::PongInputFactory::CreateNewInstanceForReplication(int inputid){
	throw std::exception("The method or operation is not implemented.");
}

DLLEXPORT void Pong::PongInputFactory::NoLongerNeeded(NetworkedInput &todiscard){
	throw std::exception("The method or operation is not implemented.");
}
// ------------------------------------ //
PongInputFactory* Pong::PongInputFactory::Get(){
	return Staticinstance;
}

PongInputFactory* Pong::PongInputFactory::Staticinstance = new PongInputFactory();
