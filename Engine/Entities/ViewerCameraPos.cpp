#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_OBJECT_CAMERAPOS
#include "ViewerCameraPos.h"
#endif
using namespace Leviathan;
// ------------------------------------ //

ViewerCameraPos::ViewerCameraPos() : Orientation(0), Position(0){

	// set all to zeros //
	FrameTime = forward = backward = left = right = zup = zdown = xmoved = ymoved = 0;
	m_SideWays = m_Forward = m_Vertical = 0;

	// mouse //
	MouseXSensitivity = 0.2f;
	MouseYSensitivity = 0.15f;


	Type = OBJECT_TYPE_FLYING_CAMERA;
	ID = IDFactory::GetID();

	MainCameraListening = false;
}

Leviathan::ViewerCameraPos::~ViewerCameraPos(){
	if(MainCameraListening){
		// needs to stop listening //
		KeyPressManager::Get()->Unregister((InputReceiver*)this);
	}
}

void ViewerCameraPos::UpdatePos(int mspassed){
	FrameTime = mspassed/4.f;
	if(ymoved || xmoved){

		// set pitch
		Orientation.Y -= ymoved/2.f*MouseYSensitivity*FrameTime*0.5f;
		Orientation.X -= xmoved/2.f*MouseXSensitivity*FrameTime*0.5f;

		if(!Misc::IsFiniteNumber(Orientation.X)){
			Orientation.X = 0;
		}
		if(!Misc::IsFiniteNumber(Orientation.Y)){
			Orientation.Y  = 0;
		}

		if(Orientation.Y  > 90){
			Orientation.Y  = 90;
		} else if (Orientation.Y  < -90)
			Orientation.Y  = -90;

		while(Orientation.X > 360){
			float over = Orientation.X-360.f;

			if(!Misc::IsFiniteNumber(over)){
				Orientation.X = 0;
			} else {
				Orientation.X = over;
			}
		}
		while (Orientation.X < 0){
			float under = Orientation.X;

			if(!Misc::IsFiniteNumber(under)){
				Orientation.X = 0;
			} else {
				Orientation.X = 360.0f+under;
			}
		}
	}

	SideWays(m_SideWays);
	Forward(m_Forward);
	Vertical(m_Vertical);
}

void ViewerCameraPos::SideWays(int dir){
	if(dir > 0){
		RollValueTowards(right, FrameTime*0.01f, true, 0.08f);
		RollValueTowards(left, -FrameTime*0.03f, false, 0.f);
	} else if(dir == 0){
		// slow down //
		RollValueTowards(right, -FrameTime*0.03f, false, 0.f);
		RollValueTowards(left, -FrameTime*0.03f, false, 0.f);
	
	} else {
		RollValueTowards(left, FrameTime*0.01f, true, 0.08f);
		RollValueTowards(right, -FrameTime*0.03f, false, 0.f);
	}
	
	// actual movement calculations here //
	if(right > left){
		Position.X += (float)(sin(Convert::DegreesToRadians(Orientation.X+90))*right)*DEFAULTMOVEMENTMODIFIER;
		Position.Z += (float)(cos(Convert::DegreesToRadians(Orientation.X+90))*right)*DEFAULTMOVEMENTMODIFIER;
	} else if(left > right){
		Position.X += (float)(sin(Convert::DegreesToRadians(Orientation.X-90))*left)*DEFAULTMOVEMENTMODIFIER;
		Position.Z += (float)(cos(Convert::DegreesToRadians(Orientation.X-90))*left)*DEFAULTMOVEMENTMODIFIER;
	}
}
void ViewerCameraPos::Forward(int dir){
	if(dir > 0){
		RollValueTowards(forward, FrameTime*0.01f, true, 0.1f);
		RollValueTowards(backward, -FrameTime*0.03f, false, 0.f);
	} else if(dir == 0){
		// slow down //
		RollValueTowards(forward, -FrameTime*0.03f, false, 0.f);
		RollValueTowards(backward, -FrameTime*0.03f, false, 0.f);
	
	} else {
		RollValueTowards(backward, FrameTime*0.01f, true, 0.1f);
		RollValueTowards(forward, -FrameTime*0.03f, false, 0.f);
	}

	if(forward > backward){
		Position.X -= (float)(sin(Convert::DegreesToRadians(Orientation.X))*forward)*DEFAULTMOVEMENTMODIFIER;
		Position.Z -= (float)(cos(Convert::DegreesToRadians(Orientation.X))*forward)*DEFAULTMOVEMENTMODIFIER;
		Position.Y += (float)(sin(Convert::DegreesToRadians(Orientation.Y))*forward)*DEFAULTMOVEMENTMODIFIER;
	} else if(backward > forward){
		Position.X += (float)(sin(Convert::DegreesToRadians(Orientation.X))*backward)*DEFAULTMOVEMENTMODIFIER;
		Position.Z += (float)(cos(Convert::DegreesToRadians(Orientation.X))*backward)*DEFAULTMOVEMENTMODIFIER;
		Position.Y -= (float)(sin(Convert::DegreesToRadians(Orientation.Y))*backward)*DEFAULTMOVEMENTMODIFIER;
	}
}

