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

namespace Leviathan{

	// TextRenderer must inherit AutoUpdateableObject because it needs to be notified when resolution changes //
	class TextRenderer : public EngineComponent, public AutoUpdateableObject{
	public:
		DLLEXPORT TextRenderer::TextRenderer();
		DLLEXPORT TextRenderer::~TextRenderer();

		DLLEXPORT bool Init(ID3D11Device* dev, ID3D11DeviceContext* devcont, Window* wind, D3DXMATRIX baseview);
		DLLEXPORT void Release();

		DLLEXPORT bool Render(ID3D11DeviceContext* devcont, D3DXMATRIX worldmatrix, D3DXMATRIX orthomatrix);
		DLLEXPORT bool RenderSingle(int ID, ID3D11DeviceContext* devcont, D3DXMATRIX worldmatrix, D3DXMATRIX orthomatrix);

		DLLEXPORT int CountSentenceLength(wstring &sentence, wstring &font, float heightmod, bool IsAbsolute, bool TranslateSize = true);
		DLLEXPORT int GetFontHeight(wstring &font, float heightmod, bool IsAbsolute, bool TranslateSize = true);


		DLLEXPORT bool CreateSentence(int id, int maxlength, ID3D11Device* dev);
		DLLEXPORT bool UpdateSentenceID(int id, bool absolute, wstring &Font, wstring &text, int x, int y, Float4 &color, float sizepercent,
			ID3D11DeviceContext* devcont, bool TranslateSize = true);
		DLLEXPORT void ReleaseSentenceID(int id);
		DLLEXPORT void HideSentence(int id, bool hidden);


		DLLEXPORT void CheckUpdatedValues();

	private:
		struct SentenceType
		{
			SentenceType(){
				Hidden = false;
			}
			ID3D11Buffer *vertexBuffer, *indexBuffer;
			int vertexCount, indexCount, maxLength;
			float red, green, blue;
			float Height;

			int SentenceID;
			int FontID;
			bool Hidden;

			bool Absolute;

			wstring text;
			int posx, posy; 
			bool TranslateSize;
		};

		// ------------------ //
		void LoadFont(const wstring &file);
		int GetFontIndex(const wstring &name);

		bool InitializeSentence(SentenceType** sentence, int id, int maxlength, ID3D11Device* dev);
		bool UpdateSentence(SentenceType* sentence, bool absolute, wstring &text, int posx, int posy, float red, float green, float blue, 
			float heightpercent, int fontindex, ID3D11DeviceContext* devcont, bool TranslateSize = true);
		void ReleaseSentence(SentenceType** sentence);
		bool RenderSentence(ID3D11DeviceContext* devcont, SentenceType* sentence, D3DXMATRIX worldMatrix, D3DXMATRIX orthoMatrix);

		// ------------------ //
		//Font* m_Font;
		FontShader* _FontShader;
		int ScreenWidth, ScreenHeight;
		D3DXMATRIX BaseViewMatrix;

		ID3D11Device* device;

		//vector<SentenceType*> Sentences;

		vector<RenderingFont*> FontHolder;
		vector<SentenceType*> Sentences;
	};

}
#endif