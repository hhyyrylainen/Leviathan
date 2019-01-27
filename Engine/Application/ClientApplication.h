// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Application.h"

namespace Leviathan {

class ClientApplication : public LeviathanApplication {
public:
    DLLEXPORT ClientApplication();

    //! \brief Version for tests with incomplete engine instance
    DLLEXPORT ClientApplication(Engine* engine);

    DLLEXPORT ~ClientApplication();

    NETWORKED_TYPE GetProgramNetType() const override
    {
        return NETWORKED_TYPE::Client;
    }


protected:
};

} // namespace Leviathan
