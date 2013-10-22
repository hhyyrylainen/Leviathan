#ifndef PONG_ARENA
#define PONG_ARENA
// ------------------------------------ //
#ifndef PONGINCLUDES
#include "PongIncludes.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "PlayerSlot.h"


#define BASE_ARENASCALE		1.f	

namespace Pong{

	class PongGame;

	class Arena{
	public:
		Arena(shared_ptr<Leviathan::GameWorld> world);
		~Arena();
		// Generates an arena to the world //
		bool GenerateArena(PongGame* game, vector<PlayerSlot*> &players, int plycount, int maximumsplit, bool clearfirst = true);

	private:

		void _ClearPointers();
		// ------------------------------------ //

		// the world to which the arena is generated //
		shared_ptr<Leviathan::GameWorld> TargetWorld;

		// Stored object pointers //

		// Arena bottom //
		shared_ptr<Leviathan::BaseObject> BottomBrush;


	};

}
#endif