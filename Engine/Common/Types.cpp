// ------------------------------------ //
#include "Types.h"
// ------------------------------------ //
#include "Exceptions.h"

#include <iostream>
#include <ostream>

// specific colours //
namespace Leviathan {

DLLEXPORT const Float4 Float4::ColourBlack = Float4(0, 0, 0, 1);

DLLEXPORT const Float4 Float4::ColourWhite = Float4(1, 1, 1, 1);

DLLEXPORT const Float4 Float4::ColourTransparent = Float4(0, 0, 0, 0);

DLLEXPORT const Float3 Float3::UnitVForward = Float3(0.f, 0.f, -1.f);

DLLEXPORT const Float3 Float3::UnitVUp = Float3(0.f, 1.f, 0.f);

DLLEXPORT const Float3 Float3::Zeroed = Float3();



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

// ------------------------------------ //
// Radian
DLLEXPORT Radian::Radian(const Degree& degree) : Value(degree.ValueInRadians()) {}
// ------------------------------------ //
DLLEXPORT Radian::operator Degree() const
{
    return Degree(*this);
}

DLLEXPORT Radian& Radian::operator=(const Degree& degrees)
{
    Value = degrees.ValueInRadians();
    return *this;
}
// ------------------------------------ //
// Degree
DLLEXPORT Degree::Degree(const Radian& radians) : Value(radians.ValueInDegrees()) {}
// ------------------------------------ //
DLLEXPORT Degree::operator Radian() const
{
    return Radian(*this);
}

DLLEXPORT Degree& Degree::operator=(const Radian& radians)
{
    Value = radians.ValueInDegrees();
    return *this;
}

} // namespace Leviathan

// ------------------ Stream operators ------------------ //
namespace Leviathan {
DLLEXPORT std::ostream& operator<<(std::ostream& stream, const Float4& value)
{

    stream << "[" << value.X << ", " << value.Y << ", " << value.Z << ", " << value.W << "]";
    return stream;
}

DLLEXPORT std::ostream& operator<<(std::ostream& stream, const Float3& value)
{

    stream << "[" << value.X << ", " << value.Y << ", " << value.Z << "]";
    return stream;
}

DLLEXPORT std::ostream& operator<<(std::ostream& stream, const Leviathan::Float2& value)
{
    stream << "[" << value.X << ", " << value.Y << "]";
    return stream;
}


DLLEXPORT std::ostream& operator<<(std::ostream& stream, const PotentiallySetIndex& value)
{

    if(value.IsSet()) {

        stream << value.Index;

    } else {

        stream << "NOT SET (" << value.Index << ")";
    }

    return stream;
}

DLLEXPORT std::ostream& operator<<(std::ostream& stream, const StartEndIndex& value)
{

    stream << "[" << value.Start << " | " << value.End << "]";
    return stream;
}

} // namespace Leviathan
