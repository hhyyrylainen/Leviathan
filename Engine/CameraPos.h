#ifndef LEVIATHAN_OBJECT_CAMERAPOS
#define LEVIATHAN_OBJECT_CAMERAPOS
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "BaseObject.h"
//#include "BaseInitializable.h"
//#include "BaseTickable.h"
#include "Input.h"
#include "CallableObject.h"

namespace Leviathan{
	// callable for the ability to receive key presses
	class ViewerCameraPos : public BaseObject, public CallableObject {
	public:
		DLLEXPORT ViewerCameraPos();
		DLLEXPORT ~ViewerCameraPos();

		DLLEXPORT void SetFrameTime(float frametime);
		DLLEXPORT void GetRotation(float& pitch, float& yaw, float& roll);
		//DLLEXPORT void GetSmoothRot(float& yaw, float& pitch, float& roll);
		DLLEXPORT void GetPos(float& x, float& y, float& z);
		//DLLEXPORT void GetSmoothPos(float& x, float& y, float& z);

		DLLEXPORT void ClearInputs();
		DLLEXPORT void UpdatePos(int mspassed);
		DLLEXPORT void ReadInput(Input* inp, bool mouse, bool keys);

		DLLEXPORT void SetMouseMode(bool mode);

		DLLEXPORT void SideWays(int dir);
		DLLEXPORT void Forward(int dir);
		DLLEXPORT void Vertical(int dir);

		DLLEXPORT void SetPos(float x, float y, float z, bool smoothly = false);
		DLLEXPORT void SetRotation(float yaw, float pitch, float roll, bool smoothly = false);

		DLLEXPORT void SetSmoothing(bool toset);


		DLLEXPORT void PitchTurn(int dir);
		DLLEXPORT void YawTurn(int dir);
		DLLEXPORT void RollTurn(int dir);


		DLLEXPORT void BecomeMainListeningCamera();

		DLLEXPORT virtual void OnEvent(Event** pEvent);

	private:
		float FrameTime;
		float Pitch;
		float Yaw;
		float Roll;

		float X;
		float Y;
		float Z;

		float XSmooth;
		float YSmooth;
		float ZSmooth;

		float forward;
		float backward;
		float left;
		float right;
		float zup;
		float zdown;
	
		float pitchup;
		float pitchdown;
		float yawup;
		float yawdown;
		float rollup;
		float rolldown;

		bool TooSmooth;


		float PitchTarget;
		float YawTarget;
		float RollTarget;

		float RotSpeed;
		float MouseXSensitivity;
		float MouseYSensitivity;

		int m_SideWays;
		int m_Forward;
		int m_Vertical;

		float xmoved;
		float ymoved;


		bool UseMouse;


		bool MainCameraListening : 1;

	};
}

#endif
