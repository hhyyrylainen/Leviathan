#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUI_SCRIPT_INTERFACE
#include "GuiScriptInterface.h"
#endif
using namespace Leviathan;
namespace Leviathan{
// ------------------------------------ //
bool Gui_SetObjectText(int ID, string inset, bool doupdate){
	// get object //
	GuiManager* tempg = GuiManager::Get();
	BaseGuiObject* tempbase = tempg->GetObject(tempg->GetObjectIndexFromId(ID));
	if(tempbase == NULL)
		return false;

	// get which class it is //
	if(tempbase->Objecttype == GOBJECT_TYPE_TEXTLABEL){
		Gui::TextLabel* tlabel = (Gui::TextLabel*)tempbase;
		tlabel->Update(VAL_NOUPDATE,VAL_NOUPDATE,VAL_NOUPDATE,VAL_NOUPDATE,doupdate, Convert::StringToWstring(inset));
		return true;
	}
	return false;
}
// ------------------------------------ //
int Gui_QueueAnimationActionMove(int ID, int xtarget, int ytarget, int whichfirst, float speed, bool allowsimult, int special){
	// get object //
	GuiManager* tempg = GuiManager::Get();
	BaseGuiObject* tempbase = tempg->GetObject(tempg->GetObjectIndexFromId(ID));
	tempg = NULL; // just to be safe, not to mess qui handler up //
	if(tempbase == NULL)
		return false;

	// check is the object high enough level //
	if(tempbase->ObjectLevel < GUI_OBJECT_LEVEL_ANIMATEABLE){
		return -2;
	}
	Gui::GuiAnimateable* tempc = reinterpret_cast<Gui::GuiAnimateable*>(tempbase);

	// generate action and submit //
	Gui::GuiAnimationTypeMove* pmov = NULL;
	Gui::AnimationAction* pact = NULL;
	pmov = new Gui::GuiAnimationTypeMove(xtarget, ytarget, whichfirst, speed);
	pact = new Gui::AnimationAction(Gui::GUI_ANIMATION_MOVE, pmov , special,allowsimult);

	tempc->QueueAction(pact);

	//ptrvec->push_back(pact);

	//Gui::GuiAnimateable::QueueActionForObject(tempc, pact);
	//tempc->QueueAction(pact);
	// done //
	return true;
}
int Gui_QueueAnimationActionVisibility(int ID, bool visible){
	// get object //
	GuiManager* tempg = GuiManager::Get();
	BaseGuiObject* tempbase = tempg->GetObject(tempg->GetObjectIndexFromId(ID));
	tempg = NULL; // just to be safe, not to mess qui handler up //
	if(tempbase == NULL)
		return false;

	// check is the object high enough level //
	if(tempbase->ObjectLevel < GUI_OBJECT_LEVEL_ANIMATEABLE){
		return -2;
	}
	Gui::GuiAnimateable* tempc = reinterpret_cast<Gui::GuiAnimateable*>(tempbase);

	// generate action and submit //
	if(visible){
		tempc->QueueAction(new Gui::AnimationAction(Gui::GUI_ANIMATION_SHOW, NULL, 0, false));
	} else {
		tempc->QueueAction(new Gui::AnimationAction(Gui::GUI_ANIMATION_HIDE, NULL, 0, false));
	}
	// done //
	return true;
}
int Gui_QueuedAnimationClear(int ID){
	// get object //
	GuiManager* tempg = GuiManager::Get();
	BaseGuiObject* tempbase = tempg->GetObject(tempg->GetObjectIndexFromId(ID));
	if(tempbase == NULL)
		return false;

	// check is the object high enough level //
	if(tempbase->ObjectLevel < GUI_OBJECT_LEVEL_ANIMATEABLE){
		return -2;
	}
	Gui::GuiAnimateable* tempc = (Gui::GuiAnimateable*)tempg;

	// generate action and submit //
	for(unsigned int i = 0; i < tempc->Queue.size(); i++){
		SAFE_DELETE(tempc->Queue[i]);
		tempc->Queue.erase(tempc->Queue.begin()+i);
		i--;
	}
	tempc->Queue.clear();
	// done //
	return true;
}
int Gui_QueuedAnimationUpdate(int ID, int passedms){
	// get object //
	GuiManager* tempg = GuiManager::Get();
	BaseGuiObject* tempbase = tempg->GetObject(tempg->GetObjectIndexFromId(ID));
	if(tempbase == NULL)
		return false;

	// check is the object high enough level //
	if(tempbase->ObjectLevel < GUI_OBJECT_LEVEL_ANIMATEABLE){
		return -2;
	}
	Gui::GuiAnimateable* tempc = (Gui::GuiAnimateable*)tempg;

	// generate action and submit //
	tempc->AnimationTime(passedms);
	// done //
	return true;
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //
}