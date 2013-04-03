#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_CAMERA
#include "Camera.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
ViewCamera::ViewCamera(){
	// set rotation and pos to 0 //
	Position = Float3();
	Rotation = Float3();

	StaticViewCreated = false;
}
ViewCamera::~ViewCamera(){

}
// ------------------------------------ //
void ViewCamera::SetPosition(Float3 pos){
	Position = pos;
}
Float3 ViewCamera::GetPosition(){
	return Position;
}

void ViewCamera::SetRotation(Float3 rotation){
	Rotation = rotation;
}

Float3 ViewCamera::GetRotation(){
	return Rotation;
}
void ViewCamera::GetViewMatrix(D3DXMATRIX& get){ 
		get = ViewMatrix;
}

void ViewCamera::GetStaticViewMatrix(D3DXMATRIX& get){
	if(!StaticViewCreated)
		CreateStaticViewMatrix();
	get = StaticView;
}

void ViewCamera::CreateStaticViewMatrix(){
	// set up direction //
	D3DXVECTOR3 up;
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;

	// copy values to vectors //
	D3DXVECTOR3 position;
	position.x = 0;
	position.y = 0;
	position.z = -1.f;

	D3DXVECTOR3 lookat;
	lookat.x = 0.0f;
	lookat.y = 0.0f;
	lookat.z = 1.0f;

	// translate camera position //
	lookat = position + lookat;

	// create view matrix from vectors //
	D3DXMatrixLookAtLH(&StaticView, &position, &lookat, &up);

	StaticViewCreated = true;
}

void ViewCamera::UpdateMatrix(){
	// set up direction //
	D3DXVECTOR3 up;
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;

	// copy values to vectors //
	D3DXVECTOR3 position;
	position.x = Position.Val[0];
	position.y = Position.Val[1];
	position.z = Position.Val[2];

	D3DXVECTOR3 lookat;
	lookat.x = 0.0f;
	lookat.y = 0.0f;
	lookat.z = 1.0f;

	// convert to radians //
	float yaw, pitch, roll;
	pitch = Rotation.Val[0] * 0.0174532925f;
	yaw   = Rotation.Val[1] * 0.0174532925f;
	roll  = Rotation.Val[2] * 0.0174532925f;

	// create rotation matrix //
	D3DXMATRIX Rotationmatrix;
	D3DXMatrixRotationYawPitchRoll(&Rotationmatrix, yaw, pitch, roll);

	// transform coords //
	D3DXVec3TransformCoord(&lookat, &lookat, &Rotationmatrix);
	D3DXVec3TransformCoord(&up, &up, &Rotationmatrix);

	// translate camera position //
	lookat = position + lookat;

	// create view matrix from vectors //
	D3DXMatrixLookAtLH(&ViewMatrix, &position, &lookat, &up);

	if(!StaticViewCreated){
		CreateStaticViewMatrix();
	}

}








