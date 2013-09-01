#ifndef LEVIATHAN_TEXTUREHOLDER
#define LEVIATHAN_TEXTUREHOLDER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Logger.h"
#include <d3dx11tex.h>

namespace Leviathan{
class RenderTargetableTexture : public EngineComponent{
public:
	RenderTargetableTexture();

	bool Init(ID3D11Device*, int width, int height);
	void Release();

	void SetRenderTarget(ID3D11DeviceContext* devcont, ID3D11DepthStencilView* stencilview);
	void ClearRenderTarget(ID3D11DeviceContext* devcont, ID3D11DepthStencilView* stencilview, float red, float green, float blue, float alpha);
	ID3D11ShaderResourceView* GetShaderResourceView();

private:
	ID3D11Texture2D* pTexture;
	ID3D11RenderTargetView* RenderTargetView;
	ID3D11ShaderResourceView* ShaderResourceView;
};

}
#endif