#ifndef LEVIATHAN_2DTEXTRENDERER
#define LEVIATHAN_2DTEXTRENDERER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ResolutionScaling.h"
#include "AutoUpdateable.h"

#include "Font.h"
#include "FontShader.h"

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

		DLLEXPORT int CountSentenceLenght(wstring &sentence, wstring &font, float heightmod, bool IsAbsolute, bool TranslateSize = true);
		DLLEXPORT int GetFontHeight(wstring &font, float heightmod, bool IsAbsolute, bool TranslateSize = true);


		DLLEXPORT bool CreateSentence(int id, int maxlenght, ID3D11Device* dev);
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
		struct VertexType	// must match the one in FontClass! // ----------------
		{
			D3DXVECTOR3 position;
			D3DXVECTOR2 texture;
		};
		// ------------------ //
		void LoadFont(wstring &file);
		int GetFontIndex(wstring &name);

		bool InitializeSentence(SentenceType** sentence, int id, int maxlenght, ID3D11Device* dev);
		bool UpdateSentence(SentenceType* sentence, bool absolute, wstring &text, int posx, int posy, float red, float green, float blue, 
			float heightpercent, int fontindex, ID3D11DeviceContext* devcont, bool TranslateSize = true);
		void ReleaseSentence(SentenceType** sentence);
		bool RenderSentence(ID3D11DeviceContext* devcont, SentenceType* sentence, D3DXMATRIX worldMatrix, D3DXMATRIX orthoMatrix);

		// ------------------ //
		//Font* m_Font;
		FontShader* m_FontShader;
		int ScreenWidth, ScreenHeight;
		D3DXMATRIX m_baseViewMatrix;

		ID3D11Device* device;

		//vector<SentenceType*> Sentences;

		vector<Font*> FontHolder;
		vector<SentenceType*> Sentences;

		SentenceType* m_sentence1;
		//SentenceType* m_sentence2;

	};

}
#endif