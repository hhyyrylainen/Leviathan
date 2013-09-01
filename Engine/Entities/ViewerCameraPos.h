#ifndef LEVIATHAN_OBJECT_CAMERAPOS
#define LEVIATHAN_OBJECT_CAMERAPOS
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Entities\Bases\BaseObject.h"
#include "Input\Input.h"
#include "Input\KeyPressManager.h"

namespace Leviathan{
	// callable for the ability to receive key presses
	class ViewerCameraPos : public BaseObject, public InputReceiver {
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

		DLLEXPORT void BecomeMainListeningCamera();
		DLLEXPORT virtual bool OnEvent(InputEvent** pEvent, InputReceiver* pending);

	private:
		void SideWays(int dir);
		void Forward(int dir);
		void Vertical(int dir);

		// reduces amount of code //
		static void RollValueTowards(float &value, const float &changeamount, const bool &maxvalue, const float &limitvalue);
		// ------------------------------------ //

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


		bool MainCameraListening : 1;
	};
}

#endif
