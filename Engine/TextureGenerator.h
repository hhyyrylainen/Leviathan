#ifndef LEVIATHAN_TEXTURE_GENERATOR
#define LEVIATHAN_TEXTURE_GENERATOR
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Utility\FileRelated\DDsHandler.h"
#include "Rendering\ManagedTexture.h"

namespace Leviathan{

	class TextureGenerator{
	public:
		DLLEXPORT static ManagedTexture* GenerateCheckerBoard(int width, int height, unsigned int colours, int tilesperwidth, int tilesperheight, 
			vector<Int3>& colors);
		DLLEXPORT static ManagedTexture* GenerateCheckerBoard(int width, int height, unsigned int colours, int tilesperside, vector<Int3>& colors);


		DLLEXPORT static bool LoadSolidColourLightMaterialToMemory(const string &name, const Float4 &diffusecolour= Float4(1));

	private:
		TextureGenerator(); // private constructor to prevent instantiating this class
	};

}
#endif