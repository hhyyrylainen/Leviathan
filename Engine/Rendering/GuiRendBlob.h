#ifndef LEVIATHAN_RENDERING_GUIRBLOB
#define LEVIATHAN_RENDERING_GUIRBLOB
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "..\GuiPositionable.h"
#include "..\BaseRenderableBufferContainer.h"
#include "..\ShaderRenderTask.h"
#include ".\Rendering\RenderingQuad.h"
#include "RenderingPassInfo.h"

namespace Leviathan{

	// some forward declarations //
	class RenderingQuad;
	class Graphics;
	class TextRenderer;

	class RenderingGBlob : public Object{
	public:
		DLLEXPORT RenderingGBlob::RenderingGBlob(const int &relativez, const int &slotid, const bool &hidden);
		DLLEXPORT virtual RenderingGBlob::~RenderingGBlob();

		// for automatic rendering //
		DLLEXPORT virtual Rendering::BaseRenderableBufferContainer* GetRenderingBuffers(Graphics* graph) = 0;
		DLLEXPORT virtual ShaderRenderTask* GetShaderParameters(Graphics* graph, RenderingPassInfo* pass) = 0;

		int RelativeZ;
		int SlotID;
		bool Hidden;

	protected:
		virtual void EnsureShaderRenderTask() = 0;
		virtual void SetShaderMatrixBuffers(RenderingPassInfo* pass);

		ShaderRenderTask* SHRender;
	};

	// ----- DERIVED CLASSES ----- //

	class ColorQuadRendBlob: public RenderingGBlob{
	public:
		DLLEXPORT ColorQuadRendBlob::ColorQuadRendBlob(Graphics* graph, const int &relativez, const int &slotid, const bool &hidden);
		DLLEXPORT virtual ColorQuadRendBlob::~ColorQuadRendBlob();


		// for automatic rendering //
		DLLEXPORT virtual Rendering::BaseRenderableBufferContainer* GetRenderingBuffers(Graphics* graph);
		DLLEXPORT virtual ShaderRenderTask* GetShaderParameters(Graphics* graph, RenderingPassInfo* pass);

		DLLEXPORT void Update(Graphics* graph, const int &relativez, const Float2 &xypos, const Float4 &color, const Float4 &color2, const Float2 &size, 
			int colortranstype, int coordinatetype = GUI_POSITIONABLE_COORDTYPE_RELATIVE);
	private:
		virtual void EnsureShaderRenderTask();
		// values that should not directly be set //
		Rendering::RenderingQuad* CQuad;
	};

	class BasicTextRendBlob: public RenderingGBlob{
	public:
		DLLEXPORT BasicTextRendBlob::BasicTextRendBlob(Graphics* graph, const int &relativez, const int &slotid, const bool &hidden);
		DLLEXPORT virtual BasicTextRendBlob::~BasicTextRendBlob();
		
		DLLEXPORT void Update(int relativez, const Float2 &xypos, const Float4 &color, float sizemod, const wstring &text, 
			const wstring &font, int coordtype = GUI_POSITIONABLE_COORDTYPE_RELATIVE);

		// for automatic rendering //
		DLLEXPORT virtual Rendering::BaseRenderableBufferContainer* GetRenderingBuffers(Graphics* graph);
		DLLEXPORT virtual ShaderRenderTask* GetShaderParameters(Graphics* graph, RenderingPassInfo* pass);

	private:
		virtual void EnsureShaderRenderTask();

		// values that should not directly be set //
		int TextID;
		// text renderer instance //
		TextRenderer* TRenderer;
	};

	class ExpensiveTextRendBlob: public RenderingGBlob{
		friend class TextRenderer;
		friend class ExpensiveText;
	public:
		DLLEXPORT ExpensiveTextRendBlob::ExpensiveTextRendBlob(Graphics* graph, const int &relativez, const int &slotid, const bool &hidden);
		DLLEXPORT virtual ExpensiveTextRendBlob::~ExpensiveTextRendBlob();

		DLLEXPORT void Update(int relativez, const Float2 &xypos, const Float4 &color, float sizemod, const wstring &text, 
			const wstring &font, int coordtype = GUI_POSITIONABLE_COORDTYPE_RELATIVE, bool fittobox = false, 
			const Float2 box = (Float2)0, const float &adjustcutpercentage = 0.4f);

		// for automatic rendering //
		DLLEXPORT virtual Rendering::BaseRenderableBufferContainer* GetRenderingBuffers(Graphics* graph);
		DLLEXPORT virtual ShaderRenderTask* GetShaderParameters(Graphics* graph, RenderingPassInfo* pass);

	protected:
		virtual void EnsureShaderRenderTask();
		// values that should not directly be set //
		int TextID;
		// text renderer instance //
		TextRenderer* TRenderer;
	};

}
#endif