#ifndef LEVIATHAN_2DTEXTRENDERER
#define LEVIATHAN_2DTEXTRENDERER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Window.h"
#include "ResolutionScaling.h"
#include "AutoUpdateable.h"

#include "RenderingFont.h"
#include "ShaderDataTypes.h"
#include "GuiRendBlob.h"
#include "BaseRenderableBufferContainer.h"

namespace Leviathan{
	// forward declaration for friend classes //
	class TextRenderer;
	class Graphics;

	class CheapText : public Rendering::BaseRenderableBufferContainer{
		friend TextRenderer;
	public:
		CheapText(const int &id);
		~CheapText();

		DLLEXPORT bool Update(TextRenderer* render, const wstring &text, const wstring &font, const Float2 &pos, int coordtype, float sizemodifier, 
			int screenwidth, int screenheight);

		DLLEXPORT bool SetRenderingVariablesToSH(TextRenderer* trenderer, ShaderRenderTask* task);

		DLLEXPORT virtual inline int GetIndexCount() const{
			return VertexCount;
		}
	protected:

		virtual bool CreateBuffers(ID3D11Device* device);
		// ------------------------------------ //
		int ID;
		bool BuffersFine;

		int VertexCount, MaxTextLength;
		int ScreenWidth, ScreenHeight;
		float SizeModifier;

		wstring Text;
		wstring Font;

		Float2 Position;
		int CoordType;
	};


	// used to store expensive text //
	class ExpensiveText : public Rendering::BaseRenderableBufferContainer{
		friend TextRenderer;
	public:
		ExpensiveText(const int &id) : BaseRenderableBufferContainer("C0:T0", sizeof(VertexType)), ID(id){

			TextureID = -1;
		}
		~ExpensiveText(){
		}
		DLLEXPORT bool UpdateIfNeeded(TextRenderer* render, const wstring &text, const wstring &font, const Float2 &location, int coordtype, 
			float size, float adjustcut, const Float2 &boxtofit, bool fittobox, int screenwidth, int screenheight);
		DLLEXPORT void AdjustToFit(TextRenderer* trenderer, bool trytocenter = false);

		DLLEXPORT virtual bool SetBuffersForRendering(ID3D11DeviceContext* devcont, int &indexbuffersize);
		DLLEXPORT bool SetRenderingVariablesToSH(TextRenderer* trenderer, ShaderRenderTask* task);

		DLLEXPORT virtual inline int GetIndexCount() const{
			return 6;
		}

	protected:
		virtual bool CreateBuffers(ID3D11Device* device);
		bool _VerifyBuffers(ID3D11DeviceContext* devcont);
		bool _VerifyTextures(TextRenderer* trender);

		// ------------------------------------ //
		int ID;
		bool BuffersFine;

		float Size;

		Float2 RenderedToBox;
		int RenderedBaseline;

		bool AdjustedToFit;
		float AdjustedSize;
		float AdjustCutModifier;

		Float2 Coord;
		int CoordType;

		Float2 BoxToFit;
		bool FitToBox;

		wstring Font;
		wstring Text;
		wstring AdjustedText;

		int ScreenWidth, ScreenHeight;

		int TextureID;
	};


	class TextRenderer : public EngineComponent{
		friend ExpensiveText;
	public:
		DLLEXPORT TextRenderer::TextRenderer();
		DLLEXPORT TextRenderer::~TextRenderer();

		DLLEXPORT bool Init(Graphics* graph);
		DLLEXPORT void Release();
		DLLEXPORT bool ReleaseText(const int &ID);

		DLLEXPORT float CountSentenceLength(const wstring &sentence, const wstring &font, bool expensive, float heightmod, int coordtype);
		DLLEXPORT float GetFontHeight(const wstring &font, float heightmod, int coordtype);

		DLLEXPORT RenderingFont* GetFontFromName(const wstring &name);

		DLLEXPORT ExpensiveText* GetExpensiveText(const int &ID);
		DLLEXPORT CheapText* GetCheapText(const int &ID);

		DLLEXPORT Rendering::BaseRenderableBufferContainer* GetRenderingBuffers(int TextID, ShaderRenderTask* SHRender);
		DLLEXPORT void UpdateShaderRenderTask(int TextID, ShaderRenderTask* SHRender, RenderingPassInfo* pass);

		DLLEXPORT bool RenderExpensiveTextToTexture(ExpensiveText* text, const int &TextureID);
		DLLEXPORT bool AdjustTextToFitBox(const Float2 &BoxToFit, const wstring &text, const wstring &font, int CoordType, size_t &Charindexthatfits,
			float &EntirelyFitModifier, float &HybridScale, Float2 &Finallength, float scaletocutfrom = 0.5f);

		DLLEXPORT inline Graphics* GetOwningGraphics(){
			return Graph;
		}
	private:

		bool LoadFont(const wstring &file);
		int GetFontIndex(const wstring &name);

		// ------------------------------------ //

		vector<RenderingFont*> FontHolder;

		vector<CheapText*> CheapTexts;
		vector<ExpensiveText*> ExpensiveTexts;

		// we can use this to access all directx devices //
		Graphics* Graph;
	};

}
#endif