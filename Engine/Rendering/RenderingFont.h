#ifndef LEVIATHAN_FONT
#define LEVIATHAN_FONT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ResolutionScaling.h"
#include "TextureArray.h"
#include <ft2build.h>
#include ".\DataStore.h"
#include FT_FREETYPE_H
#include "ShaderDataTypes.h"

namespace Leviathan{

	class RenderingFont : public EngineComponent{
	public:
		DLLEXPORT RenderingFont::RenderingFont();
		DLLEXPORT RenderingFont::~RenderingFont();

		DLLEXPORT bool Init(ID3D11Device* dev, wstring FontFile);
		DLLEXPORT void Release();

		DLLEXPORT float CountLength(const wstring &sentence, float heightmod, int Coordtype);
		DLLEXPORT float GetHeight(float heightmod, int Coordtype);


		ID3D11ShaderResourceView* GetTexture();

		bool BuildVertexArray(VertexType* vertexptr, const wstring &text, float drawx, float drawy, float textmodifier, int Coordtype);

		wstring Name;

		static FT_Library FreeTypeLibrary;
	private:
		struct FontType
		{
			float left, right;
			int size;
			//int height;
		};
		// --------------------- //
		//FontType* Fontdata;
		vector<FontType> FontData;
		TextureArray* Textures;
		int FontHeight;
		static bool FreeTypeLoaded;

		// --------------------- //
		static bool CheckFreeTypeLoad();
		bool CreateFontData(wstring texture, wstring texturedatafile);
		bool LoadFontData(ID3D11Device* dev,wstring file);
		bool LoadTexture(ID3D11Device* dev, wstring file, bool forcegen = false);




	};

}
#endif