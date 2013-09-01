#ifndef LEVIATHAN_RENDERING_GUIRBLOB
#define LEVIATHAN_RENDERING_GUIRBLOB
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "GUI\Components\GuiPositionable.h"
#include "Rendering\RenderingPassInfo.h"

namespace Leviathan{

	// some forward declarations //
	class Graphics;

	class RenderingGBlob : public Object{
	public:
		DLLEXPORT RenderingGBlob::RenderingGBlob(const int &relativez, const int &slotid, const bool &hidden);
		DLLEXPORT virtual RenderingGBlob::~RenderingGBlob();

		// for automatic rendering //
		//DLLEXPORT virtual Rendering::BaseRenderableBufferContainer* GetRenderingBuffers(Graphics* graph) = 0;

		int RelativeZ;
		int SlotID;
		bool Hidden;

	protected:


	};

	// ----- DERIVED CLASSES ----- //

	class ColorQuadRendBlob: public RenderingGBlob{
	public:
		DLLEXPORT ColorQuadRendBlob::ColorQuadRendBlob(Graphics* graph, const int &relativez, const int &slotid, const bool &hidden);
		DLLEXPORT virtual ColorQuadRendBlob::~ColorQuadRendBlob();


		// for automatic rendering //
		//DLLEXPORT virtual Rendering::BaseRenderableBufferContainer* GetRenderingBuffers(Graphics* graph);

		DLLEXPORT void Update(Graphics* graph, const int &relativez, const Float2 &xypos, const Float4 &color, const Float4 &color2, const Float2 &size, 
			int colortranstype, int coordinatetype = GUI_POSITIONABLE_COORDTYPE_RELATIVE);
	private:
		// values that should not directly be set //

	};

	class TextRendBlob: public RenderingGBlob{
		friend class TextRenderer;
		friend class ExpensiveText;
	public:
		DLLEXPORT TextRendBlob::TextRendBlob(Graphics* graph, const int &relativez, const int &slotid, const bool &hidden);
		DLLEXPORT virtual TextRendBlob::~TextRendBlob();

		DLLEXPORT void Update(int relativez, const Float2 &xypos, const Float4 &color, float sizemod, const wstring &text, 
			const wstring &font, int coordtype = GUI_POSITIONABLE_COORDTYPE_RELATIVE, bool fittobox = false, 
			const Float2 box = (Float2)0, const float &adjustcutpercentage = 0.4f);

		// for automatic rendering //
		//DLLEXPORT virtual Rendering::BaseRenderableBufferContainer* GetRenderingBuffers(Graphics* graph);

	protected:

	};

}
#endif