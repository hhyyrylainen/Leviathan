#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "../../Common/Types.h"
#include "Input/InputController.h"

#define DEFAULTMOVEMENTMODIFIER		1.f
#define TURBOMOVEMENTMODIFIER		20.f

namespace Leviathan{
	// callable for the ability to receive key presses
	class ViewerCameraPos : public InputReceiver{
	public:
		DLLEXPORT ViewerCameraPos();
		DLLEXPORT ~ViewerCameraPos();

		DLLEXPORT inline Float3& GetRotation(){
			return Orientation;
		}
		DLLEXPORT inline Float3& GetPosition(){
			return Position;
		}

		DLLEXPORT void UpdatePos(int mspassed);

		DLLEXPORT void SetPos(const Float3 &pos);
		DLLEXPORT void SetRotation(const Float3 &orientation);


		// input receiving //
		DLLEXPORT virtual bool ReceiveInput(int32_t key, int modifiers, bool down);
		DLLEXPORT virtual void ReceiveBlockedInput(int32_t key, int modifiers, bool down);
		// clears mouse movement //
		DLLEXPORT virtual void BeginNewReceiveQueue();

		DLLEXPORT virtual bool OnMouseMove(int xmove, int ymove);

		// sound receiving //
		// Warning: only have one of these set at a time to avoid weird sound issues //
		DLLEXPORT void BecomeSoundPerceiver();
		DLLEXPORT void StopSoundPerceiving();



	private:
		void SideWays(int dir);
		void Forward(int dir);
		void Vertical(int dir);

		void SendPositionIfSet();

		// reduces amount of code //
		static void RollValueTowards(float &value, const float &changeamount,
            const bool &maxvalue, const float &limitvalue);

		// ------------------------------------ //

		bool SendSoundPosition = false;

		float FrameTime = 0;

        /* Yaw, Pitch, Roll*/
		Float3 Orientation = Float3(0, 0, 0); 
		Float3 Position = Float3(0, 0, 0);

		// internal inertia store //
		float forward = 0;
		float backward = 0;
		float left = 0;
		float right = 0;
		float zup = 0;
		float zdown = 0;

		// mouse //
		float MouseXSensitivity = 0.2f;
		float MouseYSensitivity = 0.15f;
		float xmoved = 0;
		float ymoved = 0;

		// stored key input //
		int m_SideWays = 0;
		int m_Forward = 0;
		int m_Vertical = 0;
	};
}

