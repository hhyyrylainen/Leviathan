#include "Int3.h"

using namespace Leviathan;

DLLEXPORT std::ostream& Leviathan::operator<<(std::ostream& stream, const Int3& value)
{
    stream << "[" << value.X << ", " << value.Y << ", " << value.Z << "]";
    return stream;
}
