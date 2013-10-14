#ifndef LEVIATHAN_OBJECT_CAMERAPOS
#define LEVIATHAN_OBJECT_CAMERAPOS
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Entities\Bases\BaseObject.h"
#include "Input\InputController.h"

#define DEFAULTMOVEMENTMODIFIER		1.f
#define TURBOMOVEMENTMODIFIER		20.f

namespace Leviathan{
	// callable for the ability to receive key presses
	class ViewerCameraPos : public BaseObject, public InputReceiver{
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
		DLLEXPORT virtual bool ReceiveInput(OIS::KeyCode key, int modifiers, bool down);
		DLLEXPORT virtual void ReceiveBlockedInput(OIS::KeyCode key, int modifiers, bool down);

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
		static void RollValueTowards(float &value, const float &changeamount, const bool &maxvalue, const float &limitvalue);
		// ------------------------------------ //

		bool SendSoundPosition;

		float FrameTime;

		Float3 Orientation; /* Yaw, Pitch, Roll*/
		Float3 Position;

		// internal inertia store //
		float forward;
		float backward;
		float left;
		float right;
		float zup;
		float zdown;

		// mouse //
		float MouseXSensitivity;
		float MouseYSensitivity;
		float xmoved;
		float ymoved;

		// stored key input //
		int m_SideWays;
		int m_Forward;
		int m_Vertical;
	};
}

#endif
