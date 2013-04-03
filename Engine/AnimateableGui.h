#ifndef LEVIATHAN_GUI_ANIMATEABLE
#define LEVIATHAN_GUI_ANIMATEABLE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "EventableGui.h"
#include "GuiAnimation.h"

#define GUI_ANIMATEABLE_SEMANTIC_X		1
#define GUI_ANIMATEABLE_SEMANTIC_Y		2
#define GUI_ANIMATEABLE_SEMANTIC_WIDTH	3
#define GUI_ANIMATEABLE_SEMANTIC_HEIGHT	4

namespace Leviathan{ namespace Gui{
	
	class AnimationAction;

class GuiAnimateable : public BaseEventable{
	public:
		DLLEXPORT GuiAnimateable::GuiAnimateable();
		DLLEXPORT virtual GuiAnimateable::~GuiAnimateable();



		DLLEXPORT virtual int AnimationTime(int mspassed); // this can be passed to animation manager for handling

		DLLEXPORT virtual void AnimationFinish();
		DLLEXPORT virtual void QueueAction(AnimationAction* act);


		DLLEXPORT virtual void SetValue(int semanticid, float val);
		DLLEXPORT virtual float GetValue(int semanticid);


		vector<AnimationAction*> Queue;

		// script interface //
		//DLLEXPORT static void QueueActionForObject(GuiAnimateable* object, AnimationAction* action); // not required //
	


	};

}}
#endif