#ifndef LEVIATHAN_RENDERING_GUIRBLOB
#define LEVIATHAN_RENDERING_GUIRBLOB
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#define GUIRENDERING_BLOB_TYPE_CQUAD	1
#define GUIRENDERING_BLOB_TYPE_TEXT		2

namespace Leviathan{

	// some forward declarations //
	class ColorQuad;

	class RenderingGBlob : public Object{
	public:
		DLLEXPORT RenderingGBlob::RenderingGBlob();
		DLLEXPORT RenderingGBlob::RenderingGBlob(int relativez, int slotid);
		DLLEXPORT virtual RenderingGBlob::~RenderingGBlob();
		DLLEXPORT bool IsThisType(int tochecktype);

		int RelativeZ;
		int SlotID;
		bool Hidden;
		bool Updated;
	protected:
		int TypeName;
	};

	// ----- DERIVED CLASSES ----- //

	class ColorQuadRendBlob: public RenderingGBlob{
	public:
		DLLEXPORT ColorQuadRendBlob::ColorQuadRendBlob();
		DLLEXPORT ColorQuadRendBlob::ColorQuadRendBlob(int relativez, int slotid, const Int2 &xypos, const Float4 &color, 
			const Float4 &color2, int width, int height, int colortranstype, bool absolute = false);
		DLLEXPORT ColorQuadRendBlob::~ColorQuadRendBlob();

		DLLEXPORT void Update(int relativez, const Int2 &xypos, const Float4 &color, const Float4 &color2, int width, int height, 
			int colortranstype, bool absolute = false);
		DLLEXPORT void Get(Int2 &xypos, Float4 &color, Float4 &color2, Int2 &size, int &colortranstype, bool &absolute);
		DLLEXPORT bool HasUpdated();
		DLLEXPORT bool ConsumeUpdate();

		// objects that renderer uses, should not be touched //
		ColorQuad* CQuad;
		//int SmoothX;
		//int SmoothY;
		//int SmoothWidth;
		//int SmoothHeight;

	private:
		// values that should not directly be set //
		bool AbsoluteCoord;
		Int2 Coord;
		Int2 Size;
		Float4 Color1;
		Float4 Color2;
		int ColorTransType;
		bool Updated;
	};

	class BasicTextRendBlob: public RenderingGBlob{
	public:
		DLLEXPORT BasicTextRendBlob::BasicTextRendBlob();
		DLLEXPORT BasicTextRendBlob::BasicTextRendBlob(int relativez, int slotid, const Int2 &xypos, const Float4 &color, float sizemod, 
			const wstring &text, bool absolute = false, const wstring &font = L"Arial");
		DLLEXPORT BasicTextRendBlob::~BasicTextRendBlob();
		
		DLLEXPORT void Update(int relativez, const Int2 &xypos, const Float4 &color, float sizemod, const wstring &text, 
			bool absolute = false, const wstring &font = L"Arial");
		DLLEXPORT void Get(Int2 &xypos, Float4 &color, float &size, wstring &text, wstring &font, bool &absolute, int& textid);
		DLLEXPORT bool HasUpdated();
		DLLEXPORT bool ConsumeUpdate();
		DLLEXPORT void SetUpdated();

		// objects that renderer uses, should not be touched //
		bool HasText;
		//int SmoothX;
		//int SmoothY;

	private:
		// values that should not directly be set //
		bool AbsoluteCoord;
		float Size;
		Int2 Coord;
		Float4 Color;
		bool Updated;
		wstring Font;
		wstring Text;
		int TextID;
	};
}
#endif