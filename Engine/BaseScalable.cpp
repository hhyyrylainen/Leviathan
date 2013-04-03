#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_BASE_SCALABLE
#include "BaseScalable.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
BaseScalable::BaseScalable(){
	ScaleUpdated = true;

	XScale = 1.0f;
	YScale = 1.0f;
	ZScale = 1.0f;
}
BaseScalable::~BaseScalable(){

}
// ------------------------------------ //

// ------------------------------------ //
float BaseScalable::GetScale(){
	return XScale;
}
void BaseScalable::GetScale(float &outx, float &outy, float &outz){
	outx = XScale;
	outy = YScale;
	outz = ZScale;
}

float BaseScalable::GetXScale(){
	return XScale;
}
float BaseScalable::GetYScale(){
	return YScale;
}
float BaseScalable::GetZScale(){
	return ZScale;
}
// ------------------------------------ //
void BaseScalable::SetScale(float all){
	OnUpdateScale();
	XScale = all;
	YScale = all;
	ZScale = all;
}
void BaseScalable::SetScale(float x, float y, float z){
	OnUpdateScale();
	XScale = x;
	YScale = y;
	ZScale = z;
}

void BaseScalable::SetXScale(float x){
	OnUpdateScale();
	XScale = x;
}
void BaseScalable::SetYScale(float y){
	OnUpdateScale();
	YScale = y;
}
void BaseScalable::SetZScale(float z){
	OnUpdateScale();
	ZScale = z;
}
// ------------------------------------ //
void BaseScalable::OnUpdateScale(){
	ScaleUpdated = true;
}
bool BaseScalable::IsScaleUpdated(){
	return ScaleUpdated;
}