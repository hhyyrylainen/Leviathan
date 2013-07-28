#ifndef LEVIATHAN_RENDERING_GUIRBLOB
#define LEVIATHAN_RENDERING_GUIRBLOB
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "..\GuiPositionable.h"

#define GUIRENDERING_BLOB_TYPE_CQUAD				1
#define GUIRENDERING_BLOB_TYPE_TEXT					2
#define GUIRENDERING_BLOB_TYPE_EXPENSIVETEXT		3

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
		DLLEXPORT ColorQuadRendBlob::ColorQuadRendBlob(int relativez, int slotid, const Float2 &xypos, const Float4 &color, 
			const Float4 &color2, const Float2 &size, int colortranstype, int coordinatetype = GUI_POSITIONABLE_COORDTYPE_RELATIVE);
		DLLEXPORT virtual ColorQuadRendBlob::~ColorQuadRendBlob();

		DLLEXPORT void Update(int relativez, const Float2 &xypos, const Float4 &color, const Float4 &color2, const Float2 &size, 
			int colortranstype, int coordinatetype = GUI_POSITIONABLE_COORDTYPE_RELATIVE);
		DLLEXPORT void Get(Float2 &xypos, Float4 &color, Float4 &color2, Float2 &size, int &colortranstype, int &coordinatetype);
		DLLEXPORT bool HasUpdated();
		DLLEXPORT bool ConsumeUpdate();

		// objects that renderer uses, should not be touched //
		ColorQuad* CQuad;
	private:
		// values that should not directly be set //
		int CoordType;
		Float2 Coord;
		Float2 Size;
		Float4 Color1;
		Float4 Color2;
		int ColorTransType;
		bool Updated;
	};

	class BasicTextRendBlob: public RenderingGBlob{
	public:
		DLLEXPORT BasicTextRendBlob::BasicTextRendBlob();
		DLLEXPORT BasicTextRendBlob::BasicTextRendBlob(int relativez, int slotid, const Float2 &xypos, const Float4 &color, float sizemod, 
			const wstring &text, const wstring &font, int coordtype = GUI_POSITIONABLE_COORDTYPE_RELATIVE);
		DLLEXPORT virtual BasicTextRendBlob::~BasicTextRendBlob();
		
		DLLEXPORT void Update(int relativez, const Float2 &xypos, const Float4 &color, float sizemod, const wstring &text, 
			const wstring &font, int coordtype = GUI_POSITIONABLE_COORDTYPE_RELATIVE);
		DLLEXPORT void Get(Float2 &xypos, Float4 &color, float &size, wstring &text, wstring &font, int &coordtype, int& textid);
		DLLEXPORT bool HasUpdated();
		DLLEXPORT bool ConsumeUpdate();
		DLLEXPORT void SetUpdated();

		// objects that renderer uses, should not be touched //
		bool HasText;

	private:
		// values that should not directly be set //
		int CoordType;
		float Size;
		Float2 Coord;
		Float4 Color;
		bool Updated;
		wstring Font;
		wstring Text;
		int TextID;
	};

	class ExpensiveTextRendBlob: public RenderingGBlob{
		friend class TextRenderer;
		friend class ExpensiveText;
	public:
		DLLEXPORT ExpensiveTextRendBlob::ExpensiveTextRendBlob(int relativez, int slotid, const Float2 &xypos, const Float4 &color, float sizemod, 
			const wstring &text, const wstring &font, int coordtype = GUI_POSITIONABLE_COORDTYPE_RELATIVE, bool fittobox = false, 
			const Float2 box = (Float2)0, const float &adjustcutpercentage = 0.4f);
		DLLEXPORT virtual ExpensiveTextRendBlob::~ExpensiveTextRendBlob();

		DLLEXPORT void Update(int relativez, const Float2 &xypos, const Float4 &color, float sizemod, const wstring &text, 
			const wstring &font, int coordtype = GUI_POSITIONABLE_COORDTYPE_RELATIVE, bool fittobox = false, 
			const Float2 box = (Float2)0, const float &adjustcutpercentage = 0.4f);

	protected:
		// values that should not directly be set //
		float Size;

		Float2 Coord;
		Float2 BoxToFit;
		bool FitToBox;
		int CoordType;

		Float4 Color;

		wstring Font;
		wstring Text;

		int TextID;
		float AdjustCutModifier;
	};

}
#endif