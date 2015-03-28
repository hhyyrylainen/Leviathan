// ------------------------------------ //
#ifndef LEVIATHAN_LEAPLISTENER
#include "LeapListener.h"
#endif
#include "Leap.h"
using namespace Leviathan;
using namespace Leap;
// ------------------------------------ //
#include "LeapManager.h"

Leviathan::LeapListener::LeapListener(LeapManager* owner) : Owner(owner){
	// set everything to default //
	Connected = false;
	Focused = false;
}

Leviathan::LeapListener::~LeapListener(){
}
// ------------------------------------ //

// ------------------------------------ //
void Leviathan::LeapListener::onInit(const Leap::Controller &control){

	Logger::Get()->Info(L"LeapListener: initialized");
}

void Leviathan::LeapListener::onConnect(const Leap::Controller &control){

	control.enableGesture(Gesture::TYPE_CIRCLE);
	control.enableGesture(Gesture::TYPE_KEY_TAP);
	control.enableGesture(Gesture::TYPE_SCREEN_TAP);
	control.enableGesture(Gesture::TYPE_SWIPE);
	// set as connected //
	Connected = true;

	Logger::Get()->Info(L"LeapListener: connected");
}

void Leviathan::LeapListener::onDisconnect(const Leap::Controller &control){
	// SDK Note: not dispatched when running in a debugger
	Connected = false;
}

void Leviathan::LeapListener::onExit(const Leap::Controller &control){
	Connected = false;
}

void Leviathan::LeapListener::onFrame(const Leap::Controller &control){
	// get most recent frame //
	const Frame frame = control.frame();

	// process the frame gestures //
	const GestureList gestures = frame.gestures();
	for(int i = 0; i < gestures.count(); i++){
		// get current gesture //
		Gesture gesture = gestures[i];
		// switch based on type and process //
		switch(gesture.type()) {
		case Gesture::TYPE_CIRCLE:
			{
				// instantiate correct gesture subclass //
				CircleGesture circle = gesture;
				wstring datastr;
				// check direction //
				if(circle.pointable().direction().angleTo(circle.normal()) <= PI/4) {
					// clockwise rotation //
					datastr += L"clockwise ";
				} else {
					// counterclockwise //
					datastr += L"counterclockwise ";
				}

				// Calculate angle difference since last frame //
				float sweptAngle = 0;
				if(circle.state() != Gesture::STATE_START){
					// retrieve this gesture in last frame based on id //
					CircleGesture previousgesturestate = CircleGesture(control.frame(1).gesture(circle.id()));
					// get progress change and change it to a radian angle //
					sweptAngle = (float)((circle.progress() - previousgesturestate.progress())*2*PI);
				}

				datastr += L"Circle id: "+Convert::ToWstring(circle.id());
				datastr += L", state: " +Convert::ToWstring(circle.state());
				datastr += L", progress: " +Convert::ToWstring(circle.progress());
				datastr += L", radius: " +Convert::ToWstring(circle.radius());
				datastr += L", angle " +Convert::ToWstring(sweptAngle * RAD_TO_DEG);

			}
			break;
		case Gesture::TYPE_SWIPE:
			{
				// instantiate correct gesture subclass //
				SwipeGesture swipe = gesture;
				wstring datastr;


				// check for down going sweep //
				if(swipe.direction().y < -0.7){
					// down //

					// add to threshold //
					Owner->DownWardSwipeThresshold((int)swipe.speed());

				}


				// this is one of the wanted ones //
				datastr += L"Swipe id: "+Convert::ToWstring(swipe.id());
				datastr += L", state: " +Convert::ToWstring(swipe.state());
				datastr += L", direction: " +Convert::StringToWstring(Convert::ToString(swipe.direction()));
				datastr += L", speed: " +Convert::ToWstring(swipe.speed());
			}
			break;
		case Gesture::TYPE_KEY_TAP:
			{
				// instantiate correct gesture subclass //
				KeyTapGesture tap = gesture;
				wstring datastr;

				datastr += L"Key Tap id: "+Convert::ToWstring(tap.id());
				datastr += L", state: " +Convert::ToWstring(tap.state());
				datastr += L", position: " +Convert::StringToWstring(Convert::ToString(tap.position()));
				datastr += L", direction: " +Convert::StringToWstring(Convert::ToString(tap.direction()));

			}
			break;
		case Gesture::TYPE_SCREEN_TAP:
			{
				// instantiate correct gesture subclass //
				ScreenTapGesture screentap = gesture;
				wstring datastr;

				datastr += L"Key Tap id: "+Convert::ToWstring(screentap.id());
				datastr += L", state: " +Convert::ToWstring(screentap.state());
				datastr += L", position: " +Convert::StringToWstring(Convert::ToString(screentap.position()));
				datastr += L", direction: " +Convert::StringToWstring(Convert::ToString(screentap.direction()));
			}
			break;
		default:
			Logger::Get()->Error(L"LeapListener: unknown gesture type: "+Convert::ToWstring(gesture.type()));
			break;
		}
	}
}

void Leviathan::LeapListener::onFocusGained(const Leap::Controller &control){
	Focused = true;
}

void Leviathan::LeapListener::onFocusLost(const Leap::Controller &control){
	Focused = false;
}
// ------------------------------------ //

// ------------------------------------ //




