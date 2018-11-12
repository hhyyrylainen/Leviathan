// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //

#include "VectorTypes/Float2.h"
#include "VectorTypes/Float3.h"
#include "VectorTypes/Float4.h"
#include "VectorTypes/Int2.h"
#include "VectorTypes/Int3.h"

#include "OgreVector4.h"
#include <optional>

namespace Leviathan {

struct StartEndIndex {
    constexpr StartEndIndex(size_t start, size_t end) noexcept : Start(start), End(end) {}

    constexpr StartEndIndex(size_t start) noexcept  : Start(start) {}

    constexpr StartEndIndex() noexcept = default;

    //! Reset the Start and End to unset
    inline void Reset() noexcept
    {
        Start.reset();
        End.reset();
    }

    //! Calculates the length of the indexes between start and end
    //! \returns The length or if either is unset 0 Or if Start > End
    constexpr size_t Length() const noexcept
    {
        if(!Start.has_value() || !End.has_value() || Start.value() > End.value())
            return 0;

        return 1 + (End.value() - Start.value());
    }

	std::optional<size_t> Start;
    std::optional<size_t> End;
};

// Stream operators //

DLLEXPORT std::ostream& operator<<(
    std::ostream& stream, const Leviathan::StartEndIndex& value);

#ifdef LEVIATHAN_USING_OGRE
//! \brief Newton compatible matrix orthogonal check
//! \exception InvalidArgument if the matrix isn't orthogonal
//! \warning The matrix needs to be transposed with PrepareOgreMatrixForNewton
//! \todo Fix this. This doesn't work because I used Float3 here instead of Float4 as that
//! doesn't have Cross or Dot and this seems to claim that not even an identity matrix is
//! orthogonal
void ThrowIfMatrixIsNotOrthogonal(const Ogre::Matrix4& matrix, float tol = 1.0e-4f);
#endif

}

#ifdef LEAK_INTO_GLOBAL
using Leviathan::Float2;
using Leviathan::Float3;
using Leviathan::Float4;
using Leviathan::Int2;
using Leviathan::Int3;
#endif