void ViewerCameraPos::Vertical(int dir){
	if(dir > 0){
		RollValueTowards(zup, FrameTime*0.01f, true, 0.10f);
		RollValueTowards(zdown, -FrameTime*0.04f, false, 0.f);
	} else if(dir == 0){
		// slow down //
		RollValueTowards(zup, -FrameTime*0.04f, false, 0.f);
		RollValueTowards(zdown, -FrameTime*0.04f, false, 0.f);
	
	} else {
		RollValueTowards(zdown, FrameTime*0.01f, true, 0.10f);
		RollValueTowards(zup, -FrameTime*0.04f, false, 0.f);
	}
	
	Position.Y += zup*DEFAULTMOVEMENTMODIFIER;
	Position.Y -= zdown*DEFAULTMOVEMENTMODIFIER;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ViewerCameraPos::OnEvent(InputEvent** pEvent, InputReceiver* pending){

	if(pending == this){
		// shouldn't happen, since this doesn't use pending //
		DEBUG_BREAK;
	}

	// update control based on event //

	switch((*pEvent)->GetType()){
	case EVENT_TYPE_EVENT_SEQUENCE_BEGIN:
		{
			// clear all //
			xmoved = 0;
			ymoved = 0;
			m_SideWays = 0;
			m_Forward = 0;
			m_Vertical = 0;
		}
		return false;
	case EVENT_TYPE_MOUSEMOVED:
		{

			xmoved = (float)(((Int2*)(*pEvent)->Data))->X;
			ymoved = (float)(((Int2*)(*pEvent)->Data))->Y;
			xmoved /= 3.0f;
			ymoved /= 3.0f;
		}
		goto cameraposoneventendreleaseevent;
	case EVENT_TYPE_KEYDOWN:
	case EVENT_TYPE_KEYPRESS:
		{
			// switch on Vkey code //

			int &VKey = *(int*)(*pEvent)->Data;
			// handle keys //
			switch(VKey){
			case (int)L'A': m_SideWays = -1; goto cameraposoneventendreleaseevent;
			case (int)L'D': m_SideWays = 1; goto cameraposoneventendreleaseevent;
			case (int)L'W': m_Forward = 1; goto cameraposoneventendreleaseevent;
			case (int)L'S': m_Forward = -1; goto cameraposoneventendreleaseevent;
			case VK_SPACE: m_Vertical = 1; goto cameraposoneventendreleaseevent;
			case VK_CONTROL: m_Vertical = -1; goto cameraposoneventendreleaseevent;
			}
		}
		return false;
	default:
		return false;
	}

cameraposoneventendreleaseevent:
	// delete event to indicate that it has been processed //
	SAFE_DELETE(*pEvent);
	// no pending //
	return false;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ViewerCameraPos::BecomeMainListeningCamera(){
	// set as main camera //
	MainCameraListening = true;

	// start listening //
	KeyPressManager::Get()->RegisterForEvent((InputReceiver*)this, KEYPRESS_MANAGER_ORDERNUMBER_LAST_CAMERA);
}

void Leviathan::ViewerCameraPos::RollValueTowards(float &value, const float &changeamount, const bool &maxvalue, const float &limitvalue){
	value += changeamount;
	// limit check //
	if(maxvalue ? value > limitvalue: value < limitvalue){

		value = limitvalue;
	}
}

DLLEXPORT void Leviathan::ViewerCameraPos::SetPos(const Float3 &pos){
	Position = pos;
}

DLLEXPORT void Leviathan::ViewerCameraPos::SetRotation(const Float3 &orientation){
	Orientation = orientation;
}
