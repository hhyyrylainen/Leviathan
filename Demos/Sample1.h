// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "SampleCommon.h"

//! \file This is the first sample that shows how first person character works and how pbs
//! materials works

namespace Demos {
//! Sample1 based on the Ogre sample PbsMaterials
class Sample1 : public SampleCommon {
public:
    void Start(Leviathan::StandardWorld& world);
};

} // namespace Demos
