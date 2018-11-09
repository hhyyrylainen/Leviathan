// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Networking/NetworkClientInterface.h"
#include "Networking/NetworkInterface.h"

namespace Demos {

//! \brief Demos packet handler (not used)
class DemosNetHandler : public Leviathan::NetworkClientInterface {
public:
    DemosNetHandler();
    virtual ~DemosNetHandler();

protected:
    void _OnProperlyConnected() override;
};

} // namespace Demos
