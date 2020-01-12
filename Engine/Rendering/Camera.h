// Leviathan Game Engine
// Copyright (c) 2012-2020 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "SceneNode.h"

#include "Common/Matrix.h"
#include "Common/Ray.h"
#include "Common/Types.h"

namespace Leviathan {

//! \brief 2D rectangle
struct Rect {

    float X, Y, Width, Height;
};

} // namespace Leviathan

namespace Leviathan { namespace Rendering {

enum class PROJECTION_TYPE { Perspective, Orthogonal };

//! \brief Rendering camera for rendering a Scene from
class Camera : public SceneAttachable {
protected:
    // These are protected for only constructing properly reference
    // counted instances through MakeShared
    friend ReferenceCounted;
    Camera() = default;

public:
    DLLEXPORT const Matrix4& GetProjectionMatrix() const;

    //! \note Not cached
    DLLEXPORT Matrix4 GetViewMatrix() const;

    DLLEXPORT Rect GetViewportRect() const;

    DLLEXPORT void NotifyRenderTargetResolution(int width, int height);

    Degree GetFOV() const
    {
        return FOV;
    }

    void SetFOV(Degree newfov)
    {
        if(newfov == FOV)
            return;

        FOV = newfov;
        MarkDirty();
    }

    void MarkDirty()
    {
        Dirty = true;
    }

    // ------------------------------------ //
    // Camera math methods from bs::framework with modifications, see License.txt
    /**
     * Converts a point in world space to screen coordinates.
     *
     * @param[in]	worldPoint		3D point in world space.
     * @return						2D point on the render target attached to the camera's
     * viewport, in pixels.
     */
    DLLEXPORT Int2 WorldToScreenPoint(const Float3& worldPoint) const;

    /**
     * Converts a point in world space to normalized device coordinates.
     *
     * @param[in]	worldPoint		3D point in world space.
     * @return						2D point in normalized device coordinates ([-1, 1] range),
     * relative to the camera's viewport.
     */
    DLLEXPORT Float2 WorldToNdcPoint(const Float3& worldPoint) const;

    /**
     * Converts a point in world space to view space coordinates.
     *
     * @param[in]	worldPoint		3D point in world space.
     * @return						3D point relative to the camera's coordinate system.
     */
    DLLEXPORT Float3 WorldToViewPoint(const Float3& worldPoint) const;

    /**
     * Converts a point in screen space to a point in world space.
     *
     * @param[in]	screenPoint	2D point on the render target attached to the camera's
     *viewport, in pixels.
     * @param[in]	depth		Depth to place the world point at, in world coordinates. The
     *depth is applied to the vector going from camera origin to the point on the near plane.
     * @return					3D point in world space.
     */
    DLLEXPORT Float3 ScreenToWorldPoint(const Int2& screenPoint, float depth = 0.5f) const;

    /**
     * Converts a point in screen space (pixels corresponding to render target attached to the
     * camera) to a point in world space.
     *
     * @param[in]	screenPoint	Point to transform.
     * @param[in]	deviceDepth	Depth to place the world point at, in normalized device
     * coordinates.
     * @return					3D point in world space.
     */
    DLLEXPORT Float3 ScreenToWorldPointDeviceDepth(
        const Int2& screenPoint, float deviceDepth = 0.5f) const;

    /**
     * Converts a point in screen space to a point in view space.
     *
     * @param[in]	screenPoint	2D point on the render target attached to the camera's
     *viewport, in pixels.
     * @param[in]	depth		Depth to place the world point at, in device depth. The depth
     *is applied to the vector going from camera origin to the point on the near plane.
     * @return					3D point relative to the camera's coordinate system.
     */
    DLLEXPORT Float3 ScreenToViewPoint(const Int2& screenPoint, float depth = 0.5f) const;

    /**
     * Converts a point in screen space to normalized device coordinates.
     *
     * @param[in]	screenPoint		2D point on the render target attached to the camera's
     *viewport, in pixels.
     * @return						2D point in normalized device coordinates ([-1, 1] range),
     *relative to the camera's viewport.
     */
    DLLEXPORT Float2 ScreenToNdcPoint(const Int2& screenPoint) const;

    /**
     * Converts a point in view space to world space.
     *
     * @param[in]	viewPoint		3D point relative to the camera's coordinate system.
     * @return						3D point in world space.
     */
    DLLEXPORT Float3 ViewToWorldPoint(const Float3& viewPoint) const;

