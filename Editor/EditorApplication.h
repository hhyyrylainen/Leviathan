// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "EditorNetHandler.h"
#include "EditorConfiguredHeader.h"

#include "Application/Application.h"
#include "Generated/StandardWorld.h"

namespace Editor {

class EditorApplication : public Leviathan::LeviathanApplication {
public:
    EditorApplication();
    ~EditorApplication();

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

    static EditorApplication* Get();

    // Game configuration checkers //
    static void CheckGameConfigurationVariables(Lock& guard, GameConfiguration* configobj);
    static void CheckGameKeyConfigVariables(Lock& guard, KeyConfiguration* keyconfigobj);


protected:
    Leviathan::NetworkInterface* _GetApplicationPacketHandler() override;
    void _ShutdownApplicationPacketHandler() override;

protected:
    std::unique_ptr<EditorNetHandler> ClientInterface;

    static EditorApplication* StaticGame;
};

} // namespace Editor
// ------------------------------------ //
