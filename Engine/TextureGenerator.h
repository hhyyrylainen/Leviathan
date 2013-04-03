#ifndef LEVIATHAN_TEXTURE_GENERATOR
#define LEVIATHAN_TEXTURE_GENERATOR
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "DDsHandler.h"
#include "Rendering\ManagedTexture.h"

namespace Leviathan{

	class TextureGenerator/* : public Object "static class" don't inherit anything*/{
	public:
		DLLEXPORT static ManagedTexture* GenerateCheckerBoard(int width, int height, unsigned int colours, int tilesperwidth, int tilesperheight, vector<Int3>& colors, ID3D11Device* dev);
		DLLEXPORT static ManagedTexture* GenerateCheckerBoard(int width, int height, unsigned int colours, int tilesperside, vector<Int3>& colors, ID3D11Device* dev);
	private:
		TextureGenerator(); // private constructor to prevent instantiating this class
	};

}
#endif