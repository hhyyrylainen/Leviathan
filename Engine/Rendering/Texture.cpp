// ------------------------------------ //
#include "Texture.h"


using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Texture::Texture(const bs::HTexture& texture) : BsTexture(texture)
{
    LEVIATHAN_ASSERT(texture, "Given bsf texture is null");
}
