#pragma once

#include "Define.h"

#include <iostream>
#include <optional>

namespace Leviathan {

struct StartEndIndex {
    constexpr StartEndIndex() noexcept = default;

    constexpr StartEndIndex(size_t start, size_t end) noexcept;
    constexpr StartEndIndex(size_t start) noexcept;

    //! Reset the Start and End to unset
    inline void Reset() noexcept;

    //! Calculates the length of the indexes between start and end
    //! \returns The length or if either is unset 0 Or if Start > End
    constexpr size_t Length() const noexcept;

    std::optional<size_t> Start;
    std::optional<size_t> End;
};

// Stream operator //
DLLEXPORT std::ostream& operator<<(
    std::ostream& stream, const Leviathan::StartEndIndex& value);

//// ------------------  IMPLEMENTATION ------------------  ////

constexpr StartEndIndex::StartEndIndex(size_t start, size_t end) noexcept :
    Start(start), End(end)
{}

constexpr StartEndIndex::StartEndIndex(size_t start) noexcept : Start(start) {}

inline void StartEndIndex::Reset() noexcept
{
    Start.reset();
    End.reset();
}

constexpr size_t StartEndIndex::Length() const noexcept
{
    if(!Start.has_value() || !End.has_value() || Start.value() > End.value())
        return 0;

    return 1 + (End.value() - Start.value());
}

} // namespace Leviathan
