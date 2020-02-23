// ------------------------------------ //
#include "Camera.h"

#include "Exceptions.h"

using namespace Leviathan;
using namespace Leviathan::Rendering;
// ------------------------------------ //
DLLEXPORT const Matrix4& Camera::GetProjectionMatrix() const
{
    if(Dirty) {
        // TODO: perhaps this needs to detect when OpenGL is used
        ProjectionMatrix =
            Matrix4::ProjectionPerspective(FOV, AspectRatio, NearClip, FarClip, false);
    }

    return ProjectionMatrix;
}

DLLEXPORT Matrix4 Camera::GetViewMatrix() const
{
    if(!HasParent())
        throw InvalidState("Can't compute view matrix for unattached camera");

    const auto& transform = GetParent()->GetWorldTransform();

    return Matrix4::View(transform.Translation, transform.Orientation);
}

DLLEXPORT Rect Camera::GetViewportRect() const
{
    return {NormalizedViewport.X * ViewportPixelWidth,
        NormalizedViewport.Y * ViewportPixelHeight,
        NormalizedViewport.Width * ViewportPixelWidth,
        NormalizedViewport.Height * ViewportPixelHeight};
}
// ------------------------------------ //
DLLEXPORT void Camera::NotifyRenderTargetResolution(int width, int height)
{
    if(ViewportPixelWidth == width && ViewportPixelHeight == height)
        return;

    ViewportPixelWidth = width;
    ViewportPixelHeight = height;

    if(AutoAspectRatio) {

        AspectRatio = static_cast<float>(width) / height;
        Dirty = true;
    }
}
// ------------------------------------ //
DLLEXPORT void Camera::OnAttachedToParent(SceneNode& parent)
{
    MarkDirty();
}

DLLEXPORT void Camera::OnNotifyParentDirty()
{
    MarkDirty();
}
// ------------------------------------ //
// Camera math methods from bs::framework with modifications, see License.txt, original license
// note:
//************************************ bs::framework - Copyright 2018 Marko Pintera
//**************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not
// to be removed. ***********//
DLLEXPORT Int2 Camera::WorldToScreenPoint(const Float3& worldPoint) const
{
    Float2 ndcPoint = WorldToNdcPoint(worldPoint);
    return NdcToScreenPoint(ndcPoint);
}

DLLEXPORT Float2 Camera::WorldToNdcPoint(const Float3& worldPoint) const
{
    Float3 viewPoint = WorldToViewPoint(worldPoint);
    return ViewToNdcPoint(viewPoint);
}

DLLEXPORT Float3 Camera::WorldToViewPoint(const Float3& worldPoint) const
{
    return GetViewMatrix().MultiplyAffine(worldPoint);
}

DLLEXPORT Float3 Camera::ScreenToWorldPoint(const Int2& screenPoint, float depth) const
{
    Float2 ndcPoint = ScreenToNdcPoint(screenPoint);
    return NdcToWorldPoint(ndcPoint, depth);
}

DLLEXPORT Float3 Camera::ScreenToWorldPointDeviceDepth(
    const Int2& screenPoint, float deviceDepth) const
{
    Float2 ndcPoint = ScreenToNdcPoint(screenPoint);
    Float4 worldPoint(ndcPoint.X, ndcPoint.Y, deviceDepth, 1.0f);
    worldPoint = GetProjectionMatrix().Inverse().Multiply(worldPoint);

    Float3 worldPoint3D;
    if(std::abs(worldPoint.W) > 1e-7f) {
        float invW = 1.0f / worldPoint.W;

        worldPoint3D.X = worldPoint.X * invW;
        worldPoint3D.Y = worldPoint.Y * invW;
        worldPoint3D.Z = worldPoint.Z * invW;
    }

    return ViewToWorldPoint(worldPoint3D);
}

DLLEXPORT Float3 Camera::ScreenToViewPoint(const Int2& screenPoint, float depth) const
{
    Float2 ndcPoint = ScreenToNdcPoint(screenPoint);
    return NdcToViewPoint(ndcPoint, depth);
}

DLLEXPORT Float2 Camera::ScreenToNdcPoint(const Int2& screenPoint) const
{
    Rect viewport = GetViewportRect();

    Float2 ndcPoint;
    ndcPoint.X = (float)(((screenPoint.X - viewport.X) / (float)viewport.Width) * 2.0f - 1.0f);

    // TODO: if normalized device coordinates has a flipped y axis this needs doing something
    // about

    // const Conventions& rapiConventions = ct::gCaps().conventions;
    // if(rapiConventions.ndcYAxis == Conventions::Axis::Down)
    ndcPoint.Y = (float)(((screenPoint.Y - viewport.Y) / viewport.Height) * 2.0f - 1.0f);
    // else
    // ndcPoint.Y =
    //     (float)((1.0f - ((screenPoint.Y - viewport.Y) / viewport.Height)) * 2.0f - 1.0f);

    return ndcPoint;
}

DLLEXPORT Float3 Camera::ViewToWorldPoint(const Float3& viewPoint) const
{
    return GetViewMatrix().InverseAffine().MultiplyAffine(viewPoint);
}

