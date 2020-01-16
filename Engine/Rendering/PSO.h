// Leviathan Game Engine
// Copyright (c) 2012-2020 Henri Hyyryläinen
#pragma once
#include "Define.h"
//! \file Contains stuff needed to deal with PSOs
// ------------------------------------ //

// part of the hack
#undef LOG_ERROR

#include "DiligentCore/Common/interface/RefCntAutoPtr.h"
#include "DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "DiligentCore/Graphics/GraphicsEngine/interface/PipelineState.h"

// hack workaround
#undef LOG_ERROR
#define LOG_ERROR(x) Logger::Get()->Error(x);


namespace Leviathan {

//! \brief Pipeline State Object wrapper. All rendering must use a PSO.
//!
//! These objects should be created asynchronously as creating these takes time that shouldn't
//! be allowed to cause rendering to slow down
class PSO {
public:
    PSO(const Diligent::RefCntAutoPtr<Diligent::IPipelineState>& pso);


    auto& GetInternal()
    {
        return _PSO;
    }

private:
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> _PSO;
};

} // namespace Leviathan
