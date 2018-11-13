#include "Float3.h"

using namespace Leviathan;

const Float3 Float3::UnitVForward = Float3(0.f, 0.f, -1.f);
const Float3 Float3::UnitVUp = Float3(0.f, 1.f, 0.f);
const Float3 Float3::Zeroed = Float3::zero();

DLLEXPORT std::ostream& Leviathan::operator<<(std::ostream& stream, const Float3& value)
{
    stream << "[" << value.X << ", " << value.Y << ", " << value.Z << "]";
    return stream;
}
