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
#include FT_GLYPH_H
#include "ShaderDataTypes.h"
#include "..\ScaleableFreeTypeBitmap.h"
#include "..\GuiPositionable.h"
#include "..\ComplainOnce.h"


#define RENDERINGFONT_CHARCOUNT		233
#define RENDERINGFONT_MAXCHARCODE	255

#define FONT_BASEPIXELHEIGHT		32

namespace Leviathan{
	// forward declaration for friend declaration //
	class RenderingFont;

	struct GeneratingDataForCharacter{
		GeneratingDataForCharacter();
		// used when creating the char map, contains this glyph rendered from FT bitmap //
		//ScaleableFreeTypeBitmap* ThisRendered;
		FT_Glyph ThisRendered;
		//FT_Bitmap ThisRendered;
		int RenderedTop;
		int RenderedLeft;
	};

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
		// pixel count that rendering uses to advance to next character //
		int AdvancePixels;
		// used when generating font data //
		GeneratingDataForCharacter* Generating;
	};

	class RenderingFont : public EngineComponent{
	public:
		DLLEXPORT RenderingFont::RenderingFont();
		DLLEXPORT RenderingFont::~RenderingFont();

		DLLEXPORT bool Init(ID3D11Device* dev, const wstring &FontFile);
		DLLEXPORT void Release();

		// functions to get/calculate various things about this font (with a string) //
		DLLEXPORT inline float CountLengthNonExpensive(const wstring &sentence, float heightmod, int Coordtype){
			// call another function, this is so that the parameters don't all need to be created to call it //
			float delimiter = 0;
			size_t lastfit = 0;
			float fitlength = 0;
			// actual counting //
			return CalculateTextLengthAndLastFittingNonExpensive(heightmod, Coordtype, sentence, fitlength, lastfit, delimiter);
		}
		DLLEXPORT inline float CountLengthExpensive(const wstring &sentence, float heightmod, int Coordtype){
			// call another function, this is so that the parameters don't all need to be created to call it //
			float delimiter = 0;
			size_t lastfit = 0;
			float fitlength = 0;
			// actual counting //
			return CalculateTextLengthAndLastFittingExpensive(heightmod, Coordtype, sentence, fitlength, lastfit, delimiter);
		}
		DLLEXPORT inline float GetHeight(float heightmod, int Coordtype){
			if(Coordtype != GUI_POSITIONABLE_COORDTYPE_RELATIVE)
				//return FontHeight*heightmod;
				return (float)CalculatePixelSizeAtScale(heightmod);

			// scale from screen height to promilles //
			//return (FontHeight*heightmod)/DataStore::Get()->GetHeight();
			return CalculatePixelSizeAtScale(heightmod)/(float)DataStore::Get()->GetHeight();
		}
		DLLEXPORT float CalculateTextLengthAndLastFittingNonExpensive(float TextSize, int CoordType, const wstring &text, const float &fitlength, 
			size_t & Charindexthatfits, float delimiterlength);
		DLLEXPORT float CalculateTextLengthAndLastFittingExpensive(float TextSize, int CoordType, const wstring &text, const float &fitlength, 
			size_t & Charindexthatfits, float delimiterlength);

		DLLEXPORT inline int CalculatePixelSizeAtScale(const float &scale){
			return (int)ceilf(FONT_BASEPIXELHEIGHT*scale);
		}

		// gets length of string "..." at a specified scale //
		DLLEXPORT float CalculateDotsSizeAtScale(const float &scale);

		DLLEXPORT inline bool EnsurePixelSize(const int &size){
			// we only need to do something if size isn't the one already set //
			if(SetSize != size){
				return _SetFTPixelSize(size);
			}
			return true;
		}

		


		ID3D11ShaderResourceView* GetTexture();
		DLLEXPORT inline wstring& GetName(){
			return Name;
		}

		DLLEXPORT bool BuildVertexArray(VertexType* vertexptr, const wstring &text, float drawx, float drawy, float textmodifier);
		DLLEXPORT bool AdjustTextSizeToFitBox(const Float2 &BoxToFit, const wstring &text, int CoordType, size_t &Charindexthatfits, 
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

		DLLEXPORT inline float GetKerningBetweenCharacters(const float &scaleonkerning, const int &char1, const int &char2){
			// hopefully FreeType interface is already loaded //
			if(!FontsFace)
				_VerifyFontFTDataLoaded();
			// return if no kerning //
			if(!FT_HAS_KERNING(FontsFace))
				return 0;

			// we should really verify pixel size here //
			if(!EnsurePixelSize(CalculatePixelSizeAtScale(scaleonkerning))){

			}

			// result receiver //
			FT_Vector kerning;

			// get kerning //
			FT_Error errorcode = FT_Get_Kerning(FontsFace, FontData[ConvertCharCodeToIndex(char1)]->CharacterGlyphIndex, 
				FontData[ConvertCharCodeToIndex(char2)]->CharacterGlyphIndex, FT_KERNING_DEFAULT, &kerning);
			if(errorcode){
				// report //
				ComplainOnce::PrintWarningOnce(L"Failed_kerningdata", L"failed to retrieve kerning data");
			}
			// this should work for converting to pixels //
			return (float)(kerning.x >> 6);
		}

		//DLLEXPORT static inline void CopyFTBitmapToAnother(FT_Bitmap &receiver, const FT_Bitmap &original);

	private:
		// retrieves FontsFace from FT library //
		bool _VerifyFontFTDataLoaded();
		// sets internal pixel size of the FreeType face associated with this object //
		bool _SetFTPixelSize(const int & size);
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
		int SetSize;

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