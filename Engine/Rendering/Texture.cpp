// ------------------------------------ //
#include "Texture.h"


using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Texture::Texture(
    const Diligent::RefCntAutoPtr<Diligent::ITexture>& texture, int width, int height) :
    _Texture(texture),
    Width(width), Height(height)
{}

DLLEXPORT Texture::Texture(const Diligent::RefCntAutoPtr<Diligent::ITexture>& texture) :
    _Texture(texture)
{
    Width = _Texture->GetDesc().Width;
    Height = _Texture->GetDesc().Height;
}
