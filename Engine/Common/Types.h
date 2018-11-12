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

namespace Leviathan {


struct PotentiallySetIndex {

    inline PotentiallySetIndex(size_t index) : ValueSet(true), Index(index) {}
    inline PotentiallySetIndex() = default;

    inline operator bool() const
    {

        return ValueSet;
    }

    inline operator size_t() const
    {
#ifdef _DEBUG
        LEVIATHAN_ASSERT(
            ValueSet, "PotentiallySetIndex size_t() called when ValueSet is false");
#endif // _DEBUG

        return Index;
    }

    bool operator==(const PotentiallySetIndex& other) const
    {

        if(!ValueSet)
            return ValueSet == other.ValueSet;

        return Index == other.Index;
    }

    PotentiallySetIndex& operator=(const PotentiallySetIndex& other)
    {

        ValueSet = other.ValueSet;
        Index = other.Index;
        return *this;
    }

    inline PotentiallySetIndex& operator=(const size_t& value)
    {

        ValueSet = true;
        Index = value;
        return *this;
    }

    inline bool IsSet() const
    {

        return ValueSet;
    }

    bool ValueSet = false;
    size_t Index = 0;
};

struct StartEndIndex {

    using Index = PotentiallySetIndex;

    inline StartEndIndex(size_t start, size_t end) : Start(start), End(end) {}

    inline StartEndIndex(size_t start) : Start(start) {}

    inline StartEndIndex() = default;

    //! Reset the Start and End to unset
    inline void Reset()
    {
        Start = Index();
        End = Index();
    }

    //! Calculates the length of the indexes between start and end
    //! \returns The length or if either is unset 0 Or if Start > End
    inline size_t Length() const
    {
        if(!Start || !End || static_cast<size_t>(Start) > static_cast<size_t>(End))
            return 0;

        return 1 + (static_cast<size_t>(End) - static_cast<size_t>(Start));
    }


    Index Start;
    Index End;
};

// Stream operators //

DLLEXPORT std::ostream& operator<<(
    std::ostream& stream, const Leviathan::StartEndIndex& value);

DLLEXPORT std::ostream& operator<<(
    std::ostream& stream, const Leviathan::PotentiallySetIndex& value);

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
using Leviathan::Int1; // Where even is this??
using Leviathan::Int2;
using Leviathan::Int3;
#endif
