// Leviathan Game Engine
// Copyright (c) 2012-2020 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ReferenceCounted.h"

#include "bsfCore/BsCorePrerequisites.h"

namespace Leviathan {

class Shader : public ReferenceCounted {
protected:
    // These are protected for only constructing properly reference
    // counted instances through MakeShared
    friend ReferenceCounted;
    DLLEXPORT Shader(const bs::HShader& shader);

public:
    bs::HShader GetInternal()
    {
        return BsShader;
    }

    REFERENCE_COUNTED_PTR_TYPE(Shader);

private:
    bs::HShader BsShader;
};

} // namespace Leviathan
