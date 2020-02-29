// Leviathan Game Engine
// Copyright (c) 2012-2020 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/Quaternion.h"
#include "Common/Types.h"
#include "Entities/EntityCommon.h"

#include <memory>

namespace Leviathan {

class Window;
class Engine;
class StandardWorld;
class Camera;
class Position;

namespace Editor {

//! \brief Main class that handles all editor functionality
class Editor {
    static constexpr auto DEFAULT_ROTATE_SPEED = 45.f;

public:
    Editor(Window* targetwindow, Engine* engine);
    ~Editor();

    //! \brief Instructs ShownOnWindow to become foreground
    void BringToFront();

    //! \brief Called by Engine to update the editor
    void Tick(float elapsed);

    //! \brief Loads a model to be shown in the editor
    DLLEXPORT void LoadModel(const std::string& file);

    //! \brief Unloads the loaded model if there is one
    DLLEXPORT void UnloadModel();

    //! \brief Moves loaded model
    //! \see LoadModel
    DLLEXPORT void PositionModel(const Float3& pos);

    //! \brief Rotates loaded model
    DLLEXPORT void RotateModel(const Quaternion& rotation);

    //! \brief Scales loaded model
    DLLEXPORT void ScaleModel(const Float3& scales);

    //! \brief When called with true the loaded model automatically rotates
    DLLEXPORT void AutoRotateModel(bool autorotate);

    //! \brief Same operations as for the loaded model but for the camera looking at it
    DLLEXPORT void PositionCamera(const Float3& pos);
    DLLEXPORT void RotateCamera(const Quaternion& rotation);

protected:
    void _SetupOnWindow(Window* targetwindow);

    //! \brief Closes the editor from the current window. This has to be called before
    //! destroying this or the Window
    void _CloseEditor();

protected:
    //! \todo This needs some mechanism to detect when the window is closed and the editor
    //! should also close then
    Window* ShownOnWindow = nullptr;

    std::shared_ptr<StandardWorld> World;

    ObjectID LoadedModel = NULL_OBJECT;

    ObjectID CameraID;
    Position* CameraPos;
    Camera* CameraProps;

    float CurrentRotationPassed = 0.f;
    bool RotateModelSet = false;
    float RotateSpeed = DEFAULT_ROTATE_SPEED;

    Engine* _Engine;
};

} // namespace Editor
} // namespace Leviathan
