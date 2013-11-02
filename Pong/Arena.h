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
#define BALL_SPEED_MAX		40

namespace Pong{

	class PongGame;

	class Arena{
	public:
		Arena(shared_ptr<Leviathan::GameWorld> world);
		~Arena();
		// Generates an arena to the world //
		bool GenerateArena(PongGame* game, vector<PlayerSlot*> &players, int plycount, int maximumsplit, bool clearfirst = true);

		void ServeBall();
		// Does what ever is needed to ditch old ball //
		void LetGoOfBall();

		void GiveBallSpeed(float mult);

		inline shared_ptr<Leviathan::BaseObject> GetBallPtr(){
			return Ball;
		}

	private:

		void _ClearPointers();
		// ------------------------------------ //

		// the world to which the arena is generated //
		shared_ptr<Leviathan::GameWorld> TargetWorld;

		// Stored object pointers //

		// Arena bottom //
		shared_ptr<Leviathan::BaseObject> BottomBrush;

		// ball prop //
		shared_ptr<Leviathan::BaseObject> Ball;

	};

}
#endif