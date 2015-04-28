#pragma once
// ------------------------------------ //
#include "PongIncludes.h"
// ------------------------------------ //
#include "PlayerSlot.h"
#include "Entities/Objects/TrailEmitter.h"
#include "Common/ThreadSafe.h"
#include <memory>
#include <string>
#include <map>


#define BASE_ARENASCALE		1.f	

namespace Pong{

	class BasePongParts;

    using namespace std;

	class Arena : public ThreadSafe{
	public:
		Arena(shared_ptr<Leviathan::GameWorld> world);
		~Arena();
		// Generates an arena to the world //
		bool GenerateArena(BasePongParts* game, PlayerList &plys);

        //! Makes sure the trail object exists
        void VerifyTrail();

		void ServeBall();
		// Does what ever is needed to ditch old ball //
		void LetGoOfBall();

		inline shared_ptr<Leviathan::GameWorld> GetWorld(){
			return TargetWorld;
		}

        void RegisterBall(ObjectPtr ball){

            GUARD_LOCK();
            
            Ball.reset();
            Ball = ball;
        }

		string GetMaterialNameForPlayerColour(const Float4 &colour);

		void ColourTheBallTrail(const Float4 &colour);

		inline ObjectPtr GetBallPtr(){
            GUARD_LOCK();
			return Ball;
		}

		// Checks based on generated arena if ball intersects (or could) with a paddle area //
		bool IsBallInPaddleArea();

	private:

		void _ClearPointers();
		// ------------------------------------ //

		// the world to which the arena is generated //
        std::shared_ptr<Leviathan::GameWorld> TargetWorld;

		// Stored object pointers //

		// Arena bottom //
		ObjectPtr BottomBrush;

		// The ball trail object //
		ObjectPtr TrailKeeper;
		Leviathan::Entity::TrailEmitter* DirectTrail;

		// ball prop //
		ObjectPtr Ball;

		// Used to store already generated materials for paddles //
		std::map<Float4, std::string> ColourMaterialName;

	};

}

