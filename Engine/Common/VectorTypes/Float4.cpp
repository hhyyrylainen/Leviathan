#include "Float4.h"

using namespace Leviathan;

const Float4 Float4::ColourBlack = Float4(0, 0, 0, 1);
const Float4 Float4::ColourWhite = Float4(1, 1, 1, 1);
const Float4 Float4::ColourTransparent = Float4(0, 0, 0, 0);

DLLEXPORT const Float4& Float4::GetColourBlack()
{
    return ColourBlack;
}

DLLEXPORT const Float4& Float4::GetColourWhite()
{
    return ColourWhite;
}

DLLEXPORT const Float4& Float4::GetColourTransparent()
{
    return ColourTransparent;
}

DLLEXPORT std::ostream& Leviathan::operator<<(std::ostream& stream, const Float4& value)
{
    stream << "[" << value.X << ", " << value.Y << ", " << value.Z << ", " << value.W << "]";
    return stream;
}
