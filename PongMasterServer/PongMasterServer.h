#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "Application/MasterServerApplication.h"
#include "PongMasterNetworking.h"

namespace Pong {

class PongMasterServer : public Leviathan::MasterServerApplication {
public:
    PongMasterServer();
    ~PongMasterServer();

    void Tick(int mspassed) override;

    void CustomizeEnginePostLoad() override;
    void EnginePreShutdown() override;

    static std::string GenerateWindowTitle();

    // customized callbacks //
    bool InitLoadCustomScriptTypes(asIScriptEngine* engine) override;

    // Game configuration checkers //
    static void CheckGameConfigurationVariables(Lock& guard, GameConfiguration* configobj);
    static void CheckGameKeyConfigVariables(Lock& guard, KeyConfiguration* keyconfigobj);

protected:
    Leviathan::NetworkInterface* _GetApplicationPacketHandler() override;
    void _ShutdownApplicationPacketHandler() override;

protected:
    std::unique_ptr<PongMasterNetworking> MasterInterface;
};

} // namespace Pong
