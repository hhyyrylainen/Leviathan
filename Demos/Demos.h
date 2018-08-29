// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "DemoNetHandler.h"
#include "DemoVersion.h"
#include "SampleCommon.h"

#include "Application/Application.h"
#include "Generated/StandardWorld.h"

namespace Demos {

class DemosApplication : public Leviathan::LeviathanApplication {
public:
    DemosApplication();
    ~DemosApplication();

    void PlaySample1();

    // Overrides from LeviathanApplication
    Leviathan::NETWORKED_TYPE GetProgramNetType() const override
    {
        return Leviathan::NETWORKED_TYPE::Client;
    }

    void Tick(int mspassed) override;

    void CustomizeEnginePostLoad() override;
    void EnginePreShutdown() override;
    bool InitLoadCustomScriptTypes(asIScriptEngine* engine) override;

    static std::string GenerateWindowTitle();

    static DemosApplication* Get();

    // Game configuration checkers //
    static void CheckGameConfigurationVariables(Lock& guard, GameConfiguration* configobj);
    static void CheckGameKeyConfigVariables(Lock& guard, KeyConfiguration* keyconfigobj);


protected:
    Leviathan::NetworkInterface* _GetApplicationPacketHandler() override;
    void _ShutdownApplicationPacketHandler() override;

protected:
    std::unique_ptr<DemosNetHandler> ClientInterface;

    std::shared_ptr<Leviathan::StandardWorld> World;

    std::unique_ptr<SampleCommon> CurrentSample;

    static DemosApplication* StaticGame;
};

} // namespace Demos
// ------------------------------------ //
