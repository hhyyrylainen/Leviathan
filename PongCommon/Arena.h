#pragma once
// ------------------------------------ //
#include "PongIncludes.h"
// ------------------------------------ //
#include "PlayerSlot.h"
#include "Entities/Components.h"
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

        //! \warning The world has to be valid while this object is used
		Arena(Leviathan::GameWorld* world);
		~Arena();
        
		// Generates an arena to the world //
		bool GenerateArena(BasePongParts* game, PlayerList &plys);

        //! Makes sure the trail object exists
        void VerifyTrail(Lock &guard);

        inline void VerifyTrail(){

            GUARD_LOCK();
            VerifyTrail(guard);
        }

		void ServeBall();
		// Does what ever is needed to ditch old ball //
		void LetGoOfBall();

        void RegisterBall(Lock &guard, ObjectID ball){

            Ball = ball;
        }

		string GetMaterialNameForPlayerColour(const Float4 &colour);

		void ColourTheBallTrail(Lock &guard, const Float4 &colour);

        inline void ColourTheBallTrail(const Float4 &colour){

            GUARD_LOCK();
            ColourTheBallTrail(guard, colour);
        }

		inline ObjectID GetBall(Lock &guard){

			return Ball;
		}

        inline ObjectID GetBall(){

            GUARD_LOCK();
            return GetBall(guard);
        }

		// Checks based on generated arena if ball intersects (or could) with a paddle area //
		bool IsBallInPaddleArea();

	private:

		void _ClearPointers(Lock &guard);
		// ------------------------------------ //

		// the world to which the arena is generated //
        Leviathan::GameWorld* TargetWorld;

		// Stored object pointers //

		// Arena bottom //
		ObjectID BottomBrush;

		// The ball trail object //
		ObjectID TrailKeeper;

		// ball prop //
		ObjectID Ball;

		//! Used to store already generated materials for paddles
		std::map<Float4, std::string> ColourMaterialName;

        //! Lock for ColourMaterialName
        Mutex ColourMaterialNameMutex;

	};

}

