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
#include ".\DataStore.h"
#include <ft2build.h>
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
		DLLEXPORT bool AdjustTextSizeToFitBox(const float &Size, const Float2 &BoxToFit, const wstring &text, int CoordType, size_t &Charindexthatfits, 
			float &EntirelyFitModifier, float &HybridScale, Float2 &Finallength, float scaletocutfrom);
		DLLEXPORT bool RenderSentenceToTexture(const int &TextureID, const float &sizemodifier, const wstring &text, Float2 &RenderedToBox);


		wstring Name;

		static FT_Library FreeTypeLibrary;
	private:
		struct FontType{

			float left, right;
			int size;
			//int height;
		};
		// --------------------- //
		vector<FontType> FontData;
		TextureArray* Textures;
		int FontHeight;
		static bool FreeTypeLoaded;

		wstring FontPath;
		FT_Face FontsFace;

		// --------------------- //
		static bool CheckFreeTypeLoad();
		bool _VerifyFontFTDataLoaded();

		bool CreateFontData(wstring texture, wstring texturedatafile);
		bool LoadFontData(ID3D11Device* dev,wstring file);
		bool LoadTexture(ID3D11Device* dev, wstring file, bool forcegen = false);
		float CalculateTextLengthAndLastFitting(float TextSize, int CoordType, const wstring &text, const float &fitlength, size_t & Charindexthatfits, float delimiterlength);
		float CalculateDotsSizeAtScale(const float &scale);

	};

}
#endif