#pragma once

#include <exd/geometry/types.hpp>  // geometry::Bounds
#include <exd/math/vec3.hpp>

namespace exd::interaction
{

// ── Camera pose ──

/// Oriented camera position + view parameters.
struct CameraPose
{
    math::Vec3f position = {0.0f, 0.0f, 10.0f};
    math::Vec3f target = {0.0f, 0.0f, 0.0f};
    math::Vec3f up = {0.0f, 1.0f, 0.0f};
    float fov = 45.0f;          // vertical field of view (degrees)
    float nearClip = 0.1f;
    float farClip = 1000.0f;
};

// ── Camera framing ──

/// Compute a camera pose that frames the given bounds with padding.
/// For 2D ortho‑like views, position is centered above the XY plane.
/// For 3D perspective views, set use3D = true.
CameraPose frameBounds(
    const geometry::Bounds& bounds,
    float aspectRatio = 16.0f / 9.0f,
    float paddingFactor = 1.15f,
    bool use3D = false
);

/// Compute a smoothed camera pose by interpolating between current and target.
CameraPose smoothFrame(
    const CameraPose& current,
    const CameraPose& target,
    float t = 0.1f   // interpolation factor (0 = stay current, 1 = snap to target)
);

/// Zoom to a specific visual given its bounds and the current camera.
/// Returns the target pose (caller applies smooth transition).
CameraPose zoomToVisual(
    const CameraPose& current,
    const geometry::Bounds& visualBounds,
    float aspectRatio = 16.0f / 9.0f,
    float paddingFactor = 1.1f
);

/// Pan the camera by a screen‑space delta.
CameraPose panCamera(
    const CameraPose& current,
    math::Vec3f screenDelta,
    float viewportWidth,
    float viewportHeight
);

/// Orbit the camera around the target point.
CameraPose orbitCamera(
    const CameraPose& current,
    float deltaAzimuth,
    float deltaElevation
);

}  // namespace exd::interaction
