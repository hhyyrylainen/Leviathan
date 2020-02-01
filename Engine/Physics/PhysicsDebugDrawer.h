// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#include "Define.h"
#pragma once
// ------------------------------------ //
#include "LinearMath/btIDebugDraw.h"


namespace Leviathan {
class Window;

//! \todo Reimplement
class PhysicsDebugDrawer : public btIDebugDraw {
public:
    //! \note drawtarget Is currently ignored as bsf debug drawing is drawn on all windows
    PhysicsDebugDrawer(Window& drawtarget, GameWorld& cameraworld);

    void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;

    void drawSphere(const btVector3& p, btScalar radius, const btVector3& color) override;

    void drawContactPoint(const btVector3& pointOnB, const btVector3& normalOnB,
        btScalar distance, int lifeTime, const btVector3& color) override;

    void reportErrorWarning(const char* warningString) override;

    void draw3dText(const btVector3& location, const char* textString) override {}

    void setDebugMode(int mode) override
    {
        DebugMode = mode;
    }

    int getDebugMode() const override
    {
        return DebugMode;
    }

    DLLEXPORT void OnBeginDraw();

protected:
    int DebugMode = btIDebugDraw::DBG_DrawWireframe;

private:
    Window& Target;
    GameWorld& World;
};
} // namespace Leviathan
