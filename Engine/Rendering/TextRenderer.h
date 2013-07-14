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
#include "FontShader.h"
#include "ShaderDataTypes.h"
#include "GuiRendBlob.h"

namespace Leviathan{
	// forward declaration for friend classes //
	class TextRenderer;



	struct SentenceType{

		SentenceType(){
			Hidden = false;
		}
		ID3D11Buffer *VertexBuffer, *IndexBuffer;
		int vertexCount, indexCount, maxLength;
		Float4 Colour;
		float SizeModifier;

		int SentenceID;
		int FontID;
		bool Hidden;

		int CoordType;

		wstring text;
		Float2 Position;
	};
	// used to store expensive text //
	class ExpensiveText{
		friend TextRenderer;
	public:
		ExpensiveText(const int &id) : ID(id), VertexBuffer(NULL), IndexBuffer(NULL){

			TextureID = -1;
		}
		~ExpensiveText(){
			// release buffers //

		}
		DLLEXPORT bool UpdateIfNeeded(ExpensiveTextRendBlob* renderptr, TextRenderer* render, int ScreenWidth, int ScreenHeight);
		DLLEXPORT bool PrepareRender(ID3D11DeviceContext* devcont);
		DLLEXPORT bool Render(FontShader* shader, TextRenderer* trender, ID3D11DeviceContext* devcont, D3DXMATRIX worldmatrix, D3DXMATRIX orthomatrix);
		DLLEXPORT void AdjustToFit(TextRenderer* trenderer, bool trytocenter = false);
	protected:

		bool _VerifyBuffers(ID3D11DeviceContext* devcont);
		bool _VerifyTextures(TextRenderer* trender);

		// ------------------------------------ //
		int ID;
		bool BuffersFine;

		ID3D11Buffer* VertexBuffer;
		ID3D11Buffer* IndexBuffer;


		float Size;

		Float2 RenderedToBox;
		int RenderedBaseline;

		bool AdjustedToFit;
		float AdjustedSize;

		Float2 Coord;
		int CoordType;

		Float2 BoxToFit;
		bool FitToBox;

		Float4 Color;

		wstring Font;
		wstring Text;
		wstring AdjustedText;

		int ScreenWidth, ScreenHeight;

		int TextureID;
	};

	// TextRenderer must inherit AutoUpdateableObject because it needs to be notified when resolution changes //
	class TextRenderer : public EngineComponent, public AutoUpdateableObject{
		friend ExpensiveText;
	public:
		DLLEXPORT TextRenderer::TextRenderer();
		DLLEXPORT TextRenderer::~TextRenderer();

		DLLEXPORT bool Init(ID3D11Device* dev, ID3D11DeviceContext* devcont, Window* wind, D3DXMATRIX baseview);
		DLLEXPORT void Release();

		DLLEXPORT bool Render(ID3D11DeviceContext* devcont, D3DXMATRIX worldmatrix, D3DXMATRIX orthomatrix);
		DLLEXPORT bool RenderSingle(int ID, ID3D11DeviceContext* devcont, D3DXMATRIX worldmatrix, D3DXMATRIX orthomatrix);
		DLLEXPORT bool RenderExpensiveText(ExpensiveTextRendBlob* renderptr, ID3D11DeviceContext* devcont, D3DXMATRIX worldmatrix, D3DXMATRIX orthomatrix);


		DLLEXPORT float CountSentenceLength(const wstring &sentence, const wstring &font, float heightmod, int coordtype);
		DLLEXPORT float GetFontHeight(const wstring &font, float heightmod, int coordtype);


		DLLEXPORT ExpensiveText* GetExpensiveText(const int &ID);
		DLLEXPORT bool ReleaseExpensiveText(const int &ID);
		DLLEXPORT bool RenderExpensiveTextToTexture(ExpensiveText* text, const int &TextureID);
		DLLEXPORT bool AdjustTextToFitBox(const float &Size, const Float2 &BoxToFit, const wstring &text, const wstring &font, 
			int CoordType, size_t &Charindexthatfits, float &EntirelyFitModifier, float &HybridScale, Float2 &Finallength, float scaletocutfrom = 0.5f);

		DLLEXPORT bool CreateSentence(int id, int maxlength, ID3D11Device* dev);
		DLLEXPORT bool UpdateSentenceID(int id, int Coordtype, const wstring &font, const wstring &text, const Float2 &coordinates, 
			const Float4 &color, float sizepercent, ID3D11DeviceContext* devcont);
		DLLEXPORT void ReleaseSentenceID(int id);
		DLLEXPORT void HideSentence(int id, bool hidden);


		DLLEXPORT void CheckUpdatedValues();

	private:


		// ------------------ //
		void LoadFont(const wstring &file);
		int GetFontIndex(const wstring &name);

		bool InitializeSentence(SentenceType** sentence, int id, int maxlength, ID3D11Device* dev);
		bool UpdateSentence(SentenceType* sentence, int Coordtype, const wstring &text, const Float2 &position, const Float4 &colour, 
			float textmodifier, int fontindex, ID3D11DeviceContext* devcont);
		void ReleaseSentence(SentenceType** sentence);
		bool RenderSentence(ID3D11DeviceContext* devcont, SentenceType* sentence, D3DXMATRIX worldMatrix, D3DXMATRIX orthoMatrix);
		// ------------------ //
		FontShader* _FontShader;

		int ScreenWidth, ScreenHeight;
		D3DXMATRIX BaseViewMatrix;

		ID3D11Device* device;

		vector<RenderingFont*> FontHolder;
		vector<SentenceType*> Sentences;

		vector<ExpensiveText*> ExpensiveTexts;
	};

}
#endif