#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "Application.h"


namespace Leviathan{

    class MasterServerApplication : public LeviathanApplication{
    public:
        DLLEXPORT MasterServerApplication();
        DLLEXPORT ~MasterServerApplication();

        //! Makes sure doesn't start in GUI mode
        DLLEXPORT bool PassCommandLine(int argcount, char* args[]) override;

        NETWORKED_TYPE GetProgramNetType() const override {

            return NETWORKED_TYPE::Master;
        }

    private:

    };

}
