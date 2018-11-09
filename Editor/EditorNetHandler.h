// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Networking/NetworkClientInterface.h"
#include "Networking/NetworkInterface.h"

namespace Editor {

//! \brief Editor packet handler (not used currently)
class EditorNetHandler : public Leviathan::NetworkClientInterface {
public:
    EditorNetHandler();
    virtual ~EditorNetHandler();

protected:
    void _OnProperlyConnected() override;
};

} // namespace Editor
