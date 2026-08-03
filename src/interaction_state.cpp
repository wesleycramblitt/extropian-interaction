#include <exd/interaction/interaction_state.hpp>

namespace exd::interaction
{

InteractionState::InteractionState() = default;
InteractionState::~InteractionState() = default;

std::optional<VisualEvent> InteractionState::process(
    const PointerEvent& pointer,
    GestureKind gesture,
    VisualId visualId,
    const InteractionConfig& config)
{
    isModifierDown_ = pointer.ctrl || pointer.shift;

    switch (pointer.type)
    {
    case PointerEvent::Type::Enter:
    case PointerEvent::Type::Move:
        if (config.hoverable && hovered_ != visualId)
        {
            VisualId prev = hovered_;
            hovered_ = visualId;
            if (prev != kInvalidVisualId)
                return VisualUnhovered{ prev };
            return VisualHovered{ visualId, pointer.position };
        }
        break;

    case PointerEvent::Type::Down:
        if (config.focusable)
        {
            focused_ = visualId;
            return VisualFocused{ visualId };
        }
        if (config.selectable)
        {
            // Toggle: if clicked visual is already selected, deselect.
            // Otherwise, clear (if no modifier) and select.
            if (selected_.count(visualId))
            {
                selected_.erase(visualId);
                return VisualDeselected{ visualId };
            }
            if (!isModifierDown_) selected_.clear();
            selected_.insert(visualId);
            return VisualSelected{ visualId };
        }
        if (config.draggable)
        {
            dragging_ = visualId;
            dragStart_ = pointer.position;
        }
        break;

    case PointerEvent::Type::Up:
        if (dragging_ == visualId)
        {
            dragging_ = kInvalidVisualId;
            // emit drag ended (VisualDragged with zero delta?)
        }
        break;

    default:
        break;
    }

    if (gesture == GestureKind::Drag && dragging_ != kInvalidVisualId)
    {
        return VisualDragged{ dragging_, dragStart_, pointer.position, pointer.delta };
    }

    return {};
}

std::optional<VisualEvent> InteractionState::clearHover(VisualId currentVisual)
{
    if (hovered_ != kInvalidVisualId && hovered_ != currentVisual)
    {
        VisualId prev = hovered_;
        hovered_ = kInvalidVisualId;
        return VisualUnhovered{ prev };
    }
    return {};
}

void InteractionState::reset()
{
    hovered_ = kInvalidVisualId;
    previousHovered_ = kInvalidVisualId;
    focused_ = kInvalidVisualId;
    dragging_ = kInvalidVisualId;
    resizing_ = kInvalidVisualId;
    selected_.clear();
    currentActionId_.clear();
}

}  // namespace exd::interaction
