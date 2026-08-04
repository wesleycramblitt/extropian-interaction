#include <exd/interaction/camera_framing.hpp>

#include <algorithm>
#include <cmath>

namespace exd::interaction
{

static math::Vec3f boundsCenter(const geometry::Bounds& b)
{
    return { (b.min.x + b.max.x) * 0.5f, (b.min.y + b.max.y) * 0.5f, (b.min.z + b.max.z) * 0.5f };
}
static math::Vec3f boundsExtent(const geometry::Bounds& b)
{
    return { (b.max.x - b.min.x), (b.max.y - b.min.y), (b.max.z - b.min.z) };
}

CameraPose frameBounds(
    const geometry::Bounds& bounds,
    float aspectRatio,
    float paddingFactor,
    bool /* use3D */)
{
    CameraPose pose;
    math::Vec3f center = boundsCenter(bounds);
    math::Vec3f extent = boundsExtent(bounds);

    // Pad the extent
    float paddedW = extent.x * paddingFactor;
    float paddedH = extent.y * paddingFactor;

    // Compute distance to fit both dimensions
    float halfFovRad = (pose.fov * 0.5f) * 3.14159265359f / 180.0f;
    float distH = paddedH * 0.5f / std::tan(halfFovRad);
    float distW = (paddedW * 0.5f / std::tan(halfFovRad)) / aspectRatio;
    float distance = std::max(distH, distW);

    pose.target = center;
    pose.position = { center.x, center.y, center.z + distance };
    return pose;
}

CameraPose smoothFrame(const CameraPose& current, const CameraPose& target, float t)
{
    t = std::clamp(t, 0.0f, 1.0f);
    CameraPose result;
    result.position = current.position + (target.position - current.position) * t;
    result.target   = current.target   + (target.target   - current.target)   * t;
    result.up       = current.up;
    result.fov      = current.fov + (target.fov - current.fov) * t;
    result.nearClip = current.nearClip;
    result.farClip  = current.farClip;
    return result;
}

CameraPose zoomToVisual(
    const CameraPose& current,
    const geometry::Bounds& visualBounds,
    float aspectRatio,
    float paddingFactor)
{
    CameraPose target = frameBounds(visualBounds, aspectRatio, paddingFactor, false);
    return target;  // caller applies smoothFrame if desired
}

CameraPose panCamera(
    const CameraPose& current,
    math::Vec3f screenDelta,
    float viewportWidth,
    float viewportHeight)
{
    // Scale screen‑space delta to world space at the current camera distance
    float halfFovRad = (current.fov * 0.5f) * 3.14159265359f / 180.0f;
    float worldHeight = 2.0f * std::tan(halfFovRad) * (current.position.z - current.target.z);
    float worldWidth = worldHeight * (viewportWidth / viewportHeight);

    math::Vec3f worldDelta = {
        -(screenDelta.x / viewportWidth) * worldWidth,
        (screenDelta.y / viewportHeight) * worldHeight,
        0.0f
    };

    CameraPose result = current;
    result.position = result.position + worldDelta;
    result.target = result.target + worldDelta;
    return result;
}

CameraPose orbitCamera(
    const CameraPose& current,
    float deltaAzimuth,
    float deltaElevation)
{
    CameraPose result = current;

    // Vector from target to camera position
    math::Vec3f dir = current.position - current.target;
    float dirLen = dir.length();
    if (dirLen < 1e-8f) {
        return result;  // degenerate: position == target, nothing to orbit
    }

    // ── Azimuth: rotate dir around world up (Y) in the XZ plane ──
    float cosA = std::cos(deltaAzimuth);
    float sinA = std::sin(deltaAzimuth);
    float newX = dir.x * cosA - dir.z * sinA;
    float newZ = dir.x * sinA + dir.z * cosA;
    dir.x = newX;
    dir.z = newZ;
    // Y unchanged by azimuth

    // ── Elevation: tilt dir toward/away from the up vector ──
    // Compute the current elevation angle from the XZ plane, then add delta.
    float hLen = std::sqrt(dir.x * dir.x + dir.z * dir.z);
    float currentElev = std::atan2(dir.y, hLen);
    float newElev = currentElev + deltaElevation;

    // Clamp elevation to avoid flipping past straight up/down.
    // Clamp sin(newElev) so |dir.y| <= 0.99 * dirLen.
    float sinElev = std::sin(newElev);
    sinElev = std::clamp(sinElev, -0.99f, 0.99f);
    float cosElev = std::sqrt(1.0f - sinElev * sinElev);

    // Reconstruct dir with the new elevation angle, preserving XZ direction.
    dir.y = dirLen * sinElev;
    if (hLen > 1e-8f) {
        float scale = (dirLen * cosElev) / hLen;
        dir.x *= scale;
        dir.z *= scale;
    } else {
        // dir was nearly vertical; pick an arbitrary XZ direction (preserve previous XZ if possible)
        dir.x = dirLen * cosElev;
        dir.z = 0.0f;
    }

    result.position = result.target + dir;
    return result;
}

}  // namespace exd::interaction
