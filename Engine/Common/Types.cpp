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

DLLEXPORT const Float3 Float3::UnitXAxis = Float3(1.f, 0.f, 0.f);

DLLEXPORT const Float3 Float3::UnitYAxis = Float3(0.f, 1.f, 0.f);

DLLEXPORT const Float3 Float3::UnitZAxis = Float3(0.f, 0.f, 1.f);


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
// HSB operations. Code for these is taken from bs::framework with modifications, see
// License.txt for details
DLLEXPORT void Float4::ConvertToHSB(float& hue, float& saturation, float& brightness) const
{
    const float vMin = std::min(X, std::min(Y, Z));
    const float vMax = std::max(X, std::max(Y, Z));
    const float delta = vMax - vMin;

    brightness = vMax;

    if(std::abs(delta - 0.f) < EPSILON) {
        // grey
        hue = 0;
        saturation = 0;
    } else {
        // a colour
        saturation = delta / vMax;

        float deltaR = (((vMax - X) / 6.0f) + (delta / 2.0f)) / delta;
        float deltaG = (((vMax - Y) / 6.0f) + (delta / 2.0f)) / delta;
        float deltaB = (((vMax - Z) / 6.0f) + (delta / 2.0f)) / delta;

        if(std::abs(X - vMax) < EPSILON)
            hue = deltaB - deltaG;
        else if(std::abs(Y - vMax) < EPSILON)
            hue = 0.3333333f + deltaR - deltaB;
        else if(std::abs(Z - vMax) < EPSILON)
            hue = 0.6666667f + deltaG - deltaR;

        if(hue < 0.0f)
            hue += 1.0f;
        if(hue > 1.0f)
            hue -= 1.0f;
    }
}

DLLEXPORT Float4 Float4::FromHSB(float hue, float saturation, float brightness)
{
    Float4 output;
    output.W = 1.f;

    // wrap hue
    if(hue > 1.0f)
        hue -= (int)hue;
    else if(hue < 0.0f)
        hue += (int)hue + 1;

    // clamp saturation / brightness
    saturation = std::min(saturation, (float)1.0);
    saturation = std::max(saturation, (float)0.0);
    brightness = std::min(brightness, (float)1.0);
    brightness = std::max(brightness, (float)0.0);

    if(brightness == 0.0f) {
        // early exit, this has to be black
        output.X = output.Y = output.Z = 0.0f;
        return output;
    }

    if(saturation == 0.0f) {
        // early exit, this has to be grey

        output.X = output.Y = output.Z = brightness;
        return output;
    }

    float hueDomain = hue * 6.0f;
    if(hueDomain >= 6.0f) {
        // wrap around, and allow mathematical errors
        hueDomain = 0.0f;
    }

    const auto domain = (unsigned short)hueDomain;
    const float f1 = brightness * (1 - saturation);
    const float f2 = brightness * (1 - saturation * (hueDomain - domain));
    const float f3 = brightness * (1 - saturation * (1 - (hueDomain - domain)));

    switch(domain) {
    case 0:
        // red domain; green ascends
        output.X = brightness;
        output.Y = f3;
        output.Z = f1;
        break;
    case 1:
        // yellow domain; red descends
        output.X = f2;
        output.Y = brightness;
        output.Z = f1;
        break;
    case 2:
        // green domain; blue ascends
        output.X = f1;
        output.Y = brightness;
        output.Z = f3;
        break;
    case 3:
        // cyan domain; green descends
        output.X = f1;
        output.Y = f2;
        output.Z = brightness;
        break;
    case 4:
        // blue domain; red ascends
        output.X = f3;
        output.Y = f1;
        output.Z = brightness;
        break;
    case 5:
        // magenta domain; blue descends
        output.X = brightness;
        output.Y = f1;
        output.Z = f2;
        break;
    }

    return output;
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
