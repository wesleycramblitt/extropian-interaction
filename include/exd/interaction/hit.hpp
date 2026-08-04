#pragma once

#include <exd/interaction/event.hpp>

#include <functional>
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

/// Callback: given a VisualId (ECS entity), returns its parent VisualId,
/// or kInvalidVisualId if at root.
using ParentLookupFn = std::function<VisualId(VisualId entity)>;

/// Resolve a world-space position to a visual hit.
/// Pass-through version for simple cases where no hierarchy walk is needed.
/// For hierarchy-aware resolution, use the overload taking ParentLookupFn.
HitResult resolve_hit(VisualId hitEntity, math::Vec3f position, float depth);

/// Resolve a hit entity up the parent chain using the provided lookup function.
/// Walks upward from hitEntity, calling lookup until it finds a non-invalid parent
/// or reaches the root. Returns the highest non-invalid VisualId found.
/// The original hitEntity is checked first, then each parent in order.
HitResult resolve_hit(VisualId hitEntity, math::Vec3f position, float depth, const ParentLookupFn& lookup);

}  // namespace exd::interaction
