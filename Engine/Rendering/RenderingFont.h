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


#define RENDERINGFONT_CHARCOUNT		233
#define RENDERINGFONT_MAXCHARCODE	255

#define FONT_BASEPIXELHEIGHT		32

namespace Leviathan{
	// forward declaration for friend declaration //
	class RenderingFont;

	// "internal" type to define single character's image in non expensive text //
	class FontsCharacter{
		friend RenderingFont;
	public:
		FontsCharacter(const int &charcode, const FT_UInt &glyphindex = 0);
		FontsCharacter(const int &charcode, const int &pixelwidth, const Float2 &texturecoordtopleft, const Float2 &texturecoordbotomright,
			const FT_UInt &glyphindex = 0);
		
	protected:
		// the hopefully right unicode code point that this character maps to //
		int CharCode;
		// cached version of character index in FreeType font face (defaults to index 0 which is missing glyph character) //
		FT_UInt CharacterGlyphIndex;
		// texture coordinates that can be used to calculate TCs for the quad that this character should be displayed in //
		Float2 TopLeft, BottomRight;
		// BottomRight.X-Topleft.X (x distance (in pixels - converted to pixels)) //
		int PixelWidth;
	};

	class RenderingFont : public EngineComponent{
	public:
		DLLEXPORT RenderingFont::RenderingFont();
		DLLEXPORT RenderingFont::~RenderingFont();

		DLLEXPORT bool Init(ID3D11Device* dev, const wstring &FontFile);
		DLLEXPORT void Release();

		// functions to get/calculate various things about this font (with a string) //
		DLLEXPORT float CountLength(const wstring &sentence, float heightmod, int Coordtype);
		DLLEXPORT float GetHeight(float heightmod, int Coordtype);
		DLLEXPORT float CalculateTextLengthAndLastFitting(float TextSize, int CoordType, const wstring &text, const float &fitlength, 
			size_t & Charindexthatfits, float delimiterlength);

		// gets length of string "..." at a specified scale //
		DLLEXPORT float CalculateDotsSizeAtScale(const float &scale);


		ID3D11ShaderResourceView* GetTexture();

		DLLEXPORT bool BuildVertexArray(VertexType* vertexptr, const wstring &text, float drawx, float drawy, float textmodifier, int Coordtype);
		DLLEXPORT bool AdjustTextSizeToFitBox(const float &Size, const Float2 &BoxToFit, const wstring &text, int CoordType, size_t &Charindexthatfits, 
			float &EntirelyFitModifier, float &HybridScale, Float2 &Finallength, float scaletocutfrom);
		DLLEXPORT bool RenderSentenceToTexture(const int &TextureID, const float &sizemodifier, const wstring &text, Int2 &RenderedToBox, 
			int &baselinefromimagetop);

		DLLEXPORT bool WriteDataToFile();

		// inline functions that are used to reduce chance of errors //
		DLLEXPORT static inline size_t ConvertCharCodeToIndex(const int &ccode){
			return (size_t)(ccode-32);
		}
		DLLEXPORT static inline int ConvertIndexToCharCode(const size_t &index){
			return (int)(index+32);
		}

	private:
		// retrieves FontsFace from FT library //
		bool _VerifyFontFTDataLoaded();
		// internal loading functions //
		bool LoadFontData(ID3D11Device* dev,const wstring &file);
		bool LoadTexture(ID3D11Device* dev, const wstring &file, bool forcegen = false);
		// ------------------------------------ //
		vector<FontsCharacter*> FontData;
		TextureArray* Textures;
		// should be the height (in pixels) of FontsCharacter texture coordinate box heights //
		int FontHeight;

		// name of this font (as used in GUI objects) //
		wstring Name;
		// path to this font's file //
		wstring FontPath;


		FT_Face FontsFace;
		// ------------------------------------ //
		// verifies that FreeType library is properly loaded //
		static bool CheckFreeTypeLoad();

		// FreeType instance //
		static bool FreeTypeLoaded;
		static FT_Library FreeTypeLibrary;
	};

}
#endif