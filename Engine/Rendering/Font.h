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

namespace Leviathan{

	class Font : public EngineComponent{
	public:
		DLLEXPORT Font::Font();
		DLLEXPORT Font::~Font();

		DLLEXPORT bool Init(ID3D11Device* dev, wstring FontFile);
		DLLEXPORT void Release();

		DLLEXPORT int CountLenght(wstring &sentence, float heightmod, bool IsAbsolute, bool TranslateSize = true);
		DLLEXPORT int GetHeight(float heightmod, bool IsAbsolute, bool TranslateSize = true);


		ID3D11ShaderResourceView* GetTexture();

		void BuildVertexArray(void* vertices, wstring text, float drawx, float draw, float heightmod, bool IsAbsolute, bool TranslateSize = true);

		wstring Name;

		static FT_Library FreeTypeLibrary;
	private:
		struct FontType
		{
			float left, right;
			int size;
			//int height;
		};
		struct VertexType	// must match the one in FontClass! // ----------------
		{
			D3DXVECTOR3 position;
			D3DXVECTOR2 texture;
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