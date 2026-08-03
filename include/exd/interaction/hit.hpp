#pragma once

#include <exd/interaction/event.hpp>

#include <vector>
#include <optional>

namespace exd::interaction
{

/// Resolves a picker hit (world-space position which entity was hit)
/// to a logical VisualId by walking the render::Parent hierarchy chain.
///
/// The hit entity may be a leaf mesh; this walks upward until it finds
/// an entity tagged with a canvas-level VisualId, or returns nullopt.

struct HitResult
{
    VisualId visualId = kInvalidVisualId;
    math::Vec3f hitPoint = {};
    float depth = 0.0f;
};

/// Resolve a world-space position to a visual hit.
/// Pure function — call with picker results from render.
HitResult resolve_hit(VisualId hitEntity, math::Vec3f position, float depth);

}  // namespace exd::interaction
