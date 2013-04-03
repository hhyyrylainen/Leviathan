#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RENDERING_LIGHT
#include "Light.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
RenderingLight::RenderingLight(){

}

RenderingLight::~RenderingLight(){

}
// ------------------------------------ //
void RenderingLight::SetAmbientColor(float red, float green, float blue, float alpha){
	AmbientColor = Float4(red, green, blue, alpha);
}


void RenderingLight::SetDiffuseColor(float red, float green, float blue, float alpha){
	DiffuseColor = Float4(red, green, blue, alpha);
}


void RenderingLight::SetDirection(float x, float y, float z){
	Direction = Float3(x, y, z);
}


void RenderingLight::SetSpecularColor(float red, float green, float blue, float alpha){
	SpecularColor = Float4(red, green, blue, alpha);
}


void RenderingLight::SetSpecularPower(float power){
	SpecularPower = power;
}
// ------------------------------------ //
Float4 RenderingLight::GetAmbientColor(){
	return AmbientColor;
}


Float4 RenderingLight::GetDiffuseColor(){
	return DiffuseColor;
}


Float3 RenderingLight::GetDirection(){
	return Direction;
}


Float4 RenderingLight::GetSpecularColor(){
	return SpecularColor;
}


float RenderingLight::GetSpecularPower(){
	return SpecularPower;
}
// ------------------------------------ //

// ------------------------------------ //







