#ifndef LEVIATHAN_GUI_ANIMATION_ACTION
#define LEVIATHAN_GUI_ANIMATION_ACTION
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


#define GUI_ANIMATION_TYPEMOVE_PRIORITY_X				1
#define GUI_ANIMATION_TYPEMOVE_PRIORITY_Y				2
#define GUI_ANIMATION_TYPEMOVE_PRIORITY_BOTH			3
#define GUI_ANIMATION_TYPEMOVE_PRIORITY_SLOPE			4
#define GUI_ANIMATION_TYPEMOVE_PRIORITY_SLOPEY			5
#define GUI_ANIMATION_TYPEMOVE_PRIORITY_SMOOTH_DIVIDE	6

namespace Leviathan{ namespace Gui{

	// structs for all animation action types, to hold specific data //
	enum GUI_ANIMATION_ACTION{ GUI_ANIMATION_ERROR = 0, GUI_ANIMATION_FADE_OUT, GUI_ANIMATION_GENERAL , GUI_ANIMATION_FADE_IN, 
		GUI_ANIMATION_MOVE, GUI_ANIMATION_GLOW, GUI_ANIMATION_HIDE, GUI_ANIMATION_SHOW,
	
		GUI_ANIMATION_ALL};


	struct GuiAnimationTypeMove{
		GuiAnimationTypeMove(float xtarget, float ytarget, int whichfirst, float speed);

		float X;
		float Y;
		int Priority;
		float Speed;
	};


	class AnimationAction : public Object{
	public:
		DLLEXPORT AnimationAction::AnimationAction();
		DLLEXPORT AnimationAction::AnimationAction(GUI_ANIMATION_ACTION type, void* data, int special, bool allowsimult);
		DLLEXPORT AnimationAction::~AnimationAction();

		GUI_ANIMATION_ACTION Type;
		int SpecialInstr;
		bool AllowSimultanous;

		void* Data;

		DLLEXPORT GUI_ANIMATION_ACTION GetType();

	};

}}
#endif