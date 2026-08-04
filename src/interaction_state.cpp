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

    // ── Gesture-based events (high priority) ──
    if ((gesture == GestureKind::DoubleTap || gesture == GestureKind::LongPress)
        && config.focusable)
    {
        return VisualActivated{ visualId };
    }
    if (gesture == GestureKind::Brush) {
        return AxisBrushed{ visualId, 0, 0.0, 0.0 };
    }
    if (gesture == GestureKind::Scrub) {
        return ValueScrubbed{ visualId, 0, 0, 0.0 };
    }

    switch (pointer.type)
    {
    case PointerEvent::Type::Enter:
        if (config.hoverable && hovered_ != visualId)
        {
            VisualId prev = hovered_;
            hovered_ = visualId;
            if (prev != kInvalidVisualId)
                return VisualUnhovered{ prev };
            return VisualHovered{ visualId, pointer.position };
        }
        break;

    case PointerEvent::Type::Move:
        // Drag / resize take priority over hover changes.
        if (dragging_ != kInvalidVisualId)
        {
            return VisualDragged{ dragging_, dragStart_, pointer.position, pointer.delta };
        }
        if (resizing_ != kInvalidVisualId)
        {
            math::Vec3f delta = pointer.position - resizeStart_;
            return VisualResized{ resizing_, delta };
        }
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
        // Unfocus previous visual if a different visual is clicked.
        if (focused_ != kInvalidVisualId && focused_ != visualId)
        {
            VisualId oldFocused = focused_;
            focused_ = kInvalidVisualId;
            return VisualUnfocused{ oldFocused };
        }
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
        if (config.resizable)
        {
            resizing_ = visualId;
            resizeStart_ = pointer.position;
        }
        break;

    case PointerEvent::Type::Up:
        if (dragging_ == visualId)
        {
            auto event = VisualDragged{ dragging_, dragStart_, pointer.position, pointer.delta };
            dragging_ = kInvalidVisualId;
            return event;
        }
        if (resizing_ == visualId)
        {
            auto event = VisualResized{ resizing_, pointer.position - resizeStart_ };
            resizing_ = kInvalidVisualId;
            return event;
        }
        break;

    case PointerEvent::Type::Scroll:
        if (config.scrollable)
        {
            return ScrollChanged{ visualId, pointer.position };
        }
        break;

    case PointerEvent::Type::Leave:
        if (hovered_ == visualId)
        {
            hovered_ = kInvalidVisualId;
            return VisualUnhovered{ visualId };
        }
        break;

    default:
        break;
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
    dragStart_ = {};
    resizeStart_ = {};
}

}  // namespace exd::interaction
