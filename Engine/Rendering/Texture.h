// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ReferenceCounted.h"

#include "bsfCore/BsCorePrerequisites.h"

namespace Leviathan {

class Texture : public ReferenceCounted {
public:
    REFERENCE_COUNTED_PTR_TYPE(Texture);

private:
    bs::HTexture BsTexture;
};

} // namespace Leviathan
