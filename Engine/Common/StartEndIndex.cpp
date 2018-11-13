#include "StartEndIndex.h"

using namespace Leviathan;

// This looks dumb but i can't figure out a better way of doing it...
DLLEXPORT std::ostream& operator<<(std::ostream& stream, const StartEndIndex& value)
{
    stream << "[";
    if(value.Start.has_value())
        stream << value.Start.value();
    else
        stream << "NOT SET";
    stream << " | ";
    if(value.End.has_value())
        stream << value.End.value();
    else
        stream << "NOT SET";
    stream << "]";
    return stream;
}
