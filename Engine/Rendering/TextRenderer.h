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

		DLLEXPORT float CountSentenceLength(const wstring &sentence, const wstring &font, float heightmod, int coordtype);
		DLLEXPORT float GetFontHeight(const wstring &font, float heightmod, int coordtype);


		DLLEXPORT bool CreateSentence(int id, int maxlength, ID3D11Device* dev);
		DLLEXPORT bool UpdateSentenceID(int id, int Coordtype, const wstring &font, const wstring &text, const Float2 &coordinates, 
			const Float4 &color, float sizepercent, ID3D11DeviceContext* devcont);
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
			Float4 Colour;
			float SizeModifier;

			int SentenceID;
			int FontID;
			bool Hidden;

			int CoordType;

			wstring text;
			Float2 Position;
		};

		// ------------------ //
		void LoadFont(const wstring &file);
		int GetFontIndex(const wstring &name);

		bool InitializeSentence(SentenceType** sentence, int id, int maxlength, ID3D11Device* dev);
		bool UpdateSentence(SentenceType* sentence, int Coordtype, const wstring &text, const Float2 &position, const Float4 &colour, float textmodifier, int fontindex, ID3D11DeviceContext* devcont);
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