    /**
     * Converts a point in view space to screen space.
     *
     * @param[in]	viewPoint		3D point relative to the camera's coordinate system.
     * @return						2D point on the render target attached to the camera's
     * viewport, in pixels.
     */
    DLLEXPORT Int2 ViewToScreenPoint(const Float3& viewPoint) const;

    /**
     * Converts a point in view space to normalized device coordinates.
     *
     * @param[in]	viewPoint		3D point relative to the camera's coordinate system.
     * @return						2D point in normalized device coordinates ([-1, 1] range),
     *relative to the camera's viewport.
     */
    DLLEXPORT Float2 ViewToNdcPoint(const Float3& viewPoint) const;

    /**
     * Converts a point in normalized device coordinates to world space.
     *
     * @param[in]	ndcPoint	2D point in normalized device coordinates ([-1, 1] range),
     *relative to the camera's viewport.
     * @param[in]	depth		Depth to place the world point at. The depth is applied to the
     *							vector going from camera origin to the point on the near plane.
     * @return					3D point in world space.
     */
    DLLEXPORT Float3 NdcToWorldPoint(const Float2& ndcPoint, float depth = 0.5f) const;

    /**
     * Converts a point in normalized device coordinates to view space.
     *
     * @param[in]	ndcPoint	2D point in normalized device coordinates ([-1, 1] range),
     *relative to the camera's viewport.
     * @param[in]	depth		Depth to place the world point at. The depth is applied to the
     *							vector going from camera origin to the point on the near plane.
     * @return					3D point relative to the camera's coordinate system.
     */
    DLLEXPORT Float3 NdcToViewPoint(const Float2& ndcPoint, float depth = 0.5f) const;

    /**
     * Converts a point in normalized device coordinates to screen space.
     *
     * @param[in]	ndcPoint	2D point in normalized device coordinates ([-1, 1] range),
     *relative to the camera's viewport.
     * @return					2D point on the render target attached to the camera's
     *viewport, in pixels.
     */
    DLLEXPORT Int2 NdcToScreenPoint(const Float2& ndcPoint) const;

    /**
     * Converts a point in screen space to a ray in world space.
     *
     * @param[in]	screenPoint		2D point on the render target attached to the camera's
     * viewport, in pixels.
     * @return						Ray in world space, originating at the selected point on
     * the camera near plane.
     */
    DLLEXPORT Ray ScreenPointToRay(const Int2& screenPoint) const;

    /**
     * Projects a point in view space to normalized device coordinates. Similar to
     *viewToNdcPoint() but preserves the depth component.
     *
     * @param[in]	point			3D point relative to the camera's coordinate system.
     * @return						3D point in normalized device coordinates ([-1, 1] range),
     *relative to the camera's viewport. Z value range depends on active render API.
     */
    DLLEXPORT Float3 ProjectPoint(const Float3& point) const;

    /**	Un-projects a point in normalized device space to view space.
     *
     * @param[in]	point			3D point in normalized device coordinates ([-1, 1] range),
     *relative to the camera's viewport. Z value range depends on active render API.
     * @return						3D point relative to the camera's coordinate system.
     */
    DLLEXPORT Float3 UnprojectPoint(const Float3& point) const;

    // End of code from bsf
    // ------------------------------------ //

    REFERENCE_COUNTED_PTR_TYPE(Camera);

protected:
    DLLEXPORT void OnAttachedToParent(SceneNode& parent) override;
    DLLEXPORT void OnNotifyParentDirty() override;

private:
    // Camera view settings
    float NearClip = 0.05f;
    float FarClip = 5000.f;
    Degree FOV = 90.f;
    float AspectRatio = 1.77778f;

    bool AutoAspectRatio = true;

    PROJECTION_TYPE ProjectionType = PROJECTION_TYPE::Perspective;

    //! If true the projection matrix needs to be rebuilt
    mutable bool Dirty = true;
    mutable Matrix4 ProjectionMatrix;

    // TODO: make an actual view port
    Rect NormalizedViewport{0.f, 0.f, 1.f, 1.f};

    int ViewportPixelWidth = 1;
    int ViewportPixelHeight = 1;
};

}} // namespace Leviathan::Rendering
