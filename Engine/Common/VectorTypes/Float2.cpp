#include "Float2.h"

using namespace Leviathan;

DLLEXPORT std::ostream& Leviathan::operator<<(
    std::ostream& stream, const Leviathan::Float2& value)
{
    stream << "[" << value.X << ", " << value.Y << "]";
    return stream;
}
