#include "Include.h"
// ------------------------------------ //
#include "Types.h"
// ------------------------------------ //
#include <ostream>
#include <iostream>

// specific colours //
namespace Leviathan {

const Float4 Float4::ColourBlack = Float4(0, 0, 0, 1);

const Float4 Float4::ColourWhite = Float4(1, 1, 1, 1);

const Float4 Float4::ColourTransparent = Float4(0, 0, 0, 0);

const Float3 Float3::UnitVForward = Float3(0.f, 0.f, -1.f);

const Float3 Float3::UnitVUp = Float3(0.f, 1.f, 0.f);

const Float3 Float3::Zeroed = Float3::zero();



DLLEXPORT const Float4& Float4::GetColourBlack() {
    return ColourBlack;
}

DLLEXPORT const Float4& Float4::GetColourWhite() {
    return ColourWhite;
}

DLLEXPORT const Float4& Float4::GetColourTransparent() {
    return ColourTransparent;
}
}

// ------------------ Stream operators ------------------ //
namespace Leviathan{
DLLEXPORT std::ostream& operator <<(std::ostream &stream,
    const Float4 &value) {

    stream << "[" << value.X << ", " << value.Y << ", " << value.Z << ", " << value.W << "]";
    return stream;
}

DLLEXPORT std::ostream& operator <<(std::ostream &stream,
    const Float3 &value) {

    stream << "[" << value.X << ", " << value.Y << ", " << value.Z << "]";
    return stream;
}

DLLEXPORT std::ostream& operator <<(std::ostream &stream,
    const Leviathan::Float2 &value)
{
    stream << "[" << value.X << ", " << value.Y << "]";
    return stream;
}


DLLEXPORT std::ostream& operator<<(std::ostream &stream, const PotentiallySetIndex &value) {

    if (value.IsSet()) {

        stream << value.Index;

    } else {

        stream << "NOT SET (" << value.Index << ")";
    }

    return stream;
}

DLLEXPORT std::ostream& operator<<(std::ostream &stream, const StartEndIndex &value) {

    stream << "[" << value.Start << " | " << value.End << "]";
    return stream;
}

}
