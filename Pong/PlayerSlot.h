#ifndef PONG_PLAYERSLOT
#define PONG_PLAYERSLOT
// ------------------------------------ //
#ifndef PONGINCLUDES
#include "PongIncludes.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Pong{

	enum PLAYERTYPE {PLAYERTYPE_HUMAN, PLAYERTYPE_COMPUTER, PLAYERTYPE_EMPTY, PLAYERTYPE_CLOSED};
	enum PLAYERCONTROLS {PLAYERCONTROLS_NONE, PLAYERCONTROLS_AI, PLAYERCONTROLS_WASD, PLAYERCONTROLS_ARROWS, PLAYERCONTROLS_IJKL,
		PLAYERCONTROLS_NUMPAD, PLAYERCONTROLS_CONTROLLER};

	class PlayerSlot{
	public:
		// creates an empty slot //
		PlayerSlot(int slotnumber, bool empty);
		// Creates fully customized slot, Control identifier is for choosing controller number
		PlayerSlot(int slotnumber, PLAYERTYPE type, int playeridentifier, PLAYERCONTROLS controltype, int ctrlidentifier);

		~PlayerSlot();

		void SetPlayer(PLAYERTYPE type, int identifier);
		PLAYERTYPE GetPlayerType();
		int GetPlayerIdentifier();

		void SetControls(PLAYERCONTROLS type, int identifier);
		PLAYERCONTROLS GetControlType();
		int GetControlIdentifier();

		int GetSlotNumber();

		// returns true if player type isn't empty or closed //
		inline bool IsSlotActive(){

			return PlayerType != PLAYERTYPE_CLOSED && PlayerType != PLAYERTYPE_EMPTY;
		}

		// recursively looks through children and counts maximum depth (usually 0 or 1) //
		int GetSplitCount();

	private:
		// data //
		int Slot;
		PLAYERTYPE PlayerType;
		int PlayerIdentifier;
		PLAYERCONTROLS ControlType;
		int ControlIdentifier;

		// slot splitting //
		PlayerSlot* SplitSlot;
	};

}
#endif