#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "Application.h"

namespace Leviathan{

class ClientApplication : public LeviathanApplication{
public:
    DLLEXPORT ClientApplication();
    DLLEXPORT ~ClientApplication();

    NETWORKED_TYPE GetProgramNetType() const override {
            
        return NETWORKED_TYPE::Client;
    }


protected:

};

}
