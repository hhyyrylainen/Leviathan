#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RENDERINGPASSINFO
#include "RenderingPassInfo.h"
#endif
using namespace Leviathan;
// ------------------------------------ //


DLLEXPORT Leviathan::RenderingPassInfo::RenderingPassInfo(const D3DXMATRIX &view, const D3DXMATRIX &proj, const D3DXMATRIX &world, ViewCamera* camera)
	: ViewMatrix(view), ProjectionMatrix(proj), WorldMatrix(world), ActiveCamera(camera)
{

}

DLLEXPORT Leviathan::RenderingPassInfo::~RenderingPassInfo(){

}
// ------------------------------------ //


