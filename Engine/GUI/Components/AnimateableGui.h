#ifndef LEVIATHAN_GUI_ANIMATEABLE
#define LEVIATHAN_GUI_ANIMATEABLE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "GUI\GuiAnimation.h"

#define GUI_ANIMATEABLE_SEMANTIC_X		1
#define GUI_ANIMATEABLE_SEMANTIC_Y		2
#define GUI_ANIMATEABLE_SEMANTIC_WIDTH	3
#define GUI_ANIMATEABLE_SEMANTIC_HEIGHT	4

namespace Leviathan{ namespace Gui{
	
	class GuiManager;

	#define GUIANIMATEABLE_ADD_PROXIESFORANGELSCRIPT_DEFINITIONS(classname) void QueueActionProxy(AnimationAction* act){ this->QueueAction(act); }; void RemoveActionFromQueueProxy(AnimationAction* anim){ this->RemoveActionFromQueue(anim); };

class GuiAnimateable{
	public:
		DLLEXPORT GuiAnimateable::GuiAnimateable();
		DLLEXPORT virtual GuiAnimateable::~GuiAnimateable();
		// called by GuiManager when animating //
		DLLEXPORT virtual int AnimationTime(int mspassed) = 0;

		DLLEXPORT virtual void AnimationFinish();
		// please make sure that you don't directly pass new object to this since reference count should end up as 0 when used in this function 
		// (script will automatically destroy the local copy and decrement) 
		DLLEXPORT virtual void QueueAction(AnimationAction* act);
		DLLEXPORT virtual void RemoveActionFromQueue(AnimationAction* actionptr);
		DLLEXPORT virtual void RemoveActionFromQueue(const size_t index);

		DLLEXPORT virtual void SetValue(const int &semanticid, const float &val) = 0;
		DLLEXPORT virtual float GetValue(const int &emanticid) const = 0;


		
protected:
		virtual int _RunAnimationTimeDefault(GuiManager* owner, const int &mspassed) = 0;

		vector<AnimationAction*> AnimationQueue;
	};

}}
#endif