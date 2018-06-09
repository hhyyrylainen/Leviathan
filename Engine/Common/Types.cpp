// ------------------------------------ //
#include "Types.h"
// ------------------------------------ //
#include "Exceptions.h"

#ifdef LEVIATHAN_USING_OGRE
#include "OgreMatrix4.h"
#endif

#include <iostream>
#include <ostream>

// specific colours //
namespace Leviathan {

const Float4 Float4::ColourBlack = Float4(0, 0, 0, 1);

const Float4 Float4::ColourWhite = Float4(1, 1, 1, 1);

const Float4 Float4::ColourTransparent = Float4(0, 0, 0, 0);

const Float3 Float3::UnitVForward = Float3(0.f, 0.f, -1.f);

const Float3 Float3::UnitVUp = Float3(0.f, 1.f, 0.f);

const Float3 Float3::Zeroed = Float3::zero();



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

#ifdef LEVIATHAN_USING_OGRE
void ThrowIfMatrixIsNotOrthogonal(const Ogre::Matrix4& matrix, float tol /*= 1.0e-4f*/)
{
    DEBUG_BREAK;

    // // This is done like exactly in bool dgMatrix::TestOrthogonal
    // // Except that the last elements are always 0, 0, 0, 1 so we use
    // // Float3's to use the math
    // Float3 m_front(*matrix[0], *matrix[1], *matrix[2] /*, *matrix[3]*/);
    // Float3 m_up(*matrix[4], *matrix[5], *matrix[6] /*, *matrix[7]*/);
    // Float3 m_right(*matrix[8], *matrix[9], *matrix[10] /*, *matrix[11]*/);
    // Float3 m_posit(*matrix[12], *matrix[13], *matrix[14] /*, *matrix[15]*/);

    // // This is done in NewtonBodySetMatrix
    // Float3 n(m_front.Cross(m_up));
    // float a = m_right.Dot(m_right);
    // float b = m_up.Dot(m_up);
    // float c = m_front.Dot(m_front);
    // float d = n.Dot(m_right);

    // // See above why this check should always succeed
    // // (m_front[3] == float(0.0f)) & (m_up[3] == float(0.0f)) &
    // //     (m_right[3] == float(0.0f)) & (m_posit[3] == float(1.0f)) &

    // if(std::abs(a - float(1.0f)) > tol)
    //     throw InvalidArgument(
    //         "Not orthogonal m_right.Dot(m_right) wasn't 1, it was: " + std::to_string(a));
    // if(std::abs(b - float(1.0f)) > tol)
    //     throw InvalidArgument(
    //         "Not orthogonal m_up.Dot(m_up) wasn't 1, it was: " + std::to_string(b));
    // if(std::abs(c - float(1.0f)) > tol)
    //     throw InvalidArgument(
    //         "Not orthogonal m_front.Dot(m_front) wasn't 1, it was: " + std::to_string(c));
    // if(std::abs(d - float(1.0f)) > tol)
    //     throw InvalidArgument(
    //         "Not orthogonal n.Dot(m_right) wasn't 1, it was: " + std::to_string(d));
}
#endif

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