DLLEXPORT Int2 Camera::ViewToScreenPoint(const Float3& viewPoint) const
{
    Float2 ndcPoint = ViewToNdcPoint(viewPoint);
    return NdcToScreenPoint(ndcPoint);
}

DLLEXPORT Float2 Camera::ViewToNdcPoint(const Float3& viewPoint) const
{
    Float3 projPoint = ProjectPoint(viewPoint);

    return Float2(projPoint.X, projPoint.Y);
}

DLLEXPORT Float3 Camera::NdcToWorldPoint(const Float2& ndcPoint, float depth) const
{
    Float3 viewPoint = NdcToViewPoint(ndcPoint, depth);
    return ViewToWorldPoint(viewPoint);
}

DLLEXPORT Float3 Camera::NdcToViewPoint(const Float2& ndcPoint, float depth) const
{
    return UnprojectPoint(Float3(ndcPoint.X, ndcPoint.Y, depth));
}

DLLEXPORT Int2 Camera::NdcToScreenPoint(const Float2& ndcPoint) const
{
    Rect viewport = GetViewportRect();

    Int2 screenPoint;
    screenPoint.X = static_cast<int>(
        std::round((viewport.X + ((ndcPoint.X + 1.0f) * 0.5f) * viewport.Width)));

    // TODO: if normalized device coordinates has a flipped y axis this needs doing something
    // about

    // const Conventions& rapiConventions = ct::gCaps().conventions;
    // if(rapiConventions.ndcYAxis == Conventions::Axis::Down)
    screenPoint.Y = static_cast<int>(
        std::round((viewport.Y + (ndcPoint.Y + 1.0f) * 0.5f * viewport.Height)));
    // else
    //     screenPoint.Y = Math::roundToInt(
    //         viewport.Y + (1.0f - (ndcPoint.Y + 1.0f) * 0.5f) * viewport.height);

    return screenPoint;
}

DLLEXPORT Ray Camera::ScreenPointToRay(const Int2& screenPoint) const
{
    Float2 ndcPoint = ScreenToNdcPoint(screenPoint);

    Float3 near = UnprojectPoint(Float3(ndcPoint.X, ndcPoint.Y, NearClip));
    Float3 far = UnprojectPoint(Float3(ndcPoint.X, ndcPoint.Y, NearClip + 1.0f));

    Ray ray(near, (far - near).Normalize());
    ray.TransformAffine(GetViewMatrix().InverseAffine());

    return ray;
}

DLLEXPORT Float3 Camera::ProjectPoint(const Float3& point) const
{
    Float4 projPoint4(point.X, point.Y, point.Z, 1.0f);
    projPoint4 = GetProjectionMatrix().Multiply(projPoint4);

    if(std::abs(projPoint4.W) > 1e-7f) {
        float invW = 1.0f / projPoint4.W;
        projPoint4.X *= invW;
        projPoint4.Y *= invW;
        projPoint4.Z *= invW;
    } else {
        projPoint4.X = 0.0f;
        projPoint4.Y = 0.0f;
        projPoint4.Z = 0.0f;
    }

    return Float3(projPoint4.X, projPoint4.Y, projPoint4.Z);
}

DLLEXPORT Float3 Camera::UnprojectPoint(const Float3& point) const
{
    // Point.Z is expected to be in view space, so we need to do some extra work to get the
    // proper coordinates (as opposed to if point.Z was in device coordinates, in which case we
    // could just inverse project)

    // Get world position for a point near the far plane (0.95f)
    Float4 farAwayPoint(point.X, point.Y, 0.95f, 1.0f);
    farAwayPoint = GetProjectionMatrix().Inverse().Multiply(farAwayPoint);

    // Can't proceed if w is too small
    if(std::abs(farAwayPoint.W) > 1e-7f) {
        // Perspective divide, to get the values that make sense in 3D space
        float invW = 1.0f / farAwayPoint.W;

        Float3 farAwayPoint3D;
        farAwayPoint3D.X = farAwayPoint.X * invW;
        farAwayPoint3D.Y = farAwayPoint.Y * invW;
        farAwayPoint3D.Z = farAwayPoint.Z * invW;

        // Find the distance to the far point along the camera's viewing axis
        float distAlongZ = farAwayPoint3D.Dot(-Float3::UnitZAxis);

        // Do nothing if point is behind the camera
        if(distAlongZ >= 0.0f) {
            if(ProjectionType == PROJECTION_TYPE::Perspective) {
                // Direction from origin to our point
                Float3 dir = farAwayPoint3D; // Camera is at (0, 0, 0) so it's the same vector

                // Our view space depth (point.Z) is distance along the camera's viewing axis.
                // Since our direction vector is not parallel to the viewing axis, instead of
                // normalizing it with its own length, we "normalize" with the length projected
                // along the camera's viewing axis.
                dir /= distAlongZ;

                // And now we just find the final position along the direction
                return dir * point.Z;
            } else // Ortographic
            {
                // Depth difference between our arbitrary point and actual depth
                float depthDiff = distAlongZ - point.Z;

                // Depth difference along viewing direction
                Float3 depthDiffVec = -Float3::UnitZAxis * depthDiff;

                // Return point that is depthDiff closer than our arbitrary point
                return farAwayPoint3D - depthDiffVec;
            }
        }
    }

    return Float3(0.0f, 0.0f, 0.0f);
}
