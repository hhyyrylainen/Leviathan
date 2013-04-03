#ifndef LEVIATHAN_GUI_SCRIPT_INTERFACE
#define LEVIATHAN_GUI_SCRIPT_INTERFACE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "GuiManager.h"

namespace Leviathan{

bool Gui_SetObjectText(int ID, string inset, bool doupdate = true);
// ----------------- //
//int Gui_QueueAnimationActionMove(int ID, GuiAnimationTypeMove& );
int Gui_QueueAnimationActionMove(int ID, int xtarget, int ytarget, int whichfirst, float speed, bool allowsimult = false, int special = 0);
int Gui_QueueAnimationActionVisibility(int ID, bool visible);
int Gui_QueuedAnimationClear(int ID);
int Gui_QueuedAnimationUpdate(int ID, int passedms);
// ----------------- //
}
#endif