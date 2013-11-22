#ifndef PONG_PLAYERSLOT
#define PONG_PLAYERSLOT
// ------------------------------------ //
#ifndef PONGINCLUDES
#include "PongIncludes.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Entities\Objects\TrackEntityController.h"


#define INPUTFORCE_APPLYSCALE		20.f
#define INPUT_TRACK_ADVANCESPEED	2.0f

namespace Pong{

	enum PLAYERTYPE {PLAYERTYPE_HUMAN, PLAYERTYPE_COMPUTER, PLAYERTYPE_EMPTY, PLAYERTYPE_CLOSED};
	enum PLAYERCONTROLS {PLAYERCONTROLS_NONE, PLAYERCONTROLS_AI, PLAYERCONTROLS_WASD, PLAYERCONTROLS_ARROWS, PLAYERCONTROLS_IJKL,
		PLAYERCONTROLS_NUMPAD, PLAYERCONTROLS_CONTROLLER};
	enum CONTROLKEYACTION {CONTROLKEYACTION_LEFT, CONTROLKEYACTION_RIGHT, CONTROLKEYACTION_POWERUPDOWN, CONTROLKEYACTION_POWERUPUP};

	// TODO: implement powerups

	class PlayerSlot{
	public:
		// creates an empty slot //
		PlayerSlot(int slotnumber, bool empty);
		// Creates fully customized slot, Control identifier is for choosing controller number
		PlayerSlot(int slotnumber, PLAYERTYPE type, int playeridentifier, PLAYERCONTROLS controltype, int ctrlidentifier, const Float4 &playercolour);

		~PlayerSlot();

		void SetPlayer(PLAYERTYPE type, int identifier);
		PLAYERTYPE GetPlayerType();
		int GetPlayerIdentifier();

		void SetControls(PLAYERCONTROLS type, int identifier);
		PLAYERCONTROLS GetControlType();
		int GetControlIdentifier();

		int GetSlotNumber();
		int GetScore();
		inline void SetScore(int score){
			Score = score;
		}
		// active should be if the key is down (or false if it is up) //
		void PassInputAction(CONTROLKEYACTION actiontoperform, bool active);
		// resets all input state //
		void InputDisabled();

		shared_ptr<Leviathan::BaseObject> GetPaddle(){
			return PaddleObject;
		}
		void SetPaddleObject(shared_ptr<Leviathan::BaseObject> obj){
			PaddleObject = obj;
		}
		shared_ptr<Leviathan::BaseObject> GetGoalArea(){
			return GoalAreaObject;
		}
		void SetGoalAreaObject(shared_ptr<Leviathan::BaseObject> obj){
			GoalAreaObject = obj;
		}
		void SetTrackObject(shared_ptr<Leviathan::BaseObject> obj, Leviathan::Entity::TrackEntityController* direct){
			TrackObject = obj;
			TrackDirectptr = direct;
		}

		inline Float4 GetColour(){
			return Colour;
		}
		void SetColour(const Float4 &colour){
			Colour = colour;
		}

		// returns true if player type isn't empty or closed //
		inline bool IsSlotActive(){

			return PlayerType != PLAYERTYPE_CLOSED && PlayerType != PLAYERTYPE_EMPTY;
		}

		// recursively looks through children and counts maximum depth (usually 0 or 1) //
		int GetSplitCount();

		PlayerSlot* GetSplit(){
			return SplitSlot;
		}

	private:
		// data //
		int Slot;
		PLAYERTYPE PlayerType;
		int PlayerIdentifier;
		PLAYERCONTROLS ControlType;
		int ControlIdentifier;

		Float4 Colour;

		int Score;

		int MoveState;

		shared_ptr<Leviathan::BaseObject> PaddleObject;
		shared_ptr<Leviathan::BaseObject> GoalAreaObject;

		shared_ptr<Leviathan::BaseObject> TrackObject;
		Leviathan::Entity::TrackEntityController* TrackDirectptr;

		// slot splitting //
		PlayerSlot* SplitSlot;
	};

}
#endif