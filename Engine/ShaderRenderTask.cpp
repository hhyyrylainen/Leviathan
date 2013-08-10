#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SHADERRENDERTASK
#include "ShaderRenderTask.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::ShaderRenderTask::ShaderRenderTask() : BMatData(NULL), TextureObjects(NULL), BLightData(NULL), VertexSkinningData(NULL),
	CameraLocationData(NULL), ColourBuffer2Data(NULL)
{
	

	bool PatternCreated;
	string ShaderPattern;
}

DLLEXPORT Leviathan::ShaderRenderTask::~ShaderRenderTask(){
	SAFE_DELETE(BMatData);
	SAFE_DELETE(TextureObjects);
	SAFE_DELETE(BLightData);
	SAFE_DELETE(VertexSkinningData);
	SAFE_DELETE(CameraLocationData);
	SAFE_DELETE(ColourBuffer2Data);
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //



