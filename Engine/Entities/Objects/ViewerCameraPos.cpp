#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_OBJECT_CAMERAPOS
#include "ViewerCameraPos.h"
#endif
#include "Sound\SoundDevice.h"
using namespace Leviathan;
// ------------------------------------ //

ViewerCameraPos::ViewerCameraPos() : BaseObject(IDFactory::GetID(), NULL), Orientation(0), Position(0), SendSoundPosition(false){

	// set all to zeros //
	FrameTime = forward = backward = left = right = zup = zdown = xmoved = ymoved = 0;
	m_SideWays = m_Forward = m_Vertical = 0;

	// mouse //
	MouseXSensitivity = 0.2f;
	MouseYSensitivity = 0.15f;
}

Leviathan::ViewerCameraPos::~ViewerCameraPos(){

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

	SendPositionIfSet();
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
DLLEXPORT bool Leviathan::ViewerCameraPos::ReceiveInput(OIS::KeyCode key, int modifiers, bool down){
	// reset state only if the state is the same that the key would set //
	switch(key){
	case OIS::KC_A: if(!down && m_SideWays == -1) m_SideWays = 0; else if(down) m_SideWays = -1; return true;
	case OIS::KC_D: if(!down && m_SideWays == 1) m_SideWays = 0; else if(down) m_SideWays = 1; return true;
	case OIS::KC_W: if(!down && m_Forward == 1) m_Forward = 0; else if(down) m_Forward = 1; return true;
	case OIS::KC_S: if(!down && m_Forward == -1) m_Forward = 0; else if(down) m_Forward = -1; return true;
	case OIS::KC_SPACE: if(!down && m_Vertical == 1) m_Vertical = 0; else if(down) m_Vertical = 1; return true;
	case OIS::KC_LCONTROL: if(!down && m_Vertical == -1) m_Vertical = 0; else if(down) m_Vertical = -1; return true;
	}
	return false;
}

DLLEXPORT bool Leviathan::ViewerCameraPos::OnMouseMove(int xmove, int ymove){
	// set internal moved variable //
	xmoved = (float)xmove;
	ymoved = (float)ymove;

	// always processed //
	return true;
}

DLLEXPORT void Leviathan::ViewerCameraPos::BeginNewReceiveQueue(){
	xmoved = ymoved = 0;
}

DLLEXPORT void Leviathan::ViewerCameraPos::ReceiveBlockedInput(OIS::KeyCode key, int modifiers, bool down){
	// reset control state of any keys received //
	// reset state only if the state is the same that the key would set //
	switch(key){
	case OIS::KC_A: if(!down && m_SideWays == -1) m_SideWays = 0; break;
	case OIS::KC_D: if(!down && m_SideWays == 1) m_SideWays = 0; break;
	case OIS::KC_W: if(!down && m_Forward == 1) m_Forward = 0; break;
	case OIS::KC_S: if(!down && m_Forward == -1) m_Forward = 0; break;
	case OIS::KC_SPACE: if(!down && m_Vertical == 1) m_Vertical = 0; break;
	case OIS::KC_LCONTROL: if(!down && m_Vertical == -1) m_Vertical = 0; break;
	}
}
// ------------------------------------ //
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

void Leviathan::ViewerCameraPos::SendPositionIfSet(){
	if(!SendSoundPosition)
		return;
	// send own position to sound device for 3d sound //
	SoundDevice::Get()->SetSoundListenerPosition(Position, Orientation);
}

DLLEXPORT void Leviathan::ViewerCameraPos::BecomeSoundPerceiver(){
	SendSoundPosition = true;
}

DLLEXPORT void Leviathan::ViewerCameraPos::StopSoundPerceiving(){
	SendSoundPosition = false;
}




