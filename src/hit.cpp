#include <exd/interaction/hit.hpp>

namespace exd::interaction
{

HitResult resolve_hit(VisualId hitEntity, math::Vec3f position, float depth)
{
    // Pass-through for simple cases where no hierarchy walk is needed.
    // Use the ParentLookupFn overload for hierarchy-aware resolution.
    return HitResult{ hitEntity, position, depth };
}

HitResult resolve_hit(VisualId hitEntity, math::Vec3f position, float depth, const ParentLookupFn& lookup)
{
    VisualId best = kInvalidVisualId;
    VisualId current = hitEntity;

    while (current != kInvalidVisualId)
    {
        best = current;

        VisualId parent = lookup(current);

        // Self-loop guard or reached root
        if (parent == current || parent == kInvalidVisualId)
        {
            break;
        }

        current = parent;
    }

    return HitResult{ best, position, depth };
}

}  // namespace exd::interaction
