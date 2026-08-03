#include <exd/interaction/hit.hpp>

namespace exd::interaction
{

HitResult resolve_hit(VisualId hitEntity, math::Vec3f position, float depth)
{
    // TODO: walk render::Parent hierarchy chain to resolve VisualId
    return HitResult{ hitEntity, position, depth };
}

}  // namespace exd::interaction
