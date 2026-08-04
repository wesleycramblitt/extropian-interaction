#pragma once

#include <exd/interaction/event.hpp>
#include <exd/interaction/gestures.hpp>

#include <unordered_set>
#include <optional>

namespace exd::interaction
{

// ── Interaction state machine ──

/// Per‑visual interaction state. Tracks hover, focus, selection, and drag.
/// The state machine is driven by PointerEvent + GestureKind input.
class InteractionState
{
public:
    InteractionState();
    ~InteractionState();

    /// Process an event against a given visual. Returns the resulting VisualEvent
    /// if the state changed (so callers can dispatch to listeners).
    std::optional<VisualEvent> process(
        const PointerEvent& pointer,
        GestureKind gesture,
        VisualId visualId,
        const InteractionConfig& config
    );

    /// Walk all visuals with stale hover state and emit leave events.
    std::optional<VisualEvent> clearHover(VisualId currentVisual);

    /// Current hovered visual.
    VisualId hoveredVisual() const { return hovered_; }

    /// Current focused visual.
    VisualId focusedVisual() const { return focused_; }

    /// Currently selected visual ids.
    const std::unordered_set<VisualId>& selectedVisuals() const { return selected_; }

    /// Is a drag in progress?
    bool isDragging() const { return dragging_ != kInvalidVisualId; }

    /// Is a resize in progress?
    bool isResizing() const { return resizing_ != kInvalidVisualId; }

    /// The action_id from the current interaction config (for conductor routing).
    std::string currentActionId() const { return currentActionId_; }

    void reset();

private:
    VisualId hovered_ = kInvalidVisualId;
    VisualId previousHovered_ = kInvalidVisualId;
    VisualId focused_ = kInvalidVisualId;
    VisualId dragging_ = kInvalidVisualId;
    VisualId resizing_ = kInvalidVisualId;
    std::unordered_set<VisualId> selected_;
    std::string currentActionId_;
    math::Vec3f dragStart_ = {};
    math::Vec3f resizeStart_ = {};
    bool isModifierDown_ = false;
};

}  // namespace exd::interaction
