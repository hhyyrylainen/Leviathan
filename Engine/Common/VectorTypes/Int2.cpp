#include "Int2.h"

using namespace Leviathan;

DLLEXPORT std::ostream& Leviathan::operator<<(
    std::ostream& stream, const Leviathan::Int2& value)
{
    stream << "[" << value.X << ", " << value.Y << "]";
    return stream;
}

// ------------------------------------ //
// Why these two?
DLLEXPORT void Int2::SetData(const int& data)
{
    X = data;
    Y = data;
}

DLLEXPORT void Int2::SetData(const int& data1, const int& data2)
{
    X = data1;
    Y = data2;
}
